#ifndef SOLUTION_H
#define SOLUTION_H

#include "Instance.h"

#include <cassert>
#include <cstddef>
#include <limits>
#include <ostream>
#include <vector>

class Solution {

  public:
    // Probably don't need constructors that copy the sequence
    // inline Solution(const std::vector<std::size_t> &sequence, const Instance &instance);
    // inline Solution(const std::vector<std::size_t> &sequence, std::size_t cost, const Instance &instance);

    Solution(std::vector<Vertex> &&sequence, const Instance &instance);
    Solution(std::vector<Vertex> &&sequence, std::size_t cost, const Instance &instance);

    [[nodiscard]] inline std::size_t cost() const { return m_cost; }

    // TODO:
    void ApplySwap();
    void ApplyReinsertion();

    // NOTE: read only access to private member
    const std::vector<Vertex> &sequence = m_sequence;
    friend std::ostream &operator<<(std::ostream &os, const Solution &sol);

  private:
    // TODO:
    void RecalculateCost();
    bool CorrectCost(size_t old);

    const Instance &m_instance;
    std::vector<Vertex> m_sequence;
    std::size_t m_cost = std::numeric_limits<std::size_t>::max();
};

#endif
