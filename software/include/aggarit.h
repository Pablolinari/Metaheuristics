#pragma once

#include "agg.h"

class AggArit: public Agg {
public:
  AggArit(int populationSize, double crossoverProb,
                    double mutationProb, int tournamentSize = 3)
      : AggARit(populationSize, crossoverProb, mutationProb, tournamentSize) {}

protected:
  std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1,
            const tSolution<double> &p2) override;
};
