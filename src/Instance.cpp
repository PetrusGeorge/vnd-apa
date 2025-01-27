#include "Instance.h"

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using std::size_t;
using std::vector;

namespace rv = std::ranges::views;

void Instance::SetVector(const std::string &line, vector<std::size_t> &fill_vector) const {

    std::stringstream iss(line);
    size_t number = std::numeric_limits<size_t>::max();

    fill_vector.reserve(m_instance_real_size);
    while (iss >> number) {
        fill_vector.emplace_back(number);
    }

    if (fill_vector.size() != m_instance_real_size) {
        throw std::runtime_error("Failed to correctly set a vector");
    }
}

bool GetNextLine(std::ifstream &file, std::string &line) {
    while (true) {
        if (!std::getline(file, line)) {
            return false;
        }

        auto filter_spaces = [](char c) { return std::isspace(static_cast<unsigned char>(c)) == 0; };

        auto filtered = line | rv::filter(filter_spaces);

        if (!filtered.empty()) {
            break;
        }
    }
    return true;
}

Instance::Instance(const std::filesystem::path &filepath) : m_instance_name(filepath.filename()) {
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    // Read number of jobs
    std::string line{};
    std::getline(file, line);
    m_instance_real_size = std::stoi(line);

    // The order matters
    const vector<std::reference_wrapper<vector<size_t>>> vecs = {m_process_times, m_deadlines, m_weights};

    for (auto vec : vecs) {
        GetNextLine(file, line);
        SetVector(line, vec);
    }

    for(size_t i = 0; i < m_weights.size(); i++){
        if(m_weights[i] == 0){
            m_zero_weight_nodes.emplace_back(i);
        }
    }
    m_instance_size = m_instance_real_size - m_zero_weight_nodes.size();

    m_setup_times.reserve(m_instance_real_size + 1);

    // Read setup times
    while (GetNextLine(file, line)) {
        m_setup_times.emplace_back();
        SetVector(line, m_setup_times.back());
    }

    if (m_setup_times.size() != m_instance_real_size + 1) {
        throw std::runtime_error("Failed to correctly set a setup times");
    }
}

std::pair<size_t, size_t> Instance::EvalVertexWithStart(const Vertex &order, const Vertex &order_behind,
                                                        size_t start_time) const {

    const size_t finish_time = setup_time(order, order_behind) + process_time(order) + start_time;

    if (finish_time < deadline(order)) {
        return {0, finish_time};
    }

    return {(finish_time - deadline(order)) * weight(order), finish_time};
}

size_t Instance::EvalVertex(const Vertex &order, const Vertex &order_behind, long shift) const {

    const size_t finish_time = setup_time(order, order_behind) + process_time(order) + order_behind.finish_time + shift;

    if (finish_time < deadline(order)) {
        return 0;
    }

    return (finish_time - deadline(order)) * weight(order);
}

size_t Instance::CalculateVertex(Vertex &order, const Vertex &order_behind) const {

    order.finish_time = AddedTime(order, order_behind);
    order.penalty = Penalty(order);
    return order.penalty;
}
