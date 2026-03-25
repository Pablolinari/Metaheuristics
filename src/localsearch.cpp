#include "../include/localsearch.h"
#include "../include/random.hpp"
#include <iostream>
#include <utility>

ResultMH<double> LocalSearch::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  double ratio = 0.4;
  tSolution<double> startSolution = problem.createSolution();
  double bestFitness = problem.fitness(startSolution);
  bool improved = true;
  std::vector<std::pair<int, int>> neighbors;

  for (int i = 0; i < problem.getSolutionSize(); i++) {
    for (int j = 0; j < problem.getSolutionSize(); j++) {
      if (i != j) {
        neighbors.emplace_back(make_pair(i, j));
      }
    }
  }
  int evals = 0;
  double amount = 0;
  while (evals < maxevals) {
    improved = false;
    Random::shuffle(neighbors);
    for (std::pair<int, int> p : neighbors) {
      amount = startSolution(p.first) * ratio;
      startSolution(p.first) -= amount;
      startSolution(p.second) += amount;

      double currentFitness = problem.fitness(startSolution);
      evals++;
      if (currentFitness> bestFitness &&
          problem.isValid(startSolution)) {

        bestFitness =currentFitness; 

        improved = true;
        break;
      } else {
        startSolution(p.first) += amount;
        startSolution(p.second) -= amount;
      }
      if (evals == maxevals) {
        break;
      }
    }
  }
  return ResultMH(startSolution, problem.fitness(startSolution), evals);
}
