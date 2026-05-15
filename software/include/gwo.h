#pragma once
#include "mh.h"
#include "problem.h"
#include <utility>

using namespace std;

/**
 * Implementation of the Grey Wolf Optimizer metaheuristic.
 *
 * @see MH
 * @see Problem
 */
using MHDouble = MH<double>;
using ProblemDouble = Problem<double>;
using ResultMHDouble = ResultMH<double>;

class Gwo : public MHDouble {
public:
  Gwo(int populationSize, double aMax = 2.0);
  virtual ~Gwo() {}
  ResultMHDouble optimize(ProblemDouble &problem, int maxevals) override;

private:
  int populationSize;
  double aMax;
};
