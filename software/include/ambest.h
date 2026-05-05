#pragma once

#include "am.h"
#include "operator.h"
#include <utility>

class AmBest : public Am {
public:
  AmBest(int populationSize, double crossoverProb, double mutationProb,
         int tournamentSize = 3);
  virtual ~AmBest() {}

  int applyLocalSearch(std::vector<tSolution<double>> &poblation,
                       std::vector<double> &fitnesspoblation,
                       double &bestFitness, tSolution<double> &bestsolution,
                       Problem<double> &problem) override;

  Operator op;
  int populationSize;
  double mutationProb;
  double crossoverProb;
  int tournamentSize;
};
