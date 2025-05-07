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
#include <up-cpp/communication/Subscriber.h>
#include <up-transport-zenoh-cpp/ZenohUTransport.h>

#include <csignal>
#include <iostream>

#include "common.h"

using namespace uprotocol::communication;
using namespace uprotocol::v1;

using ZenohUTransport = uprotocol::transport::ZenohUTransport;

bool gTerminate = false;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		std::cout << "Ctrl+C received. Exiting..." << std::endl;
		gTerminate = true;
	}
}

void onReceiveTime(const UMessage& message) {
	if (message.attributes().payload_format() ==
	    UPayloadFormat::UPAYLOAD_FORMAT_TEXT) {
		const char* payload =
		    reinterpret_cast<const char*>(message.payload().data());
		spdlog::info("received time = {}", payload);
	}
}

void onReceiveRandom(const uprotocol::v1::UMessage& message) {
	if (message.attributes().payload_format() ==
	    UPayloadFormat::UPAYLOAD_FORMAT_TEXT) {
		const char* payload =
		    reinterpret_cast<const char*>(message.payload().data());
		spdlog::info("received random = {}", payload);
	}
}

void onReceiveCounter(const uprotocol::v1::UMessage& message) {
	if (message.attributes().payload_format() ==
	    UPayloadFormat::UPAYLOAD_FORMAT_TEXT) {
		const char* payload =
		    reinterpret_cast<const char*>(message.payload().data());
		spdlog::info("received counter = {}", payload);
	}
}

/* The sample sub applications demonstrates how to consume data using uTransport
 * -
 * There are three topics that are received - random number, current time and a
 * counter */
int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	if (argc < 2) {
		std::cout << "No Zenoh config has been provided" << std::endl;
		std::cout << "Usage: sub <config_file>" << std::endl;
		return 1;
	}

	signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);

	UStatus status;
	UUri source = getUUri(0);
	auto topic_time = getTimeUUri();
	auto topic_random = getRandomUUri();
	auto topic_counter = getCounterUUri();
	auto transport = std::make_shared<ZenohUTransport>(source, argv[1]);

	auto resTime =
	    Subscriber::subscribe(transport, std::move(topic_time), onReceiveTime);
	auto resRandom = Subscriber::subscribe(transport, std::move(topic_random),
	                                       onReceiveRandom);
	auto resCounter = Subscriber::subscribe(transport, std::move(topic_counter),
	                                        onReceiveCounter);

	while (!gTerminate) {
		sleep(1);
	}

	return 0;
}
