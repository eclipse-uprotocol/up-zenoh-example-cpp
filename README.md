# up-zenoh-example-cpp
C++ Example application and service that utilizes up-transport-zenoh-cpp

## Getting Started
### Requirements:
- Compiler: GCC/G++ 11 or Clang 13
- Ubuntu 22.04
- conan : 1.59 or latest 2.X

#### up-client-zenoh-cpp dependencies

Use the up-conan-recipes repo and compile the following recipes:
```
$ conan create --version 1.6.0 --build=missing up-core-api/developer
$ conan create --version 1.0.1-rc1 --build=missing up-cpp/developer
$ conan create --version 1.0.0-dev --build=missing up-transport-zenoh-cpp/developer
```

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

## Usage
After completing the project build, the build artifacts can generally be found in the `build/Release/bin` directory within your workspace.
You can run the example using the supplied configuration file located in the `/resources` directory.For instance, to run the RPC example, use the following commands:

```bash
$ ./rpc_server <path/to/config>
$ ./rpc_client <path/to/config>
```