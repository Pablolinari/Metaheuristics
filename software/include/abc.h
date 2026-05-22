
#pragma once

#include "mh.h"
#include "problem.h"
#include "solution.h"
#include <vector>

using namespace std;

// Instanciamos la plantilla con el tipo que nos interese
using MHDouble = MH<double>;
using ProblemDouble = Problem<double>;
using ResultMDouble = ResultMH<double>;

class Abc: public MHDouble {

public:
  Abc(int colonySize = 30, int trialLimit = 100);
	~Abc() {}

  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

private:
  int colonySize;
  int trialLimit;
};
