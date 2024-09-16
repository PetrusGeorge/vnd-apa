#include "LocalSearch.h"
#include "Util.h"

using std::size_t;

enum class Searchs { SWAP, REINSERTION };

inline long EvalSwap(const Solution &s, size_t i, size_t j) {
    long delta = 0;
    // size_t start_time =  s.sequence[i-1].finish_time;
    // delta += eval(start_time(i), i+1, j-1);
    // delta += eval(start_time(j), j + 1, s.size() - 1);
    return delta;
}

bool Swap(Solution &s) {

    long best_delta = 0;
    size_t best_i = std::numeric_limits<size_t>::max();
    size_t best_j = std::numeric_limits<size_t>::max();

    for (size_t i = 0; i < s.sequence.size() - 1; i++) {

        for (size_t j = i + 2; j < s.sequence.size(); j++) {
            long delta = EvalSwap(s, i, j);

            if (delta < best_delta) {
                best_i = i;
                best_j = j;
                best_delta = delta;
            }
        }
    }
    for (size_t i = 0; i < s.sequence.size(); i++) {
        size_t j = i + 1;
        long delta = EvalSwap(s, i, j);

        if (delta < best_delta) {
            best_i = i;
            best_j = j;
            best_delta = delta;
        }
    }

    if (best_delta < 0) {
        s.ApplySwap(best_i, best_j);
        return true;
    }

    return false;
}

bool Reinsertion(Solution & /*s*/, size_t /*block_size*/) { return false; }

void LocalSearch(Solution &s) {
    std::vector searchs = {Searchs::SWAP, Searchs::REINSERTION};
    while (!searchs.empty()) {
        bool improved = false;
        auto chose = my_rand::choose_random_value(searchs.begin(), searchs.end());
        switch (*chose) {
        case Searchs::SWAP:
            improved = Swap(s);
            break;
        // case Searchs::REINSERTION:
        //     improved = Reinsertion(s, 1);
        //     break;
        }

        if (improved) {
            searchs = {Searchs::SWAP, Searchs::REINSERTION};
            continue;
        }
        searchs.erase(chose);
    }
}

Solution Pertubation(Solution best) { return best; }
