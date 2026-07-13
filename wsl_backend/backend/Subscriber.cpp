#include <iostream>
#include <string>
#include <chrono>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include "DeviceLogPubSubTypes.h"

using namespace eprosima::fastdds::dds;

class SubListener : public DataReaderListener {
public:
    SubListener() = default;
    ~SubListener() override = default;

    void on_data_available(DataReader* reader) override {
        SampleInfo info;
        DeviceLog sample;
        if (reader->take_next_sample(&sample, &info) == ReturnCode_t::RETCODE_OK) {
            if (info.valid_data) {
                std::cout << "{"
                          << "\"device_id\":\"" << sample.device_id() << "\","
                          << "\"device_type\":\"" << sample.device_type() << "\","
                          << "\"temperature\":" << sample.temperature() << ","
                          << "\"humidity\":" << sample.humidity() << ","
                          << "\"co2\":" << sample.co2() << ","
                          << "\"light\":" << sample.light() << ","
                          << "\"occupancy\":" << (sample.occupancy() ? "true" : "false") << ","
                          << "\"battery\":" << sample.battery() << ","
                          << "\"signal_strength\":" << sample.signal_strength() << ","
                          << "\"timestamp\":" << sample.timestamp() << ","
                          << "\"sequence_number\":" << sample.sequence_number()
                          << "}" << std::endl;
            }
        }
    }
    
    void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override {
        if (info.current_count_change == 1) {
            std::cerr << "[INFO] Publisher matched." << std::endl;
        } else if (info.current_count_change == -1) {
            std::cerr << "[INFO] Publisher unmatched." << std::endl;
        }
    }
};

int main() {
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("WSL2_Dashboard_Subscriber");
    
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant == nullptr) {
        std::cerr << "[ERROR] Failed to create DomainParticipant" << std::endl;
        return 1;
    }

    TypeSupport type(new DeviceLogPubSubType());
    type.register_type(participant);

    Topic* topic = participant->create_topic("DeviceLogTopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic == nullptr) {
        std::cerr << "[ERROR] Failed to create Topic" << std::endl;
        return 1;
    }

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber == nullptr) {
        std::cerr << "[ERROR] Failed to create Subscriber" << std::endl;
        return 1;
    }

    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.history().depth = 10;

    SubListener* listener = new SubListener();
    DataReader* reader = subscriber->create_datareader(topic, rqos, listener);
    if (reader == nullptr) {
        std::cerr << "[ERROR] Failed to create DataReader" << std::endl;
        return 1;
    }

    std::cerr << "[INFO] DDS Subscriber running on WSL2..." << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
