#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include "SensorData.h"
#include <mutex>
#include <string>

class BackendSubscriber {
public:
    BackendSubscriber();
    virtual ~BackendSubscriber();

    bool init(uint32_t domain_id, const std::string& topic_name);
    void run();

private:
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataReader* reader_;
    eprosima::fastdds::dds::TypeSupport type_;

    class PartListener : public eprosima::fastdds::dds::DomainParticipantListener {
    public:
        PartListener() = default;
        ~PartListener() override = default;
        void on_participant_discovery(
                eprosima::fastdds::dds::DomainParticipant* participant,
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
    } part_listener_;

    class ReadListener : public eprosima::fastdds::dds::DataReaderListener {
    public:
        ReadListener() = default;
        ~ReadListener() override = default;
        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;
        void on_data_available(eprosima::fastdds::dds::DataReader* reader) override;
        void on_liveliness_changed(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::LivelinessChangedStatus& status) override;
    } read_listener_;
};
