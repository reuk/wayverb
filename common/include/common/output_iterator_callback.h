#pragma once

template <typename It>
class output_iterator_callback final {
public:
    constexpr explicit output_iterator_callback(It output_iterator)
            : output_iterator_(std::move(output_iterator)) {}

    template <typename T>
    void operator()(T&& t) {
        *output_iterator_++ = std::forward<T>(t);
    }

private:
    It output_iterator_;
};

template <typename It>
output_iterator_callback<It> make_output_iterator_callback(It it) {
    return output_iterator_callback<It>{std::move(it)};
}
