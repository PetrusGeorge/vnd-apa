#include "Instance.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using std::size_t;

void Instance::SetVector(const std::string& line, std::vector<std::size_t>& fill_vector) const {

    std::stringstream iss(line);
    std::size_t number = std::numeric_limits<size_t>::max();

    fill_vector.reserve(m_instance_size);
    while(iss >> number){
        fill_vector.emplace_back(number);
    }

    if(fill_vector.size() != m_instance_size){
        throw std::runtime_error("Failed to correctly set a vector");
    }
}

Instance::Instance(const std::filesystem::path& filename){

    std::ifstream file(filename);

    if(!file.is_open()){
        std::runtime_error("Could not open file");
    }

    // Read number of jobs
    std::string line{};
    std::getline(file, line);
    m_instance_size = std::stoi(line);

    // The order matters
    std::getline(file, line);
    SetVector(line, m_process_times);

    std::getline(file, line);
    SetVector(line, m_deadlines);

    std::getline(file, line);
    SetVector(line, m_idk);

    // Read setup times
    m_setup_times.reserve(m_instance_size);
    while(std::getline(file, line)){

        m_setup_times.emplace_back();
        SetVector(line, m_setup_times.back());
    }

    if(m_setup_times.size() != m_instance_size){
        throw std::runtime_error("Failed to correctly set a setup times");
    }
}
