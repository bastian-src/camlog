# Credits:
# - [dockcross musl image](https://github.com/dockcross/dockcross/tree/master/linux-arm64-musl)
# - [Vladimir Semyonov](https://tempname11.github.io/posts/cross-compliing-for-arm64#linux)

FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

# Update and install build-tools from official sources
RUN apt-get update && \
    apt-get install -y \
    ca-certificates \
    cmake git curl bzip2 make pkg-config libtool \
    build-essential \
    autogen autopoint automake

RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set Linux kernel cross compilation variables and install musl-based toolchain
ENV XCC_PREFIX=/usr/xcc
ENV CROSS_TRIPLE=aarch64-linux-musl
ENV CROSS_ROOT=${XCC_PREFIX}/${CROSS_TRIPLE}-cross

RUN mkdir -p ${XCC_PREFIX}
RUN curl --max-time 30 --retry 5 -LO http://musl.cc/${CROSS_TRIPLE}-cross.tgz
RUN tar -C ${XCC_PREFIX} -xvf ${CROSS_TRIPLE}-cross.tgz

ENV AS=${CROSS_ROOT}/bin/${CROSS_TRIPLE}-as \
    AR=${CROSS_ROOT}/bin/${CROSS_TRIPLE}-ar \
    CC=${CROSS_ROOT}/bin/${CROSS_TRIPLE}-gcc \
    CPP=${CROSS_ROOT}/bin/${CROSS_TRIPLE}-cpp \
    CXX=${CROSS_ROOT}/bin/${CROSS_TRIPLE}-g++ \
    LD=${CROSS_ROOT}/bin/${CROSS_TRIPLE}-ld \
    FC=${CROSS_ROOT}/bin/${CROSS_TRIPLE}-gfortran

ENV PATH=${PATH}:${CROSS_ROOT}/bin
ENV CROSS_COMPILE=${CROSS_TRIPLE}-
ENV ARCH=arm64

WORKDIR /src
