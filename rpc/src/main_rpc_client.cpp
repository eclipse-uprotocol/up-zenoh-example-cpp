#include <iostream>
#include <chrono>
#include <csignal>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <up-client-zenoh-cpp/rpc/zenohRpcClient.h>
#include <up-cpp/uuid/factory/Uuidv8Factory.h>
#include <up-cpp/uri/serializer/LongUriSerializer.h>

using namespace uprotocol::utransport;
using namespace uprotocol::uuid;
using namespace uprotocol::uri;

bool gTerminate = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl+C received. Exiting..." << std::endl;
        gTerminate = true; 
    }
}

UPayload sendRPC(UUri& uri) {
   
    auto uuid = Uuidv8Factory::create();
   
    UAttributesBuilder builder(uuid, UMessageType::REQUEST, UPriority::STANDARD);
    UAttributes attributes = builder.build();
  
    constexpr uint8_t BUFFER_SIZE = 1;
    uint8_t buffer[BUFFER_SIZE] = {0}; 

    UPayload payload(buffer, sizeof(buffer), UPayloadType::VALUE);

    std::future<UPayload> result = ZenohRpcClient::instance().invokeMethod(uri, payload, attributes);

    if (!result.valid()) {
        spdlog::error("Future is invalid");
        return UPayload(nullptr, 0, UPayloadType::UNDEFINED);   
    }

    result.wait();

    return result.get();
}

int main(int argc, char** argv) {
   
    signal(SIGINT, signalHandler);

    UStatus status;
    ZenohRpcClient *rpcClient = &ZenohRpcClient::instance();

    status = rpcClient->init();
    if (UCode::OK != status.code()) {
        spdlog::error("init failed");
        return -1;
    }

    auto rpcUri = LongUriSerializer::deserialize("/test_rpc.app/1/rpc.milliseconds");

    while (!gTerminate) {

        auto response = sendRPC(rpcUri);

        uint64_t milliseconds = 0;

        if (response.data() != nullptr && response.size() >= sizeof(uint64_t)) {
            memcpy(&milliseconds, response.data(), sizeof(uint64_t));
            spdlog::info("Received = {}", milliseconds);
        }

        sleep(1);
    }

    status = rpcClient->term();
    if (UCode::OK != status.code()) {
        spdlog::error("term failed");
        return -1;
    }

    return 0;
}