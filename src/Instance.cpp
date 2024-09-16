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
#include <vector>

using std::size_t;
using std::vector;

namespace rv = std::ranges::views;

void Instance::SetVector(const std::string &line, vector<std::size_t> &fill_vector) const {

    std::stringstream iss(line);
    size_t number = std::numeric_limits<size_t>::max();

    fill_vector.reserve(m_instance_size);
    while (iss >> number) {
        fill_vector.emplace_back(number);
    }

    if (fill_vector.size() != m_instance_size) {
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

Instance::Instance(const std::filesystem::path &filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    // Read number of jobs
    std::string line{};
    std::getline(file, line);
    m_instance_size = std::stoi(line);

    // The order matters
    const vector<std::reference_wrapper<vector<size_t>>> vecs = {m_process_times, m_deadlines, m_weights};

    for (auto vec : vecs) {
        GetNextLine(file, line);
        SetVector(line, vec);
    }

    // Create a line of 0's for indexing with -1
    m_setup_times.reserve(m_instance_size + 1);
    m_setup_times.emplace_back(m_instance_size, 0);

    // Read setup times
    while (GetNextLine(file, line)) {
        m_setup_times.emplace_back();
        SetVector(line, m_setup_times.back());
    }

    if (m_setup_times.size() != m_instance_size + 1) {
        throw std::runtime_error("Failed to correctly set a setup times");
    }
}

size_t Instance::CalculateVertex(Vertex &order, const Vertex &order_behind) const {

    AddedTime(order, order_behind);
    return Penalty(order);
}
