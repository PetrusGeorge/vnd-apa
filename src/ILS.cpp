#include "ILS.h"
#include "Instance.h"
#include "LocalSearch.h"
#include "Solution.h"
#include "Util.h"
#include "argparse/argparse.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

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

    const long r = ceil(parameters::R_MAX * static_cast<double>(CL.size()));
    const long range = rng::rand_int(static_cast<long>(1), r);

    auto chose_it = rng::pick_iter(CL.end() - range, CL.end());
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
    for (size_t i = 0; i < instance.real_size(); i++) {
        if (instance.weight({static_cast<long>(i), 0, 0}) == 0) {
            continue;
        }
        CL.emplace_back(i, 0, 0);
    }

    sequence.emplace_back(Vertex::Departure());
    while (!CL.empty()) {
        sequence.emplace_back(ChooseBestVertex(CL, sequence.back(), instance));
        cost += sequence.back().penalty;
    }

    return {std::move(sequence), cost, instance};
}

Solution ILS(std::unique_ptr<argparse::ArgumentParser> args, const Instance &instance) {

    const int max_iter = args->get<int>("-i");
    const int max_iter_ils = args->get<int>("-ils");
    const int num_threads = args->get<int>("-j");
    bool endless = args->get<bool>("-apa");
    bool benchmark = args->get<bool>("--benchmark");

    std::atomic<int> iter = 0;
    std::mutex mtx;
    Solution best_of_all(instance);
    auto best_of_all_cost = args->get<size_t>("--bks");

    double mean_time_construction = 0;
    std::size_t mean_cost_construction = 0;

    auto ils_lambda = [&]() {
        while (true) {
            if (iter++ >= max_iter && !endless) {
                break;
            }

            Solution best(instance);
            double time_construction = 0;
            Solution s = benchmark::timeFuncInvocation(Construction, time_construction, instance);
            // This doesn't work correctly with multithreading, but this variable should never be used in this case
            mean_time_construction += time_construction / max_iter;
            mean_cost_construction += s.cost() / max_iter;

            for (int iter_ils = 0; iter_ils < max_iter_ils; iter_ils++) {
                LocalSearch(s, instance);

                if (s.cost() < best.cost()) {
                    best = s;
                    iter_ils = -1;
                }

                s = Pertubation(best);
            }
            const std::lock_guard<std::mutex> lock(mtx);
            if (best.cost() < best_of_all_cost) {
                best_of_all = std::move(best);
                best_of_all_cost = best_of_all.cost();
                if (args->get<bool>("-apa")) {
                    best_of_all.ToFile();
                }
                if (best_of_all.cost() == 0) {
                    iter = max_iter + 1;
                    endless = false;
                    break;
                }
            }
        }
    };

    if (num_threads == 1) {
        ils_lambda();
        if (benchmark) {
            std::cout << "Construction mean time: " << mean_time_construction << '\n';
            std::cout << "Construction mean value: " << mean_cost_construction << '\n';
        }
        return best_of_all;
    }

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(ils_lambda);
    }

    for (auto &t : threads) {
        t.join();
    }

    return best_of_all;
}
