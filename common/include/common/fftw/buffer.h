#pragma once

#include <memory>

namespace fftwf {

template <typename T>
class buffer final {
public:
    using value_type = T;

    explicit buffer(size_t buffer_size = 0);
    buffer(const buffer& rhs);
    buffer& operator=(buffer rhs);

    buffer(buffer&& rhs) noexcept = default;
    buffer& operator=(buffer&& rhs) noexcept = default;

    void swap(buffer& rhs) noexcept;

    size_t size() const noexcept;
    T* data();
    const T* data() const;

    T* begin();
    const T* begin() const;

    T* end();
    const T* end() const;

    void zero();

private:
    struct destructor final {
        void operator()(T* t) const noexcept;
    };

    size_t buf_size;
    std::unique_ptr<T[], destructor> buf;
};

using rbuf = buffer<float>;

}  // namespace fftwf
