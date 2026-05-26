#pragma once
#include "mhtrayectory.h"
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

class LocalSearch : public MHTrayectory<double> {
private:
	double ratio;
public:
  LocalSearch(double ratio) : MHTrayectory(),ratio(ratio) {}
  virtual ~LocalSearch() {}
  virtual ResultMH<double> optimize(Problem<double> &problem,
                                    const tSolution<double> &current,
                                    tFitness fitness, int maxevals) override;
};
