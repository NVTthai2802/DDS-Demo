cmake_minimum_required(VERSION 3.16)
project(MeshDDSBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags (removed -Wall -Wextra for MSVC compatibility)

# Find Fast DDS
find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)

# No fastddsgen, we provide our own SensorData.h manually
# Fetch nlohmann_json for robust JSON logging
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE SRC_FILES "src/*.cpp")
list(REMOVE_ITEM SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/dummy_pub.cpp")

add_executable(backend_node
    ${SRC_FILES}
)

target_link_libraries(backend_node
    fastrtps
    fastcdr
    nlohmann_json::nlohmann_json
)

add_executable(dummy_pub
    src/dummy_pub.cpp
)

target_link_libraries(dummy_pub
    fastrtps
    fastcdr
)
