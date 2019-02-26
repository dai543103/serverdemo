cmake_minimum_required(VERSION 2.8)
project(Server)

## 编译选项
#####################################################################
add_definitions("-DASIO_STANDALONE")
if(WIN32)
    add_definitions("-D_WIN32_WINNT=0x0600")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267 /wd4290 /MP /MT")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /wd4996 /wd4355 /wd4244 /wd4267 /wd4290 /MP /MT")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267 /wd4290 /MP /W3 /MTd")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267 /wd4290 /MP /W3 /MTd")
else()
    set(CMAKE_CXX_FLAGS "-g -Wall -Wfatal-errors -Wextra -Wfloat-equal -Wno-unused -std=c++0x -fPIC -pg -static")
endif(WIN32)

## 依赖库定义
#####################################################################
#第三方库
set(THIRDPART_DIR ${CMAKE_SOURCE_DIR}/thirdpart)
#asio
include_directories(${THIRDPART_DIR}/asio)
#protobuf
include_directories(${THIRDPART_DIR}/protobuf/include)
link_directories(${THIRDPART_DIR}/protobuf/lib)
#mysql
include_directories(${THIRDPART_DIR}/mysql/include)
link_directories(${THIRDPART_DIR}/mysql/lib)
#spdlog
include_directories(${THIRDPART_DIR}/spdlog)
#mysql
link_directories(${THIRDPART_DIR}/zmq)

#####################################################################

set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
add_subdirectory(${CMAKE_SOURCE_DIR}/src)

