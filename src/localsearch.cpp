#include "../include/localsearch.h"
#include "../include/greedy.h"
#include "../include/random.hpp"
#include <utility>

ResultMH<double> LocalSearch::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  double ratio = 0.4;
  Greedy greedyAlg;
  tSolution<double> startSolution = greedyAlg.optimize(problem, 1).solution;
  tSolution<double> bestSolution = startSolution;
  double bestFitness = problem.fitness(bestSolution);
  std::vector<std::pair<int, int>> neighbors;


  for (int i = 0; i < problem.getSolutionSize(); i++) {
    for (int j = 0; j < problem.getSolutionSize(); j++) {
      neighbors.emplace_back(make_pair(i, j));
    }
  }
  Random::shuffle(neighbors);

  for (std::pair<int, int> p : neighbors) {
    startSolution(p.first) *= (1 - ratio);
    startSolution(p.second) += (ratio * startSolution(p.first));
    if (problem.fitness(startSolution) > bestFitness) {
      bestSolution = startSolution;
      bestFitness = problem.fitness(bestSolution);
      Random::shuffle(neighbors);
    }
  }

  return ResultMH(bestSolution, problem.fitness(bestSolution), maxevals);
}
