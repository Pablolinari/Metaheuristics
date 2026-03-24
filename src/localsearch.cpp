#include "../include/localsearch.h"
#include "../include/greedy.h"
#include "../include/random.hpp"
#include <iostream>
#include <utility>

ResultMH<double> LocalSearch::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  double ratio = 0.4;
  Greedy greedyAlg;
  tSolution<double> startSolution = greedyAlg.optimize(problem, 1).solution;
  double bestFitness = problem.fitness(startSolution);
  bool improved = true;
  std::vector<std::pair<int, int>> neighbors;

  for (int i = 0; i < problem.getSolutionSize(); i++) {
    for (int j = 0; j < problem.getSolutionSize(); j++) {
      neighbors.emplace_back(make_pair(i, j));
    }
  }
  int evals = 0;
  double amount = 0;
  while (evals < maxevals && improved) {
    improved = false;
    Random::shuffle(neighbors);
    for (std::pair<int, int> p : neighbors) {
      amount = ratio * startSolution(p.first);
      startSolution(p.second) += amount;
      startSolution(p.first) -= amount;

      if (problem.fitness(startSolution) > bestFitness &&
          problem.isValid(startSolution)) {
        bestFitness = problem.fitness(startSolution);
        improved = true;
        break;

      } else {
        startSolution(p.first) += amount;
        startSolution(p.second) -= amount;
      }
    }

    evals++;
  }
  return ResultMH(startSolution, problem.fitness(startSolution), evals);
}
