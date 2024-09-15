#include "Instance.h"
#include "Solution.h"

#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string_view>
#include <vector>

using std::size_t;

int main(int argc, char *argv[]) {

    std::vector<std::string_view> args{argv + 1, argv + argc};

    if (args.size() != 1) {
        std::cerr << "Usage: ./path/to/bin /path/to/instance\n";
        return 1;
    }

    const Instance instance(args.front());

    const Solution teste({0, 1, 2, 3, 4}, instance);

    std::cout << teste.cost() << '\n';
}
