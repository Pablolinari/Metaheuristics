#include "../include/agg.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <set>
#include <utility>
#include <vector>

ResultMH<double> Agg::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  int evals;
  int problemSize = problem.getSolutionSize();
  int npob = 100;
  double probcruce = 0.8;
  double probmut = 0.8;
  int k = 3;
  double bestFitness = -std::numeric_limits<double>::infinity();
  tSolution<double> bestSolution(problemSize);
  std::vector<tSolution<double>> poblation;
  std::vector<tSolution<double>> parents;
  std::vector<tSolution<double>> sons;
  std::set<int> idxparents;
  for (int i = 0; i < npob; i++) {
    poblation.push_back(problem.createSolution());
  }
  for (int i = 0; i < npob; i++) {
    double tempFitness = problem.fitness(poblation[i]);
    if (tempFitness > bestFitness) {
      bestFitness = tempFitness;
      bestSolution = poblation[i];
    }
  }
  std::vector<int> idx;
  // torneo para npob veces
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
    idxparents.insert(idxBestSolution);
  }

  // genero los padres de la siguiente generación
  for (auto it = idxparents.begin(); it != idxparents.end(); it++) {
    parents.push_back(poblation[*it]);
  }
  // cruce
  // TODO revisar limites que hacer cuando es impar
  int expectedCrossover =
      std::ceil(this->crossoverProb * (parents.size() / 2.0));
  if (expectedCrossover % 2 != 0) {
    expectedCrossover--;
  }
  for (int i = 0; i < expectedCrossover; i += 2) {

    std::pair<tSolution<double>, tSolution<double>> son =
        crossover(parents[i], parents[i + 1]);
    if (!problem.isValid(son.first)) {
      problem.fix(son.first);
    }
    if (!problem.isValid(son.second)) {
      problem.fix(son.second);
    }
    sons.push_back(son.first);
    sons.push_back(son.second);
  }
  for (int i = expectedCrossover; i < this->populationSize; i++) {
    sons.push_back(parents[i]);
  }

  // mutación
  int expectedMutation =
      std::ceil(this->mutationProb * this->populationSize * problemSize);

  for (int i = 0; i < expectedMutation; i++) {

    int idxSolution = Random::get(0, this->populationSize - 1);
    int idxGene = Random::get(0, problemSize - 1);

    tSolution<double> mutSol = sons[idxSolution];

    double ratio = mutSol(idxGene) * 0.15;

    if (mutSol(i) - ratio < problem.getSolutionDomainRange().first ||
        mutSol(i) - ratio != 0) {
    }
  }

  return ResultMH(bestSolution, bestFitness, evals);
}
