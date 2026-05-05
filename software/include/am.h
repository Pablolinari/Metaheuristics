#pragma once

#include "mh.h"
#include "operator.h"
#include <utility>

class Am : public MH<double> {
public:
  virtual ~Am() {}

  ResultMH<double> localSearch(Problem<double> &problem, int maxevals,
                               tSolution<double> solInit);
};
