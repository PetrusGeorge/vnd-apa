#include "LocalSearch.h"
#include "Util.h"
#include "src/Instance.h"
#include "src/Solution.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>

using std::size_t;

enum class Searchs { SWAP, REINSERTION };

inline long EvalRange(long delta_time, size_t begin, size_t end, const Solution &s, const Instance &instance) {
    long delta = 0;
    for (size_t i = begin; i < end; i++) {
        delta -= static_cast<long>(s.sequence[i].penalty);
        delta += static_cast<long>(instance.EvalVertex(s.sequence[i], s.sequence[i - 1], delta_time));
    }
    return delta;
}

inline long EvalSwap(const Solution &s, size_t i, size_t j, const Instance &instance) {
    long delta = 0;

    Vertex v_j = s.sequence[j];
    delta += static_cast<long>(-v_j.penalty + instance.CalculateVertex(v_j, s.sequence[i - 1]));

    const long set_delta_1 =
        instance.setup_time(s.sequence[i + 1], v_j) - instance.setup_time(s.sequence[i + 1], s.sequence[i]);
    const long shift1 = static_cast<long>(v_j.finish_time) - static_cast<long>(s.sequence[i].finish_time) + set_delta_1;
    delta += EvalRange(shift1, i + 1, j, s, instance);

    Vertex v_i = s.sequence[i];
    Vertex v_before_i = s.sequence[j - 1];
    v_before_i.finish_time += shift1;
    delta += static_cast<long>(-v_i.penalty + instance.CalculateVertex(v_i, v_before_i));
    long set_delta_2 = 0;
    if (j != s.sequence.size() - 1) {
        set_delta_2 =
            instance.setup_time(s.sequence[j + 1], v_i) - instance.setup_time(s.sequence[j + 1], s.sequence[j]);
    }
    const long shift2 = static_cast<long>(v_i.finish_time) - static_cast<long>(s.sequence[j].finish_time) + set_delta_2;
    delta += EvalRange(shift2, j + 1, s.sequence.size(), s, instance);

    return delta;
}

bool IsCorrect(Solution s, size_t i, size_t j, size_t estimated) {
    s.ApplySwap(i, j);
    if (s.cost() != estimated) {
        std::cerr << s << '\n';
        std::cerr << "Correct: " << s.cost() << ", Received: " << estimated << '\n';
    }

    return s.CorrectCost() == estimated;
}

bool Swap(Solution &s, const Instance &instance) {

    long best_delta = 0;
    size_t best_i = std::numeric_limits<size_t>::max();
    size_t best_j = std::numeric_limits<size_t>::max();

    for (size_t i = 1; i < s.sequence.size() - 1; i++) {

        for (size_t j = i + 2; j < s.sequence.size(); j++) {
            const long delta = EvalSwap(s, i, j, instance);
            assert(IsCorrect(s, i, j, s.cost() + delta));

            if (delta < best_delta) {
                best_i = i;
                best_j = j;
                best_delta = delta;
            }
        }
    }

    if (best_delta < 0) {
        s.ApplySwap(best_i, best_j);
        return true;
    }

    return false;
}

bool Reinsertion(Solution & /*s*/, size_t /*block_size*/) { return false; }

void LocalSearch(Solution &s, const Instance &instance) {
    std::vector searchs = {Searchs::SWAP, Searchs::REINSERTION};
    while (!searchs.empty()) {
        bool improved = false;
        auto chose = my_rand::choose_random_value(searchs.begin(), searchs.end());
        switch (*chose) {
        case Searchs::SWAP:
            improved = Swap(s, instance);
            break;
            // case Searchs::REINSERTION:
            //     improved = Reinsertion(s, 1);
            //     break;
        }

        if (improved) {
            searchs = {Searchs::SWAP, Searchs::REINSERTION};
            continue;
        }
        searchs.erase(chose);
    }
}

[[nodiscard]] Solution Pertubation(Solution best, const Instance & /*instance*/) { return best; }
