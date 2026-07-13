#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include "DeviceLogPubSubTypes.h"

using namespace eprosima::fastdds::dds;

int main() {
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Test_Local_Publisher");

    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (!participant) {
        std::cerr << "[ERROR] Failed to create participant" << std::endl;
        return 1;
    }

    TypeSupport type(new DeviceLogPubSubType());
    type.register_type(participant);

    Topic* topic = participant->create_topic("DeviceLogTopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    Publisher* pub = participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 10;

    DataWriter* writer = pub->create_datawriter(topic, wqos, nullptr);

    std::cerr << "[TEST] Local publisher started. Sending 10 samples..." << std::endl;

    for (int i = 0; i < 10; ++i) {
        DeviceLog sample;
        sample.device_id("TestRoom-WSL2");
        sample.device_type("Test Sensor");
        sample.temperature(26.5 + i * 0.1);
        sample.humidity(55.0 + i * 0.2);
        sample.co2(800 + i);
        sample.light(450 + i * 5);
        sample.occupancy(true);
        sample.battery(95 - i);
        sample.signal_strength(-50 - i);
        sample.sequence_number(i);
        
        auto now = std::chrono::system_clock::now();
        sample.timestamp(std::chrono::duration<double>(now.time_since_epoch()).count());

        writer->write(&sample);
        std::cerr << "[TEST] Sent sample seq=" << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cerr << "[TEST] Done!" << std::endl;
    return 0;
}
