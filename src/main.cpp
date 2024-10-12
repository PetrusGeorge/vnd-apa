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
#include <utility>
#include <vector>

using std::size_t;

int main(int argc, char *argv[]) {

    std::iostream::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::unique_ptr<argparse::ArgumentParser> args = Parse({argv, argv + argc});

    const Instance instance(args->get<std::string>("instance"));

    // Set seed if given, Doesn't work well with threads
    if (auto op = args->present<size_t>("-s")) {
        if (args->get<int>("-j") == 1) {
            rng::set_seed(*op);
        } else {
            std::cerr << "Warning: seed was ignored because multi threading is active\n";
        }
    }
    
    if (args->get<bool>("--benchmark")) {
        if (args->get<int>("-j") != 1) {
            std::cerr << "Benchmark have to be done with single thread\n";
            return 1;
        }
    }

    double total_time = 0;
    const Solution result = benchmark::timeFuncInvocation(ILS, total_time, std::move(args), instance);

    std::cout << result << '\n';
    std::cout << "Time: " << total_time << '\n';
    result.ToFile();
}
