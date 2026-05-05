
#include "../include/amall.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

std::pair<tSolution<double>, tSolution<double>>
AmAll::crossover(const tSolution<double> &p1, const tSolution<double> &p2) {
  return op.blxAlpha(p1, p2, 0.3);
}
AmAll::AmAll(int populationSize, double crossoverProb, double mutationProb,
             int tournamentSize)
    : populationSize(populationSize), crossoverProb(crossoverProb),
      mutationProb(mutationProb), tournamentSize(tournamentSize) {}

ResultMH<double> AmAll::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  int evals = 0;
  int nGenerations = 0;
  int problemSize = problem.getSolutionSize();
  std::pair<double, double> problembounds = problem.getSolutionDomainRange();
  double bestFitness = -std::numeric_limits<double>::infinity();
  tSolution<double> bestSolution(problemSize);
  std::vector<tSolution<double>> poblation;
  std::vector<double> fitnessPoblation;
  std::vector<tSolution<double>> sons;
  std::vector<int> idxparents;
  for (int i = 0; i < this->populationSize; i++) {
    poblation.push_back(problem.createSolution());
  }
  for (int i = 0; i < this->populationSize; i++) {
    double tempFitness = problem.fitness(poblation[i]);
    evals++;
    fitnessPoblation.push_back(tempFitness);
    if (tempFitness > bestFitness) {
      bestFitness = tempFitness;
      bestSolution = poblation[i];
    }
  }
  while (evals < maxevals) {
    // torneo para npob veces
    for (int i = 0; i < this->populationSize; i++) {

      double fitnessTournament = -std::numeric_limits<double>::infinity();
      int idxBestSolution = 0;
      int idx;
      for (int j = 0; j < this->tournamentSize; j++) {
        idx = Random::get(0, this->populationSize - 1);
        if (fitnessPoblation[idx] > fitnessTournament) {
          fitnessTournament = fitnessPoblation[idx];
          idxBestSolution = idx;
        }
      }
      idxparents.push_back(idxBestSolution);
    }
    // cruce

    // esta variable indica el numero de padres que tienen que cruzarse , no el
    // numero de cruces , es lo mismo que el guión pero sin dividir por 2.
    int expectednParentsCrossover =
        std::ceil(this->crossoverProb * (idxparents.size()));
    if (expectednParentsCrossover % 2 != 0) {
      expectednParentsCrossover--;
    }
    for (int i = 0; i < expectednParentsCrossover; i += 2) {

      std::pair<tSolution<double>, tSolution<double>> son =
          crossover(poblation[idxparents[i]], poblation[idxparents[i + 1]]);
      if (!problem.isValid(son.first)) {
        problem.fix(son.first);
      }
      if (!problem.isValid(son.second)) {
        problem.fix(son.second);
      }
      sons.push_back(son.first);
      sons.push_back(son.second);
    }
    for (int i = expectednParentsCrossover; i < this->populationSize; i++) {
      sons.push_back(poblation[idxparents[i]]);
    }

    // mutación
    int expectedMutation =
        std::ceil(this->mutationProb * this->populationSize * problemSize);

    for (int i = 0; i < expectedMutation; i++) {

      int idxSolution = Random::get(0, this->populationSize - 1);
      tSolution<double> mutSol = sons[idxSolution];

      bool mutated = op.mutation(mutSol, problem, 0.15, problemSize * 3);

      if (mutated) {
        sons[idxSolution] = mutSol;
      }
    }
    // reemplazo generacional
    //  busco el peor de la generación nueva y lo elimino
    //  busco el mejor de la generación nueva y compruebo si es mejor que el
    fitnessPoblation.clear();
    double bestFitnessSons = -std::numeric_limits<double>::infinity();
    double worstFitnessSons = std::numeric_limits<double>::infinity();
    int idxbest = 0;
    int idxworst = 0;
    for (int i = 0; i < this->populationSize; i++) {
      double tempFitness = problem.fitness(sons[i]);
      evals++;
      fitnessPoblation.push_back(tempFitness);
      if (tempFitness > bestFitnessSons) {
        bestFitnessSons = tempFitness;
        idxbest = i;
      }

      if (tempFitness < worstFitnessSons) {
        worstFitnessSons = tempFitness;
        idxworst = i;
      }
      if (evals == maxevals) {
        break;
      }
    }
    if (bestFitness > worstFitnessSons) {
      sons[idxworst] = bestSolution;
      fitnessPoblation[idxworst] = bestFitness;
    }

    if (bestFitnessSons > bestFitness) {
      bestSolution = sons[idxbest];
      bestFitness = bestFitnessSons;
    }

    poblation.clear();
    poblation = sons;

    sons.clear();
    idxparents.clear();
    nGenerations++;
    if (nGenerations % 10 == 0) {
      for (int i = 0; i < populationSize; i++) {
        ResultMH<double> result = localSearch(problem, 100, poblation[i]);
        if (result.fitness > fitnessPoblation[i]) {
          fitnessPoblation[i] = result.fitness;
          poblation[i] = result.solution;
        }
        if (result.fitness > bestFitness) {
          bestFitness = result.fitness;
          bestSolution = result.solution;
        }
        evals += 100;
      }
    }
  }
  return ResultMH(bestSolution, bestFitness, evals);
}
