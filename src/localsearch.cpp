#include "../include/localsearch.h"
#include "../include/greedy.h"

ResultMH<double> LocalSearch::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  tSolution<double> bestSolution(problem.getSolutionSize());
  std::vector<int> bestcompanies(problem.getSolutionSize());
  int company = 0;
  std::iota(bestcompanies.begin(), bestcompanies.end(), 0);
  std::sort(bestcompanies.begin(), bestcompanies.end(), [&](int i, int j) {
    return problem.heuristic(i) > problem.heuristic(j);
  });

  bestSolution.setZero();
  pair<double, double> domain = problem.getSolutionDomainRange();
  while (bestSolution.sum() < 1.0) {
    company = bestcompanies.front();
    bestcompanies.erase(bestcompanies.begin());
    bestSolution(company) = std::min(domain.second, 1 - bestSolution.sum());
  }
  return ResultMH(bestSolution, problem.fitness(bestSolution), maxevals);
}
