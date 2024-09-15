#ifndef INSTANCE_H
#define INSTANCE_H

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

class Instance {

  public:
    explicit Instance(const std::filesystem::path &filename);

    [[nodiscard]] inline std::size_t setup_time(std::size_t id, std::size_t id_behind) const {
        return m_setup_times[id_behind + 1][id];
    }
    [[nodiscard]] inline std::size_t process_time(std::size_t id) const { return m_process_times[id]; }
    [[nodiscard]] inline std::size_t deadline(std::size_t id) const { return m_deadlines[id]; }
    [[nodiscard]] inline std::size_t weight(std::size_t id) const { return m_weights[id]; }

    [[nodiscard]] inline std::size_t AddedTime(std::size_t id, std::size_t id_behind) const {
        return setup_time(id, id_behind) + process_time(id);
    }
    [[nodiscard]] inline std::size_t Penalty(std::size_t id, std::size_t finish_time) const {
        if (finish_time < deadline(id)) {
            return 0;
        }
        return (finish_time - deadline(id)) * weight(id);
    }

  private:
    void SetVector(const std::string &line, std::vector<std::size_t> &fill_vector) const;

    std::size_t m_instance_size = 0;
    std::vector<std::vector<std::size_t>> m_setup_times;
    std::vector<std::size_t> m_weights;
    std::vector<std::size_t> m_process_times;
    std::vector<std::size_t> m_deadlines;
};

#endif
