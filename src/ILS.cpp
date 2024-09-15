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

using pair_vertex_it = std::pair<Vertex &, list<Vertex>::iterator>;

[[nodiscard]] Vertex ChooseBestVertex(vector<Vertex> &CL, const Vertex &before, const Instance &instance) {
    for (auto &vertex : CL) {
        instance.CalculateVertex(vertex, before);
    }

    // Decresent order, the last will be the best one
    auto criteria = [instance](const Vertex &first, const Vertex &second) {
        return first.penalty < second.penalty ||
               instance.deadline(first) > instance.deadline(second);
    };

    std::sort(CL.begin(), CL.end(), criteria);

    Vertex best = CL.back();
    CL.pop_back();
    return best;
}

Solution Construction(const Instance &instance) {
    vector<Vertex> sequence;
    size_t cost = 0;

    vector<Vertex> CL;
    CL.reserve(instance.size());
    for (size_t i = 0; i < instance.size(); i++) {
        CL.emplace_back(i, 0, 0);
    }

    // TODO: aleatorio
    sequence.emplace_back(CL.front());
    CL.erase(CL.begin());
    cost = instance.CalculateVertex(sequence.back());

    while (!CL.empty()) {
        sequence.emplace_back(ChooseBestVertex(CL, sequence.back(), instance));
        cost += sequence.back().penalty;
    }

    return {std::move(sequence), cost, instance};
}

Solution ILS(int max_iter, int max_iter_ils, const Instance &instance) { return Construction(instance); }
