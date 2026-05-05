#pragma once

#include "am.h"
#include "operator.h"
#include <utility>

class AmAll : public Am {
public:
  AmAll(int populationSize, double crossoverProb, double mutationProb,
        int tournamentSize = 3);
  virtual ~AmAll() {}

  int applyLocalSearch(std::vector<tSolution<double>> &poblation,
                       std::vector<double> &fitnesspoblation,
                       double bestFitness, tSolution<double> &bestsolution,
                       Problem<double> &problem) override;
};
