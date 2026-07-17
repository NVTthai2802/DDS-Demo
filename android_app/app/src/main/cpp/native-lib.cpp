#include <jni.h>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <android/log.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "SensorDataPubSubTypes.h"

#define LOG_TAG "MeshDDS_JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace eprosima::fastdds::dds;

JavaVM* g_jvm = nullptr;
jobject g_mainActivity = nullptr;

class SubListener : public DataReaderListener {
public:
    void on_data_available(DataReader* reader) override {
        SensorData data;
        SampleInfo info;
        while (reader->take_next_sample(&data, &info) == ReturnCode_t::RETCODE_OK) {
            if (info.valid_data) {
                long long current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                long long latency = current_time - data.timestamp();
                
                // C++ to Kotlin via JNI
                if (g_jvm && g_mainActivity) {
                    JNIEnv* env;
                    int envStat = g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
                    bool attached = false;
                    if (envStat == JNI_EDETACHED) {
                        if (g_jvm->AttachCurrentThread(&env, NULL) != 0) {
                            LOGE("Failed to attach thread for JNI");
                            return;
                        }
                        attached = true;
                    }
                    
                    jclass clazz = env->GetObjectClass(g_mainActivity);
                    jmethodID methodId = env->GetMethodID(clazz, "onSensorDataReceived", "(Ljava/lang/String;IFFJJ)V");
                    if (methodId) {
                        jstring jDeviceId = env->NewStringUTF(data.device_id().c_str());
                        env->CallVoidMethod(g_mainActivity, methodId, jDeviceId, 
                                            (jint)data.sequence_number(), (jfloat)data.temperature(), (jfloat)data.humidity(), 
                                            (jlong)data.timestamp(), (jlong)latency);
                        env->DeleteLocalRef(jDeviceId);
                    } else {
                        LOGE("Could not find method onSensorDataReceived");
                    }
                    env->DeleteLocalRef(clazz);
                    
                    if (attached) {
                        g_jvm->DetachCurrentThread();
                    }
                }
            }
        }
    }
};

class MeshDdsNode {
public:
    MeshDdsNode() : participant_(nullptr), publisher_(nullptr), subscriber_(nullptr),
                    topic_(nullptr), writer_(nullptr), reader_(nullptr), type_(new SensorDataPubSubType()), running_(false) {}

    ~MeshDdsNode() { stop(); }

    bool start(const std::string& device_id) {
        device_id_ = device_id;
        
        DomainParticipantQos pqos;
        pqos.name(device_id.c_str());

        // Multicast lock is held by Java side. UDPv4 SPDP should work.
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
        if (!participant_) {
            LOGE("Failed to create DomainParticipant");
            return false;
        }

        type_.register_type(participant_);
        topic_ = participant_->create_topic("SensorData", type_.get_type_name(), TOPIC_QOS_DEFAULT);
        
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
        DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
        wqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;
        wqos.liveliness().lease_duration = eprosima::fastrtps::Duration_t(10, 0);
        wqos.liveliness().announcement_period = eprosima::fastrtps::Duration_t(3, 0);
        writer_ = publisher_->create_datawriter(topic_, wqos);
        
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &sub_listener_);

        running_ = true;
        pub_thread_ = std::thread(&MeshDdsNode::publish_loop, this);

        LOGI("MeshDDS Node started for device: %s", device_id.c_str());
        return true;
    }

    void stop() {
        running_ = false;
        if (pub_thread_.joinable()) pub_thread_.join();

        if (participant_) {
            if (publisher_ && writer_) publisher_->delete_datawriter(writer_);
            if (subscriber_ && reader_) subscriber_->delete_datareader(reader_);
            if (publisher_) participant_->delete_publisher(publisher_);
            if (subscriber_) participant_->delete_subscriber(subscriber_);
            if (topic_) participant_->delete_topic(topic_);
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
            participant_ = nullptr;
        }
        LOGI("MeshDDS Node stopped.");
    }

private:
    void publish_loop() {
        SensorData data;
        data.device_id(device_id_);
        uint32_t seq = 0;

        while (running_) {
            data.sequence_number(seq++);
            auto now = std::chrono::system_clock::now().time_since_epoch();
            data.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
            data.temperature(25.0f + (rand() % 100) / 10.0f);
            data.humidity(50.0f + (rand() % 100) / 10.0f);
            data.co2(400.0f + (rand() % 200)); // 400-600 ppm
            data.light(300.0f + (rand() % 500)); // 300-800 lux
            data.occupancy((rand() % 100) > 80); // 20% true
            data.battery(80.0f + (rand() % 20)); // 80-100%
            data.signal_strength(-40.0f - (rand() % 40)); // -40 to -80 dBm
            
            if (writer_) {
                writer_->write(&data);
                LOGI("Published SensorData seq: %d from %s", seq, device_id_.c_str());
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    DomainParticipant* participant_;
    Publisher* publisher_;
    Subscriber* subscriber_;
    Topic* topic_;
    DataWriter* writer_;
    DataReader* reader_;
    TypeSupport type_;
    std::string device_id_;
    std::thread pub_thread_;
    std::atomic<bool> running_;
    SubListener sub_listener_;
};

MeshDdsNode* g_node = nullptr;

extern "C" JNIEXPORT void JNICALL
Java_com_bee_meshdds_MainActivity_startDds(JNIEnv* env, jobject thiz, jstring deviceId) {
    if (!g_node) {
        env->GetJavaVM(&g_jvm);
        g_mainActivity = env->NewGlobalRef(thiz);
        
        g_node = new MeshDdsNode();
        const char* device_id_cstr = env->GetStringUTFChars(deviceId, 0);
        g_node->start(std::string(device_id_cstr));
        env->ReleaseStringUTFChars(deviceId, device_id_cstr);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_bee_meshdds_MainActivity_stopDds(JNIEnv* env, jobject /* this */) {
    if (g_node) {
        g_node->stop();
        delete g_node;
        g_node = nullptr;
    }
    if (g_mainActivity) {
        env->DeleteGlobalRef(g_mainActivity);
        g_mainActivity = nullptr;
    }
}

