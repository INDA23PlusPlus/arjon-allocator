//
// Created by Arvid Jonasson on 2024-02-14.
//

#include "ArenaAllocator.h"
auto ArenaAllocator::allocate(std::size_t size, std::size_t alignment) -> void * {
    auto space = reinterpret_cast<std::uintptr_t>(page.get()) + PAGE_SIZE - reinterpret_cast<std::uintptr_t>(next);
    auto res = std::align(alignment, size, *reinterpret_cast<void**>(&next), space);
    if(res == nullptr) {
        throw std::bad_alloc();
    }
    next = static_cast<char*>(res) + size;
    return res;
}
auto ArenaAllocator::deallocate(void *ptr) noexcept -> void {}
