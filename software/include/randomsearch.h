#pragma once
#include "mh.h"

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
using ProblemDouble= Problem<double>;
using ResultMHDouble = ResultMH<double>;

class RandomSearch : public MHDouble {

public:
  RandomSearch() : MH() {}
  virtual ~RandomSearch() {}
  virtual ResultMHDouble optimize(Problem<double> &problem, int maxevals);
};
