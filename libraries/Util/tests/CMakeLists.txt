cmake_minimum_required(VERSION 3.5)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(SRC_PATH ../src/Util)
include_directories(${SRC_PATH})
add_executable(test_cycle_buffer CycleBuffer.cpp ${SRC_PATH}/Assertion.cpp)

enable_testing()
add_test(CycleBuffer test_cycle_buffer)
