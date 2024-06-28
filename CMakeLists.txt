cmake_minimum_required(VERSION 2.8)

project(webserver)

include_directories(./ ./base)

aux_source_directory(./ MAIN_DIR)
aux_source_directory(./base BASE_DIR)

add_executable(main ${MAIN_DIR} ${BASE_DIR})

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")