#ifndef RANDOM_H
#define RANDOM_H

#include <cstddef>
#include <iterator>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <chrono>

namespace parameters {
constexpr double R_MAX = 0.25;
}
namespace benchmark {

    inline double UpTime(){
        const static thread_local auto start_time = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();

        return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>( now - start_time ).count())/1e6;
    }
    const auto timeFuncInvocation = 
    [](auto&& func, double& value, auto&&... params) {
        // Get time before function invocation
        const auto start = std::chrono::high_resolution_clock::now();

        // Function invocation using perfect forwarding
        auto result = std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);

        // Get time after function invocation
        const auto stop = std::chrono::high_resolution_clock::now();

        // Output the duration
        std::chrono::duration<double> duration = stop - start;
        value = duration.count();
        // Return the result of the function invocation
        return result;
    };
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
