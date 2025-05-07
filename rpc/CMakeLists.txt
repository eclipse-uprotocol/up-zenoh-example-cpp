#  SPDX-FileCopyrightText: 2025 Contributors to the Eclipse Foundation
#
#  See the NOTICE file(s) distributed with this work for additional
#  information regarding copyright ownership.
#
#  This program and the accompanying materials are made available under the
#  terms of the Apache License Version 2.0 which is available at
#  https://www.apache.org/licenses/LICENSE-2.0
#
#  SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.20)
project(rpc VERSION 0.1.0 LANGUAGES CXX)

find_package(spdlog REQUIRED)
find_package(up-cpp REQUIRED)
find_package(up-transport-zenoh-cpp REQUIRED)

add_definitions(-DSPDLOG_FMT_EXTERNAL)

# rpc client
add_executable(rpc_client src/main_rpc_client.cpp)
target_link_libraries(rpc_client
  PUBLIC
    spdlog::spdlog
    up-cpp::up-cpp
    up-core-api::up-core-api
    up-transport-zenoh-cpp::up-transport-zenoh-cpp)
target_include_directories(rpc_client 
  PRIVATE
    include)

# rpc server
add_executable(rpc_server src/main_rpc_server.cpp)
target_link_libraries(rpc_server
  PUBLIC
    spdlog::spdlog
    up-cpp::up-cpp
    up-core-api::up-core-api
	up-transport-zenoh-cpp::up-transport-zenoh-cpp)
target_include_directories(rpc_server 
  PRIVATE
    include)
