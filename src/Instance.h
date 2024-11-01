#ifndef INSTANCE_H
#define INSTANCE_H

#include <cstddef>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

struct Vertex {
    long id;
    std::size_t finish_time;
    std::size_t penalty;

    static Vertex Departure() { return {-1, 0, 0}; }
};

class Instance {

  public:
    explicit Instance(const std::filesystem::path &filepath);

    [[nodiscard]] inline std::size_t setup_time(const Vertex &order, const Vertex &order_behind) const {
        return m_setup_times[order_behind.id + 1][order.id];
    }
    [[nodiscard]] inline std::size_t process_time(const Vertex &order) const { return m_process_times[order.id]; }
    [[nodiscard]] inline std::size_t deadline(const Vertex &order) const { return m_deadlines[order.id]; }
    [[nodiscard]] inline std::size_t weight(const Vertex &order) const { return m_weights[order.id]; }

    [[nodiscard]] inline std::size_t AddedTime(const Vertex &order,
                                               const Vertex &order_behind = Vertex::Departure()) const {
        return setup_time(order, order_behind) + process_time(order) + order_behind.finish_time;
    }
    [[nodiscard]] inline std::size_t Penalty(const Vertex &order) const {
        if (order.finish_time < deadline(order)) {
            return 0;
        }

        return (order.finish_time - deadline(order)) * weight(order);
    }

    [[nodiscard]] std::size_t EvalVertex(const Vertex &order, const Vertex &order_behind, long shift = 0) const;
    [[nodiscard]] std::pair<std::size_t, std::size_t>
    EvalVertexWithStart(const Vertex &order, const Vertex &order_behind, size_t start_time) const;
    std::size_t CalculateVertex(Vertex &order, const Vertex &order_behind = Vertex::Departure()) const;

    [[nodiscard]] inline std::size_t size() const { return m_instance_size; }
    [[nodiscard]] inline std::size_t real_size() const { return m_instance_real_size; }
    [[nodiscard]] inline const std::vector<std::size_t> &zero_weight_nodes() const { return m_zero_weight_nodes; }
    [[nodiscard]] inline const std::string &instance_name() const { return m_instance_name; }

  private:
    void SetVector(const std::string &line, std::vector<std::size_t> &fill_vector) const;

    std::size_t m_instance_size = 0;
    std::size_t m_instance_real_size = 0;
    std::string m_instance_name;
    std::vector<std::vector<std::size_t>> m_setup_times;
    std::vector<std::size_t> m_weights;
    std::vector<std::size_t> m_process_times;
    std::vector<std::size_t> m_deadlines;
    std::vector<std::size_t> m_zero_weight_nodes;
};

#endif
