#ifndef SOLUTION_H
#define SOLUTION_H

#include "Instance.h"

#include <cassert>
#include <cstddef>
#include <limits>
#include <ostream>
#include <utility>
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

    void ApplySwap(std::size_t i, std::size_t j);
    void ApplyReinsertion(std::size_t i, std::size_t j, std::size_t block_size);
    void ApplyDoubleBridge(long i, long j, long block_size_i, long block_size_j);

    // NOTE: read only access to private member
    const std::vector<Vertex> &sequence = m_sequence;
    const std::vector<std::pair<long, long>> &lbw = m_lbw;
    std::size_t DebugCost();
    void PrintLBW();

  private:
    void RecalculateCost();
    void UpdateLBW();

    const Instance &m_instance;
    std::vector<Vertex> m_sequence;
    std::vector<std::pair<long, long>> m_lbw;
    std::size_t m_cost = std::numeric_limits<std::size_t>::max();
};
std::ostream &operator<<(std::ostream &os, const Solution &sol);

#endif
