#include "../include/localsearch.h"
#include "../include/random.hpp"
#include <iostream>
#include <ostream>
#include <utility>

ResultMH<double> LocalSearch::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals >=1);
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
  while (evals < maxevals && improved) {
    improved = false;
    Random::shuffle(neighbors);
    for (std::pair<int, int> p : neighbors) {
      double amount = startSolution(p.first) * ratio;
			double originalFirst = startSolution(p.first);
			double originalSecond = startSolution(p.second);
      startSolution(p.first) -= amount;
      startSolution(p.second) += amount;

      if (problem.isValid(startSolution)) {
        double currentFitness = problem.fitness(startSolution);
        evals++;
        if (currentFitness > bestFitness) {

          bestFitness = currentFitness;

          improved = true;
          break;
        }
      }
      startSolution(p.first) =originalFirst;
      startSolution(p.second) =originalSecond;
      
      if (evals == maxevals) {
        break;
      }
    }
  }
  return ResultMH(startSolution, bestFitness, evals);
}

