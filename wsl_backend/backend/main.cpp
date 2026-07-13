#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include "DeviceLogPubSubTypes.h"
#include "DDSEventListener.hpp"
#include "JsonOutputAdapter.hpp"
#include "Logger.hpp"

#include <thread>
#include <chrono>
#include <memory>

using namespace eprosima::fastdds::dds;

int main() {
    LOG_INFO("Initializing WSL2 Backend Gateway...");

    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("WSL2_Gateway_Participant");
    
    // Inject dependency
    std::shared_ptr<DeviceLogProcessor> processor = std::make_shared<JsonOutputAdapter>();
    DDSEventListener* listener = new DDSEventListener(processor);

    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos, listener);
    if (participant == nullptr) {
        LOG_ERROR("Failed to create DomainParticipant");
        return 1;
    }

    TypeSupport type(new DeviceLogPubSubType());
    type.register_type(participant);

    Topic* topic = participant->create_topic("DeviceLogTopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic == nullptr) {
        LOG_ERROR("Failed to create Topic");
        return 1;
    }

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber == nullptr) {
        LOG_ERROR("Failed to create Subscriber");
        return 1;
    }

    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.history().depth = 10;
    
    // Configure Liveliness for Node Discovery (Lease Duration = 3s)
    rqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;
    rqos.liveliness().lease_duration = eprosima::fastrtps::Duration_t(3, 0);

    DataReader* reader = subscriber->create_datareader(topic, rqos, listener);
    if (reader == nullptr) {
        LOG_ERROR("Failed to create DataReader");
        return 1;
    }

    LOG_INFO("WSL2 Gateway is up and running. Waiting for DDS events...");

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
