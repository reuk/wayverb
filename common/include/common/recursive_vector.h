#pragma once

#include <algorithm>
#include <memory>

namespace detail {

/// Does *NOT* automatically construct objects.
/// *DOES* automatically destruct up to 'size_' elements when destructed.
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
            : size_{size}
            , capacity_{size}
            , ptr_{size ? allocator_traits::allocate(alloc_, size) : nullptr} {}

    recursive_vector_impl(const recursive_vector_impl&) = delete;
    recursive_vector_impl(recursive_vector_impl&&) = delete;

    recursive_vector_impl& operator=(const recursive_vector_impl&) = delete;
    recursive_vector_impl& operator=(recursive_vector_impl&&) = delete;

    void swap(recursive_vector_impl& other) noexcept {
        using std::swap;
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
        swap(alloc_, other.alloc_);
        swap(ptr_, other.ptr_);
    }

    ~recursive_vector_impl() noexcept {
        destroy(ptr_, ptr_ + size_);
        allocator_traits::deallocate(alloc_, ptr_, size_);
    }

    template <typename... Ts>
    void construct(Ts&&... ts) {
        allocator_traits::construct(
                alloc_, ptr_ + size_, std::forward<Ts>(ts)...);
        size_ += 1;
    }

    template <typename Ptr>
    void destroy(Ptr p) noexcept {
        allocator_traits::destroy(alloc_, p);
    }

    template <typename It>
    void destroy(It begin, It end) noexcept {
        for (; begin != end; ++begin) {
            destroy(begin);
        }
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

    recursive_vector() = default;

    void swap(recursive_vector& other) noexcept {
        using std::swap;
        swap(impl_, other.impl_);
    }

    recursive_vector(const recursive_vector& other)
            : impl_{other.size()} {
        for (auto i{other.cbegin()}, end{other.cend()}; i != end; ++i) {
            impl_.construct(*i);
        }
    }

    recursive_vector(recursive_vector&& other) noexcept { swap(other); }

    recursive_vector& operator=(const recursive_vector& other) {
        auto copy{other};
        swap(copy);
        return *this;
    }

    recursive_vector& operator=(recursive_vector&& other) noexcept {
        swap(other);
        return *this;
    }

    /// Forgive me father for I have sinned.
    template <typename It>
    iterator insert(const_iterator pos, It begin, It end) {
        const auto extra_elements{std::distance(begin, end)};
        const auto space_required{extra_elements + impl_.size()};
        if (impl_.capacity() < space_required) {
            //  We have to reallocate.
            //  Create a new impl to store the data.
            impl_type to_copy{std::max(space_required, impl_.size() * 2 + 1)};
            for (auto i{impl_.begin()}; i != pos; ++i) {
                to_copy.construct(std::move(*i));
            }
            const auto ret{to_copy.end()};
            for (auto i{begin}; i != end; ++i) {
                to_copy.construct(std::move(*i));
            }
            for (auto i{pos}; i != impl_.end(); ++i) {
                to_copy.construct(std::move(*i));
            }
            //  Swap internals.
            impl_.swap(to_copy);
            return ret;
        }
        //  Make space at pos by moving elements along the allocated space.
        const auto lim{pos - 1};
        for (auto i{impl_.end() - 1}, last{impl_.begin() + space_required - 1};
             i != lim;
             --i, --last) {
            *last = std::move(*i);
        }
        //  Move elements in.
        const auto offset{impl_.begin() + std::distance(impl_.cbegin(), pos)};
        for (iterator i{offset}; begin != end; ++begin, ++i) {
            *i = std::move(*begin);
        }
        return offset;
    }

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
