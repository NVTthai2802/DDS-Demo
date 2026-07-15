#include "BackendSubscriber.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/builtin/topic/PublicationBuiltinTopicData.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>

using namespace eprosima::fastdds::dds;
using json = nlohmann::json;

BackendSubscriber::BackendSubscriber()
    : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr), type_(new SensorDataPubSubType()) {}

BackendSubscriber::~BackendSubscriber() {
    if (participant_ != nullptr) {
        if (subscriber_ != nullptr) {
            if (reader_ != nullptr) {
                subscriber_->delete_datareader(reader_);
            }
            participant_->delete_subscriber(subscriber_);
        }
        if (topic_ != nullptr) {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool BackendSubscriber::init(uint32_t domain_id, const std::string& topic_name) {
    try {
        DomainParticipantQos pqos;
        pqos.name("Backend_Dashboard_Participant");

        participant_ = DomainParticipantFactory::get_instance()->create_participant(domain_id, pqos, &part_listener_);
        if (participant_ == nullptr) {
            throw std::runtime_error("Failed to create DomainParticipant");
        }

        type_.register_type(participant_);

        topic_ = participant_->create_topic(topic_name, type_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr) {
            throw std::runtime_error("Failed to create Topic");
        }

        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (subscriber_ == nullptr) {
            throw std::runtime_error("Failed to create Subscriber");
        }

        DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
        rqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS; 

        reader_ = subscriber_->create_datareader(topic_, rqos, &read_listener_);
        if (reader_ == nullptr) {
            throw std::runtime_error("Failed to create DataReader");
        }

        return true;
    } catch (const std::exception& e) {
        json log;
        log["event"] = "error";
        log["message"] = std::string("Init error: ") + e.what();
        std::cout << log.dump() << std::endl;
        return false;
    } catch (...) {
        json log;
        log["event"] = "error";
        log["message"] = "Unknown init error";
        std::cout << log.dump() << std::endl;
        return false;
    }
}

void BackendSubscriber::run() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void BackendSubscriber::PartListener::on_participant_discovery(
        DomainParticipant* participant,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) {
    (void)participant;
    try {
        json log;
        log["event"] = "spdp";
        
        std::stringstream ss;
        ss << info.info.m_guid;
        log["guid"] = ss.str();
        log["name"] = info.info.m_participantName.to_string();

        if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT) {
            log["status"] = "DISCOVERED";
        } else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT || 
                   info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT) {
            log["status"] = "REMOVED";
        } else {
            return;
        }

        std::cout << log.dump() << std::endl;
    } catch (const std::exception& e) {
        json err;
        err["event"] = "error";
        err["message"] = std::string("SPDP error: ") + e.what();
        std::cout << err.dump() << std::endl;
    } catch (...) {
        json err;
        err["event"] = "error";
        err["message"] = "Unknown SPDP error";
        std::cout << err.dump() << std::endl;
    }
}

void BackendSubscriber::ReadListener::on_subscription_matched(
        DataReader* reader,
        const SubscriptionMatchedStatus& info) {
    try {
        json log;
        log["event"] = "sedp";

        std::stringstream ss;
        ss << info.last_publication_handle;
        log["writer_guid"] = ss.str();

        if (info.current_count_change == 1) {
            log["status"] = "MATCHED";
            
            eprosima::fastdds::dds::builtin::PublicationBuiltinTopicData pub_data;
            if (reader->get_matched_publication_data(pub_data, info.last_publication_handle) == ReturnCode_t::RETCODE_OK) {
                if (pub_data.reliability.kind == RELIABLE_RELIABILITY_QOS) {
                    log["qos_reliability"] = "RELIABLE";
                } else {
                    log["qos_reliability"] = "BEST_EFFORT";
                }
            } else {
                log["qos_reliability"] = "UNKNOWN";
            }
        } else if (info.current_count_change == -1) {
            log["status"] = "UNMATCHED";
        } else {
            return;
        }

        std::cout << log.dump() << std::endl;
    } catch (const std::exception& e) {
        json err;
        err["event"] = "error";
        err["message"] = std::string("SEDP error: ") + e.what();
        std::cout << err.dump() << std::endl;
    } catch (...) {
        json err;
        err["event"] = "error";
        err["message"] = "Unknown SEDP error";
        std::cout << err.dump() << std::endl;
    }
}

void BackendSubscriber::ReadListener::on_data_available(DataReader* reader) {
    try {
        SensorData data;
        SampleInfo info;

        while (reader->take_next_sample(&data, &info) == ReturnCode_t::RETCODE_OK) {
            if (info.valid_data) {
                json log;
                log["event"] = "data";
                log["device_id"] = data.device_id();
                log["sequence_number"] = data.sequence_number();
                log["timestamp"] = data.timestamp();
                log["temperature"] = data.temperature();
                log["humidity"] = data.humidity();
                log["co2"] = data.co2();
                log["light"] = data.light();
                log["occupancy"] = data.occupancy();
                log["battery"] = data.battery();
                log["signal_strength"] = data.signal_strength();

                auto now = std::chrono::system_clock::now().time_since_epoch();
                auto current_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
                log["receive_time"] = current_ms;
                
                std::stringstream ss;
                ss << info.sample_identity.writer_guid();
                log["writer_guid"] = ss.str();

                std::cout << log.dump() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        json err;
        err["event"] = "error";
        err["message"] = std::string("Data read error: ") + e.what();
        std::cout << err.dump() << std::endl;
    } catch (...) {
        json err;
        err["event"] = "error";
        err["message"] = "Unknown data read error";
        std::cout << err.dump() << std::endl;
    }
}
