//  SPDX-FileCopyrightText: 2025 Contributors to the Eclipse Foundation
//
//  See the NOTICE file(s) distributed with this work for additional
//  information regarding copyright ownership.
//
//  This program and the accompanying materials are made available under the
//  terms of the Apache License Version 2.0 which is available at
//  https://www.apache.org/licenses/LICENSE-2.0
//
//  SPDX-License-Identifier: Apache-2.0
//
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <up-cpp/communication/RpcServer.h>
#include <up-transport-zenoh-cpp/ZenohUTransport.h>

#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>

#include "common.h"

using namespace uprotocol::v1;
using namespace uprotocol::communication;
using namespace uprotocol::datamodel::builder;
using ZenohUTransport = uprotocol::transport::ZenohUTransport;

bool gTerminate = false;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		std::cout << "Ctrl+C received. Exiting..." << std::endl;
		gTerminate = true;
	}
}

std::optional<uprotocol::datamodel::builder::Payload> OnReceive(
    const UMessage& message) {
	// Validate message is an RPC request
	if (message.attributes().type() != UMessageType::UMESSAGE_TYPE_REQUEST) {
		spdlog::error("Received message is not a request\n{}",
		              message.DebugString());
		return {};
	}

	// Validate message has no payload (no payload expected)
	if (message.has_payload()) {
		spdlog::error("Received message has non-empty payload\n{}",
		              message.DebugString());
		return {};
	}

	// Received request with empty payload, generate response with
	// sequence number, current time, and random value
	static uint64_t seqNum = 0;
	uint64_t randVal = std::rand();
	uint64_t timeVal = std::chrono::duration_cast<std::chrono::milliseconds>(
	                       std::chrono::system_clock::now().time_since_epoch())
	                       .count();
	std::vector<uint64_t> payload_data = {seqNum++, timeVal, randVal};

	spdlog::debug("(Server) Received request:\n{}", message.DebugString());

	Payload payload(reinterpret_cast<std::vector<uint8_t>&>(payload_data),
	                UPayloadFormat::UPAYLOAD_FORMAT_RAW);
	spdlog::info("Sending payload:  {} - {}, {}", payload_data[0],
	             payload_data[1], payload_data[2]);

	return payload;
}

/* The sample RPC server applications demonstrates how to receive RPC requests
 * and send a response back to the client -
 * The response in this example will be the current time */
int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	if (argc < 2) {
		std::cout << "No Zenoh config has been provided" << std::endl;
		std::cout << "Usage: rpc_server <config_file>" << std::endl;
		return 1;
	}

	signal(SIGINT, signalHandler);
	UUri source = getRpcUUri(0);
	UUri method = getRpcUUri(12);
	auto transport = std::make_shared<ZenohUTransport>(source, argv[1]);
	auto server = RpcServer::create(transport, method, OnReceive);

	if (!server.has_value()) {
		spdlog::error("Failed to create RPC server: {}",
		              server.error().DebugString());
		return 1;
	}

	while (!gTerminate) {
		sleep(1);
	}

	return 0;
}
