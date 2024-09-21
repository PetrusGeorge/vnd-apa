#include "LocalSearch.h"
#include "Instance.h"
#include "Solution.h"
#include "Util.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>

using std::size_t;

enum class Searchs : bool { SWAP, REINSERTION };

inline long EvalRange(long delta_time, size_t begin, size_t end, const Solution &s, const Instance &instance) {
    long delta = 0;
    for (size_t i = begin; i < end; i++) {
        delta -= static_cast<long>(s.sequence[i].penalty);
        delta += static_cast<long>(instance.EvalVertex(s.sequence[i], s.sequence[i - 1], delta_time));
    }
    return delta;
}

inline long CalcShift(const Vertex &inserted, const Vertex &removed, const Vertex &next, const Instance &instance) {

    const long set_delta = static_cast<long>(instance.setup_time(next, inserted) - instance.setup_time(next, removed));
    return static_cast<long>(inserted.finish_time) - static_cast<long>(removed.finish_time) + set_delta;
}

inline long EvalSwap(size_t i, size_t j, const Solution &s, const Instance &instance) {
    long delta = 0;

    Vertex v_j = s.sequence[j];
    // Calculate the penalty delta of v_j node, alters the values of v_j
    delta += static_cast<long>(-v_j.penalty + instance.CalculateVertex(v_j, s.sequence[i - 1]));

    // Calculate the shift of time between the indeces i and j caused by the swap.
    const long shift1 = CalcShift(v_j, s.sequence[i], s.sequence[i + 1], instance);

    Vertex v_before_i = s.sequence[j - 1];
    v_before_i.finish_time += shift1;
    Vertex v_i = s.sequence[i];
    // Calculate the penalty delta of v_i node, alters the values of v_i
    delta += static_cast<long>(-v_i.penalty + instance.CalculateVertex(v_i, v_before_i));

    delta += EvalRange(shift1, i + 1, j, s, instance);

    if (j == s.sequence.size() - 1) {
        return delta;
    }

    // Calculate the shift from index j until the end of the sequence.
    const long shift2 = CalcShift(v_i, s.sequence[j], s.sequence[j + 1], instance);
    delta += EvalRange(shift2, j + 1, s.sequence.size(), s, instance);

    return delta;
}

bool IsCorrect(size_t delta, size_t i, size_t j, Solution s) {
    const size_t estimated = delta + s.cost();
    s.ApplySwap(i, j);
    if (s.cost() != estimated) {
        std::cerr << s << '\n';
        std::cerr << "Correct: " << s.cost() << ", Received: " << estimated << '\n';
    }

    return s.DebugCost() == estimated;
}

bool Swap(Solution &s, const Instance &instance) {

    long best_delta = 0;
    size_t best_i = std::numeric_limits<size_t>::max();
    size_t best_j = std::numeric_limits<size_t>::max();

    for (size_t i = 1; i < s.sequence.size() - 1; i++) {

        for (size_t j = i + 2; j < s.sequence.size(); j++) {
            const long delta = EvalSwap(i, j, s, instance);
            assert(IsCorrect(delta, i, j, s));

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
        auto chose = rng::pick_iter(searchs.begin(), searchs.end());
        switch (*chose) {
        case Searchs::SWAP:
            improved = Swap(s, instance);
            break;
        case Searchs::REINSERTION:
            improved = Reinsertion(s, 1);
            break;
        }

        if (improved) {
            searchs = {Searchs::SWAP, Searchs::REINSERTION};
            continue;
        }
        searchs.erase(chose);
    }
}

[[nodiscard]] Solution Pertubation(Solution best, const Instance & /*instance*/) {

    // TODO: make a better pertubation
    for (int i = 0; i < 3; i++) {
        const size_t a = rng::rand_int(static_cast<size_t>(1), best.sequence.size() - 1);
        size_t b = a;
        while (a == b) {
            b = rng::rand_int(static_cast<size_t>(1), best.sequence.size() - 1);
        }
        best.ApplySwap(a, b);
    }

    return best;
}
