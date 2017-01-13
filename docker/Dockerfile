FROM ubuntu:14.04
RUN apt-get -y update

# Install gcc
# * Install add-apt-repository
RUN apt-get -y install software-properties-common
# * Install packages
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt-get -y update
# RUN apt-get -y install gcc-4.8 g++-4.8
# RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8
RUN apt-get -y install gcc-4.9 g++-4.9
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.9

RUN apt-get -y install git

# Checkout LLVM 3.7.1
RUN cd /opt && git clone https://github.com/llvm-mirror/llvm -b release_37
RUN cd /opt/llvm/tools && git clone https://github.com/llvm-mirror/clang -b release_37
RUN cd /opt/llvm/projects && git clone https://github.com/llvm-mirror/compiler-rt -b release_37
RUN cd /opt/llvm/projects && git clone https://github.com/llvm-mirror/libcxx -b release_37
RUN cd /opt/llvm/projects && git clone https://github.com/llvm-mirror/libcxxabi -b release_37

RUN apt-get install -y cmake
RUN apt-get install -y python

# Build LLVM 3.7.1 + libc++
RUN mkdir /opt/llvm-build-3.7
RUN cd /opt/llvm-build-3.7 && cmake -DCMAKE_INSTALL_PREFIX=/opt/llvm-install-3.7 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=On -DLLVM_ENABLE_ASSERTIONS=Off -DLLVM_ENABLE_SPHINX=Off -DLLVM_TARGETS_TO_BUILD="X86" /opt/llvm && make -j4 install
RUN rm -rf /opt/llvm-build-3.7

# Build LLVM 3.7.1 + libc++ using previous build of LLVM 3.7.1 + libc++
RUN mkdir /opt/llvm-build-3.7-self-hosting
RUN cd /opt/llvm-build-3.7-self-hosting && PATH=$PATH:/opt/llvm-install-3.7/bin LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/llvm-install-3.7/lib CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-stdlib=libc++" -DCMAKE_EXE_LINKER_FLAGS="-lc++abi" -DCMAKE_SHARED_LINKER_FLAGS="-lc++abi" -DBUILD_SHARED_LIBS=On -DLLVM_ENABLE_ASSERTIONS=Off -DLLVM_ENABLE_SPHINX=Off -DLLVM_TARGETS_TO_BUILD="X86" -DCMAKE_INSTALL_PREFIX=/opt/llvm-install-3.7-self-hosting /opt/llvm && make -j4 install
RUN rm -rf /opt/llvm-build-3.7-self-hosting

# Build irreducible-cfg
RUN cd /opt && git clone --recursive https://github.com/smanilov/irreducible-cfg
RUN cd /opt/irreducible-cfg && PATH=$PATH:/opt/llvm-install-3.7-self-hosting/bin LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/llvm-install-3.7-self-hosting/lib ./tools/build.sh
# RUN cd /opt/irreducible-cfg && PATH=$PATH:/opt/llvm-install-3.7-self-hosting/bin LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/llvm-install-3.7-self-hosting/lib ./tools/run.sh ./examples/specimen3/main.cpp
# RUN cd /opt/irreducible-cfg && PATH=$PATH:/opt/llvm-install-3.7-self-hosting/bin LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/llvm-install-3.7-self-hosting/lib ./tools/run.sh ./examples/specimen4/main.cpp
