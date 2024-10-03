#ifndef ILS_H
#define ILS_H

#include <memory>

#include "Instance.h"
#include "Solution.h"
#include "argparse/argparse.hpp"

Solution ILS(std::unique_ptr<argparse::ArgumentParser> args, const Instance &instance);

#endif
