cmake_minimum_required(VERSION 3.16)
project(MeshDDSBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: Add compiler flags for warnings
add_compile_options(-Wall -Wextra)

# Find Fast DDS
find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)

# Try to find fastddsgen
find_program(FASTDDSGEN fastddsgen)
if(NOT FASTDDSGEN)
    message(FATAL_ERROR "fastddsgen executable not found. Make sure it is in PATH.")
endif()

# Generate sources from IDL
set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SensorData.idl)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(GENERATED_SOURCES
    ${GENERATED_DIR}/SensorData.cxx
    ${GENERATED_DIR}/SensorDataPubSubTypes.cxx
)

add_custom_command(
    OUTPUT ${GENERATED_SOURCES}
    COMMAND ${FASTDDSGEN} -replace -d ${GENERATED_DIR} ${IDL_FILE}
    DEPENDS ${IDL_FILE}
    COMMENT "Generating C++ source files from SensorData.idl using fastddsgen"
)

# Fetch nlohmann_json for robust JSON logging
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Include generated headers directory
include_directories(${GENERATED_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(backend_node
    ${SRC_FILES}
    ${GENERATED_SOURCES}
)

target_link_libraries(backend_node
    fastrtps
    fastcdr
    nlohmann_json::nlohmann_json
    pthread
    dl
)
