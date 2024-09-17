#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include "Instance.h"
#include "Solution.h"

void LocalSearch(Solution &s, const Instance &instance);
Solution Pertubation(Solution best, const Instance &instance);

#endif // !LOCAL_SEARCH_H
