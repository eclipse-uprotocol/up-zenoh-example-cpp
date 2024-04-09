// Copyright (c) 2024 General Motors GTO LLC
//
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: 2024 General Motors GTO LLC
// SPDX-License-Identifier: Apache-2.0

#ifndef PUBSUB_COMMON_H
#define PUBSUB_COMMON_H

#include <up-core-api/uri.pb.h>
#include <up-cpp/uri/builder/BuildUUri.h>
#include <up-cpp/uri/builder/BuildUAuthority.h>
#include <up-cpp/uri/builder/BuildEntity.h>
#include <up-cpp/uri/builder/BuildUResource.h>

uprotocol::v1::UUri getUri(
        std::string_view const entName, int const entId, int const entMajorVers,
        std::string_view const resName, int const resId) {

    using namespace uprotocol::uri;

    return BuildUUri()
        .setAutority(BuildUAuthority().build())
        .setEntity(BuildUEntity()
                .setName(static_cast<std::string>(entName))
                .setMajorVersion(entMajorVers)
                .setId(entId)
                .build())
        .setResource(BuildUResource()
                .setName(static_cast<std::string>(resName))
                .setID(resId)
                .build())
        .build();
}

constexpr std::string_view ENTITY_NAME{"test.app"};
constexpr int ENTITY_ID{2};
constexpr int ENTITY_MAJOR_VERS{1};

uprotocol::v1::UUri const& getTimeUri() {
    static auto uri =
        getUri(ENTITY_NAME, ENTITY_ID, ENTITY_MAJOR_VERS, "milliseconds", 1);
    return uri;
}

uprotocol::v1::UUri const& getRandomUri() {
    static auto uri =
        getUri(ENTITY_NAME, ENTITY_ID, ENTITY_MAJOR_VERS, "32bit", 2);
    return uri;
}

uprotocol::v1::UUri const& getCounterUri() {
    static auto uri =
        getUri(ENTITY_NAME, ENTITY_ID, ENTITY_MAJOR_VERS, "counter", 3);
    return uri;
}

#endif // PUBSUB_COMMON_H
