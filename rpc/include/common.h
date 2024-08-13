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

#ifndef RPC_COMMON_H
#define RPC_COMMON_H

#include <uprotocol/v1/uri.pb.h>

uprotocol::v1::UUri getRpcUUri(const int resource_id) {
    uprotocol::v1::UUri uuri;
    uuri.set_authority_name("test_rpc.app");
    uuri.set_ue_id(0x10001);
    uuri.set_ue_version_major(1);
    uuri.set_resource_id(resource_id);
    return uuri;
}

#endif // RPC_COMMON_H
