#ifndef DDSEVENTLISTENER_HPP
#define DDSEVENTLISTENER_HPP

#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include "DeviceLogProcessor.hpp"
#include <memory>

class DDSEventListener : public eprosima::fastdds::dds::DomainParticipantListener {
public:
    explicit DDSEventListener(std::shared_ptr<DeviceLogProcessor> processor);
    ~DDSEventListener() override = default;

    // DomainParticipantListener
    void on_participant_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    // DataReaderListener
    void on_data_available(eprosima::fastdds::dds::DataReader* reader) override;
    
    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader, 
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

private:
    std::shared_ptr<DeviceLogProcessor> processor_;
};

#endif // DDSEVENTLISTENER_HPP
