#pragma once

#include "am.h"
#include "operator.h"
#include <utility>

class AmRand : public Am {
public:
  AmRand(int populationSize, double crossoverProb, double mutationProb,
         int tournamentSize = 3);
  virtual ~AmRand() {}

  void applyLocalSearch(std::vector<tSolution<double>> &poblation,
                        std::vector<double> &fitnesspoblation,
                        double &bestFitness, tSolution<double> &bestsolution,
                        Problem<double> &problem, int maxevals) override;
};
