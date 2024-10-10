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

    const Solution result = ILS(std::move(args), instance);

    std::cout << result << '\n';
    result.ToFile();
}
