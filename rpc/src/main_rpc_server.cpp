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
#include <up-client-zenoh-cpp/client/upZenohClient.h>
#include <up-cpp/uuid/factory/Uuidv8Factory.h>
#include <up-cpp/uri/serializer/LongUriSerializer.h>
#include <up-cpp/transport/builder/UAttributesBuilder.h>
#include <spdlog/spdlog.h>

using namespace uprotocol::utransport;
using namespace uprotocol::uuid;
using namespace uprotocol::uri;
using namespace uprotocol::client;

bool gTerminate = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl+C received. Exiting..." << std::endl;
        gTerminate = true; 
    }
}

class RpcListener : public UListener {

    public:
       
         UStatus onReceive(UMessage &message) const override {
            /* Construct response payload with the current time */
            auto currentTime = std::chrono::system_clock::now();
            auto duration = currentTime.time_since_epoch();
            uint64_t currentTimeMilli = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

            UPayload responsePayload(reinterpret_cast<const uint8_t*>(&currentTimeMilli), sizeof(currentTimeMilli), UPayloadType::VALUE);
           
            auto builder = UAttributesBuilder::response(message.attributes().source(), message.attributes().sink(), UPriority::UPRIORITY_CS0, message.attributes().id());
            /* Build response attributes - the same UUID should be used to send the response 
             * it is also possible to send the response outside of the callback context */

            UAttributes responseAttributes = builder.build();

            UMessage messageResp(responsePayload, responseAttributes);
            /* Send the response */
            return UpZenohClient::instance()->send(messageResp);
        }
};

/* The sample RPC server applications demonstrates how to receive RPC requests and send a response back to the client -
 * The response in this example will be the current time */
int main(int argc, 
         char** argv) {

    (void)argc;
    (void)argv;
    
    RpcListener listener;

    signal(SIGINT, signalHandler);

    UStatus status;
    std::shared_ptr<UpZenohClient> transport = UpZenohClient::instance();

    /* init zenoh utransport */
    if (nullptr == transport) {
        spdlog::error("UpZenohClient init failed");
        return -1;
    }

    auto rpcUri = LongUriSerializer::deserialize("/test_rpc.app/1/rpc.milliseconds");

    /* register listener to handle RPC requests */
    status = transport->registerListener(rpcUri, listener);

    if (UCode::OK != status.code()) {
        spdlog::error("registerListener failed");
        return -1;
    }

    while (!gTerminate) {
        sleep(1);
    }

    status = transport->unregisterListener(rpcUri, listener);
    if (UCode::OK != status.code()) {
        spdlog::error("unregisterListener failed");
        return -1;
    }

    return 0;
}
