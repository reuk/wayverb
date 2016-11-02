#pragma once

#include "utilities/foldl.h"
#include "utilities/map.h"

#include <cmath>
#include <complex>

namespace core {

template <typename T>
struct item_and_index final {
    T item;
    size_t index;
};

template <typename T>
constexpr auto make_item_and_index(T item, size_t index) {
    return item_and_index<T>{std::move(item), index};
}

template <typename T, size_t... Ix>
constexpr auto zip_with_indices(const std::array<T, sizeof...(Ix)>& arr,
                                std::index_sequence<Ix...>) {
    return std::array<item_and_index<T>, sizeof...(Ix)>{
            {make_item_and_index(std::get<Ix>(arr), Ix)...}};
}

template <typename T, size_t N>
constexpr auto zip_with_indices(const std::array<T, N>& arr) {
    return zip_with_indices(arr, std::make_index_sequence<N>{});
}

////////////////////////////////////////////////////////////////////////////////

template <size_t N>
auto freqz(const std::array<double, N>& coeffs, double frequency) {
    return util::foldl(
            [&](const auto& accumulator, const auto& item) {
                using namespace std::complex_literals;
                return accumulator +
                       item.item * std::exp(-1i * frequency *
                                            static_cast<double>(item.index));
            },
            0.0,
            zip_with_indices(coeffs));
}

template <size_t B, size_t A>
auto freqz(const std::array<double, B>& b_coeffs,
           const std::array<double, A>& a_coeffs,
           double frequency) {
    return freqz(b_coeffs, frequency) / freqz(a_coeffs, frequency);
}

template <size_t N, size_t O>
auto freqz(const std::array<double, N>& coeffs,
           const std::array<double, O>& frequencies) {
    return map([&](const auto& f) { return freqz(coeffs, f); }, frequencies);
}

template <size_t B, size_t A, size_t O>
auto freqz(const std::array<double, B>& b_coeffs,
           const std::array<double, A>& a_coeffs,
           const std::array<double, O>& frequencies) {
    return map([&](const auto& f) { return freqz(b_coeffs, a_coeffs, f); },
               frequencies);
}

}  // namespace core
