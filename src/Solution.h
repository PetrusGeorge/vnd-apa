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
    explicit Solution(const Instance &instance);
    Solution(Solution &&) noexcept;
    Solution &operator=(const Solution &);
    Solution &operator=(Solution &&) noexcept;
    Solution(const Solution &other);
    ~Solution() = default;

    Solution(std::vector<Vertex> &&sequence, const Instance &instance);
    Solution(std::vector<Vertex> &&sequence, std::size_t cost, const Instance &instance);

    [[nodiscard]] inline std::size_t cost() const { return m_cost; }

    void ApplySwap(size_t i, size_t j);
    // TODO:
    void ApplyReinsertion(size_t i, size_t j);

    // NOTE: read only access to private member
    const std::vector<Vertex> &sequence = m_sequence;
    std::size_t DebugCost();

  private:
    void RecalculateCost();

    const Instance &m_instance;
    std::vector<Vertex> m_sequence;
    std::size_t m_cost = std::numeric_limits<std::size_t>::max();
};
std::ostream &operator<<(std::ostream &os, const Solution &sol);

#endif
