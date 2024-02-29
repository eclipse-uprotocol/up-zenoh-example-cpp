#include <iostream>
#include <chrono>
#include <csignal>
#include <unistd.h> // For sleep
#include <up-client-zenoh-cpp/transport/zenohUTransport.h>
#include <up-cpp/uuid/factory/Uuidv8Factory.h>
#include <up-cpp/uri/serializer/LongUriSerializer.h>
#include <spdlog/spdlog.h>

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

class RpcListener : public UListener {

    public:
        UStatus onReceive(const UUri& uri,
                          const UPayload& payload,
                          const UAttributes& attributes) const override {
            
            /* Construct response payload with the current time */
            auto currentTime = std::chrono::system_clock::now();
            auto duration = currentTime.time_since_epoch();
            uint64_t currentTimeMilli = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

            UPayload responsePayload(reinterpret_cast<const uint8_t*>(&currentTimeMilli), sizeof(currentTimeMilli), UPayloadType::VALUE);

            /* Build response attributes */
            UAttributesBuilder builder(attributes.id(), UMessageType::RESPONSE, UPriority::STANDARD);
            UAttributes responseAttributes = builder.build();

            /* Send the response */
            return ZenohUTransport::instance().send(uri, responsePayload, responseAttributes);
        }
};

int main(int argc, char** argv) {

    RpcListener listener;

    signal(SIGINT, signalHandler);

    UStatus status;
    ZenohUTransport *transport = &ZenohUTransport::instance();
    
    status = transport->init();
    if (UCode::OK != status.code()) {
        spdlog::error("ZenohUTransport init failed");
        return -1;
    }

    auto rpcUri = LongUriSerializer::deserialize("/test_rpc.app/1/rpc.milliseconds");

    status = transport->registerListener(rpcUri, listener);
    if (UCode::OK != status.code()) {
        spdlog::error("registerListener failed");
        return -1;
    }

    while (!gTerminate) {
        sleep(1);
    }

    status = transport->unregisterListener(rpcUri, listener);
    if (UCode::OK != status.code()) {
        spdlog::error("unregisterListener failed");
        return -1;
    }

    status = transport->term();
    if (UCode::OK != status.code()) {
        spdlog::error("ZenohUTransport term failed");
        return -1;
    }

    return 0;
}
