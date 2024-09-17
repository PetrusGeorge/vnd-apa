#include "Solution.h"
#include "Instance.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <utility>

using std::size_t;
using std::vector;

std::ostream &operator<<(std::ostream &os, const Solution &sol) {
    const int id_width = 10;
    const int finish_time_width = 15;
    const int penalty_width = 10;

    // Header
    os << std::left;
    os << std::setw(id_width) << "ID" << std::setw(finish_time_width) << "Finish Time" << std::setw(penalty_width)
       << "Penalty" << '\n';

    // Line
    os << std::setw(id_width) << std::setfill('-') << "" << std::setw(finish_time_width) << ""
       << std::setw(penalty_width) << "" << '\n';
    os << std::setfill(' ');

    // Rows
    for (const auto &order : sol.sequence) {
        os << std::setw(id_width) << order.id << std::setw(finish_time_width) << order.finish_time
           << std::setw(penalty_width) << order.penalty << '\n';
    }

    os << "\nFinal cost: " << sol.cost() << '\n';
    return os;
}

Solution::Solution(const Instance &instance) : m_instance(instance) {}

Solution::Solution(const Solution &other)
    : m_instance(other.m_instance), m_sequence(other.m_sequence), m_cost(other.cost()) {}

Solution::Solution(Solution &&other) noexcept
    : m_instance(other.m_instance), m_sequence(std::move(other.m_sequence)), m_cost(other.cost()) {}

Solution &Solution::operator=(const Solution &other) {
    m_sequence = other.m_sequence;
    m_cost = other.cost();

    return *this;
}

Solution &Solution::operator=(Solution &&other) noexcept {
    m_sequence = std::move(other.m_sequence);
    m_cost = other.cost();

    return *this;
}

size_t Solution::CorrectCost() {

    assert(m_sequence.front().id == -1);
    size_t cost = 0;

    for (size_t i = 1; i < m_sequence.size(); i++) {
        const Vertex prev = m_sequence[i - 1];
        const Vertex &current = m_sequence[i];

        cost += m_instance.EvalVertex(current, prev);
    }

    if (cost != m_cost) {
        std::cerr << "Correct: " << cost << ", Received: " << m_cost << '\n';
        assert(false);
    }
    return cost;
}

Solution::Solution(vector<Vertex> &&sequence, const Instance &instance)
    : m_instance(instance), m_sequence(std::move(sequence)) {
    RecalculateCost();
}

Solution::Solution(vector<Vertex> &&sequence, size_t cost, const Instance &instance)
    : m_instance(instance), m_sequence(std::move(sequence)), m_cost(cost) {

    assert(this->CorrectCost() == m_cost);
}

void Solution::ApplySwap(size_t i, size_t j) {
    std::swap(m_sequence[i], m_sequence[j]);
    RecalculateCost();
}
void Solution::ApplyReinsertion(size_t i, size_t j) {}

void Solution::RecalculateCost() {
    assert(m_sequence.front().id == -1);
    m_cost = 0;

    for (size_t i = 1; i < m_sequence.size(); i++) {
        const Vertex prev = m_sequence[i - 1];
        Vertex &current = m_sequence[i];

        m_cost += m_instance.CalculateVertex(current, prev);
    }
}
