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

#ifndef UTRANSPORT_DOMAIN_SOCKETS_H
#define UTRANSPORT_DOMAIN_SOCKETS_H

#include <filesystem>
#include <thread>
#include <up-cpp/transport/UTransport.h>

using namespace uprotocol;

class UTransportDomainSockets : public transport::UTransport {
public:
	explicit UTransportDomainSockets(const v1::UUri& uuri);
        virtual ~UTransportDomainSockets();

private:
        int fdSocket_; // socket handle
        int fdClient_; // connected client handle
        std::filesystem::path socketPath_; // path to the socket file (same as executable)
	std::atomic<size_t> send_count_;
        std::thread listener_thread_;
        bool stopFlag_; // stop flag for listener thread
        std::map<size_t, CallableConn> cbListeners_;

	[[nodiscard]] v1::UStatus sendImpl(const v1::UMessage& message) override;

	[[nodiscard]] v1::UStatus registerListenerImpl(
	    const v1::UUri& sink_filter, CallableConn&& listener,
	    std::optional<v1::UUri>&& source_filter) override;

        void notifyListener(const v1::UMessage& message);
        void listenThread(); // listen for incoming messages (thread)
	void cleanupListener(CallableConn listener) override {}
};  // class UTransportDomainSockets

#endif  // UTRANSPORT_DOMAIN_SOCKETS_H