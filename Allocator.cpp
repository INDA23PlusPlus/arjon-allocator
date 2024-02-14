//
// Created by Arvid Jonasson on 2024-02-09.
//

#include "Allocator.h"
#include <__ranges/all.h>
#include <cmath>
#include <cstddef>
#include <sys/mman.h>


decltype(Allocator::pageSize) Allocator::pageSize = sysconf(_SC_PAGESIZE);


auto Allocator::allocate(const std::size_t size, const std::size_t alignment) -> void* {
    if(size == 0) {
        return nullptr;
    }
    if(size > pageSize) {
        std::cerr << "Size is larger than the page size" << std::endl;
        throw std::bad_alloc();
    }


    for(auto &[page, freeList] : allocatedPages) {
        auto *const ptr = tryAllocateFromPage(freeList, size, alignment);
        if(ptr != nullptr) {
            return ptr;
        }
    }
    auto pageIterator = allocateNewPage();
    if(pageIterator->first == nullptr) {
        std::cerr << "Failed to allocate new page: " << strerror(errno) << std::endl;
        throw std::bad_alloc();
    }
    auto *const ptr = tryAllocateFromPage(pageIterator->second, size, alignment);
    if(ptr == nullptr) {
        std::cerr << "Failed to allocate memory" << std::endl;
        throw std::bad_alloc();
    }
    allocatedMemory.emplace(ptr, size);
    return ptr;
}

auto Allocator::deallocate(void *ptr) noexcept -> void {
    // Find the page that the pointer is on
    const auto uintPtr = reinterpret_cast<std::uintptr_t>(ptr);
    const auto pagePtr = uintPtr - (uintPtr % pageSize);
    const void *voidPagePtr = reinterpret_cast<void*>(pagePtr);
    const auto page = allocatedPages.find(voidPagePtr);

    if(page == allocatedPages.end()) {
        std::cerr << "Failed to deallocate memory" << std::endl;
        std::terminate();
    }

    auto &freeList = page->second;
    auto [freeListIt, freeListEmplaced] = freeList.emplace(ptr, allocatedMemory[ptr]);
    allocatedMemory.erase(ptr);

    if(!freeListEmplaced) {
        std::cerr << "Failed to deallocate memory" << std::endl;
        std::terminate();
    }

}

auto Allocator::InternalTypes::MallocPageAllocator::operator()() -> void* {
    void *const ptr = malloc(pageSize);

    if (ptr == nullptr) {
        std::cerr << "malloc failed: " << strerror(errno) << std::endl;
        throw std::bad_alloc();
    }

    return ptr;
}

auto Allocator::InternalTypes::FreePageDeallocator::operator()(void *const ptr) noexcept -> void {
    free(ptr);
}

auto Allocator::InternalTypes::MmapPageAllocator::operator()() -> void * {
    void *const ptr = mmap(nullptr, pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        const auto errstr = strerror(errno);
        std::cerr << "mmap failed: " << errstr << std::endl;
        throw std::bad_alloc();
    }
    return ptr;
}

auto Allocator::InternalTypes::MunmapPageDeallocator::operator()(void *const ptr) noexcept -> void {
    if (munmap(ptr, pageSize) == -1) {
        std::cerr << "munmap failed: " << strerror(errno) << std::endl;

        // Call std::terminate() since it's unlikely that the program can recover from this error.
        std::terminate();
    }
}
auto Allocator::allocateNewPage() -> AllocatedPages::iterator {
    auto [pageIterator, emplacedPage] = allocatedPages.try_emplace(std::unique_ptr<void, PageDeallocator>{pageAllocator()}, FreeList{});

    if (!emplacedPage) {
        std::cerr << "Failed to allocate a new page" << std::endl;
        throw std::bad_alloc();
    }

    auto &page = pageIterator->first;
    auto &freeList = pageIterator->second;

    auto const [freeListIterator, emplacedFreeList] = freeList.emplace(page.get(), pageSize);

    if (!emplacedFreeList) {
        std::cerr << "Failed to add the new page to the free list" << std::endl;
        throw std::bad_alloc();
    }

    return pageIterator;
}
auto Allocator::tryAllocateFromPage(FreeList &freeList, const std::size_t size, const std::size_t alignment)
        -> void * {
    for(auto &[availablePtr, availableSpace] : freeList) {
        void* mutPtr = availablePtr;
        auto mutSpace = availableSpace;
        auto *const alignedPtr = std::align(alignment, size, mutPtr, mutSpace);

        if(alignedPtr != nullptr) {
            // If the aligned pointer is not the same as the available pointer,
            // then there is space before the aligned pointer
            if(alignedPtr != availablePtr) {
                availableSpace = reinterpret_cast<std::uintptr_t>(alignedPtr) - reinterpret_cast<std::uintptr_t>(availablePtr);
            } else {
                freeList.erase(availablePtr);
            }

            // If there is space after the aligned pointer, then add it to the free list
            if(reinterpret_cast<std::uintptr_t>(alignedPtr) + size != reinterpret_cast<std::uintptr_t>(mutPtr) + mutSpace) {
                freeList[reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(alignedPtr) + size)]
                = reinterpret_cast<std::uintptr_t>(mutPtr) + mutSpace - (reinterpret_cast<std::uintptr_t>(alignedPtr) + size);
            }

            return alignedPtr;
        }
    }
    return nullptr;
}


