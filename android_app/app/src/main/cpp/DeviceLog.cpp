#include "DeviceLog.h"
#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/exceptions/Exception.h>

DeviceLogPubSubType::DeviceLogPubSubType() {
    setName("DeviceLog");
    // Kich thuoc toi da du tru cho viec cap nhat (co the uoc luong khoan 256 bytes)
    m_typeSize = 256; 
    m_isGetKeyDefined = false;
}

DeviceLogPubSubType::~DeviceLogPubSubType() {}

bool DeviceLogPubSubType::serialize(void* data, eprosima::fastrtps::rtps::SerializedPayload_t* payload) {
    DeviceLog* p = static_cast<DeviceLog*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    
    try {
        ser.serialize_encapsulation();
        ser << p->device_id;
        ser << p->device_type;
        ser << p->temperature;
        ser << p->humidity;
        ser << p->co2;
        ser << p->light;
        ser << p->occupancy;
        ser << p->battery;
        ser << p->signal_strength;
        ser << p->timestamp;
        ser << p->sequence_number;
    } catch (eprosima::fastcdr::exception::Exception& /*exception*/) {
        return false;
    }
    payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
    return true;
}

bool DeviceLogPubSubType::deserialize(eprosima::fastrtps::rtps::SerializedPayload_t* payload, void* data) {
    DeviceLog* p = static_cast<DeviceLog*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
    
    try {
        deser.read_encapsulation();
        deser >> p->device_id;
        deser >> p->device_type;
        deser >> p->temperature;
        deser >> p->humidity;
        deser >> p->co2;
        deser >> p->light;
        deser >> p->occupancy;
        deser >> p->battery;
        deser >> p->signal_strength;
        deser >> p->timestamp;
        deser >> p->sequence_number;
    } catch (eprosima::fastcdr::exception::Exception& /*exception*/) {
        return false;
    }
    return true;
}

std::function<uint32_t()> DeviceLogPubSubType::getSerializedSizeProvider(void* /*data*/) {
    return []() -> uint32_t { return 256; };
}

void* DeviceLogPubSubType::createData() {
    return reinterpret_cast<void*>(new DeviceLog());
}

void DeviceLogPubSubType::deleteData(void* data) {
    delete reinterpret_cast<DeviceLog*>(data);
}

bool DeviceLogPubSubType::getKey(void* /*data*/, eprosima::fastrtps::rtps::InstanceHandle_t* /*ihandle*/, bool /*force_md5*/) {
    return false;
}
