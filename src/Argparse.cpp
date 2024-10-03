#include "Argparse.h"
#include "argparse/argparse.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

std::unique_ptr<argparse::ArgumentParser> Parse(const std::vector<std::string> &args) {

    auto parser = std::make_unique<argparse::ArgumentParser>("Apa");

    parser->add_argument("instance").help("Instance path");

    parser->add_argument("-i", "--max-iter")
        .help("number of constructions")
        .metavar("MAX_ITER")
        .default_value(50)
        .scan<'i', int>();

    parser->add_argument("-ils", "--max-iter-ils")
        .help("number of Perturbations without improvement")
        .metavar("MAX_ITER_ILS")
        .default_value(150)
        .scan<'i', int>();

    parser->add_argument("-j", "--num-jobs")
        .help("number of threads used by ILS")
        .metavar("JOBS")
        .default_value(1)
        .scan<'i', int>();

    parser->add_argument("-s", "--seed")
        .help("set random generator seed, only aplied if number of threads is 1")
        .metavar("SEED")
        .scan<'i', std::size_t>();

    parser->add_argument("-apa").help("endless loop").default_value(false).flag();

    try {
        parser->parse_args(args);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << '\n';
        exit(EXIT_FAILURE);
    }

    return parser;
}
