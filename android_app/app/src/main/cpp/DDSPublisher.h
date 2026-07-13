#ifndef DDSPUBLISHER_H
#define DDSPUBLISHER_H

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include "DeviceLogPubSubTypes.h"

class DDSPublisher {
public:
    DDSPublisher();
    ~DDSPublisher();

    bool init(bool is_reliable);
    bool publish(DeviceLog& data);
    void stop();

private:
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::TypeSupport type_;
};

#endif // DDSPUBLISHER_H
