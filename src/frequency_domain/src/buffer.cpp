#include "frequency_domain/buffer.h"

#include "traits.h"

namespace frequency_domain {

template <typename T>
buffer<T>::buffer(size_t buffer_size)
        : size_{buffer_size}
        , buf_{type_trait<T>::alloc(buffer_size)} {}

template <typename T>
buffer<T>::buffer(const buffer& rhs)
        : size_{rhs.size_}
        , buf_{type_trait<T>::alloc(size_)} {
    memcpy(data(), rhs.data(), size_ * sizeof(T));
}

template <typename T>
buffer<T>& buffer<T>::operator=(buffer rhs) {
    swap(rhs);
    return *this;
}

template <typename T>
void buffer<T>::swap(buffer& rhs) noexcept {
    using std::swap;
    swap(size_, rhs.size_);
    swap(buf_, rhs.buf_);
}

template <typename T>
size_t buffer<T>::size() const noexcept {
    return size_;
}

template <typename T>
T* buffer<T>::data() {
    return buf_.get();
}

template <typename T>
const T* buffer<T>::data() const {
    return buf_.get();
}

template <typename T>
T* buffer<T>::begin() {
    return buf_.get();
}

template <typename T>
const T* buffer<T>::begin() const {
    return buf_.get();
}

template <typename T>
T* buffer<T>::end() {
    return begin() + size_;
}

template <typename T>
const T* buffer<T>::end() const {
    return begin() + size_;
}

template <typename T>
void buffer<T>::zero() {
    memset(data(), 0, size_ * sizeof(T));
}

template <typename T>
void buffer<T>::destructor::operator()(T* t) const noexcept {
    fftwf_free(t);
};

template class buffer<float>;
template class buffer<fftwf_complex>;

}  // namespace frequency_domain

