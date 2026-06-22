#include "../include/age.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>
Age::Age(int populationSize, double mutationProb, int tournamentSize)
    : populationSize(populationSize), mutationProb(mutationProb),
      tournamentSize(tournamentSize) {}
ResultMH<double> Age::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  int evals = 0;
  int problemSize = problem.getSolutionSize();
  std::pair<double, double> problembounds = problem.getSolutionDomainRange();
  double bestFitness = -std::numeric_limits<double>::infinity();

  double worstFitness = std::numeric_limits<double>::infinity();
  double secondWorstFitness = std::numeric_limits<double>::infinity();

  tSolution<double> bestSolution(problemSize);
  std::vector<tSolution<double>> poblation;
  std::vector<double> fitnessPoblation;
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

    idxparents.clear();
    // torneo para npob veces
    idxparents = op.tournament(this->tournamentSize, this->populationSize, 2,
                               fitnessPoblation);

    // cruce

    std::vector<tSolution<double>> son(2);
    std::pair<tSolution<double>, tSolution<double>> auxson =
        crossover(poblation[idxparents[0]], poblation[idxparents[1]]);

    if (!problem.isValid(auxson.first)) {
      problem.fix(auxson.first);
    }
    if (!problem.isValid(auxson.second)) {
      problem.fix(auxson.second);
    }
    son[0] = auxson.first;
    son[1] = auxson.second;

    // mutación
    //
    for (int i = 0; i < 2; i++) {
      if (Random::get<bool>(this->mutationProb)) {
        tSolution<double> mutSol = son[i];
        bool mutated = op.mutation(mutSol, problem, 0.15, problemSize * 5);
        if (mutated) {
          son[i] = mutSol;
        }
      }
    }
    // reemplazo

    double fitnessFirst = problem.fitness(son[0]);
    evals++;
    if (evals == maxevals) {
      break;
    }
    double fitnessSecond = problem.fitness(son[1]);

    evals++;
    if (evals == maxevals) {
      break;
    }
    // ordeno los hijos , primero el mejor
    if (fitnessSecond > fitnessFirst) {
      std::swap(fitnessFirst, fitnessSecond);
      std::swap(son[0], son[1]);
    }

    int idxWorst = 0;
    int idxSecondWorst = 0;

    worstFitness = std::numeric_limits<double>::infinity();
    secondWorstFitness = std::numeric_limits<double>::infinity();
		// busco los indices del primero y segundo peor de la poblacion
    for (int i = 0; i < this->populationSize; i++) {
      double tempFitness = fitnessPoblation[i];
      if (tempFitness < worstFitness) {
        secondWorstFitness = worstFitness;
        idxSecondWorst = idxWorst;
        worstFitness = tempFitness;
        idxWorst = i;
      } else if (tempFitness < secondWorstFitness) {
        secondWorstFitness = tempFitness;
        idxSecondWorst = i;
      }
    }
		// actualizo el mejor resultado
    if (fitnessSecond > bestFitness) {
      bestFitness = fitnessSecond;
      bestSolution = son[1];
    }
    if (fitnessFirst > bestFitness) {
      bestFitness = fitnessFirst;
      bestSolution = son[0];
    }
    bool usado = false;
		// si el peor hijo supera al peor de la poblacion lo cambio
    if (fitnessFirst > worstFitness) {
      poblation[idxWorst] = son[0];
      fitnessPoblation[idxWorst] = fitnessFirst;
    } else if (fitnessSecond > worstFitness) {// en caso de que el primer hijo no entre 
																							// pruebo con el segundo

      poblation[idxWorst] = son[1];
      fitnessPoblation[idxWorst] = fitnessSecond;
      usado = true;
    }
		// si el segundo hijo no ha entrado 
    if (!usado && fitnessSecond > secondWorstFitness) {
      poblation[idxSecondWorst] = son[1];
      fitnessPoblation[idxSecondWorst] = fitnessSecond;
    }
  }
  return ResultMH(bestSolution, bestFitness, evals);
}
