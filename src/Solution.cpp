#include "Solution.h"
#include "Instance.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using std::size_t;
using std::vector;

namespace rv = std::ranges::views;

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
    for (const auto &order : sol.sequence | rv::drop(1)) {
        os << std::setw(id_width) << order.id << std::setw(finish_time_width) << order.finish_time
           << std::setw(penalty_width) << order.penalty << '\n';
    }

    os << "\nFinal cost: " << sol.cost() << '\n';
    return os;
}

void Solution::PrintLBW() {
    std::cout << *this;
    std::cout << "lbw\n";
    for (const size_t lbw : m_lbw) {

        std::cout << lbw << '\n';
    }
}

void Solution::ToFile() const {
    std::filesystem::create_directory("solution");

    auto now = std::chrono::system_clock::now();

    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream datetime;
    datetime << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    auto ms = duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    datetime << ':' << std::setfill('0') << std::setw(3) << ms.count();

    std::string date = datetime.str();
    std::replace(date.begin(), date.end(), ' ', '_');
    const std::filesystem::path teste(m_instance.instance_name());
    std::ofstream file("solution/" + teste.stem().string() + "-" + date + ".txt");
    for (const auto &order : m_sequence | rv::take(m_sequence.size() - 1) | rv::drop(1)) {
        file << order.id + 1 << ',';
    }
    file << m_sequence.back().id + 1 << '\n';
    file << m_cost << '\n';
}

Solution::Solution(const Instance &instance) : m_instance(instance) {}

Solution::Solution(const Solution &other)
    : m_instance(other.m_instance), m_sequence(other.m_sequence), m_lbw(other.m_lbw), m_cost(other.cost()) {

    assert(DebugCost() == m_cost);
}

Solution::Solution(Solution &&other) noexcept
    : m_instance(other.m_instance), m_sequence(std::move(other.m_sequence)), m_lbw(std::move(other.m_lbw)),
      m_cost(other.cost()) {

    assert(DebugCost() == m_cost);
}

Solution &Solution::operator=(const Solution &other) {
    m_sequence = other.m_sequence;
    m_lbw = other.m_lbw;
    m_cost = other.cost();

    assert(DebugCost() == m_cost);

    return *this;
}

Solution &Solution::operator=(Solution &&other) noexcept {
    m_sequence = std::move(other.m_sequence);
    m_lbw = std::move(other.m_lbw);
    m_cost = other.cost();
    assert(DebugCost() == m_cost);

    return *this;
}

size_t Solution::DebugCost() {

    assert(m_sequence.front().id == -1);
    size_t sum_cost = 0;

    for (size_t i = 1; i < m_sequence.size(); i++) {
        const Vertex prev = m_sequence[i - 1];
        const Vertex current = m_sequence[i];

        sum_cost += m_instance.EvalVertex(current, prev);
    }

    if (sum_cost != m_cost) {
        std::cerr << "Correct: " << sum_cost << ", Received: " << m_cost << '\n';
        assert(false);
    }
    return sum_cost;
}

Solution::Solution(vector<Vertex> &&sequence, const Instance &instance)
    : m_instance(instance), m_sequence(std::move(sequence)) {
    RecalculateCost();
}

Solution::Solution(vector<Vertex> &&sequence, size_t cost, const Instance &instance)
    : m_instance(instance), m_sequence(std::move(sequence)), m_cost(cost) {

    UpdateLBW();
    assert(DebugCost() == m_cost);
}

void Solution::ApplySwap(size_t i, size_t j) {
    assert(i != 0 || j != 0);
    std::swap(m_sequence[i], m_sequence[j]);
    RecalculateCost();
}

void Solution::ApplyReinsertion(size_t i, size_t j, size_t block_size) {

    const long i_index = static_cast<long>(i);
    const long j_index = static_cast<long>(j);
    const long block = static_cast<long>(block_size);

    if (i < j) {
        std::rotate(m_sequence.begin() + i_index, m_sequence.begin() + i_index + block,
                    m_sequence.begin() + j_index + 1);
    } else {
        std::rotate(m_sequence.begin() + j_index, m_sequence.begin() + i_index, m_sequence.begin() + i_index + block);
    }

    RecalculateCost();
}

void Solution::ApplyDoubleBridge(long i, long j, long block_size_i, long block_size_j) {

    std::rotate(m_sequence.begin() + i, m_sequence.begin() + j, m_sequence.begin() + j + block_size_j);
    std::rotate(m_sequence.begin() + i + block_size_j, m_sequence.begin() + i + block_size_i + block_size_j,
                m_sequence.begin() + j + block_size_j);
    RecalculateCost();
}

void Solution::UpdateLBW() {

    std::vector<long> lbw(m_instance.size() + 1);

    if (m_sequence.back().penalty > 0) {
        lbw.back() = static_cast<long>(m_instance.weight(m_sequence.back()));
    } else {
        lbw.back() = 0;
    }

    for (long i = static_cast<long>(lbw.size()) - 2; i >= 0; i--) {
        long w = lbw[i + 1];

        if (m_sequence[i].penalty > 0) {
            w += static_cast<long>(m_instance.weight(m_sequence[i]));
        }
        lbw[i] = w;
    }

    m_lbw = std::move(lbw);
}

void Solution::RecalculateCost() {
    assert(m_sequence.front().id == -1);
    m_cost = 0;

    for (size_t i = 1; i < m_sequence.size(); i++) {
        const Vertex prev = m_sequence[i - 1];
        Vertex &current = m_sequence[i];

        m_cost += m_instance.CalculateVertex(current, prev);
    }
    UpdateLBW();
}
