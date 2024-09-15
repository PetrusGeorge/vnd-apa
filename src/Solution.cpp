#include "Solution.h"
#include "Instance.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>

using std::size_t;
using std::vector;

bool Solution::CorrectCost(size_t old){

    RecalculateCost();
    bool check = old == m_cost;

    if(!check){
        std::cerr << "Cost should be: " << m_cost << ", but received: " << old << '\n';
    }

    return check;
}

Solution::Solution(vector<Vertex> &&sequence, const Instance &instance)
    : m_instance(instance), m_sequence(std::move(sequence)) {
    RecalculateCost();
}

Solution::Solution(vector<Vertex> &&sequence, size_t cost, const Instance &instance)
    : m_instance(instance), m_sequence(std::move(sequence)), m_cost(cost) {

    assert(CorrectCost(cost));
}

// TODO:
void Solution::ApplySwap() {}
void Solution::ApplyReinsertion() {}


void Solution::RecalculateCost() {
    m_cost = 0;

    m_cost += m_instance.CalculateVertex(m_sequence.front());

    for (size_t i = 1; i < m_sequence.size(); i++) {
        const Vertex prev = m_sequence[i - 1];
        Vertex& current = m_sequence[i];

        m_cost += m_instance.CalculateVertex(current, prev);
    }
}
