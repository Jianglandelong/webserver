cmake_minimum_required(VERSION 2.8)

project(webserver)

include_directories(./ ./base)

aux_source_directory(./ ROOT_DIR)
aux_source_directory(./base BASE_DIR)
aux_source_directory(./test TEST_DIR)

# 为 test 目录下每个 test 文件创建一个可执行文件
foreach(TEST ${TEST_DIR})
    string(REPLACE "./test/" "" TEST_NAME ${TEST})
    string(REPLACE ".cpp" "" TEST_NAME ${TEST_NAME})
    add_executable(${TEST_NAME} ${TEST} ${ROOT_DIR} ${BASE_DIR})
endforeach()

# add_executable(main ${ROOT_DIR} ${BASE_DIR})
# add_executable(test4 ${MAIN_DIR} ${BASE_DIR})

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")