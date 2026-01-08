FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    gdb \
    clangd \
    clang-format \
    ninja-build \ 
    python3 \
    python3-pyparsing \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN git clone --filter=tree:0 https://github.com/microsoft/vcpkg.git

ENV VCPKG_FORCE_SYSTEM_BINARIES=1

RUN ./vcpkg/bootstrap-vcpkg.sh

ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH=$VCPKG_ROOT:$PATH

WORKDIR /workspace

CMD ["sleep", "infinity"]