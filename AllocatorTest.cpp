//
// Created by Arvid Jonasson on 2024-02-13.
//

#define BOOST_TEST_MODULE AllocatorTest
#include <boost/test/included/unit_test.hpp>
#include <iostream>


#include "Allocator.h"

BOOST_AUTO_TEST_SUITE(AllocatorTests)

BOOST_AUTO_TEST_CASE(AllocateDeallocateTest)
{
    Allocator allocator;

    // Test allocation and deallocation of an array of integers
    size_t arraySize = 10;
    int* arrayPtr = static_cast<int*>(allocator.allocate(sizeof(int) * arraySize, alignof(int)));
    BOOST_TEST(arrayPtr != nullptr); // Check allocation was successful
    for (size_t i = 0; i < arraySize; ++i) {
        arrayPtr[i] = static_cast<int>(i);
    }
    // Verify each element in the array
    for (size_t i = 0; i < arraySize; ++i) {
        BOOST_TEST(arrayPtr[i] == static_cast<int>(i)); // Check if the write was successful
    }
    allocator.deallocate(arrayPtr);
}

BOOST_AUTO_TEST_CASE(AlignmentTest)
{
    Allocator allocator;

    // Test allocation with different alignments
    for (size_t align = 1; align <= 128; align *= 2) {
        void* ptr = allocator.allocate(4, align);
        BOOST_TEST(ptr != nullptr); // Check allocation was successful
        BOOST_TEST(reinterpret_cast<uintptr_t>(ptr) % align == 0); // Check alignment
        std::cout << "Allocation with alignment " << align << " has address " << ptr << std::endl;
        allocator.deallocate(ptr);
    }
}

BOOST_AUTO_TEST_CASE(SimpleAllocateDeallocateTest)
{
    Allocator allocator;
    // Test allocation and deallocation of a single integer
    int* intPtr = static_cast<int*>(allocator.allocate(sizeof(int), alignof(int)));
    BOOST_TEST(intPtr != nullptr); // Check allocation was successful
    *intPtr = 42; // Test writing to allocated memory
    BOOST_TEST(*intPtr == 42); // Check if the write was successful
    std::cout << "Allocated int value: " << *intPtr << std::endl;
    allocator.deallocate(intPtr);
}


BOOST_AUTO_TEST_SUITE_END()
