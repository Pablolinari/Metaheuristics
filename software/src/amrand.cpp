
#include "../include/amrand.h"
#include <cassert>
#include <cmath>
#include <vector>

AmRand::AmRand(int populationSize, double crossoverProb, double mutationProb,
               int tournamentSize)
    : Am(populationSize, crossoverProb, mutationProb) {}

 void AmRand::applyLocalSearch(std::vector<tSolution<double>> &poblation,
                             std::vector<double> &fitnesspoblation,
                             double &bestFitness,
                             tSolution<double> &bestsolution,
                             Problem<double> &problem,int maxevals) {
  int nRand = std::ceil(poblation.size() * 0.10);
  for (int i = 0; i < nRand; i++) {
    if (evals + 100 > maxevals) {
      break;
    } else {
      evals += 100;
    }
    ResultMH<double> result = localSearch(problem, 100, poblation[i]);
    if (result.fitness > fitnesspoblation[i]) {
      fitnesspoblation[i] = result.fitness;
      poblation[i] = result.solution;
    }
    if (result.fitness > bestFitness) {
      bestFitness = result.fitness;
      bestsolution = result.solution;
    }
  }
}
