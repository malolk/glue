Glue
=====

A c++ network library for personal use.

## purpose

The project Glue is built for three reasons: 
1. Customizing my personal network library when building other network-related projects
2. Sharpening my programming skills both in network programming and C++ programming
3. Sharing the contributions that I made

## programming model

* non-blocking I/O
* one epoll per thread
* thread-pool

## environment required

* Linux(Version 2.6.8+)
* cmake(Version 2.6+)
* g++(Version 4.8.1+) support C++11 

## compile and run

- specify install directory for header and library

```
INSTALL_DIR=<the header and library install path that you specify> ./build.sh
```
Headers and static library will be installed into **release-install** directory in default while you didn't specify any install directory.

- specify build type(debug or release)

```
BUILD_TYPE=<debug or release> ./build.sh
```
Build type is release when not specified

- executable files
The executable files of tests for the library could be found in directory build/release(debug)/bin

