

#pragma once
#include "mh.h"
#include "problem.h"
#include "solution.h"

using namespace std;

/**
 * Implementation of the Random Search metaheuristic
 *  - Randomly generates solutions and selects the best one
 *
 * @see MH
 * @see Problem
 */
// Instanciamos la plantilla con el tipo que nos interese
//
using MHDouble = MH<double>;
using ProblemDouble = Problem<double>;
using ResultMDouble = ResultMH<double>;

class IlsEs : public MHDouble {

public:
  IlsEs() : MH() {}
  virtual ~IlsEs() {}

  tSolution<double> mutation(Problem<double> &problem, tSolution<double> mutSol,
                             double ratio);
  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;
};
