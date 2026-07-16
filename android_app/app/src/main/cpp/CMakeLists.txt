cmake_minimum_required(VERSION 3.18.1)
project("meshdds")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. Asio (Header-only)
FetchContent_Declare(asio
    URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-24-0.tar.gz
)
FetchContent_MakeAvailable(asio)
include_directories(${asio_SOURCE_DIR}/asio/include)
add_definitions(-DASIO_STANDALONE)

# 2. foonathan_memory (Prebuilt)
# foonathan_memory không tự build được như fastcdr, phải build sẵn ngoài bằng 
# scripts/build_foonathan_memory_android.bat trước khi Gradle sync.
# Dùng _DIR (cache) thay vì CMAKE_PREFIX_PATH để find_package() trong Fast-DDS chắc chắn tìm được.
set(foonathan_memory_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty_native/foonathan_memory/${ANDROID_ABI}/lib/foonathan_memory/cmake" CACHE PATH "" FORCE)

# 3. Fast-DDS
set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include CACHE PATH "" FORCE)
set(THIRDPARTY ON CACHE BOOL "" FORCE)
set(COMPILE_TOOLS OFF CACHE BOOL "" FORCE)
set(COMPILE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(INTERNAL_DEBUG OFF CACHE BOOL "" FORCE)
set(SHM_TRANSPORT_DEFAULT OFF CACHE BOOL "" FORCE)
set(SQLITE3_SUPPORT OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(fastdds
    GIT_REPOSITORY https://github.com/eProsima/Fast-DDS.git
    GIT_TAG v2.11.2
)
FetchContent_MakeAvailable(fastdds)

# 4. JNI Library
add_library(meshdds SHARED
    native-lib.cpp
    SensorData.cxx
    SensorDataPubSubTypes.cxx
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(meshdds
    fastrtps
    fastcdr
    log
    android
)
