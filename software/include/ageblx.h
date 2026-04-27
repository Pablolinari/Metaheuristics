#pragma once

#include "age.h"

class AgeBLX : public Age {
public:
  AgeBLX(int populationSize, double mutationProb, double alpha,
         int tournamentSize = 3)
      : Age(populationSize, mutationProb, tournamentSize), alpha(alpha) {}

protected:
  std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1, const tSolution<double> &p2) override {
    return op.blxAlpha(p1, p2, this->alpha);
  }

private:
  double alpha;
};
