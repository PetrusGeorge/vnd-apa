#include "Solution.h"
#include "Instance.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>

using std::size_t;
using std::vector;

Solution::Solution(vector<Vertex> &&sequence, const Instance &instance)
    : m_instance(instance), m_sequence(std::move(sequence)) {
    RecalculateCost();
}

Solution::Solution(vector<Vertex> &&sequence, size_t cost, const Instance &instance)
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

    m_cost += m_instance.CalculateVertex(m_sequence.front());

    std::cout << "Current: " << m_sequence.front().finish_time << '\n';

    for (size_t i = 1; i < m_sequence.size(); i++) {
        const Vertex prev = m_sequence[i - 1];
        Vertex& current = m_sequence[i];

        m_cost += m_instance.CalculateVertex(current, prev);
        std::cout << "Current: " << current.finish_time << '\n';
        std::cout << "prev: " << prev.finish_time << '\n';
    }
    std::cout << "Custo: " << m_cost << '\n';
}
