#include "DDSEventListener.hpp"
#include "Logger.hpp"
#include "DeviceLogPubSubTypes.h"
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <sstream>

using namespace eprosima::fastdds::dds;

DDSEventListener::DDSEventListener(std::shared_ptr<DeviceLogProcessor> processor)
    : processor_(std::move(processor)) {}

void DDSEventListener::on_participant_discovery(
        DomainParticipant* participant,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) {
    try {
        std::string status;
        if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT) {
            status = "joined";
        } else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT || 
                   info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT) {
            status = "left";
        }

        if (!status.empty()) {
            std::ostringstream guid_stream;
            guid_stream << info.info.m_guid;
            processor_->process_discovery(status, "participant", guid_stream.str(), std::string(info.info.m_participantName));
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in on_participant_discovery: " + std::string(e.what()));
    } catch (...) {
        LOG_ERROR("Unknown exception in on_participant_discovery");
    }
}

void DDSEventListener::on_data_available(DataReader* reader) {
    try {
        SampleInfo info;
        DeviceLog sample;
        if (reader->take_next_sample(&sample, &info) == ReturnCode_t::RETCODE_OK) {
            if (info.valid_data) {
                processor_->process_data(sample);
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in on_data_available: " + std::string(e.what()));
    } catch (...) {
        LOG_ERROR("Unknown exception in on_data_available");
    }
}

void DDSEventListener::on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) {
    try {
        if (info.current_count_change == 1) {
            LOG_INFO("Publisher matched.");
            processor_->process_discovery("matched", "publisher", "unknown_guid", "unknown_name");
        } else if (info.current_count_change == -1) {
            LOG_INFO("Publisher unmatched.");
            processor_->process_discovery("unmatched", "publisher", "unknown_guid", "unknown_name");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in on_subscription_matched: " + std::string(e.what()));
    } catch (...) {
        LOG_ERROR("Unknown exception in on_subscription_matched");
    }
}
