#pragma once

#include "mh.h"
#include "operator.h"
#include "problem.h"
#include "solution.h"
#include <utility>
#include <vector>

class Am : public MH<double> {
public:
  Am(int populationSize, double crossoverProb, double mutationProb,
     int tournamentSize = 3);
  virtual ~Am() {}

  ResultMH<double> localSearch(Problem<double> &problem, int maxevals,
                               tSolution<double> solInit);
  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

  std::pair<tSolution<double>, tSolution<double>>
  crossover(const tSolution<double> &p1, const tSolution<double> &p2);

  virtual void applyLocalSearch(std::vector<tSolution<double>> &poblation,
                               std::vector<double> &fitnesspoblation,
                               double &bestFitness,
                               tSolution<double> &bestsolution,
                               Problem<double> &problem,int maxevals) = 0;


protected:
  
	Operator op;
  int populationSize;
  double mutationProb;
  double crossoverProb;
  int tournamentSize;
	int evals=0;
};
