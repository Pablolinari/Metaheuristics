#pragma once

#include "am.h"
#include "operator.h"
#include <utility>

class AmAll : public Am {
public:
  AmAll(int populationSize, double crossoverProb, double mutationProb,
        int tournamentSize = 3);
  virtual ~AmAll() {}

  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

  std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1, const tSolution<double> &p2);

  Operator op;
  int populationSize;
  double mutationProb;
  double crossoverProb;
  int tournamentSize;
};
