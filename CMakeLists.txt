cmake_minimum_required(VERSION 3.28)
project(allocator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(AllocatorTest
        AllocatorTest.cpp
        Allocator.cpp
        Allocator.h)

target_link_libraries(AllocatorTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

include(CTest)
enable_testing()
add_test(NAME AllocatorTest COMMAND AllocatorTest)


add_executable(allocator main.cpp
        Allocator.cpp
        Allocator.h)
