#ifndef RANDOM_H
#define RANDOM_H

#include <cstddef>
#include <iterator>
#include <random>
#include <stdexcept>
#include <type_traits>

namespace parameters {
constexpr double R_MAX = 1;
}

namespace rng {

inline thread_local std::mt19937 gen(std::random_device{}()); // NOLINT

inline void set_seed(std::size_t seed) { gen.seed(seed); }

template <typename IntType> [[nodiscard]] IntType rand_int(IntType begin, IntType end) {
    static_assert(std::is_integral_v<IntType>, "Function only receives integer types");
    std::uniform_int_distribution<IntType> dis(begin, end);
    return dis(gen);
}

template <typename Iterator> [[nodiscard]] Iterator pick_iter(Iterator begin, Iterator end) {
    if (begin == end) {
        throw std::invalid_argument("Range is empty");
    }

    std::uniform_int_distribution<> dis(0, std::distance(begin, end) - 1);
    int random_index = dis(gen);
    std::advance(begin, random_index);
    return begin;
}

} // namespace rng

#endif
