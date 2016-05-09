cmake_minimum_required(VERSION 3.5)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(..)
add_executable(test_cycle_buffer CycleBuffer.cpp)

enable_testing()
add_test(CycleBuffer test_cycle_buffer)
