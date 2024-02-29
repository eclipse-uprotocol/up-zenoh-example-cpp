# Copyright (c) 2024 General Motors GTO LLC
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
# SPDX-FileType: SOURCE
# SPDX-FileCopyrightText: 2024 General Motors GTO LLC
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20)
project(rpc VERSION 0.1.0 LANGUAGES CXX)

find_package(up-client-zenoh-cpp REQUIRED)

# rpc server
add_executable(rpc_server src/main_rpc_server.cpp)
target_link_libraries(rpc_server
    PRIVATE
        up-client-zenoh-cpp::up-client-zenoh-cpp)

# rpc client
add_executable(rpc_client src/main_rpc_client.cpp)
target_link_libraries(rpc_client
    PRIVATE
        up-client-zenoh-cpp::up-client-zenoh-cpp)
