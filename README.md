# up-zenoh-example-cpp
C++ Example application and service that utilizes up-client-zenoh-cpp

## Getting Started
### Requirements:
- Compiler: GCC/G++ 11 or Clang 13
- Ubuntu 22.04
- conan : 1.59 or latest 2.X

#### up-client-zenoh-cpp dependencies

Use the up-conan-recipes repo and compile the following recipes:

$ conan create --version 1.6.0 --build=missing up-core-api/developer
$ conan create --version 1.0.1-rc1 --build=missing up-cpp/developer
$ conan create --version 1.0.0-dev --build=missing up-transport-socket-cpp/developer

## How to Build 
```
$ cd up-zenoh-example-cpp
$ conan install --build=missing .
$ cmake --preset conan-release
$ cd build/Release
$ cmake --build . -- -j
```

## Debug Build
```
$ cd up-zenoh-example-cpp
$ conan install --build=missing --settings=build_type=Debug .
$ cmake --preset conan-debug
$ cd build/Debug
$ cmake --build . -- -j
```
