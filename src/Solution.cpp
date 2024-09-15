#include "Solution.h"
#include "Instance.h"

#include <cassert>
#include <cstddef>
#include <utility>

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

void Solution::RecalculateCost() {
    m_cost = 0;

    size_t sum_time = 0;

    const size_t first_id = m_sequence.front();
    sum_time += m_instance.AddedTime(first_id, -1);
    m_cost += m_instance.Penalty(first_id, sum_time);

    for (size_t i = 1; i < m_sequence.size(); i++) {
        const size_t prev_id = m_sequence[i - 1];
        const size_t current_id = m_sequence[i];

        sum_time += m_instance.AddedTime(current_id, prev_id);
        m_cost += m_instance.Penalty(current_id, sum_time);
    }
}
