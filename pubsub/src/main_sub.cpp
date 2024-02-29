
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

        UStatus onReceive(const UUri& uri,
                          const UPayload& payload,
                          const UAttributes& attributes) const override {
                                
            if (TIME_URI_STRING == LongUriSerializer::serialize(uri)) {
            
                const uint64_t  *timeInMilliseconds = reinterpret_cast<const uint64_t*>(payload.data());
        
                spdlog::info("time = {}", *timeInMilliseconds);

            } else if (RANDOM_URI_STRING == LongUriSerializer::serialize(uri)) {
        
                const uint32_t *random = reinterpret_cast<const uint32_t*>(payload.data());
        
                spdlog::info("random = {}", *random);

            } else if (COUNTER_URI_STRING == LongUriSerializer::serialize(uri)) {
                
                const uint8_t *counter = reinterpret_cast<const uint8_t*>(payload.data());

                spdlog::info("counter = {}", *counter);
            }

            UStatus status;

            status.set_code(UCode::OK);

            return status;
        }
};

int main(int argc, char** argv) {

    signal(SIGINT, signalHandler);

    UStatus status;
    ZenohUTransport *transport = &ZenohUTransport::instance();

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

    std::vector<UUri> uris;
    for (const auto& uriString : uriStrings) {
        uris.push_back(LongUriSerializer::deserialize(uriString));
    }

    /* register listeners */
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

    status = transport->term();
    if (UCode::OK != status.code()) {
        spdlog::error("ZenohUTransport term failed");
        return -1;
    }

    return 0;
}