#include "common/fftw/buffer.h"

#include "traits.h"

namespace fftwf {

template<typename T>
buffer<T>::buffer(size_t buf_size)
        : buf_size(buf_size)
        , buf(fftwf::type_trait<T>::alloc(buf_size)) {}

template<typename T>
buffer<T>::buffer(const buffer& rhs)
        : buf_size(rhs.buf_size)
        , buf(fftwf::type_trait<T>::alloc(buf_size)) {
    memcpy(data(), rhs.data(), buf_size * sizeof(T));
}

template<typename T>
buffer<T>& buffer<T>::operator=(buffer rhs) {
    swap(rhs);
    return *this;
}

template<typename T>
void buffer<T>::swap(buffer& rhs) noexcept {
    using std::swap;
    swap(buf_size, rhs.buf_size);
    swap(buf, rhs.buf);
}

template<typename T>
size_t buffer<T>::size() const noexcept { return buf_size; }

template<typename T>
T* buffer<T>::data() { return buf.get(); }

template<typename T>
const T* buffer<T>::data() const { return buf.get(); }

template<typename T>
T* buffer<T>::begin() { return buf.get(); }

template<typename T>
const T* buffer<T>::begin() const { return buf.get(); }

template<typename T>
T* buffer<T>::end() { return begin() + buf_size; }

template<typename T>
const T* buffer<T>::end() const { return begin() + buf_size; }

template<typename T>
void buffer<T>::zero() { memset(data(), 0, buf_size * sizeof(T)); }

template <typename T>
void buffer<T>::destructor::operator()(T* t) const noexcept {
    fftwf_free(t);
};

template class buffer<float>;
template class buffer<fftwf_complex>;

}  // namespace fftwf
