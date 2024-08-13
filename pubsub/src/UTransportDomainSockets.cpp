// SPDX-FileCopyrightText: 2024 Contributors to the Eclipse Foundation
//
// See the NOTICE file(s) distributed with this work for additional
// information regarding copyright ownership.
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0
//
// SPDX-License-Identifier: Apache-2.0

#include "UTransportDomainSockets.h"

#include <pthread.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <up-cpp/datamodel/serializer/UUri.h>
#include <uprotocol/v1/ucode.pb.h>

UTransportDomainSockets::UTransportDomainSockets(const v1::UUri& uuri)
    : transport::UTransport(uuri),
      send_count_(0),
      fdClient_(-1),
      stopFlag_(false) {
	socketPath_ = std::filesystem::canonical("/proc/self/exe")
	                  .parent_path()
	                  .append(uuri.authority_name());
	fdSocket_ = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fdSocket_ == -1) {
		spdlog::error("Error on socket creation\n");
	}
}

UTransportDomainSockets::~UTransportDomainSockets() {
	stopFlag_ = true;

	if (listener_thread_.joinable()) {
		listener_thread_.join();
	}
	if (fdClient_ != -1) {
		spdlog::info("Closing client socket");
		close(fdClient_);
	}
	if (fdSocket_ != -1) {
		spdlog::info("Closing socket");
		close(fdSocket_);
		unlink(socketPath_.c_str());
	}
}

v1::UStatus UTransportDomainSockets::sendImpl(const v1::UMessage& message) {
	v1::UStatus retval;

	retval.set_code(v1::UCode::UNKNOWN);

	if (fdClient_ == -1) {
		// Bind the socket, and await client connection
		int len = 0;
		struct sockaddr_un local;
		int nIncomingConnections = 1;

		local.sun_family = AF_UNIX;
		strcpy(local.sun_path, socketPath_.c_str());
		unlink(local.sun_path);
		len = strlen(local.sun_path) + sizeof(local.sun_family);
		if (bind(fdSocket_, (struct sockaddr*)&local, len) != 0) {
			spdlog::error("Error on binding socket.  Errno={}\n", errno);
			return retval;
		}

		if (listen(fdSocket_, nIncomingConnections) != 0) {
			spdlog::error("Error on listen call.  Errno={}\n", errno);
			return retval;
		}

		spdlog::info("Waiting for client connection\n");
		fdClient_ = accept(fdSocket_, NULL, NULL);
		if (fdClient_ == -1) {
			spdlog::error("Error on accept call.  Errno={}\n", errno);
			return retval;
		}
	}

	// serialize the message
	size_t serializedSize = message.ByteSizeLong();
	std::string serializedMessage(serializedSize, 0);
	bool success =
	    message.SerializeToArray(serializedMessage.data(), serializedSize);
	spdlog::debug("Serialized message size: {} ; Serialized data: {}",
	              serializedSize, serializedMessage);

	// Send the serialized UMessage
	spdlog::debug("Sending message number {}", send_count_.load());

	// send length of serialized message
	if (::send(fdClient_, &serializedSize, sizeof(serializedSize), 0) == -1) {
		spdlog::error("Error sending message size.  Errno={}\n", errno);
		return retval;
	}
	// send serialized message
	if (::send(fdClient_, serializedMessage.data(), serializedSize, 0) == -1) {
		spdlog::error("Error sending serialized data.  Errno={}\n", errno);
		return retval;
	}

	send_count_++;
	retval.set_code(v1::UCode::OK);
	return retval;
}

v1::UStatus UTransportDomainSockets::registerListenerImpl(
    CallableConn&& listener, const v1::UUri& source_filter /* topic */,
    std::optional<v1::UUri>&& sink_filter) {
	v1::UStatus retval;

	// Start the listener thread (if not already started)
	if (!listener_thread_.joinable()) {
		listener_thread_ =
		    std::thread(&UTransportDomainSockets::listenThread, this);
	}

	// Verify the listener thread is running
	if (!listener_thread_.joinable()) {
		spdlog::error("Failed to start listener thread.  Errno={}\n", errno);
		retval.set_code(v1::UCode::UNKNOWN);
		return retval;
	}

	// Register the listener (place it in the map)
	size_t hash = std::hash<std::string>{}(
	    uprotocol::datamodel::serializer::uri::AsString::serialize(
	        source_filter));
	cbListeners_[hash] = listener;

	retval.set_code(v1::UCode::OK);
	return retval;
}

void UTransportDomainSockets::listenThread() {
	while (!stopFlag_) {
		int data_len;
		int connected;
		struct sockaddr_un addr;
		size_t serializedSize;

		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, socketPath_.c_str());
		data_len = strlen(addr.sun_path) + sizeof(addr.sun_family);

		spdlog::info("Client: Trying to connect...");
		if ((connected =
		         connect(fdSocket_, (struct sockaddr*)&addr, data_len)) == -1) {
			spdlog::info("Client: Error on connect call.  Errno = {}", errno);
			sleep(1);
		} else {
			spdlog::info("Client: Connected");
		}

		while (!stopFlag_ && (connected != -1)) {
			v1::UMessage receivedMessage;

			// receive length of serialized message
			if (recv(fdSocket_, &serializedSize, sizeof(serializedSize), 0) ==
			    -1) {
				spdlog::error("Error receiving message size.  Errno={}\n",
				              errno);
				break;
			}

			// receive serialized message
			std::string serializedMessage(serializedSize, 0);
			if (recv(fdSocket_, serializedMessage.data(), serializedSize, 0) ==
			    -1) {
				spdlog::error("Error receiving serialized data.  Errno={}\n",
				              errno);
				break;
			}

			if (!receivedMessage.ParseFromString(serializedMessage)) {
				spdlog::error("Failed to parse received message");
				break;
			} else {
				spdlog::debug("Received message number {}", send_count_.load());
				send_count_++;
				notifyListener(receivedMessage);
			}
		}
	}
}

void UTransportDomainSockets::notifyListener(const v1::UMessage& message) {
	// Look up the listener in the map based on the resource_id (each topic has
	// a unique resource_id that is used as the key in the map)
	size_t hash = std::hash<std::string>{}(
	    uprotocol::datamodel::serializer::uri::AsString::serialize(
	        message.attributes().source()));
	auto it = cbListeners_.find(hash);
	if (it != cbListeners_.end()) {
		it->second(message);
	} else {
		spdlog::error(
		    "No listener found for message with source: {}",
		    uprotocol::datamodel::serializer::uri::AsString::serialize(
		        message.attributes().source()));
	}
}
