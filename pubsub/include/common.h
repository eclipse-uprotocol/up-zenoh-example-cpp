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
#ifndef PUBSUB_COMMON_H
#define PUBSUB_COMMON_H

#include <uprotocol/v1/uri.pb.h>

uprotocol::v1::UUri getUUri(int const resource_id) {
	uprotocol::v1::UUri uuri;
	uuri.set_authority_name("test.app");
	uuri.set_ue_id(0x18002);
	uuri.set_ue_version_major(1);
	uuri.set_resource_id(resource_id);
	return uuri;
}

uprotocol::v1::UUri const& getTimeUUri() {
	static auto uuri = getUUri(0x8001);
	return uuri;
}

uprotocol::v1::UUri const& getRandomUUri() {
	static auto uuri = getUUri(0x8002);
	return uuri;
}

uprotocol::v1::UUri const& getCounterUUri() {
	static auto uuri = getUUri(0x8003);
	return uuri;
}

#endif  // PUBSUB_COMMON_H
