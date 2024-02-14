//
// Created by Arvid Jonasson on 2024-02-09.
//

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <vector>
#include <cerrno>
#include <type_traits>
#include <functional>
#include <map>
#include <unordered_map>

class Allocator {
public:
    auto allocate(std::size_t size, std::size_t alignment) -> void*;
    auto deallocate(void *ptr) noexcept -> void;
private:
    using PageSize = std::invoke_result_t<decltype(sysconf), int>;

    struct InternalTypes {
        struct MallocPageAllocator {
            static auto operator()() -> void*;
        };
        struct FreePageDeallocator {
            static auto operator()(void *ptr) noexcept -> void;
        };
        struct MmapPageAllocator {
            static auto operator()() -> void*;
        };
        struct MunmapPageDeallocator {
            static auto operator()(void *ptr) noexcept -> void;
        };

        struct PtrHash {
            using is_transparent = void;

            auto operator()(const void *ptr) const noexcept -> std::size_t {
                return std::hash<const void*>{}(ptr);
            }
            template<typename T, typename Deleter>
            auto operator()(const std::unique_ptr<T, Deleter> &ptr) const noexcept -> std::size_t {
                return operator()(ptr.get());
            }
        };

        struct PtrEqual {
            using is_transparent = void;

            template<typename T, typename Deleter>
            static auto toPtr(const std::unique_ptr<T, Deleter> &ptr) noexcept -> const auto* {
                return ptr.get();
            }

            static auto toPtr(const void *ptr) noexcept -> const void* {
                return ptr;
            }
            template<typename T, typename U>
            auto operator()(const T &lhs, const U &rhs) const noexcept -> bool {
                return toPtr(lhs) == toPtr(rhs);
            }
        };
    };

    // Typedef the allocator and deallocator in use
    using PageAllocator = InternalTypes::MmapPageAllocator;
    using PageDeallocator = InternalTypes::MunmapPageDeallocator;

    // Create the allocator and deallocator
    static constexpr auto pageAllocator = PageAllocator{};
    static constexpr auto pageDeallocator = PageDeallocator{};

    struct Page {
        std::map<void*, std::size_t> allocatedMemory;
        std::unique_ptr<void, PageDeallocator> page;
    };

    using PageType = std::unique_ptr<void, PageDeallocator>;
    using FreeList = std::map<void*, std::size_t>;
    using AllocatedPages = std::unordered_map<PageType, FreeList, InternalTypes::PtrHash, InternalTypes::PtrEqual>;

    auto allocateNewPage() -> AllocatedPages::iterator;
    static auto tryAllocateFromPage(FreeList &freeList, std::size_t size, std::size_t alignment) -> void*;

    //static_assert(sizeof(PageType) == sizeof(void*), "pageType must be the same size as a pointer");

    const static PageSize pageSize;
    AllocatedPages allocatedPages;
    std::unordered_map<void*, std::size_t> allocatedMemory;
};


#endif //ALLOCATOR_H
