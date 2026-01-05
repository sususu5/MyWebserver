find_package(protobuf CONFIG REQUIRED)

# Define output directory for generated files
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Find all .proto files
file(GLOB PROTO_FILES "*.proto")

set(PROTO_SRCS)
set(PROTO_HDRS)

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(PROTO_SRC "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.h")
    
    list(APPEND PROTO_SRCS ${PROTO_SRC})
    list(APPEND PROTO_HDRS ${PROTO_HDR})

    add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR}
        COMMAND protobuf::protoc
        ARGS --cpp_out=${PROTO_OUT_DIR} -I${CMAKE_CURRENT_SOURCE_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating C++ code from ${PROTO_NAME}.proto"
    )
endforeach()

add_library(proto_generated STATIC ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(proto_generated PUBLIC protobuf::libprotobuf)
target_include_directories(proto_generated PUBLIC ${PROTO_OUT_DIR})
