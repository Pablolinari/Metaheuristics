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
using ResultMDouble = ResultMH<double>;

class RandomSearch : public MHDouble {

public:
  RandomSearch() : MH() {}
  virtual ~RandomSearch() {}
  virtual ResultMH<double> optimize(Problem<int> &problem, int maxevals);
};
