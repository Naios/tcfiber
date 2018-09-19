FROM ubuntu:18.04

RUN apt-get update \
 && apt-get install -y \
    ca-certificates \
    build-essential \
    git \
    clang \
    ninja-build \
    wget \
 && git config --global submodule.fetchJobs 40

RUN update-alternatives --install /usr/bin/cc cc $(which clang) 100 \
 && update-alternatives --install /usr/bin/c++ c++ $(which clang++) 100

# Install boost 1.67 from source
RUN mkdir -p /opt/boost \
 && cd /opt/boost \
 && wget -O boost-dist.tar.gz https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz \
 && tar xzf boost-dist.tar.gz \
 && cd boost_* \
 && ./bootstrap.sh --with-libraries=context,system,thread,chrono,date_time,atomic,filesystem,fiber  --prefix=/usr/local \
 && ./b2 toolset=clang -j $(nproc) install \
 && rm -rf /opt/boost

# Install CMake 3.12 from a distribution
RUN mkdir -p /opt/cmake-inst \
 && cd /opt/cmake-inst \
 && wget -O dist.tar.gz https://cmake.org/files/v3.12/cmake-3.12.0-rc2-Linux-x86_64.tar.gz \
 && tar xzf dist.tar.gz \
 && mv cmake-* /opt/cmake \
 && rm -rf /opt/cmake-inst
ENV PATH="/opt/cmake/bin:${PATH}"

CMD mkdir -p /home/build \
 && cd /home/build \
 && cmake -GNinja -DCMAKE_BUILD_TYPE=Debug /home/src \
 && ninja \
 && ctest --verbose
