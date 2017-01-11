mkdir build
cd build
CC=clang CXX=clang++ cmake \
    -DCMAKE_POLICY_DEFAULT_CMP0056=NEW \
    -DLLVM_DIR=$(llvm-config --prefix)/share/llvm/cmake/ \
    -DCMAKE_BUILD_TYPE=DEBUG \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_FLAGS="-stdlib=libc++" \
    -DCMAKE_INSTALL_RPATH="$(llvm-config --libdir)" \
    -DCMAKE_INSTALL_PREFIX='../install' ..
make -j4 install
