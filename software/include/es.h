#pragma once
#include "mhtrayectory.h"
#include "operator.h"
#include <utility>

class Es : public MHTrayectory<double> {
public:
  Es() : MHTrayectory() {}
  virtual ~Es() {}

  ResultMH<double> optimize(Problem<double> &problem,
                            const tSolution<double> &current, tFitness fitness,
                            int maxevals) override;
};
