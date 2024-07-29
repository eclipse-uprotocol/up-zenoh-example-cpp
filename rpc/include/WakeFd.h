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

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

class WakeFd {
	int fd_;
	int pair_[2];
	const size_t max_read_bytes = 32768;

public:
	WakeFd(int fd) : fd_(fd) { auto pret = pipe(pair_); }

	~WakeFd() {
		close(fd_);
		close(pair_[0]);
		close(pair_[1]);
	}

	int fd() { return fd_; }

	void wake() {
		int dummy;
		auto ret = write(pair_[1], &dummy, sizeof(dummy));
	}

	bool read(std::string& data) {
		struct pollfd fds[2];
		fds[0].fd = fd_;
		fds[0].events = POLLIN;
		fds[0].revents = 0;
		fds[1].fd = pair_[0];
		fds[1].events = POLLIN;
		fds[1].revents = 0;
		int ret = poll(fds, 2, -1);
		// wake() called, return false to exit
		if (fds[1].revents) {
			return false;
		}
		// spurious wake, return true to try again
		if (fds[0].revents == 0) {
			data.resize(0);
			return true;
		}
		data.resize(max_read_bytes);
		int readSize = ::read(fd_, data.data(), max_read_bytes);
		if (readSize < 0)
			return false;
		data.resize(readSize);
		return true;
	}

	template <typename... Params>
	int send(Params&&... params) {
		return ::send(fd_, std::forward<Params>(params)...);
	}

	template <typename... Params>
	int connect(Params&&... params) {
		return ::connect(fd_, std::forward<Params>(params)...);
	}
};