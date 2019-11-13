cmake_minimum_required(VERSION 3.15)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_MODULE_PATH "/home/jimmy/CLionProjects/CS3518-ComputerNetworks/Project2/")

include(FindPCAP.cmake)

add_executable(untitled main.cpp)

target_link_libraries(untitled ${PCAP_LIBRARY})