cmake_minimum_required(VERSION 3.16.3)

project(grpc_client_example)

# Proto file
get_filename_component(hw_proto "./proto/helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.h")
set(_PROTOBUF_PROTOC "protoc")
set(_GRPC_CPP_PLUGIN_EXECUTABLE "/usr/bin/grpc_cpp_plugin")

add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")


add_executable(grpc_client 
                "src/main.cpp"
                "src/Client.cpp"
                "src/HandlerInterface.cpp"
                ${hw_proto_srcs}
                ${hw_grpc_srcs})

target_link_libraries(grpc_client
                          -lpthread
                          -lgrpc++
                          -lprotobuf)
