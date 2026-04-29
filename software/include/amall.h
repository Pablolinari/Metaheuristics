#pragma once

#include "mh.h"
#include "operator.h"
#include <utility>

class AmAll : public MH<double> {
public:
  AmAll(int populationSize, double crossoverProb ,double mutationProb, int tournamentSize = 3);
  virtual ~AmAll() {}

  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

  std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1, const tSolution<double> &p2);

  ResultMH<double> localSearch(Problem<double> &problem, int maxevals,
                               tSolution<double> solInit);
  Operator op;
  int populationSize;
  double mutationProb;
  double crossoverProb;
  int tournamentSize;
};
