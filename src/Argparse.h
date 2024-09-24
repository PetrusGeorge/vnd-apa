#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <argparse/argparse.hpp>
#include <memory>
#include <string>
#include <vector>

std::unique_ptr<argparse::ArgumentParser> Parse(const std::vector<std::string> &args);

#endif
