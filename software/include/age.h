#pragma once

#include "mh.h"
#include "operator.h"
#include "problem.h"
#include <utility>

class Age : public MH<double> {
public:
  Age(int populationSize,double mutationProb,
      int tournamentSize = 3);
  virtual ~Age() {}

  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

  virtual std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1, const tSolution<double> &p2) = 0;
  bool mutation(tSolution<double> &mutSol,Problem<double>problem,int maxAttempts);

  Operator op;
  int populationSize;
  double mutationProb;
  int tournamentSize;
};
