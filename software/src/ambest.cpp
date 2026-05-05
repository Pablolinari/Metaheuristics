
#include "../include/ambest.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <vector>
AmBest::AmBest(int populationSize, double crossoverProb, double mutationProb,
               int tournamentSize)
    : Am(populationSize, crossoverProb, mutationProb) {}

int AmBest::applyLocalSearch(std::vector<tSolution<double>> &poblation,
                             std::vector<double> &fitnessPoblation,
                             double &bestFitness,
                             tSolution<double> &bestSolution,
                             Problem<double> &problem) {
  int evals = 0;
  int nBest = std::ceil(this->populationSize * 0.1);
  std::vector<int> indices(this->populationSize);
  std::iota(indices.begin(), indices.end(), 0);
  /// implementar best
  /// busco los indices de las diez mejores soluciones
  std::nth_element(indices.begin(), indices.begin() + nBest, indices.end(),
                   [&fitnessPoblation](int i, int j) {
                     return fitnessPoblation[i] > fitnessPoblation[j];
                   });
  indices.resize(nBest);
  for (int i : indices) {
    ResultMH<double> result = localSearch(problem, 100, poblation[i]);
    evals += 100;
    if (result.fitness > bestFitness) {
      bestFitness = result.fitness;
      bestSolution = result.solution;
      poblation[i] = result.solution;
    }
  }
  return evals;
}
