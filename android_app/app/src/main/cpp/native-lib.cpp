#include <jni.h>
#include <string>
#include <chrono>
#include <android/log.h>
#include "DDSPublisher.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "DDS_NATIVE", __VA_ARGS__)

static DDSPublisher dds_publisher;
static int32_t seq_num = 0;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_ddsdemo_DDSManager_startDDSNative(JNIEnv* env, jobject /* this */, jboolean is_reliable) {
    return dds_publisher.init(is_reliable);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_ddsdemo_DDSManager_stopDDSNative(JNIEnv* env, jobject /* this */) {
    dds_publisher.stop();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_ddsdemo_DDSManager_publishDataNative(JNIEnv* env, jobject /* this */, 
    jstring device_id, jstring device_type, jdouble temp, jdouble hum, 
    jint co2, jint light, jboolean occ, jint batt, jint sig) {
    
    const char *id_str = env->GetStringUTFChars(device_id, 0);
    const char *type_str = env->GetStringUTFChars(device_type, 0);
    
    DeviceLog log_data;
    log_data.device_id(std::string(id_str));
    log_data.device_type(std::string(type_str));
    log_data.temperature(temp);
    log_data.humidity(hum);
    log_data.co2(co2);
    log_data.light(light);
    log_data.occupancy(occ);
    log_data.battery(batt);
    log_data.signal_strength(sig);
    log_data.sequence_number(seq_num++);
    
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    log_data.timestamp(std::chrono::duration<double>(duration).count());

    if (dds_publisher.publish(log_data)) {
        LOGI("Sent sensor data seq: %d", seq_num - 1);
    }

    env->ReleaseStringUTFChars(device_id, id_str);
    env->ReleaseStringUTFChars(device_type, type_str);
}