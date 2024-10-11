#include "LocalSearch.h"
#include "Instance.h"
#include "Solution.h"
#include "Util.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <limits>
#include <optional>
#include <ostream>
#include <utility>
#include <vector>

using std::size_t;

struct MoveInfo {
    long delta = 0;
    size_t i = std::numeric_limits<size_t>::max();
    size_t j = std::numeric_limits<size_t>::max();
};

enum class Searchs : std::uint8_t {
    SWAP,
    REINSERTION_1,
    REINSERTION_2,
    REINSERTION_3,
    REINSERTION_4,
    REINSERTION_5,
    REINSERTION_6,
    REINSERTION_7,
    REINSERTION_8,
    REINSERTION_9,
    REINSERTION_10,
    REINSERTION_11,
    REINSERTION_12,
    REINSERTION_13
};

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

inline long CalcShift(const Vertex &inserted, const Vertex &removed, const Vertex &next, const Instance &instance) {

    const long set_delta = static_cast<long>(instance.setup_time(next, inserted) - instance.setup_time(next, removed));
    return static_cast<long>(inserted.finish_time) - static_cast<long>(removed.finish_time) + set_delta;
}

inline std::optional<long> EvalSwap(size_t i, size_t j, long best_delta, const Solution &s, const Instance &instance) {

    long delta = 0;
    Vertex v_j = s.sequence[j];

    // Removes old penalty from j
    delta -= static_cast<long>(v_j.penalty);
    // Adds new penalty of j in position i, changes the value of the v_j var
    delta += static_cast<long>(instance.CalculateVertex(v_j, s.sequence[i - 1]));

    const long shift_after_i = CalcShift(v_j, s.sequence[i], s.sequence[i + 1], instance);

    Vertex v_before_i = s.sequence[j - 1];
    v_before_i.finish_time += shift_after_i;
    Vertex v_i = s.sequence[i];

    // Removes old penalty from j
    delta -= static_cast<long>(v_i.penalty);
    // Adds new penalty of i in position j, changes the value of the v_j var
    delta += static_cast<long>(instance.CalculateVertex(v_i, v_before_i));

    long shift_after_j = 0;
    long lb_w_after_j = 0;
    if (j != s.sequence.size() - 1) {
        shift_after_j = CalcShift(v_i, s.sequence[j], s.sequence[j + 1], instance);
        lb_w_after_j = s.lbw[j + 1];
    }

    const long lb_w_after_i = s.lbw[i + 1];

    long lb_delta1 = shift_after_i * lb_w_after_i;
    lb_delta1 -= shift_after_i * s.lbw[j];

    const long lb_delta2 = shift_after_j * lb_w_after_j;

    if (delta + lb_delta1 + lb_delta2 > best_delta) {
        return {};
    }

    auto [penalty_between, finish_time_between] = EvalRange(v_j.finish_time, i + 1, j, v_j, s, instance);
    delta += penalty_between;

    if (j == s.sequence.size() - 1) {
        return delta;
    }

    // Calculate the shift from index j until the end of the sequence.
    auto [penalty_after, _] = EvalRange(v_i.finish_time, j + 1, s.sequence.size(), v_i, s, instance);
    delta += penalty_after;

    return delta;
}

inline std::optional<long> EvalSwapAdjacent(size_t i, long best_delta, const Solution &s, const Instance &instance) {
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

    long shift_after_j = 0;
    long lb_w_j = 0;
    if (j != s.sequence.size() - 1) {
        shift_after_j = CalcShift(v_i, s.sequence[j], s.sequence[j + 1], instance);
        lb_w_j = s.lbw[j + 1];
    }

    const long lb_delta = shift_after_j * lb_w_j;

    if (delta + lb_delta > best_delta) {
        return {};
    }

    if (j == s.sequence.size() - 1) {
        return delta;
    }

    // Calculate the shift from index j until the end of the sequence.
    // const long shift2 = CalcShift(v_i, s.sequence[j], s.sequence[j + 1], instance);
    auto [penalty, _] = EvalRange(v_i.finish_time, j + 1, s.sequence.size(), v_i, s, instance);
    delta += penalty;
    return delta;
}

bool IsCorrect(long best_delta, size_t i, size_t j,const Solution &s, int block_size = -1) {
    const size_t estimated = best_delta + s.cost();

    Solution s_b = s;
    if (block_size == -1) {
        s_b.ApplySwap(i, j);
    } else {
        s_b.ApplyReinsertion(i, j, block_size);
    }
    if (s_b.cost() != estimated) {
        std::cerr << s << '\n';
        std::cerr << "Correct: " << s_b.cost() << ", Received: " << estimated << '\n';
        std::cerr << "(i,j): " << "(" << i << "," << j << ")\n";
    }

    return s_b.DebugCost() == estimated;
}

bool IsWorse(long best_delta, size_t i, size_t j, const Solution &s, int block_size = -1) {

    Solution s_b = s;
    if (block_size == -1) {
        s_b.ApplySwap(i, j);
    } else {
        s_b.ApplyReinsertion(i, j, block_size);
    }

    if (static_cast<long>(s_b.cost()) - static_cast<long>(s.cost()) < best_delta) {
        std::cout << s;
        std::cout << s_b;
        std::cout << i << ' ' << j << '\n';
        std::cout << best_delta << '\n';
        return false;
    }

    return true;
}

void SaveIfBetter(MoveInfo &best, std::tuple<std::optional<long>,size_t,size_t> move, const Solution& s, size_t block_size = -1){
    auto [delta, i, j] = move;
    if (!delta) {
        assert(IsWorse(best.delta, i, j, s, block_size));
        return;
    }

    assert(IsCorrect(*delta, i, j, s, block_size));

    if (*delta < best.delta) {
        best.i = i;
        best.j = j;
        best.delta = *delta;
    }
}

bool Swap(Solution &s, const Instance &instance) {

    MoveInfo best;

    for (size_t i = 1; i < s.sequence.size() - 1; i++) {
        for (size_t j = i + 2; j < s.sequence.size(); j++) {
            const std::optional<long> delta = EvalSwap(i, j, best.delta, s, instance);

            SaveIfBetter(best, {delta, i, j}, s);
        }
    }

    for (size_t i = 1; i < s.sequence.size() - 1; i++) {
        const std::optional<long> delta = EvalSwapAdjacent(i, best.delta, s, instance);

        SaveIfBetter(best, {delta, i, i + 1}, s);
    }

    if (best.delta < 0) {
        s.ApplySwap(best.i, best.j);
        return true;
    }

    return false;
}

inline long CalcShiftReinsertionRemove(const Vertex &last_block, const Vertex &before_block, const Vertex &after_block,
                                       const Instance &instance) {
    const long set_delta = static_cast<long>(instance.setup_time(after_block, before_block)) -
                           static_cast<long>(instance.setup_time(after_block, last_block));

    const long delta_time = static_cast<long>(before_block.finish_time) - static_cast<long>(last_block.finish_time);

    return set_delta + delta_time;
}

inline long CalcShiftReinsertion(const Vertex &last_block, const Vertex &before, const Vertex &prox,
                                 const Instance &instance) {

    const long set_delta =
        static_cast<long>(instance.setup_time(prox, last_block)) - static_cast<long>(instance.setup_time(prox, before));

    const long delta_time = static_cast<long>(last_block.finish_time) - static_cast<long>(before.finish_time);

    return set_delta + delta_time;
}

std::optional<long> EvalReinsertion(size_t i, size_t j, size_t block_size, long best_delta, const Solution &s,
                                    const Instance &instance) {
    long delta = 0;

    const Vertex &v_before_i = s.sequence[i - 1];
    const Vertex &v_after_block = s.sequence[i + block_size];
    Vertex v_last_block = s.sequence[i + block_size - 1];

    const long shift_between = CalcShiftReinsertionRemove(v_last_block, v_before_i, v_after_block, instance);

    // Calculate Delta from block in the new position
    auto [penalty_block, finish_time_block] =
        EvalRange(s.sequence[j].finish_time + shift_between, i, i + block_size, s.sequence[j], s, instance);
    delta += penalty_block;

    v_last_block.finish_time = finish_time_block;

    long shift_after = 0;
    long lb_w_after = 0;
    if (j != s.sequence.size() - 1) {

        shift_after = CalcShiftReinsertion(v_last_block, s.sequence[j], s.sequence[j + 1], instance);

        lb_w_after = s.lbw[j + 1];
    }

    const long lb_w_between = s.lbw[i + block_size];
    long lb_delta1 = shift_between * lb_w_between;
    if (j != s.sequence.size() - 1) {
        lb_delta1 -= shift_between * s.lbw[j + 1];
    }

    const long lb_delta2 = shift_after * lb_w_after;

    if (delta + lb_delta1 + lb_delta2 > best_delta) {
        return {};
    }

    // Calculate Delta from the sequence that will be put behind the block new position
    const Vertex &v_before_block = s.sequence[i - 1];
    auto [penalty_back, finish_time_back] =
        EvalRange(v_before_block.finish_time, i + block_size, j + 1, v_before_block, s, instance);
    delta += penalty_back;

    if (j == s.sequence.size() - 1) {
        return delta;
    }

    // Calculate Delta from nodes in front of the insertion point of the block
    const Vertex &v_last_from_block = s.sequence[i + block_size - 1];
    auto [penalty_front, _] = EvalRange(finish_time_block, j + 1, s.sequence.size(), v_last_from_block, s, instance);
    delta += penalty_front;

    return delta;
}

std::optional<long> EvalReinsertionBack(size_t i, size_t j, size_t block_size, long best_delta, const Solution &s,
                                        const Instance &instance) {
    long delta = 0;

    const Vertex &v_before_j = s.sequence[j - 1];
    // Calculate Delta from block in the new position
    auto [penalty_block, finish_time_block] =
        EvalRange(v_before_j.finish_time, i, i + block_size, v_before_j, s, instance);
    delta += penalty_block;

    const Vertex &v_j = s.sequence[j];
    Vertex v_last_block = s.sequence[i + block_size - 1];
    v_last_block.finish_time = finish_time_block;

    const long shift_j = CalcShiftReinsertion(v_last_block, v_before_j, v_j, instance);

    long shift_after_block = 0;
    long lb_w_after_block = 0;
    if (i != s.sequence.size() - block_size) {

        shift_after_block = CalcShiftReinsertionRemove(s.sequence[i + block_size - 1], s.sequence[i - 1],
                                                       s.sequence[i + block_size], instance) +
                            shift_j;
        lb_w_after_block = s.lbw[i + block_size];
    }

    const long lb_w_j = s.lbw[j];

    long lb_delta1 = shift_j * lb_w_j;
    lb_delta1 -= shift_j * s.lbw[i];

    const long lb_delta2 = shift_after_block * lb_w_after_block;

    if (delta + lb_delta1 + lb_delta2 > best_delta) {
        return {};
    }

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

    MoveInfo best;

    for (size_t i = 1; i < s.sequence.size() - block_size; i++) {
        for (size_t j = i + block_size; j < s.sequence.size(); j++) {
            const std::optional<long> delta = EvalReinsertion(i, j, block_size, best.delta, s, instance);

            SaveIfBetter(best, {delta, i, j}, s, block_size);
        }
    }

    for (size_t i = 2; i < s.sequence.size() - block_size; i++) {
        for (size_t j = 1; j < i; j++) {
            const std::optional<long> delta = EvalReinsertionBack(i, j, block_size, best.delta, s, instance);

            SaveIfBetter(best, {delta, i, j}, s, block_size);
        }
    }

    if (best.delta < 0) {
        s.ApplyReinsertion(best.i, best.j, block_size);
        return true;
    }

    return false;
}

// bool Reverse(Solution &s, const Instance &instance) {

//     long best_delta = 0;
//     size_t best_i = std::numeric_limits<size_t>::max();
//     size_t best_j = std::numeric_limits<size_t>::max();

//     auto save_if_better = [&](std::optional<long> delta, size_t i, size_t j) {
//         if (!delta) {
//             assert(IsWorse(best_delta, i, j, s, block_size));
//             return;
//         }

//         assert(IsCorrect(*delta, i, j, s, block_size));

//         if (*delta < best_delta) {
//             best_i = i;
//             best_j = j;
//             best_delta = *delta;
//         }
//     };

//     for (size_t i = 1; i < s.sequence.size() - block_size; i++) {
//         for (size_t j = i + block_size; j < s.sequence.size(); j++) {
//             const std::optional<long> delta = EvalReinsertion(i, j, block_size, best_delta, s, instance);

//             save_if_better(delta, i, j);
//         }
//     }

//     for (size_t i = 2; i < s.sequence.size() - block_size; i++) {
//         for (size_t j = 1; j < i; j++) {
//             const std::optional<long> delta = EvalReinsertionBack(i, j, block_size, best_delta, s, instance);

//             save_if_better(delta, i, j);
//         }
//     }

//     if (best_delta < 0) {
//         s.ApplyReinsertion(best_i, best_j, block_size);
//         return true;
//     }

//     return false;
// }

void LocalSearch(Solution &s, const Instance &instance) {
    std::vector searchs = {Searchs::SWAP,           Searchs::REINSERTION_1,  Searchs::REINSERTION_2,
                           Searchs::REINSERTION_3,  Searchs::REINSERTION_4,  Searchs::REINSERTION_5,
                           Searchs::REINSERTION_6,  Searchs::REINSERTION_7,  Searchs::REINSERTION_8,
                           Searchs::REINSERTION_9,  Searchs::REINSERTION_10, Searchs::REINSERTION_11,
                           Searchs::REINSERTION_12, Searchs::REINSERTION_13};
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
        case Searchs::REINSERTION_4:
            improved = Reinsertion(s, 4, instance);
            break;
        case Searchs::REINSERTION_5:
            improved = Reinsertion(s, 5, instance);
            break;
        case Searchs::REINSERTION_6:
            improved = Reinsertion(s, 6, instance);
            break;
        case Searchs::REINSERTION_7:
            improved = Reinsertion(s, 7, instance);
            break;
        case Searchs::REINSERTION_8:
            improved = Reinsertion(s, 8, instance);
            break;
        case Searchs::REINSERTION_9:
            improved = Reinsertion(s, 9, instance);
            break;
        case Searchs::REINSERTION_10:
            improved = Reinsertion(s, 10, instance);
            break;
        case Searchs::REINSERTION_11:
            improved = Reinsertion(s, 11, instance);
            break;
        case Searchs::REINSERTION_12:
            improved = Reinsertion(s, 12, instance);
            break;
        case Searchs::REINSERTION_13:
            improved = Reinsertion(s, 13, instance);
            break;
        }

        if (improved) {
            searchs = {Searchs::SWAP,           Searchs::REINSERTION_1,  Searchs::REINSERTION_2,
                       Searchs::REINSERTION_3,  Searchs::REINSERTION_4,  Searchs::REINSERTION_5,
                       Searchs::REINSERTION_6,  Searchs::REINSERTION_7,  Searchs::REINSERTION_8,
                       Searchs::REINSERTION_9,  Searchs::REINSERTION_10, Searchs::REINSERTION_11,
                       Searchs::REINSERTION_12, Searchs::REINSERTION_13};
            continue;
        }
        searchs.erase(chose);
    }
}

[[nodiscard]] Solution Pertubation(Solution best_copy, const Instance & /*instance*/) {

    long block_size_i = 0;
    long block_size_j = 0;
    const long size = static_cast<long>(best_copy.sequence.size()) - 1;
    const long max_size = size / 10;

    if (max_size > 2) {

        block_size_i = rng::rand_int(static_cast<long>(2), max_size);
        block_size_j = rng::rand_int(static_cast<long>(2), max_size);
    } else {

        block_size_i = block_size_j = 2;
    }

    long i = rng::rand_int(static_cast<long>(1), size - block_size_i); // chosing a random subset1 end
    long j = 0;

    const long possibilities_before_i = std::max(static_cast<long>(0), i - block_size_j);
    const long possibilities_after_i = std::max(static_cast<long>(0), size - (i + block_size_i - 1) - block_size_j + 1);
    const bool back =
        rng::rand_int(static_cast<long>(1), possibilities_after_i + possibilities_before_i) <= possibilities_before_i;

    if (back) {
        j = rng::rand_int(static_cast<long>(1), possibilities_before_i);

        // Ensures i < j
        std::swap(block_size_i, block_size_j);
        std::swap(i, j);
    } else {
        j = rng::rand_int(static_cast<long>(1), possibilities_after_i) + i + block_size_i - 1;
    }

    best_copy.ApplyDoubleBridge(i, j, block_size_i, block_size_j);

    return best_copy;
}
