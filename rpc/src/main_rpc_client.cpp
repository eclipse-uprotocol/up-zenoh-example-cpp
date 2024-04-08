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
#include <csignal>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <up-client-zenoh-cpp/client/upZenohClient.h>
#include <up-cpp/uuid/factory/Uuidv8Factory.h>
#include <up-cpp/transport/builder/UAttributesBuilder.h>

#include "common.h"

using namespace uprotocol::utransport;
using namespace uprotocol::uuid;
using namespace uprotocol::uri;
using namespace uprotocol::v1;
using namespace uprotocol::rpc;
using namespace uprotocol::client;

bool gTerminate = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl+C received. Exiting..." << std::endl;
        gTerminate = true; 
    }
}

RpcResponse sendRPC(UUri& uri) {
    
    UPayload payload(nullptr, 0, UPayloadType::REFERENCE);
   
    CallOptions options;

    options.set_priority(UPriority::UPRIORITY_CS4);
    /* send the RPC request , a future is returned from invokeMethod */
    std::future<RpcResponse> result = UpZenohClient::instance()->invokeMethod(uri, payload, options);

    if (!result.valid()) {
        spdlog::error("Future is invalid");
    }
    /* wait for the future to be fullfieled - it is possible also to specify a timeout for the future */
    result.wait();

    return result.get();
}

/* The sample RPC client applications demonstrates how to send RPC requests and wait for the response -
 * The response in this example will be the current time */
int main(int argc, 
         char** argv) {

    (void)argc;
    (void)argv;
    
    signal(SIGINT, signalHandler);

    UStatus status;
    std::shared_ptr<UpZenohClient> rpcClient = UpZenohClient::instance();

    /* init RPC client */
    if (nullptr == rpcClient) {
        spdlog::error("init failed");
        return -1;
    }

    auto rpcUri = getRpcUri();

    while (!gTerminate) {

        auto response = sendRPC(rpcUri);

        uint64_t milliseconds = 0;

        if (response.message.payload().data() != nullptr && response.message.payload().size() >= sizeof(uint64_t)) {
            memcpy(&milliseconds, response.message.payload().data(), sizeof(uint64_t));
            spdlog::info("Received = {}", milliseconds);
        }

        sleep(1);
    }

    return 0;
}
