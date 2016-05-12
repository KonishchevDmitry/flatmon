cmake_minimum_required(VERSION 3.5)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(LIBRARY_PATH ../..)
include_directories(${LIBRARY_PATH}/Util/src)

file(GLOB UTIL_SRCS ${LIBRARY_PATH}/Util/src/Util/*.cpp)
add_library(util STATIC ${UTIL_SRCS} Mocks.cpp)

enable_testing()

foreach(module CycleBuffer MonotonicTime)
    add_executable(test_${module} ${module}.cpp)
    target_link_libraries(test_${module} util)
    add_test(${module} test_${module})
endforeach(module)
