#pragma once
#include "mh.h"
#include "operator.h"
#include <utility>

class Es : public MH<double> {
public:
  Es():MH(){}
  virtual ~Es() {}

  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;
};
