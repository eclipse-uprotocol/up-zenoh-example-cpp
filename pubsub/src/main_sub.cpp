
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
#include <up-client-zenoh-cpp/client/upZenohClient.h>

#include "common.h"

using namespace uprotocol::utransport;
using namespace uprotocol::uri;
using namespace uprotocol::v1;
using namespace uprotocol::client;

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
            
            UUri uri = message.attributes().source();

            if (getTimeUri().resource().id() == uri.resource().id()) {
            
                const uint64_t  *timeInMilliseconds = reinterpret_cast<const uint64_t*>(message.payload().data());
        
                spdlog::info("time = {}", *timeInMilliseconds);

            } else if (getRandomUri().resource().id() == uri.resource().id()) {
        
                const uint32_t *random = reinterpret_cast<const uint32_t*>(message.payload().data());
        
                spdlog::info("random = {}", *random);

            } else if (getCounterUri().resource().id() == uri.resource().id()) {
                
                const uint8_t *counter = reinterpret_cast<const uint8_t*>(message.payload().data());

                spdlog::info("counter = {}", *counter);
            }

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
    std::shared_ptr<UpZenohClient> transport = UpZenohClient::instance();

    /* init zenoh utransport */
    if (nullptr == transport){
        spdlog::error("UpZenohClient init failed");
        return -1;
    }

    std::vector<std::unique_ptr<CustomListener>> listeners;
    listeners.emplace_back(std::make_unique<CustomListener>());
    listeners.emplace_back(std::make_unique<CustomListener>());
    listeners.emplace_back(std::make_unique<CustomListener>());

    /* create URI objects */
    const std::vector<UUri> uris {
        getTimeUri(),
        getRandomUri(),
        getCounterUri()
    };

    /* register listeners - in this example the same listener is used for three seperate topics */
    for (size_t i = 0; i < uris.size(); ++i) {
        status = transport->registerListener(uris[i], *listeners[i]);
        if (UCode::OK != status.code()){
            spdlog::error("registerListener failed for {}", uris[i].resource().name());
            return -1;
        }
    }

    while (!gTerminate) {
        sleep(1);
    }

    for (size_t i = 0; i < uris.size(); ++i) {
        status = transport->unregisterListener(uris[i], *listeners[i]);
        if (UCode::OK != status.code()){
            spdlog::error("unregisterListener failed for {}", uris[i].resource().name());
            return -1;
        }
    }

    return 0;
}
