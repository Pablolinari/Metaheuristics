#include "../include/randomsearch.h"
#include "../include/solution.h"
#include <limits>

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

  tSolution<double> bestSolution(problem.getSolutionSize());
	
	//valor inicial del fitness
  tFitness bestFitness = -std::numeric_limits<double>::infinity();
	//mientras no se gasten las evaluaciones , busco soluciones aleatorias
	//que vayan maximizando el fitness
  for (int i = 0; i < maxevals; i++) {
    auto temporalSolution = problem.createSolution();
		double currentFitness = problem.fitness(temporalSolution);
    if ( currentFitness> bestFitness )  {
      bestSolution = temporalSolution;
      bestFitness = currentFitness;
    }
  }

  return ResultMH(bestSolution, bestFitness, maxevals);
}
