#include "../include/randomsearch.h"
#include "../include/pportfolio.h"
#include "../include/solution.h"
#include <vector>

/**
 * Create random solutions until maxevals has been achieved, and returns the
 * best one.
 *
 * @param problem The problem to be optimized
 * @param maxevals Maximum number of evaluations allowed
 * @return A pair containing the best solution found and its fitness
 */
ResultMHDouble RandomSearch::optimize(ProblemDouble &problem, int maxevals) {
  assert(maxevals > 0);

  tSolution<double> temporalSolution;
  tSolution<double> bestSolution;
  tFitness bestFitness = 0.0;

  for (size_t i = 0; i < maxevals; i++) {
    temporalSolution = problem.createSolution();
    if (problem.fitness(temporalSolution) > bestFitness) {
      bestSolution = temporalSolution;
      bestFitness = problem.fitness(temporalSolution);
    }
  }

  return ResultMH(bestSolution, bestFitness, maxevals);
}
