#include "../include/agg.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

ResultMH<double> Agg::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  int npob = 0;
  double probcruce = 0.8;
  double probmut = 0.8;
  int k = 3;
  std::vector<tSolution<double>> poblation;
  std::vector<int> idxparents;
  for (int i = 0; i < npob; i++) {
    poblation.push_back(problem.createSolution());
  }

  std::vector<int> idx;
  for (int i = 0; i < npob; i++) {
    for (int j = 0; j < k; j++) {
      idx.push_back(Random::get(0, npob - 1));
    }
    double fitnessTournament = -std::numeric_limits<double>::infinity();
    int idxBestSolution = 0;
    for (int j = 0; j < k; j++) {
      double actualFitness = problem.fitness(poblation[idx[j]]);
      if (actualFitness > fitnessTournament) {
        fitnessTournament = actualFitness;
        idxBestSolution = idx[j];
      }
    }
    idxparents.push_back(idxBestSolution);

  }
	crossover(const tSolution<double> &p1, const tSolution<double> &p2)

  return ResultMH(bestSolution, problem.fitness(bestSolution), 1);
}
