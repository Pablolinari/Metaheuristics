
#include "../include/amrand.h"
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

AmRand::AmRand(int populationSize, double crossoverProb, double mutationProb,
               int tournamentSize)
    : Am(populationSize, crossoverProb, mutationProb) {}

int AmRand::applyLocalSearch(std::vector<tSolution<double>> &poblation,
                             std::vector<double> &fitnesspoblation,
                             double bestFitness,
                             tSolution<double> &bestsolution,
                             Problem<double> &problem) {
  double nRand = this->populationSize * 0.10;
  int evals = 0;
  for (int i = 0; i < nRand; i++) {
    ResultMH<double> result = localSearch(problem, 100, poblation[i]);
    evals += 100;
    if (result.fitness > fitnesspoblation[i]) {
      fitnesspoblation[i] = result.fitness;
      poblation[i] = result.solution;
    }
    if (result.fitness > bestFitness) {
      bestFitness = result.fitness;
      bestsolution = result.solution;
    }
  }
  return evals;
}
