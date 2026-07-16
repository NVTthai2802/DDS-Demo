#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <string>
#include <cstdint>
#include <fastcdr/Cdr.h>
#include <fastdds/dds/topic/TopicDataType.hpp>

class SensorData {
public:
    std::string m_device_id;
    uint32_t m_sequence_number;
    uint64_t m_timestamp;
    float m_temperature;
    float m_humidity;
    float m_co2;
    float m_light;
    bool m_occupancy;
    float m_battery;
    float m_signal_strength;

    SensorData() : m_sequence_number(0), m_timestamp(0), m_temperature(0), m_humidity(0), m_co2(0), m_light(0), m_occupancy(false), m_battery(0), m_signal_strength(0) {}
    ~SensorData() {}

    // Getters and setters for compatibility
    const std::string& device_id() const { return m_device_id; }
    void device_id(const std::string& v) { m_device_id = v; }

    uint32_t sequence_number() const { return m_sequence_number; }
    void sequence_number(uint32_t v) { m_sequence_number = v; }

    uint64_t timestamp() const { return m_timestamp; }
    void timestamp(uint64_t v) { m_timestamp = v; }

    float temperature() const { return m_temperature; }
    void temperature(float v) { m_temperature = v; }

    float humidity() const { return m_humidity; }
    void humidity(float v) { m_humidity = v; }

    float co2() const { return m_co2; }
    void co2(float v) { m_co2 = v; }

    float light() const { return m_light; }
    void light(float v) { m_light = v; }

    bool occupancy() const { return m_occupancy; }
    void occupancy(bool v) { m_occupancy = v; }

    float battery() const { return m_battery; }
    void battery(float v) { m_battery = v; }

    float signal_strength() const { return m_signal_strength; }
    void signal_strength(float v) { m_signal_strength = v; }

    void serialize(eprosima::fastcdr::Cdr& cdr) const {
        cdr << m_device_id;
        cdr << m_sequence_number;
        cdr << m_timestamp;
        cdr << m_temperature;
        cdr << m_humidity;
        cdr << m_co2;
        cdr << m_light;
        cdr << m_occupancy;
        cdr << m_battery;
        cdr << m_signal_strength;
    }

    void deserialize(eprosima::fastcdr::Cdr& cdr) {
        cdr >> m_device_id;
        cdr >> m_sequence_number;
        cdr >> m_timestamp;
        cdr >> m_temperature;
        cdr >> m_humidity;
        cdr >> m_co2;
        cdr >> m_light;
        cdr >> m_occupancy;
        cdr >> m_battery;
        cdr >> m_signal_strength;
    }

    static size_t getCdrSerializedSize(const SensorData& data, size_t current_alignment = 0) {
        size_t initial_alignment = current_alignment;
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.m_device_id.size() + 1;
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4); // seq
        current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8); // timestamp
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4); // temp
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4); // hum
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4); // co2
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4); // light
        current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1); // occupancy
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4); // battery
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4); // signal
        return current_alignment - initial_alignment;
    }

    static bool isKeyDefined() { return false; }
};

class SensorDataPubSubType : public eprosima::fastdds::dds::TopicDataType {
public:
    typedef SensorData type;

    SensorDataPubSubType() {
        setName("SensorData");
        auto type_size = SensorData::getCdrSerializedSize(SensorData()) + 4; // encapsulation
        type_size += 256; // margin for strings
        m_typeSize = static_cast<uint32_t>(type_size);
        m_isGetKeyDefined = SensorData::isKeyDefined();
    }

    virtual ~SensorDataPubSubType() {}

    virtual bool serialize(void* data, eprosima::fastrtps::rtps::SerializedPayload_t* payload) override {
        SensorData* p_type = static_cast<SensorData*>(data);
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
        payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        ser.serialize_encapsulation();
        try {
            p_type->serialize(ser);
        } catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/) {
            return false;
        }
        payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
        return true;
    }

    virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t* payload, void* data) override {
        try {
            SensorData* p_type = static_cast<SensorData*>(data);
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);
            eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            deser.read_encapsulation();
            payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            p_type->deserialize(deser);
        } catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/) {
            return false;
        }
        return true;
    }

    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override {
        return [data]() -> uint32_t {
            return static_cast<uint32_t>(SensorData::getCdrSerializedSize(*static_cast<SensorData*>(data))) + 4 /*encapsulation*/;
        };
    }

    virtual void* createData() override { return reinterpret_cast<void*>(new SensorData()); }
    virtual void deleteData(void* data) override { delete static_cast<SensorData*>(data); }
    virtual bool getKey(void* /*data*/, eprosima::fastrtps::rtps::InstanceHandle_t* /*ihandle*/, bool /*force_md5*/ = false) override { return false; }
};

#endif
