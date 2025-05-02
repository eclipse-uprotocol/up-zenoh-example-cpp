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
#include <up-cpp/communication/Publisher.h>
#include <up-cpp/datamodel/builder/Payload.h>
#include <up-transport-zenoh-cpp/ZenohUTransport.h>

#include <chrono>
#include <csignal>
#include <iostream>

#include "common.h"

using namespace uprotocol::datamodel::builder;
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

int64_t getTime() {
	auto currentTime = std::chrono::system_clock::now();
	auto duration = currentTime.time_since_epoch();
	int64_t timeMilli =
	    std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	return timeMilli;
}

int32_t getRandom() {
	int32_t val = std::rand();
	return val;
}

uint8_t getCounter() {
	static std::uint8_t counter = 0;
	++counter;
	return counter;
}

/* The sample pub applications demonstrates how to send data using uTransport -
 * There are three topics that are published - random number, current time and a
 * counter */
int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	if (argc < 2) {
		std::cout << "No Zenoh config has been provided" << std::endl;
		std::cout << "Usage: pub <config_file>" << std::endl;
		return 1;
	}

	signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);

	UStatus status;

	auto source = getUUri(0);
	auto topic_time = getTimeUUri();
	auto topic_random = getRandomUUri();
	auto topic_counter = getCounterUUri();
	auto transport = std::make_shared<ZenohUTransport>(source, argv[1]);
	Publisher publish_time(transport, std::move(topic_time),
	                       UPayloadFormat::UPAYLOAD_FORMAT_TEXT);
	Publisher publish_random(transport, std::move(topic_random),
	                         UPayloadFormat::UPAYLOAD_FORMAT_TEXT);
	Publisher publish_counter(transport, std::move(topic_counter),
	                          UPayloadFormat::UPAYLOAD_FORMAT_TEXT);

	while (!gTerminate) {
		// send a string with a time value (ie "15665489")
		uint64_t time_val = getTime();
		spdlog::info("sending time = {}", time_val);
		Payload string_time(std::to_string(time_val),
		                    UPayloadFormat::UPAYLOAD_FORMAT_TEXT);
		status = publish_time.publish(std::move(string_time));
		if (status.code() != UCode::OK) {
			spdlog::error("Publish time failed.");
			break;
		}

		int32_t rand_val = getRandom();
		spdlog::info("sending random = {}", rand_val);
		Payload random_payload(std::to_string(rand_val),
		                       UPayloadFormat::UPAYLOAD_FORMAT_TEXT);
		status = publish_random.publish(std::move(random_payload));
		if (status.code() != UCode::OK) {
			spdlog::error("Publish random failed.");
			break;
		}

		uint8_t counter_val = getCounter();
		spdlog::info("sending counter = {}", counter_val);
		Payload counter_payload(std::to_string(counter_val),
		                        UPayloadFormat::UPAYLOAD_FORMAT_TEXT);
		status = publish_counter.publish(std::move(counter_payload));
		if (status.code() != UCode::OK) {
			spdlog::error("Publish counter failed.");
			break;
		}

		sleep(1);
	}

	return 0;
}
