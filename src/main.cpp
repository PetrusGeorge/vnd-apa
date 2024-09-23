#include "ILS.h"
#include "Instance.h"
#include "Solution.h"

#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string_view>
#include <vector>

using std::size_t;

int main(int argc, char *argv[]) {

    std::iostream::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::vector<std::string_view> args{argv + 1, argv + argc};

    if (args.empty() || args.size() > 2) {
        std::cerr << "Usage: ./path/to/bin /path/to/instance optional:<seed>\n";
        return 1;
    }

    const Instance instance(args.front());

    const Solution result = ILS(50, 150, 8, instance);

    std::cout << result << '\n';
}
