#ifndef JSONOUTPUTADAPTER_HPP
#define JSONOUTPUTADAPTER_HPP

#include "DeviceLogProcessor.hpp"

class JsonOutputAdapter : public DeviceLogProcessor {
public:
    JsonOutputAdapter() = default;
    ~JsonOutputAdapter() override = default;

    void process_data(const DeviceLog& log) override;
    void process_discovery(const std::string& event_type, const std::string& entity, 
                           const std::string& guid, const std::string& name) override;
};

#endif // JSONOUTPUTADAPTER_HPP
