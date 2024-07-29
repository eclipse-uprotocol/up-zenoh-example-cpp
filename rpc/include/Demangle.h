// SPDX-FileCopyrightText: 2024 Contributors to the Eclipse Foundation
//
// See the NOTICE file(s) distributed with this work for additional
// information regarding copyright ownership.
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cxxabi.h>

#include <string>

template <typename T>
std::string demangle(const T& t) {
	auto info = typeid(t).name();
	int status;
	auto realname = abi::__cxa_demangle(info, NULL, NULL, &status);
	return std::string(realname);
}
