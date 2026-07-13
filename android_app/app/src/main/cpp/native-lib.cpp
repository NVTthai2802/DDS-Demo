#include <jni.h>
#include <string>
#include <thread>
#include <chrono>
#include <android/log.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>

#include "DeviceLog.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "DDS_NATIVE", __VA_ARGS__)

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

// Global variables (simplified for Demo)
DomainParticipant* participant = nullptr;
Publisher* publisher = nullptr;
Topic* topic = nullptr;
DataWriter* writer = nullptr;
TypeSupport type(new DeviceLogPubSubType());
bool isPublishing = false;
int32_t seq_num = 0;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_ddsdemo_MainActivity_startDDS(JNIEnv* env, jobject /* this */, jboolean is_reliable) {
    if (participant != nullptr) return true;

    LOGI("Initializing DDS with Reliable QoS: %d", is_reliable);

    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Android_Publisher");

    // Enable Builtin transports for Multicast Auto-Discovery (SPDP)
    // Multicast is now allowed thanks to Android MulticastLock
    pqos.transport().use_builtin_transports = true;

    participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant == nullptr) return false;

    type.register_type(participant);

    topic = participant->create_topic("DeviceLogTopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    
    publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    if (is_reliable) {
        wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    } else {
        wqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 10;
    
    // Configure Liveliness for Node Discovery (Lease Duration = 3s)
    wqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;
    wqos.liveliness().lease_duration = eprosima::fastrtps::Duration_t(3, 0);
    
    writer = publisher->create_datawriter(topic, wqos, nullptr);

    LOGI("DDS initialized successfully!");
    return true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_ddsdemo_MainActivity_stopDDS(JNIEnv* env, jobject /* this */) {
    isPublishing = false;
    if (participant != nullptr) {
        if (publisher != nullptr) {
            if (writer != nullptr) publisher->delete_datawriter(writer);
            participant->delete_publisher(publisher);
        }
        if (topic != nullptr) participant->delete_topic(topic);
        DomainParticipantFactory::get_instance()->delete_participant(participant);
        
        participant = nullptr;
        publisher = nullptr;
        writer = nullptr;
        topic = nullptr;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_ddsdemo_MainActivity_publishData(JNIEnv* env, jobject /* this */, 
    jstring device_id, jstring device_type, jdouble temp, jdouble hum, 
    jint co2, jint light, jboolean occ, jint batt, jint sig) {
    if (writer == nullptr) return;

    const char *id_str = env->GetStringUTFChars(device_id, 0);
    const char *type_str = env->GetStringUTFChars(device_type, 0);
    
    DeviceLog log_data;
    log_data.device_id = std::string(id_str);
    log_data.device_type = std::string(type_str);
    log_data.temperature = temp;
    log_data.humidity = hum;
    log_data.co2 = co2;
    log_data.light = light;
    log_data.occupancy = occ;
    log_data.battery = batt;
    log_data.signal_strength = sig;
    log_data.sequence_number = seq_num++;
    
    // Get current timestamp in seconds (double)
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    log_data.timestamp = std::chrono::duration<double>(duration).count();

    writer->write(&log_data);
    LOGI("Sent sensor data seq: %d", seq_num - 1);

    env->ReleaseStringUTFChars(device_id, id_str);
    env->ReleaseStringUTFChars(device_type, type_str);
}