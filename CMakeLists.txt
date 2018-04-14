cmake_minimum_required(VERSION 2.8)

project(BEV)

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(src)
