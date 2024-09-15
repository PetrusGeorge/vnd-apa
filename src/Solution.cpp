#include "Solution.h"
#include "Instance.h"
#include <cassert>

using std::size_t;
using std::vector;

Solution::Solution(vector<size_t> &&sequence, const Instance &instance)
    : m_instance(instance), m_sequence(std::move(sequence)) {
    RecalculateCost();
}

Solution::Solution(vector<size_t> &&sequence, size_t cost, const Instance &instance)
    : m_instance(instance), m_sequence(std::move(sequence)), m_cost(cost) {

    assert(cost == [this]() {
        RecalculateCost();
        return m_cost;
    }());
}

// TODO:
void Solution::ApplySwap() {}
void Solution::ApplyReinsertion() {}
void Solution::RecalculateCost() {}
