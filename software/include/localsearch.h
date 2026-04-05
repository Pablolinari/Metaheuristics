#pragma once
#include "mh.h"
#include "problem.h"

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

class LocalSearch : public MHDouble {

public:
  LocalSearch() : MH() {}
  virtual ~LocalSearch() {}
  virtual ResultMH<double> optimize(Problem<double> &problem, int maxevals);
};
