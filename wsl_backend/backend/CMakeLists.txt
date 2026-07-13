cmake_minimum_required(VERSION 3.16)
project(WSL2DashboardBackend CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)

# Generate C++ from IDL
execute_process(
    COMMAND fastddsgen -replace ${CMAKE_CURRENT_SOURCE_DIR}/DeviceLog.idl
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(subscriber
    main.cpp
    DDSEventListener.cpp
    JsonOutputAdapter.cpp
    DeviceLog.cxx
    DeviceLogPubSubTypes.cxx
)

target_link_libraries(subscriber
    fastrtps
    fastcdr
)

add_executable(test_publisher
    TestPublisher.cpp
    DeviceLog.cxx
    DeviceLogPubSubTypes.cxx
)

target_link_libraries(test_publisher
    fastrtps
    fastcdr
)
