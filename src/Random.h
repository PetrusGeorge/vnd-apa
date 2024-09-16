#ifndef RANDOM_H
#define RANDOM_H

#include <cstddef>
#include <iterator>
#include <random>
#include <stdexcept>

namespace my_rand {

inline thread_local std::mt19937 gen(std::random_device{}()); // NOLINT

inline void set_seed(std::size_t seed) { gen.seed(seed); }

template <typename Iterator>
typename std::iterator_traits<Iterator>::value_type choose_random_value(Iterator begin, Iterator end) {
    if (begin == end) {
        throw std::invalid_argument("Range is empty");
    }

    std::uniform_int_distribution<> dis(0, std::distance(begin, end) - 1);
    int random_index = dis(gen);
    std::advance(begin, random_index);
    return *begin;
}

} // namespace my_rand

#endif
