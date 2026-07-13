#ifndef DEVICELOGPROCESSOR_HPP
#define DEVICELOGPROCESSOR_HPP

#include "DeviceLogPubSubTypes.h"
#include <string>

// Abstract class defining the processor interface
class DeviceLogProcessor {
public:
    virtual ~DeviceLogProcessor() = default;

    virtual void process_data(const DeviceLog& log) = 0;
    virtual void process_discovery(const std::string& event_type, const std::string& entity, 
                                   const std::string& guid, const std::string& name) = 0;
};

#endif // DEVICELOGPROCESSOR_HPP
