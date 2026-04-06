#include "../include/greedy.h"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <utility>
#include <vector>

ResultMH<double> Greedy::optimize(Problem<double> &problem , int maxevals) {
	assert(maxevals>0);
  tSolution<double> bestSolution(problem.getSolutionSize());
  std::vector<int> bestcompanies(problem.getSolutionSize());
  int company = 0;
	//Relleno el vector con los indices 1,2,.....ncompany-1
	std::iota(bestcompanies.begin(),bestcompanies.end(),0);
	//ordeno según la heurística
  std::sort(bestcompanies.begin(), bestcompanies.end(), [&](int i, int j) {
    return problem.heuristic(i) > problem.heuristic(j);
  });

  bestSolution.setZero();
  pair<double, double> domain = problem.getSolutionDomainRange();
	//mientras que la solución no sume 1 , selecciono empresas 
	//del vector que se ha ordenado con la heurística
  while (bestSolution.sum() < 1.0) {
    company = bestcompanies.front();
    bestcompanies.erase(bestcompanies.begin());
    bestSolution(company) = std::min(domain.second, 1 - bestSolution.sum());
  }
  return ResultMH(bestSolution,problem.fitness(bestSolution), 1);
}
