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

class Greedy : public MHDouble {

public:
  Greedy() : MH() {}
  virtual ~Greedy() {}
  virtual ResultMH<int> optimize(Problem<int> &problem, int maxevals);
};
