#pragma once

#include "agg.h"

class AggArit: public Agg {
public:
  AggArit(int populationSize, double crossoverProb,
                    double mutationProb, int tournamentSize = 3)
      : Agg(populationSize, crossoverProb, mutationProb, tournamentSize) {}

  std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1,
            const tSolution<double> &p2) override{
		return op.arithmetic(p1,p2);
	}
};
