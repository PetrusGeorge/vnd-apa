#include "ILS.h"
#include "Instance.h"
#include "Solution.h"
#include "Util.h"

#include <algorithm>
#include <cstddef>
#include <utility>

using std::size_t;
using std::vector;

[[nodiscard]] Vertex ChooseBestVertex(vector<Vertex> &CL, const Vertex &before, const Instance &instance) {
    for (auto &vertex : CL) {
        instance.CalculateVertex(vertex, before);
    }

    auto criteria = [instance](const Vertex &first, const Vertex &second) {
        return first.penalty > second.penalty ||
               (first.penalty == second.penalty && instance.deadline(first) < instance.deadline(second));
    };

    // Decresent order, the last will be the best one
    std::sort(CL.rbegin(), CL.rend(), criteria);

    long r = ceil(parameters::R_MAX * static_cast<double>(CL.size()));
    long range = my_rand::rand_int(static_cast<long>(1), r);

    auto chose_it = my_rand::choose_random_value(CL.end() - range, CL.end());
    Vertex best = *chose_it;
    CL.erase(chose_it);
    return best;
}

Solution Construction(const Instance &instance) {
    vector<Vertex> sequence;
    sequence.reserve(instance.size());

    size_t cost = 0;

    vector<Vertex> CL;
    CL.reserve(instance.size());
    for (size_t i = 0; i < instance.size(); i++) {
        CL.emplace_back(i, 0, 0);
    }

    sequence.emplace_back(ChooseBestVertex(CL, Vertex::Departure(), instance));
    cost += sequence.back().penalty;
    while (!CL.empty()) {
        sequence.emplace_back(ChooseBestVertex(CL, sequence.back(), instance));
        cost += sequence.back().penalty;
    }

    return {std::move(sequence), cost, instance};
}

Solution ILS(int /*max_iter*/, int /*max_iter_ils*/, const Instance &instance) { return Construction(instance); }
