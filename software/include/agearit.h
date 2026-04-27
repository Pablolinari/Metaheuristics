#pragma once

#include "age.h"

class AgeArit : public Age {
public:
  AgeArit(int populationSize, double mutationProb, double alpha,
          int tournamentSize = 3)
      : Age(populationSize, mutationProb, tournamentSize) {}

protected:
  std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1, const tSolution<double> &p2) override {
    return op.arithmetic(p1, p2);
  }
};
