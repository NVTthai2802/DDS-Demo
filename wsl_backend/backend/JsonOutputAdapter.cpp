#include "JsonOutputAdapter.hpp"
#include "Logger.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

void JsonOutputAdapter::process_data(const DeviceLog& log) {
    try {
        json j;
        j["type"] = "data";
        
        json payload;
        payload["device_id"] = log.device_id();
        payload["device_type"] = log.device_type();
        payload["temperature"] = log.temperature();
        payload["humidity"] = log.humidity();
        payload["co2"] = log.co2();
        payload["light"] = log.light();
        payload["occupancy"] = log.occupancy();
        payload["battery"] = log.battery();
        payload["signal_strength"] = log.signal_strength();
        payload["timestamp"] = log.timestamp();
        payload["sequence_number"] = log.sequence_number();
        
        j["payload"] = payload;

        std::cout << j.dump() << std::endl;
    } catch (const std::exception& e) {
        LOG_ERROR("JSON Serialization failed for data: " + std::string(e.what()));
    } catch (...) {
        LOG_ERROR("JSON Serialization failed for data due to unknown error.");
    }
}

void JsonOutputAdapter::process_discovery(const std::string& event_type, const std::string& entity, 
                                          const std::string& guid, const std::string& name) {
    try {
        json j;
        j["type"] = "discovery";
        j["entity"] = entity;
        j["event"] = event_type;
        j["guid"] = guid;
        j["name"] = name;

        std::cout << j.dump() << std::endl;
    } catch (const std::exception& e) {
        LOG_ERROR("JSON Serialization failed for discovery: " + std::string(e.what()));
    } catch (...) {
        LOG_ERROR("JSON Serialization failed for discovery due to unknown error.");
    }
}
