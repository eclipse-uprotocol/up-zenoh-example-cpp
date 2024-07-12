/*
 * Copyright (c) 2024 General Motors GTO LLC
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: 2024 General Motors GTO LLC
 * SPDX-License-Identifier: Apache-2.0
 */
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <up-cpp/communication/Publisher.h>
#include <up-cpp/datamodel/builder/Payload.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "UTransportDomainSockets.h"
#include "common.h"

using namespace uprotocol::datamodel::builder;
using namespace uprotocol::communication;
using namespace uprotocol::v1;

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

	signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);

	UStatus status;

	auto source = getUUri(0);
	auto topic_time = getTimeUUri();
	auto topic_random = getRandomUUri();
	auto topic_counter = getCounterUUri();
	auto transport = std::make_shared<UTransportDomainSockets>(source);
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
