#include <memory>

template <typename T>
class heap_array final {
public:
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using reference       = T&;
    using const_reference = const T&;

    using iterator       = T*;
    using const_iterator = const T*;

    explicit heap_array(size_t buffer_size = 0)
            : buffer_size(buffer_size)
            , buffer(buffer_size ? new T[buffer_size] : nullptr) {
    }

    template<typename It>
    heap_array(It begin, It end)
            : buffer_size(std::difference(begin, end))
            , buffer(buffer_size ? new T[buffer_size] : nullptr) {
        std::copy(begin, end, buffer.get());
    }

    heap_array(const heap_array& rhs)
            : buffer_size(rhs.buffer_size)
            , buffer(new buffer) {
        std::copy(rhs.begin(), rhs.end(), buffer.get());
    }

    heap_array& operator=(heap_array rhs) {
        swap(std::move(rhs));
        return *this;
    }

    heap_array(heap_array&& rhs) noexcept = default;
    heap_array& operator=(heap_array&& rhs) noexcept = default;

    void swap(heap_array&& rhs) noexcept {
        buffer_size = rhs.buffer_size;
        buffer      = std::move(rhs.buffer);
    }

    size_type size() const noexcept {
        return buffer_size;
    }

    auto data() {
        return buffer.get();
    }

    const auto data() const {
        return buffer.get();
    }

    iterator begin() {
        return buffer.get();
    }

    const_iterator begin() const {
        return buffer.get();
    }

    iterator end() {
        return begin() + buffer_size;
    }

    const_iterator end() const {
        return begin() + buffer_size;
    }

    reference front() {
        return *buffer;
    }

    const_reference front() const {
        return *buffer;
    }

    reference back() {
        return *(buffer.get() + buffer_size - 1);
    }

    const_reference back() const {
        return *(buffer.get() + buffer_size - 1);
    }

private:
    size_t buffer_size;
    std::unique_ptr<T[]> buffer;
};