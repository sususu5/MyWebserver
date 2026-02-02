FROM ubuntu:24.04

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
    libuv1-dev \
    libssl-dev \
    gdb \
    clangd \
    clang-format \
    libclang-dev \
    ninja-build \ 
    python3 \
    python3-pip \
    protobuf-compiler \
    python3-pyparsing \
    linux-tools-common \
    linux-tools-generic \
    && rm -rf /var/lib/apt/lists/*

RUN mv /usr/bin/perf /usr/bin/perf.wrapper || true && \
    ln -s $(find /usr/lib/linux-tools -name perf | head -n 1) /usr/bin/perf

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y --profile minimal --default-toolchain nightly

RUN pip3 install --break-system-packages protobuf==5.29.5

WORKDIR /opt
RUN git clone --filter=tree:0 https://github.com/microsoft/vcpkg.git
ENV VCPKG_FORCE_SYSTEM_BINARIES=1
RUN ./vcpkg/bootstrap-vcpkg.sh
ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH=$VCPKG_ROOT:$PATH

RUN git clone --depth 1 https://github.com/brendangregg/FlameGraph.git /opt/FlameGraph
ENV PATH="/opt/FlameGraph:${PATH}"

WORKDIR /app
COPY . .