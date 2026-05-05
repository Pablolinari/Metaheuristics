
#include "../include/amall.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

AmAll::AmAll(int populationSize, double crossoverProb, double mutationProb,
             int tournamentSize)
    : Am(populationSize, crossoverProb, mutationProb) {}
int AmAll::applyLocalSearch(std::vector<tSolution<double>> &poblation,
                            std::vector<double> &fitnesspoblation,
                            double &bestFitness, tSolution<double> &bestsolution,
                            Problem<double> &problem) {
	int evals =0;
  for (int i = 0; i < populationSize; i++) {
    ResultMH<double> result = localSearch(problem, 100, poblation[i]);
    if (result.fitness > fitnesspoblation[i]) {
      fitnesspoblation[i] = result.fitness;
      poblation[i] = result.solution;
    }
    if (result.fitness > bestFitness) {
      bestFitness = result.fitness;
      bestsolution = result.solution;
    }
		evals+=100;
  }
	return evals;
}
