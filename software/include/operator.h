#pragma once

#include "problem.h"
#include "solution.h"
#include <utility>
class Operator {
public:
  virtual ~Operator() {}
  /**
   * Run the metaheuristic algorithm to find the optimal solution.
   *
   * @param problem  The problem to solve.
   * @param maxevals The maximum number of evaluations.
   * @return A pair containing the optimal solution and its fitness.
   * @throws Exception if the algorithm fails to converge
   *
   * @since   2021-05-01
   * @author  author
   * @version 1.0
   */
  std::pair<tSolution<double>, tSolution<double>>
  arithmetic(const tSolution<double> &p1, const tSolution<double> &p2);
  std::pair<tSolution<double>, tSolution<double>>
  blxAlpha(const tSolution<double> &p1, const tSolution<double> &p2,
           double alpha);

  bool mutation(tSolution<double> &mutSol, Problem<double> &problem,
                double mutationRate, int maxAttempts);
	std::vector<int>tournament(int tournamentsize,int populationsize,int selectionsize,const std::vector<double>&fitnesspoblation); 
};
