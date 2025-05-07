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
#include <random>

#include "common.h"

using Payload = uprotocol::datamodel::builder::Payload;
using Publisher = uprotocol::communication::Publisher;
using UPayloadFormat = uprotocol::v1::UPayloadFormat;
using UCode = uprotocol::v1::UCode;
using ZenohUTransport = uprotocol::transport::ZenohUTransport;

bool g_terminate = false;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		std::cout << "Ctrl+C received. Exiting..." << std::endl;
		g_terminate = true;
	}
}

int64_t getTime() {
	auto current_time = std::chrono::system_clock::now();
	auto duration = current_time.time_since_epoch();
	int64_t time_milli =
	    std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	return time_milli;
}

int32_t getRandom() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int32_t> distribution(0, INT32_MAX);
	return distribution(gen);
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

	uprotocol::v1::UStatus status;

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

	while (!g_terminate) {
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
