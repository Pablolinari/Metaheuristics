#pragma once

#include "aggarit.h"

class AggARitBLX : public AggARit {
public:
  AggARitBLX(int populationSize, double crossoverProb, double mutationProb,
             double alpha, int tournamentSize = 3)
      : AggARit(populationSize, crossoverProb, mutationProb, tournamentSize),
        alpha(alpha) {}

protected:
  std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1,
            const tSolution<double> &p2) override;

private:
  double alpha;
};
