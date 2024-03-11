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
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <unistd.h> // For sleep

#include <spdlog/spdlog.h>
#include <up-client-zenoh-cpp/transport/zenohUTransport.h>
#include <up-cpp/uuid/factory/Uuidv8Factory.h>
#include <up-cpp/uri/serializer/LongUriSerializer.h>
#include <up-cpp/transport/builder/UAttributesBuilder.h>
#include <up-core-api/ustatus.pb.h>
#include <up-core-api/uri.pb.h>

using namespace uprotocol::utransport;
using namespace uprotocol::uri;
using namespace uprotocol::uuid;
using namespace uprotocol::v1;

const std::string TIME_URI_STRING = "/test.app/1/milliseconds";
const std::string RANDOM_URI_STRING = "/test.app/1/32bit";
const std::string COUNTER_URI_STRING = "/test.app/1/counter";

bool gTerminate = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl+C received. Exiting..." << std::endl;
        gTerminate = true;
    }
}

std::uint8_t* getTime() {

    auto currentTime = std::chrono::system_clock::now();
    auto duration = currentTime.time_since_epoch();
    auto timeMilli = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    static std::uint8_t buf[8];
    std::memcpy(buf, &timeMilli, sizeof(timeMilli));

    return buf;
}

std::uint8_t* getRandom() {
    
    int32_t val = std::rand();
    static std::uint8_t buf[4];
    std::memcpy(buf, &val, sizeof(val));

    return buf;
}

std::uint8_t* getCounter() {
    
    static std::uint8_t counter = 0;
    ++counter;

    return &counter;
}

UCode sendMessage(ZenohUTransport *transport,
                  UUri &uri,
                  std::uint8_t *buffer,
                  size_t size) {
   
    auto uuid = Uuidv8Factory::create();
   
    UAttributesBuilder builder(uuid, UMessageType::UMESSAGE_TYPE_PUBLISH, UPriority::UPRIORITY_CS0);
    UAttributes attributes = builder.build();
   
    UPayload payload(buffer, size, UPayloadType::VALUE);
   
    UStatus status = transport->send(uri, payload, attributes);
    if (UCode::OK != status.code()) {
        spdlog::error("send.send failed");
        return UCode::UNAVAILABLE;
    }
    return UCode::OK;
}

/* The sample pub applications demonstrates how to send data using uTransport -
 * There are three topics that are published - random number, current time and a counter */
int main(int argc, 
         char **argv) {

    (void)argc;
    (void)argv;
    
    signal(SIGINT, signalHandler);
    
    UStatus status;
    ZenohUTransport *transport = &ZenohUTransport::instance();

    /* Initialize zenoh utransport */
    status = transport->init();
    if (UCode::OK != status.code()) {
        spdlog::error("ZenohUTransport init failed");
        return -1;
    }
    
    /* Create URI objects from string URI*/
    auto timeUri = LongUriSerializer::deserialize(TIME_URI_STRING);
    auto randomUri = LongUriSerializer::deserialize(RANDOM_URI_STRING);
    auto counterUri = LongUriSerializer::deserialize(COUNTER_URI_STRING);

    while (!gTerminate) {
        /* send current time in milliseconds */
        if (UCode::OK != sendMessage(transport, timeUri, getTime(), 8)) {
            spdlog::error("sendMessage failed");
            break;
        }

        /* send random number */
        if (UCode::OK != sendMessage(transport, randomUri, getRandom(), 4)) {
            spdlog::error("sendMessage failed");
            break;
        }

        /* send counter */
        if (UCode::OK != sendMessage(transport, counterUri, getCounter(), 1)) {
            spdlog::error("sendMessage failed");
            break;
        }
        
        sleep(1);
    }

     /* Terminate zenoh utransport */
    status = transport->term();
    if (UCode::OK != status.code()) {
        spdlog::error("ZenohUTransport term failed");
        return -1;
    }

    return 0;
}