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
#include <up-client-zenoh-cpp/rpc/zenohRpcClient.h>
#include <up-cpp/uuid/factory/Uuidv8Factory.h>
#include <up-cpp/uri/serializer/LongUriSerializer.h>
#include <up-cpp/transport/builder/UAttributesBuilder.h>

using namespace uprotocol::utransport;
using namespace uprotocol::uuid;
using namespace uprotocol::uri;
using namespace uprotocol::v1;

bool gTerminate = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl+C received. Exiting..." << std::endl;
        gTerminate = true; 
    }
}

UMessage sendRPC(UUri& uri) {
    
    UPayload payload(nullptr, 0, UPayloadType::REFERENCE);
    CallOptions options;

    options.set_priority(UPriority::UPRIORITY_CS4);
    /* send the RPC request , a future is returned from invokeMethod */
    std::future<UMessage> result = ZenohRpcClient::instance().invokeMethod(uri, payload, options);

    if (!result.valid()) {
        spdlog::error("Future is invalid");
        return UMessage();   
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
    ZenohRpcClient *rpcClient = &ZenohRpcClient::instance();

    /* init RPC client */
    status = rpcClient->init();
    if (UCode::OK != status.code()) {
        spdlog::error("init failed");
        return -1;
    }

    auto rpcUri = LongUriSerializer::deserialize("/test_rpc.app/1/rpc.milliseconds");

    while (!gTerminate) {

        auto response = sendRPC(rpcUri);

        uint64_t milliseconds = 0;

        if (response.payload().data() != nullptr && response.payload().size() >= sizeof(uint64_t)) {
            memcpy(&milliseconds, response.payload().data(), sizeof(uint64_t));
            spdlog::info("Received = {}", milliseconds);
        }

        sleep(1);
    }

    /* term RPC client */
    status = rpcClient->term();
    if (UCode::OK != status.code()) {
        spdlog::error("term failed");
        return -1;
    }

    return 0;
}
