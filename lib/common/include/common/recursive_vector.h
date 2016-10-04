#pragma once

#include <algorithm>
#include <memory>

namespace detail {

/// Handles memory management for vector-like classes.
/// Has a fixed capacity, and allows objects to be constructed or destroyed at
/// the end of the internal array.
/// For a true dynamic array, this must be wrapped in another class that
/// handles copy/move when a size greater than the current capacity is
/// requested.
template <typename T>
class recursive_vector_impl final {
public:
    using value_type = T;
    using allocator_type = std::allocator<T>;
    using size_type = std::size_t;

    using allocator_traits = std::allocator_traits<std::allocator<T>>;

    using pointer = typename allocator_traits::pointer;
    using const_pointer = typename allocator_traits::const_pointer;

    using iterator = pointer;
    using const_iterator = const_pointer;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit recursive_vector_impl(size_type size = 0)
            : capacity_{size}
            , ptr_{size ? allocator_traits::allocate(alloc_, size) : nullptr} {}

    void swap(recursive_vector_impl& other) noexcept {
        using std::swap;
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
        swap(alloc_, other.alloc_);
        swap(ptr_, other.ptr_);
    }

    recursive_vector_impl(const recursive_vector_impl& other)
            : capacity_{other.size_}
            , ptr_{other.ptr_ ? allocator_traits::allocate(alloc_, other.size_)
                              : nullptr} {
        for (auto i{other.begin()}, end{other.end()}; i != end; ++i) {
            construct(*i);
        }
    }

    recursive_vector_impl(recursive_vector_impl&& other) noexcept {
        swap(other);
    }

    recursive_vector_impl& operator=(const recursive_vector_impl& other) {
        auto copy{other};
        swap(copy);
        return *this;
    }
    recursive_vector_impl& operator=(recursive_vector_impl&& other) noexcept {
        swap(other);
        return *this;
    }

    ~recursive_vector_impl() noexcept {
        while (size_) {
            destroy();
        }
        allocator_traits::deallocate(alloc_, ptr_, capacity_);
    }

    template <typename... Ts>
    void construct(Ts&&... ts) {
        allocator_traits::construct(
                alloc_, ptr_ + size_, std::forward<Ts>(ts)...);
        size_ += 1;
    }

    void destroy() noexcept {
        size_ -= 1;
        allocator_traits::destroy(alloc_, ptr_ + size_);
    }

    size_type size() const { return size_; }
    size_type capacity() const { return capacity_; }

    iterator begin() { return ptr_; }
    const_iterator begin() const { return ptr_; }
    const_iterator cbegin() const { return ptr_; }

    iterator end() { return ptr_ + size_; }
    const_iterator end() const { return ptr_ + size_; }
    const_iterator cend() const { return ptr_ + size_; }

private:
    size_type size_{0};
    size_type capacity_{0};
    allocator_type alloc_{};
    T* ptr_{nullptr};
};

template <typename T>
void swap(recursive_vector_impl<T>& a, recursive_vector_impl<T>& b) noexcept {
    a.swap(b);
}

}  // namespace detail

template <typename T>
class recursive_vector final {
public:
    using impl_type = detail::recursive_vector_impl<T>;
    using value_type = typename impl_type::value_type;
    using allocator_type = typename impl_type::allocator_type;
    using size_type = typename impl_type::size_type;
    using difference_type = std::ptrdiff_t;

    using reference = value_type&;
    using const_reference = const value_type&;

    using pointer = typename impl_type::pointer;
    using const_pointer = typename impl_type::const_pointer;

    using iterator = pointer;
    using const_iterator = const_pointer;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    void swap(recursive_vector& other) noexcept {
        using std::swap;
        swap(impl_, other.impl_);
    }

    template <typename It>
    iterator insert(const_iterator pos, It first, It last) {
        const auto n{std::distance(first, last)};
        const auto c{impl_.capacity()};
        const auto new_size{impl_.size() + n};
        const auto old_pos{std::distance(impl_.cbegin(), pos)};
        //  If there's already room for the new elements.
        if (new_size <= c) {
            //  Default-construct some new elements at the end.
            const auto old_end{impl_.end()};
            while (impl_.size() < new_size) {
                impl_.construct();
            }
            //  Move items back.
            const auto r{impl_.begin() + old_pos};
            std::move_backward(r, old_end, impl_.begin() + new_size);
            std::copy(first, last, r);
            return r;
        }
        //  There wasn't room for the new elements, so we construct a new
        //  storage area, and move each element across.
        //  This line might throw, which is fine, because the object is still
        //  in a valid state.
        impl_type v{std::max(impl_.size() + n, impl_.capacity() * 2 + 1)};
        //  Function is nothrow from here on.
        for (auto i{impl_.begin()}; i != pos; ++i) {
            v.construct(std::move(*i));
        }
        for (auto i{first}; i != last; ++i) {
            v.construct(*i);
        }
        for (auto i{pos}; i != impl_.end(); ++i) {
            v.construct(std::move(*i));
        }
        //  Take ownership of the copy;
        impl_.swap(v);
        return impl_.begin() + old_pos;
    }

    iterator erase(const_iterator begin, const_iterator end) {
        //  Move elements down the range one-by-one.
        const auto p{impl_.begin() + std::distance(impl_.cbegin(), begin)};
        std::move(impl_.begin() + std::distance(impl_.cbegin(), end),
                  impl_.end(),
                  p);
        //  Destroy the moved-from elements.
        const auto num{std::distance(begin, end)};
        for (auto i{0u}; i != num; ++i) {
            impl_.destroy();
        }
        return p;
    }

    void reserve(size_type new_cap) {
        if (new_cap > impl_.capacity()) {
            //  We have to reallocate.
            //  Create a new impl to store the data.
            impl_type v{std::max(new_cap, impl_.capacity() * 2 + 1)};
            //  Copy/move elements across one-by-one.
            for (auto i{impl_.begin()}, end{impl_.end()}; i != end; ++i) {
                v.construct(std::move(*i));
            }
            //  Swap internals.
            impl_.swap(v);
        }
    }

    pointer data() { return impl_.begin(); }
    const_pointer data() const { return impl_.cbegin(); }

    size_type size() const { return impl_.size(); }
    size_type capacity() const { return impl_.capacity(); }

    iterator begin() { return impl_.begin(); }
    const_iterator begin() const { return impl_.begin(); }
    const_iterator cbegin() const { return impl_.begin(); }

    iterator end() { return impl_.end(); }
    const_iterator end() const { return impl_.end(); }
    const_iterator cend() const { return impl_.end(); }

private:
    impl_type impl_{};
};

template <class T, class Comp = std::less<T>>
class recursive_vector_backed_set final {
public:
    recursive_vector_backed_set() = default;
    explicit recursive_vector_backed_set(const Comp& comp)
            : comp_{comp} {}
    explicit recursive_vector_backed_set(Comp&& comp)
            : comp_{std::move(comp)} {}

    auto begin() { return data_.begin(); }
    const auto begin() const { return data_.begin(); }
    const auto cbegin() const { return data_.cbegin(); }

    auto end() { return data_.end(); }
    const auto end() const { return data_.end(); }
    const auto cend() const { return data_.cend(); }

    auto find(const T& t) const {
        const auto it{std::lower_bound(data_.begin(), data_.end(), t, comp_)};
        if (it != data_.end() && values_equal(t, *it)) {
            return it;
        }
        return data_.end();
    }

    auto insert(T t) {
        const auto it{std::lower_bound(data_.begin(), data_.end(), t, comp_)};
        if (it != data_.end() && values_equal(t, *it)) {
            return std::make_pair(it, false);
        }
        return std::make_pair(data_.insert(it, &t, &t + 1), true);
    }

    constexpr auto key_comp() const { return comp_; }
    constexpr auto value_comp() const { return comp_; }

    auto size() const { return data_.size(); }

private:
    constexpr bool values_not_equal(const T& a, const T& b) const {
        return comp_(a, b) || comp_(b, a);
    }
    constexpr bool values_equal(const T& a, const T& b) const {
        return !values_not_equal(a, b);
    }

    Comp comp_;
    recursive_vector<T> data_;
};
