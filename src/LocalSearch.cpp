#include "LocalSearch.h"
#include "Instance.h"
#include "Solution.h"
#include "Util.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <limits>
#include <ostream>
#include <utility>

using std::size_t;

enum class Searchs : std::uint8_t { SWAP, REINSERTION_1, REINSERTION_2, REINSERTION_3 };

inline std::pair<long, size_t> EvalRange(size_t start_time, size_t begin, size_t end, const Vertex &real_behind,
                                         const Solution &s, const Instance &instance) {
    long delta = 0;

    const Vertex first = s.sequence[begin];
    size_t finish_time_before = instance.setup_time(first, real_behind) + instance.process_time(first) + start_time;

    delta -= static_cast<long>(s.sequence[begin].penalty);
    if (finish_time_before > instance.deadline(first)) {
        delta += static_cast<long>((finish_time_before - instance.deadline(first)) * instance.weight(first));
    }

    for (size_t i = begin + 1; i < end; i++) {
        delta -= static_cast<long>(s.sequence[i].penalty);

        auto [penalty, finish_time] =
            instance.EvalVertexWithStart(s.sequence[i], s.sequence[i - 1], finish_time_before);
        delta += static_cast<long>(penalty);

        finish_time_before = finish_time;
    }

    return {delta, finish_time_before};
}

// inline long CalcShift(const Vertex &inserted, const Vertex &removed, const Vertex &next, const Instance &instance) {

//     const long set_delta = static_cast<long>(instance.setup_time(next, inserted) - instance.setup_time(next,
//     removed)); return static_cast<long>(inserted.finish_time) - static_cast<long>(removed.finish_time) + set_delta;
// }

// inline long CalcShiftReinsertionRemove(const Vertex &last, const Vertex &before, const Vertex &prox,
//                                        const Instance &instance) {
//
//     const long set_delta =
//         static_cast<long>(instance.setup_time(prox, before)) - static_cast<long>(instance.setup_time(prox, last));
//
//     const long delta_time = static_cast<long>(before.finish_time) - static_cast<long>(last.finish_time);
//
//     return set_delta + delta_time;
// }

inline long EvalSwap(size_t i, size_t j, const Solution &s, const Instance &instance) {
    long delta = 0;

    Vertex v_j = s.sequence[j];

    // Removes old penalty from j
    delta -= static_cast<long>(v_j.penalty);
    // Adds new penalty of j in position i, changes the value of the v_j var
    delta += static_cast<long>(instance.CalculateVertex(v_j, s.sequence[i - 1]));

    auto [penalty_between, finish_time_between] = EvalRange(v_j.finish_time, i + 1, j, v_j, s, instance);
    delta += penalty_between;

    Vertex v_before_i = s.sequence[j - 1];
    v_before_i.finish_time = finish_time_between;
    Vertex v_i = s.sequence[i];

    // Removes old penalty from j
    delta -= static_cast<long>(v_i.penalty);
    // Adds new penalty of i in position j, changes the value of the v_j var
    delta += static_cast<long>(instance.CalculateVertex(v_i, v_before_i));

    if (j == s.sequence.size() - 1) {
        return delta;
    }

    // Calculate the shift from index j until the end of the sequence.
    auto [penalty_after, _] = EvalRange(v_i.finish_time, j + 1, s.sequence.size(), v_i, s, instance);
    delta += penalty_after;

    return delta;
}

inline long EvalSwapAdjacent(size_t i, const Solution &s, const Instance &instance) {
    const size_t j = i + 1;
    long delta = 0;

    Vertex v_i = s.sequence[i];
    Vertex v_j = s.sequence[j];
    // Calculate the penalty delta of v_j node, alters the values of v_j
    delta -= static_cast<long>(v_j.penalty);
    delta += static_cast<long>(instance.CalculateVertex(v_j, s.sequence[i - 1]));

    // Calculate the penalty delta of v_i node, alters the values of v_i
    delta -= static_cast<long>(v_i.penalty);
    delta += static_cast<long>(instance.CalculateVertex(v_i, v_j));

    if (j == s.sequence.size() - 1) {
        return delta;
    }

    // Calculate the shift from index j until the end of the sequence.
    // const long shift2 = CalcShift(v_i, s.sequence[j], s.sequence[j + 1], instance);
    auto [penalty, _] = EvalRange(v_i.finish_time, j + 1, s.sequence.size(), v_i, s, instance);
    delta += penalty;
    return delta;
}

bool IsCorrect(size_t delta, size_t i, size_t j, Solution s, int block_size = -1) {
    const size_t estimated = delta + s.cost();

    if (block_size == -1) {
        s.ApplySwap(i, j);
    } else {
        s.ApplyReinsertion(i, j, block_size);
    }
    if (s.cost() != estimated) {
        std::cerr << s << '\n';
        std::cerr << "Correct: " << s.cost() << ", Received: " << estimated << '\n';
        std::cerr << "(i,j): " << "(" << i << "," << j << ")\n";
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

    for (size_t i = 1; i < s.sequence.size() - 1; i++) {
        const long delta = EvalSwapAdjacent(i, s, instance);
        assert(IsCorrect(delta, i, i + 1, s));
        if (delta < best_delta) {
            best_i = i;
            best_j = i + 1;
            best_delta = delta;
        }
    }

    if (best_delta < 0) {
        s.ApplySwap(best_i, best_j);
        return true;
    }

    return false;
}

long EvalReinsertion(size_t i, size_t j, size_t block_size, const Solution &s, const Instance &instance) {
    long delta = 0;

    // Calculate Delta from the sequence that will be put behind the block new position
    const Vertex &v_before_block = s.sequence[i - 1];
    auto [penalty_back, finish_time_back] =
        EvalRange(v_before_block.finish_time, i + block_size, j + 1, v_before_block, s, instance);
    delta += penalty_back;

    // Calculate Delta from block in the new position
    auto [penalty_block, finish_time_block] =
        EvalRange(finish_time_back, i, i + block_size, s.sequence[j], s, instance);
    delta += penalty_block;

    if (j == s.sequence.size() - 1) {
        return delta;
    }

    // Calculate Delta from nodes in front of the insertion point of the block
    const Vertex &v_last_from_block = s.sequence[i + block_size - 1];
    auto [penalty_front, _] = EvalRange(finish_time_block, j + 1, s.sequence.size(), v_last_from_block, s, instance);
    delta += penalty_front;

    return delta;
}

long EvalReinsertionBack(size_t i, size_t j, size_t block_size, const Solution &s, const Instance &instance) {
    long delta = 0;

    const Vertex &v_before = s.sequence[j - 1];
    // Calculate Delta from block in the new position
    auto [penalty_block, finish_time_block] = EvalRange(v_before.finish_time, i, i + block_size, v_before, s, instance);
    delta += penalty_block;

    // Calculate Delta from nodes in front of the insertion point of the block
    const Vertex &v_last_from_block = s.sequence[i + block_size - 1];
    auto [penalty_front, finish_time_a] = EvalRange(finish_time_block, j, i, v_last_from_block, s, instance);
    delta += penalty_front;

    if (i == s.sequence.size() - block_size) {
        return delta;
    }

    // Calculate Delta from the sequence that will be put behind the block new position
    const Vertex &v_before_block = s.sequence[i - 1];
    auto [penalty_back, finish_time_back] =
        EvalRange(finish_time_a, i + block_size, s.sequence.size(), v_before_block, s, instance);
    delta += penalty_back;

    return delta;
}
bool Reinsertion(Solution &s, size_t block_size, const Instance &instance) {

    long best_delta = 0;
    size_t best_i = std::numeric_limits<size_t>::max();
    size_t best_j = std::numeric_limits<size_t>::max();

    for (size_t i = 1; i < s.sequence.size() - block_size; i++) {
        for (size_t j = i + block_size; j < s.sequence.size(); j++) {
            const long delta = EvalReinsertion(i, j, block_size, s, instance);
            assert(IsCorrect(delta, i, j, s, block_size));

            if (delta < best_delta) {
                best_i = i;
                best_j = j;
                best_delta = delta;
            }
        }
    }

    for (size_t i = 2; i < s.sequence.size() - block_size; i++) {
        for (size_t j = 1; j < i; j++) {
            const long delta = EvalReinsertionBack(i, j, block_size, s, instance);
            assert(IsCorrect(delta, i, j, s, block_size));

            if (delta < best_delta) {
                best_i = i;
                best_j = j;
                best_delta = delta;
            }
        }
    }

    if (best_delta < 0) {
        s.ApplyReinsertion(best_i, best_j, block_size);
        return true;
    }

    return false;
}

void LocalSearch(Solution &s, const Instance &instance) {
    std::vector searchs = {Searchs::SWAP, Searchs::REINSERTION_1, Searchs::REINSERTION_2, Searchs::REINSERTION_3};
    while (!searchs.empty()) {
        bool improved = false;
        auto chose = rng::pick_iter(searchs.begin(), searchs.end());
        switch (*chose) {
        case Searchs::SWAP:
            improved = Swap(s, instance);
            break;
        case Searchs::REINSERTION_1:
            improved = Reinsertion(s, 1, instance);
            break;
        case Searchs::REINSERTION_2:
            improved = Reinsertion(s, 2, instance);
            break;
        case Searchs::REINSERTION_3:
            improved = Reinsertion(s, 3, instance);
            break;
        }

        if (improved) {
            searchs = {Searchs::SWAP, Searchs::REINSERTION_1, Searchs::REINSERTION_2, Searchs::REINSERTION_3};
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
