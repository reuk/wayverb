#pragma once

#include <memory>

namespace aligned {

template <typename T, std::size_t N = alignof(T)>
class allocator {
    //  helper type with the alignment we want
    using aligned_type = std::aligned_storage_t<sizeof(T), N>;

public:
    //  type to allocate
    using value_type = T;

    //  characteristics
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;

    //  ensure basic special class methods are enabled
    allocator() noexcept                            = default;
    allocator(const allocator&) noexcept            = default;
    allocator& operator=(const allocator&) noexcept = default;
    allocator(allocator&&) noexcept                 = default;
    allocator& operator=(allocator&&) noexcept      = default;

    //  allow construction from an allocator for another type
    //  (with the same alignment)
    template <typename U>
    allocator(const allocator<U>&) noexcept {}

    //  implementations may derive from the allocator polymorphically
    virtual ~allocator() noexcept = default;

    //  rebinding the allocator for another type should respect the alignment
    //  of that other type
    template <typename U>
    struct rebind {
        using other = allocator<U>;
    };

    //  allocate a type that we know will have the correct alignment
    //  then reinterpret the pointer
    T* allocate(std::size_t n) {
        return reinterpret_cast<T*>(new aligned_type[n]);
    }

    //  deallocate previously allocated memory
    void deallocate(T* p, std::size_t) noexcept {
        delete[] reinterpret_cast<aligned_type*>(p);
    }
};

template <typename T, typename U, std::size_t N>
constexpr bool operator==(const allocator<T, N>&,
                          const allocator<U, N>&) noexcept {
    return true;
}

template <typename T, typename U, std::size_t N>
constexpr bool operator!=(const allocator<T, N>&,
                          const allocator<U, N>&) noexcept {
    return false;
}

}  //  namespace aligned
