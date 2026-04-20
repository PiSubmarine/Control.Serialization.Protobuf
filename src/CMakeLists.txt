include("${_build_cmake_local}")

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)

set(CONTROL_PROTO "${CMAKE_CURRENT_SOURCE_DIR}/PiSubmarine/Control/Protobuf/OperatorCommand.proto")
protobuf_generate_cpp(CONTROL_PROTO_SRCS CONTROL_PROTO_HDRS "${CONTROL_PROTO}")

message("Adding library: ${PISUBMARINE_MODULE_NAME}")
add_library(${PISUBMARINE_MODULE_NAME}
        PiSubmarine/Control/Protobuf/Serializer.cpp
        PiSubmarine/Control/Protobuf/Deserializer.cpp
        "../public/PiSubmarine/Control/Protobuf/Serializer.h"
        "../public/PiSubmarine/Control/Protobuf/Deserializer.h"
        ${CONTROL_PROTO_SRCS}
        ${CONTROL_PROTO_HDRS})
target_include_directories(${PISUBMARINE_MODULE_NAME} PUBLIC "../public" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
PiSubmarineInitTarget(${PISUBMARINE_MODULE_NAME})

PiSubmarineAddDependency("https://github.com/PiSubmarine/Control.Serialization.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Control.Serialization.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME} PRIVATE protobuf::libprotobuf)
