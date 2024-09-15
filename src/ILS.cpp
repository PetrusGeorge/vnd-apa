// TODO:
//  Solution ILS(int max_iter, int max_iter_ils, const Instance& instance){}

#include "ILS.h"
#include "Instance.h"
#include "Solution.h"
#include <algorithm>
#include <cstddef>
#include <list>

using std::list;
using std::size_t;
using std::vector;

using it_vec = std::pair<Vertex&, list<Vertex>::iterator>;

auto ChooseBestVertex(list<Vertex> &CL, const Vertex &before, const Instance &instance) {

    for (auto &vertex : CL) {

        instance.CalculateVertex(vertex, before);
    }

    auto teste = [instance](const it_vec &first, const it_vec &second) {
        const auto &first_order = first.first;
        const auto &second_order = second.first;
        return first_order.penalty > second_order.penalty || instance.deadline(first_order) < instance.deadline(second_order);
    };

    vector<std::pair<Vertex&, list<Vertex>::iterator>> vec;
    vec.reserve(CL.size());
    for (auto it = CL.begin(); it != CL.end(); it++) {
        vec.emplace_back(*it, it);
    }
    std::sort(vec.begin(), vec.end(), teste);

    return vec.front().second;
}

Solution Construction(const Instance &instance) {

    vector<Vertex> sequence;
    size_t cost = 0;

    list<Vertex> CL;
    for (size_t i = 0; i < instance.size(); i++) {
        CL.emplace_back(i, 0, 0);
    }

    // TODO: aleatorio
    sequence.emplace_back(CL.front());
    CL.pop_front();
    cost = instance.CalculateVertex(sequence.back());

    while (!CL.empty()) {
        auto it = ChooseBestVertex(CL, sequence.back(), instance);
        sequence.emplace_back(*it);
        cost += it->penalty;
        CL.erase(it);
    }

    return {std::move(sequence), cost, instance};
}

Solution ILS(int max_iter, int max_iter_ils, const Instance &instance) { return Construction(instance); }
