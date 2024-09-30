#include "Argparse.h"
#include "ILS.h"
#include "Instance.h"
#include "Solution.h"
#include "Util.h"
#include "argparse/argparse.hpp"

#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using std::size_t;

int main(int argc, char *argv[]) {

    std::iostream::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::unique_ptr<argparse::ArgumentParser> parser = Parse({argv, argv + argc});

    const Instance instance(parser->get<std::string>("instance"));

    const int max_iter = parser->get<int>("-i");
    const int max_iter_ils = parser->get<int>("-ils");
    const int num_threads = parser->get<int>("-j");

    // Set seed if given, Doesn't work well with threads
    if (auto op = parser->present<size_t>("-s")) {
        if (num_threads == 1) {
            rng::set_seed(*op);
        } else {
            std::cerr << "Warning: seed was ignored because multi threading is active\n";
        }
    }

    const Solution result = ILS(max_iter, max_iter_ils, num_threads, instance);

    // std::cout << result << '\n';
    result.ToFile();
}
