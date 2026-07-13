cmake_minimum_required(VERSION 3.22.1)

project("ddsdemo")

# Thu muc chua thu vien DDS da duoc bien dich cheo bang script (dds_libs)
set(DDS_LIBS_DIR "C:/Project TTS/DDS-Demo/android_app/dds_libs")

# Include headers
include_directories(
    ${DDS_LIBS_DIR}/include
)

# Khai bao cac file ma nguon C++ cua chung ta
add_library(
        ${CMAKE_PROJECT_NAME}
        SHARED
        native-lib.cpp
        DDSPublisher.cpp
        DeviceLog.cxx
        DeviceLogPubSubTypes.cxx
)

# Tim thu vien log mac dinh cua Android
find_library(
        log-lib
        log)

# Khai bao cac thu vien dong (so) cua Fast DDS da build
add_library(fastrtps SHARED IMPORTED)
set_target_properties(fastrtps PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libfastrtps.so)

add_library(fastcdr SHARED IMPORTED)
set_target_properties(fastcdr PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libfastcdr.so)

# Lien ket cac thu vien lai voi nhau
target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        fastrtps
        fastcdr
        ${log-lib})