#include "randomsearch.h"

/**
 * Create random solutions until maxevals has been achieved, and returns the
 * best one.
 *
 * @param problem The problem to be optimized
 * @param maxevals Maximum number of evaluations allowed
 * @return A pair containing the best solution found and its fitness
 */
ResultMHInt RandomSearch::optimize(ProblemDouble &problem, int maxevals) {
  assert(maxevals > 0);
  vector<bool> values;
  
  return ResultMH(sol, fitness, 1);
}
