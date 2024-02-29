# up-zenoh-example-cpp
C++ Example application and service that utilizes up-client-zenoh-cpp

## How to Build 
```
$ cd up-zenoh-example-cpp
$ mkdir build
$ cd build

$ conan install ..
$ cmake -S .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release 
$ cmake --build .
```
