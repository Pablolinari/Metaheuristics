#pragma once

#include "mh.h"
#include "operator.h"
#include <utility>

class Agg : public MH<double> {
public:
  Agg(int populationSize, double crossoverProb, double mutationProb,
      int tournamentSize = 3);
  virtual ~Agg() {}

  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

  virtual std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1, const tSolution<double> &p2) = 0;

  bool mutation(tSolution<double> &mutSol, Problem<double> &problem,
                double mutationRate, int maxAttempts);

protected:
  Operator op;
  int populationSize;
  double crossoverProb;
  double mutationProb;
  int tournamentSize;
};
