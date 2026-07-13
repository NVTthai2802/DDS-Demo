#ifndef DEVICELOG_H
#define DEVICELOG_H

#include <string>
#include <fastdds/dds/topic/TopicDataType.hpp>

// Dinh nghia Struct tuong ung voi Environmental Sensor
struct DeviceLog {
    std::string device_id;
    std::string device_type;
    double temperature;
    double humidity;
    int32_t co2;
    int32_t light;
    bool occupancy;
    int32_t battery;
    int32_t signal_strength;
    double timestamp;
    int32_t sequence_number;
};

// Dinh nghia PubSubType (Thay the viec chay fastddsgen de don gian hoa)
class DeviceLogPubSubType : public eprosima::fastdds::dds::TopicDataType {
public:
    DeviceLogPubSubType();
    virtual ~DeviceLogPubSubType() override;
    bool serialize(void* data, eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;
    bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t* payload, void* data) override;
    std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
    void* createData() override;
    void deleteData(void* data) override;
    bool getKey(void* data, eprosima::fastrtps::rtps::InstanceHandle_t* ihandle, bool force_md5) override;
};

#endif // DEVICELOG_H
