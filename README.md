# up-zenoh-example-cpp
C++ Example application and service that utilizes up-client-zenoh-cpp

## Getting Started
### Requirements:
- Compiler: GCC/G++ 11 or Clang 13
- Ubuntu 22.04
- conan : 1.59 or latest 2.X

#### up-client-zenoh-cpp dependencies

1. install up-client-zenoh-cpp library https://github.com/eclipse-uprotocol/up-client-zenoh-cpp

## How to Build 
```
$ cd up-zenoh-example-cpp
$ mkdir build
$ cd build

$ conan install ..
$ cmake -S .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release 
$ cmake --build .
```
