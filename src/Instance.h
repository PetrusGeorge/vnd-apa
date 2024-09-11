#ifndef INSTANCE_H
#define INSTANCE_H

#include <cstddef>
#include <filesystem>
#include <vector>

class Instance {

  public:
    explicit Instance(const std::filesystem::path& filename);

  private:

    void SetVector(const std::string& line, std::vector<std::size_t>& fill_vector) const;

    std::size_t m_instance_size = -1;
    std::vector<std::vector<std::size_t>> m_setup_times;
    std::vector<std::size_t> m_idk;
    std::vector<std::size_t> m_process_times;
    std::vector<std::size_t> m_deadlines;
};

#endif
