#ifndef ILS_H
#define ILS_H

#include "Instance.h"
#include "Solution.h"

Solution ILS(int max_iter, int max_iter_ils, int num_threads, const Instance &instance);

#endif
