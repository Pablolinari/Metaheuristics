

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

class Bmb : public MHDouble {

public:
  Bmb() : MH() {};
  ~Bmb() {};
	ResultMH<double> localSearch(Problem<double> &problem, int maxevals,
                                 tSolution<double> solInit) ;
  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;
};
