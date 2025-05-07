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
#include <up-cpp/communication/RpcClient.h>
#include <up-transport-zenoh-cpp/ZenohUTransport.h>

#include <chrono>
#include <csignal>
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

void OnReceive(RpcClient::MessageOrStatus expected) {
	if (!expected.has_value()) {
		UStatus status = expected.error();
		spdlog::error("Expected value not found. -- Status: {}",
		              status.DebugString());
		return;
	}

	UMessage message = std::move(expected).value();

	if (message.attributes().payload_format() !=
	    UPayloadFormat::UPAYLOAD_FORMAT_RAW) {
		spdlog::error("Received message has unexpected payload format:\n{}",
		              message.DebugString());
		return;
	}

	if (message.payload().size() != (sizeof(uint64_t) * 3)) {
		spdlog::error("Received message has unexpected payload size:\n{}",
		              message.DebugString());
		return;
	}

	// RPC response is expected to have a payload of 3 uint64_t values
	// sequence number, current time, and random value
	spdlog::debug("(Client) Received message: {}", message.DebugString());

	const uint64_t* pdata = (uint64_t*)message.payload().data();
	spdlog::info("Received payload: {} - {}, {}", pdata[0], pdata[1], pdata[2]);
}

/* The sample RPC client applications demonstrates how to send RPC requests and
 * wait for the response
 */
int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	if (argc < 2) {
		std::cout << "No Zenoh config has been provided" << std::endl;
		std::cout << "Usage: rpc_client <config_file>" << std::endl;
		return 1;
	}

	signal(SIGINT, signalHandler);

	UUri source = getRpcUUri(0);
	UUri method = getRpcUUri(12);
	auto transport = std::make_shared<ZenohUTransport>(source, argv[1]);
	auto client =
	    RpcClient(transport, std::move(method), UPriority::UPRIORITY_CS4,
	              std::chrono::milliseconds(500));
	RpcClient::InvokeHandle handle;

	while (!gTerminate) {
		handle = client.invokeMethod(OnReceive);
		sleep(1);
	}

	return 0;
}
