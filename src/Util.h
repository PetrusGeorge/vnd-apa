#ifndef RANDOM_H
#define RANDOM_H

#include <cstddef>
#include <iterator>
#include <random>
#include <stdexcept>

namespace parameters {
constexpr double R_MAX = 0.25;
}

namespace my_rand {

inline thread_local std::mt19937 gen(std::random_device{}()); // NOLINT

inline void set_seed(std::size_t seed) { gen.seed(seed); }

template <typename IntType> IntType rand_int(IntType begin, IntType end) {
    std::uniform_int_distribution<> dis(begin, end);
    return dis(gen);
}

template <typename Iterator> Iterator choose_random_value(Iterator begin, Iterator end) {
    if (begin == end) {
        throw std::invalid_argument("Range is empty");
    }

    std::uniform_int_distribution<> dis(0, std::distance(begin, end) - 1);
    int random_index = dis(gen);
    std::advance(begin, random_index);
    return begin;
}

} // namespace my_rand

#endif
