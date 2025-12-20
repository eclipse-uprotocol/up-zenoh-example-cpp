//  SPDX-FileCopyrightText: 2025 Contributors to the Eclipse Foundation
//
//  See the NOTICE file(s) distributed with this work for additional
//  information regarding copyright ownership.
//
//  This program and the accompanying materials are made available under the
//  terms of the Apache License Version 2.0 which is available at
//  https://www.apache.org/licenses/LICENSE-2.0
//
//  SPDX-License-Identifier: Apache-2.0
//
#ifndef COMMON_H
#define COMMON_H

#include <uprotocol/v1/uri.pb.h>

constexpr int DUMMY_UE_ID = 0x18002;
constexpr int TIMER_RESOURCE_ID = 0x8001;
constexpr int RANDOM_RESOURCE_ID = 0x8002;
constexpr int COUNTER_RESOURCE_ID = 0x8003;

inline uprotocol::v1::UUri getUUri(int const resource_id) {
	uprotocol::v1::UUri uuri;
	uuri.set_authority_name("test.app");
	uuri.set_ue_id(DUMMY_UE_ID);
	uuri.set_ue_version_major(1);
	uuri.set_resource_id(resource_id);
	return uuri;
}

inline uprotocol::v1::UUri const& getTimeUUri() {
	static auto uuri = getUUri(TIMER_RESOURCE_ID);
	return uuri;
}

inline uprotocol::v1::UUri const& getRandomUUri() {
	static auto uuri = getUUri(RANDOM_RESOURCE_ID);
	return uuri;
}

inline uprotocol::v1::UUri const& getCounterUUri() {
	static auto uuri = getUUri(COUNTER_RESOURCE_ID);
	return uuri;
}

#endif  // COMMON_H
