#include "DDSPublisher.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "DDS_PUBLISHER", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "DDS_PUBLISHER", __VA_ARGS__)

using namespace eprosima::fastdds::dds;

DDSPublisher::DDSPublisher() 
    : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr), type_(new DeviceLogPubSubType()) {}

DDSPublisher::~DDSPublisher() {
    stop();
}

bool DDSPublisher::init(bool is_reliable) {
    if (participant_ != nullptr) return true;

    LOGI("Initializing DDS with Reliable QoS: %d", is_reliable);

    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Android_Publisher");
    pqos.transport().use_builtin_transports = true;

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant_ == nullptr) {
        LOGE("Failed to create DomainParticipant");
        return false;
    }

    type_.register_type(participant_);

    topic_ = participant_->create_topic("DeviceLogTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr) {
        LOGE("Failed to create Topic");
        return false;
    }

    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr) {
        LOGE("Failed to create Publisher");
        return false;
    }

    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    if (is_reliable) {
        wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    } else {
        wqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 10;
    
    wqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;
    wqos.liveliness().lease_duration = eprosima::fastrtps::Duration_t(3, 0);

    writer_ = publisher_->create_datawriter(topic_, wqos, nullptr);
    if (writer_ == nullptr) {
        LOGE("Failed to create DataWriter");
        return false;
    }

    LOGI("DDS initialized successfully!");
    return true;
}

bool DDSPublisher::publish(DeviceLog& data) {
    if (writer_ == nullptr) return false;
    return writer_->write(&data);
}

void DDSPublisher::stop() {
    if (participant_ != nullptr) {
        if (publisher_ != nullptr) {
            if (writer_ != nullptr) {
                publisher_->delete_datawriter(writer_);
                writer_ = nullptr;
            }
            participant_->delete_publisher(publisher_);
            publisher_ = nullptr;
        }
        if (topic_ != nullptr) {
            participant_->delete_topic(topic_);
            topic_ = nullptr;
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
        participant_ = nullptr;
    }
    LOGI("DDS stopped and cleaned up");
}
