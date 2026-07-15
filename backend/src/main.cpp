#include "BackendSubscriber.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <nlohmann/json.hpp>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    uint32_t domain_id = 0;
    std::string topic_name = "SensorData";

    const char* env_domain = std::getenv("DOMAIN_ID");
    if (env_domain != nullptr) {
        try {
            domain_id = std::stoul(env_domain);
        } catch (...) {
            // keep default 0
        }
    }

    const char* env_topic = std::getenv("TOPIC_NAME");
    if (env_topic != nullptr) {
        topic_name = env_topic;
    }

    BackendSubscriber sub;
    if (sub.init(domain_id, topic_name)) {
        sub.run();
    } else {
        nlohmann::json err;
        err["event"] = "error";
        err["message"] = "Failed to initialize BackendSubscriber";
        std::cerr << err.dump() << std::endl;
        return 1;
    }

    return 0;
}
