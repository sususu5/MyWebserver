find_package(protobuf CONFIG REQUIRED)

# Define output directory for generated files
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Explicit proto list to avoid stale deleted files
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/common.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/friend_service.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/message_service.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol.proto
)

set(PROTO_SRCS)
set(PROTO_HDRS)
set(PROTO_PYS)
set(PROTO_GOS)

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(PROTO_SRC "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.h")
    set(PROTO_PY "${PROTO_OUT_DIR}/${PROTO_NAME}_pb2.py")
    set(PROTO_GO "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.go")
    
    list(APPEND PROTO_SRCS ${PROTO_SRC})
    list(APPEND PROTO_HDRS ${PROTO_HDR})
    list(APPEND PROTO_PYS ${PROTO_PY})
    list(APPEND PROTO_GOS ${PROTO_GO})

    add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR} ${PROTO_PY} ${PROTO_GO}
        COMMAND protobuf::protoc
        ARGS --cpp_out=${PROTO_OUT_DIR}
            --python_out=${PROTO_OUT_DIR}
            --go_out=${PROTO_OUT_DIR} --go_opt=paths=source_relative
            -I${CMAKE_CURRENT_SOURCE_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating C++, Python and Go code from ${PROTO_NAME}.proto"
    )
endforeach()

add_custom_target(proto_python_generated ALL DEPENDS ${PROTO_PYS})
add_custom_target(proto_go_generated ALL DEPENDS ${PROTO_GOS})

add_library(proto_generated STATIC ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(proto_generated PUBLIC protobuf::libprotobuf)
target_include_directories(proto_generated PUBLIC ${PROTO_OUT_DIR})
