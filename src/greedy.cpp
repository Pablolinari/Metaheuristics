#include "../include/greedy.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

ResultMH<double> Greedy::optimize(Problem<double> &problem, int maxevals) {

  assert(maxevals > 0);

  tSolution<double> temporalSolution;
  tSolution<double> bestSolution;
  std::vector<int> bestcompanies(problem.getSolutionSize());
  double bestFitness = -std::numeric_limits<double>::infinity();
  int company=0;

  std::sort(bestcompanies.begin(), bestcompanies.end(), [&](int i, int j) {
    return problem.heuristic(i) > problem.heuristic(j);
  });

  temporalSolution.setZero();
  bestSolution.setZero();

  pair<double, double> domain = problem.getSolutionDomainRange();
  for (int i = 0; i < maxevals; i++) {
    while (temporalSolution.sum() < 1) {
      company = bestcompanies.front();
      bestcompanies.erase(bestcompanies.begin());
      bestSolution(company) = std::min(domain.second, 1 - bestSolution.sum());
    }
    if (problem.fitness(temporalSolution) > bestFitness) {
      bestFitness = problem.fitness(temporalSolution);
      bestSolution = temporalSolution;
    }
  }
  return ResultMH(bestSolution, bestFitness, maxevals);
}
