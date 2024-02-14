//
// Created by Arvid Jonasson on 2024-02-14.
//

#ifndef ARENAALLOCATOR_H
#define ARENAALLOCATOR_H

#include <cstddef>
#include <memory>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE (sysconf(_SC_PAGESIZE))


class ArenaAllocator {
    std::unique_ptr<void, decltype([](void* ptr) {munmap(ptr, PAGE_SIZE);})> page{
        mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
    };
    char *next = static_cast<char*>(page.get());
public:
    auto allocate(std::size_t size, std::size_t alignment) -> void*;
    auto deallocate(void *ptr) noexcept -> void;
};



#endif //ARENAALLOCATOR_H
