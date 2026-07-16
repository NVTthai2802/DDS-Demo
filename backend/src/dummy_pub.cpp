#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <thread>
#include <chrono>
#include <iostream>

#include "SensorData.h"

using namespace eprosima::fastdds::dds;

int main() {
    DomainParticipantQos pqos;
    pqos.name("DummyPublisher");
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    TypeSupport type(new SensorDataPubSubType());
    type.register_type(participant);

    Topic* topic = participant->create_topic("SensorData", type.get_type_name(), TOPIC_QOS_DEFAULT);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.data_sharing().off();
    wqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;
    wqos.liveliness().lease_duration = eprosima::fastrtps::Duration_t(3, 0);
    wqos.liveliness().announcement_period = eprosima::fastrtps::Duration_t(1, 0);

    DataWriter* writer = publisher->create_datawriter(topic, wqos);

    SensorData data;
    data.device_id("Dummy-01");
    uint32_t seq = 0;

    while (true) {
        data.sequence_number(seq++);
        data.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch()).count());
        bool ret = writer->write(&data);
        std::cout << "Published data seq: " << seq << " success: " << ret << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
