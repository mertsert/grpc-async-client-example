FROM ubuntu:21.04 as build-kit

RUN apt update -y
RUN apt install -y build-essential && apt install -y cmake
## Grpc related packages.
RUN apt update -y && apt install -y libgrpc++-dev && apt install -y libgrpc-dev && apt install -y libprotobuf-dev && apt install -y protobuf-compiler && apt install -y protobuf-compiler-grpc