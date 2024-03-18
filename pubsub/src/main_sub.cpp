
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

#include <csignal>
#include <iostream>
#include <spdlog/spdlog.h>
#include <unistd.h> // For sleep
#include <up-client-zenoh-cpp/transport/zenohUTransport.h>
#include <up-cpp/uri/serializer/LongUriSerializer.h>

using namespace uprotocol::utransport;
using namespace uprotocol::uri;
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

class CustomListener : public UListener {

    public:

        UStatus onReceive(UMessage &message) const override {
            
            (void)message;
            
            // if (TIME_URI_STRING == LongUriSerializer::serialize(uri)) {
            
            //     const uint64_t  *timeInMilliseconds = reinterpret_cast<const uint64_t*>(payload.data());
        
            //     spdlog::info("time = {}", *timeInMilliseconds);

            // } else if (RANDOM_URI_STRING == LongUriSerializer::serialize(uri)) {
        
            //     const uint32_t *random = reinterpret_cast<const uint32_t*>(payload.data());
        
            //     spdlog::info("random = {}", *random);

            // } else if (COUNTER_URI_STRING == LongUriSerializer::serialize(uri)) {
                
            //     const uint8_t *counter = reinterpret_cast<const uint8_t*>(payload.data());

            //     spdlog::info("counter = {}", *counter);
            // }

            UStatus status;

            status.set_code(UCode::OK);

            return status;
      
        }
};

/* The sample sub applications demonstrates how to consume data using uTransport -
 * There are three topics that are received - random number, current time and a counter */
int main(int argc, 
         char** argv) {

    (void)argc;
    (void)argv;
    
    signal(SIGINT, signalHandler);

    UStatus status;
    ZenohUTransport *transport = &ZenohUTransport::instance();

    /* init zenoh utransport */
    status = transport->init();
    if (UCode::OK != status.code()){
        spdlog::error("ZenohUTransport init failed");
        return -1;
    }

    std::vector<std::unique_ptr<CustomListener>> listeners;
    listeners.emplace_back(std::make_unique<CustomListener>());
    listeners.emplace_back(std::make_unique<CustomListener>());
    listeners.emplace_back(std::make_unique<CustomListener>());

    const std::vector<std::string> uriStrings = {
        TIME_URI_STRING,
        RANDOM_URI_STRING,
        COUNTER_URI_STRING
    };

    /* create URI objects from URI strings */
    std::vector<UUri> uris;
    for (const auto& uriString : uriStrings) {
        uris.push_back(LongUriSerializer::deserialize(uriString));
    }

    /* register listeners - in this example the same listener is used for three seperate topics */
    for (size_t i = 0; i < uris.size(); ++i) {
        status = transport->registerListener(uris[i], *listeners[i]);
        if (UCode::OK != status.code()){
            spdlog::error("registerListener failed for {}", uriStrings[i]);
            return -1;
        }
    }

    while (!gTerminate) {
        sleep(1);
    }

    for (size_t i = 0; i < uris.size(); ++i) {
        status = transport->unregisterListener(uris[i], *listeners[i]);
        if (UCode::OK != status.code()){
            spdlog::error("unregisterListener failed for {}", uriStrings[i]);
            return -1;
        }
    }

    /* term zenoh utransport */
    status = transport->term();
    if (UCode::OK != status.code()) {
        spdlog::error("ZenohUTransport term failed");
        return -1;
    }

    return 0;
}
