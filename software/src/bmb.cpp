#include "../include/bmb.h"
#include "../include/localsearch.h"
#include <limits>

ResultMH<double> Bmb::optimize(ProblemDouble &problem, int maxevals) {
  assert(maxevals > 0);
  tSolution<double> bestSolution;
  double bestFitness = -std::numeric_limits<double>::infinity();
  int numsol = 5;
  int localsearchevals = 2000;
  int evals = 0;
	LocalSearch localsearch = LocalSearch(0.4);
  for (int i = 0; i < numsol; i++) {
		tSolution<double> randsol = problem.createSolution();
		double randfitness = problem.fitness(randsol);
    ResultMH<double> localsearchSolution =
        localsearch.optimize(problem,randsol,randfitness,localsearchevals );
    evals += localsearchSolution.evaluations;
    if (localsearchSolution.fitness > bestFitness) {
      bestSolution = localsearchSolution.solution;
      bestFitness = localsearchSolution.fitness;
    }
  }

  return ResultMH(bestSolution, bestFitness, evals);
}
