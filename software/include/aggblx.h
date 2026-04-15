#pragma once

#include "agg.h"

class AggBLX : public Agg {
public:
  AggBLX(int populationSize, double crossoverProb, double mutationProb,
             double alpha, int tournamentSize = 3)
      : Agg(populationSize, crossoverProb, mutationProb, tournamentSize),
        alpha(alpha) {}

protected:
  std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1,
            const tSolution<double> &p2) override{
		return op.blxAlpha(p1,p2,this->alpha);
	}

private:
  double alpha;
};
