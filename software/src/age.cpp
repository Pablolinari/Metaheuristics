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
  std::pair<int, int> idxparents;

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

    std::vector<int> idx;

    // torneo para npob veces
    for (int i = 0; i < 2; i++) {
      idx.clear();
      for (int j = 0; j < this->tournamentSize; j++) {
        idx.push_back(Random::get(0, this->populationSize - 1));
      }
      double fitnessTournament = -std::numeric_limits<double>::infinity();
      int idxBestSolution = 0;
      for (int j = 0; j < this->tournamentSize; j++) {
        if (fitnessPoblation[idx[j]] > fitnessTournament) {
          fitnessTournament = fitnessPoblation[idx[j]];
          idxBestSolution = idx[j];
        }
      }
      if (i == 0) {
        idxparents.first = idxBestSolution;
      } else {
        idxparents.second = idxBestSolution;
      }
    }

    // cruce

    std::vector<tSolution<double>> son(2);
    std::pair<tSolution<double>, tSolution<double>> auxson =
        crossover(poblation[idxparents.first], poblation[idxparents.second]);

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
		// preguntar si la mutacion esta bien hecha , sobre todo en agg 
    for (int i = 0; i < 2; i++) {

      tSolution<double> mutSol = son[i];
      const double mutationRate = 0.15;
      const int maxAttempts = std::max(10, problemSize * 4);

      int idxGene = -1;
      double amount = 0.0;

      for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        int candidateFirst = Random::get(0, problemSize - 1);
        double candidateAmount = mutSol(candidateFirst) * mutationRate;
        if (candidateAmount <= 0.0) {
          continue;
        }

        double firstAfter = mutSol(candidateFirst) - candidateAmount;
        bool firstValid =
            firstAfter == 0.0 || (firstAfter >= problembounds.first &&
                                  firstAfter <= problembounds.second);

        if (firstValid) {
          idxGene = candidateFirst;
          amount = candidateAmount;
          break;
        }
      }

      if (idxGene == -1) {
        continue;
      }

      int idxNextGene = -1;
      for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        int candidateSecond = Random::get(0, problemSize - 1);
        if (candidateSecond == idxGene) {
          continue;
        }

        double secondAfter = mutSol(candidateSecond) + amount;
        bool secondValid =
            secondAfter == 0.0 || (secondAfter >= problembounds.first &&
                                   secondAfter <= problembounds.second);

        if (secondValid) {
          idxNextGene = candidateSecond;

          break;
        }
      }

      if (idxNextGene == -1) {
        continue;
      }

      mutSol(idxGene) -= amount;
      mutSol(idxNextGene) += amount;

      if (problem.isValid(mutSol)) {
        son[i] = mutSol;
      }
    }
    // reemplazo generacional
    //  busco el peor de la generación nueva y lo elimino
    //  busco el mejor de la generación nueva y compruebo si es mejor que el
    double fitnessFirst = problem.fitness(son[0]);
    double fitnessSecond = problem.fitness(son[1]);
    evals += 2;

    if (fitnessSecond > fitnessFirst) {
      std::swap(fitnessFirst, fitnessSecond);
      std::swap(son[0], son[1]);
    }

    int idxWorst = 0;
    int idxSecondWorst = 0;

    worstFitness = std::numeric_limits<double>::infinity();
    secondWorstFitness = std::numeric_limits<double>::infinity();

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
    if (fitnessSecond > bestFitness) {
      bestFitness = fitnessSecond;
      bestSolution = son[1];
    }
    if (fitnessFirst > bestFitness) {
      bestFitness = fitnessFirst;
      bestSolution = son[0];
    }
    bool usado = false;
    if (fitnessFirst > worstFitness) {
      poblation[idxWorst] = son[0];
      fitnessPoblation[idxWorst] = fitnessFirst;
    } else if (fitnessSecond > worstFitness) {
      poblation[idxWorst] = son[1];
      fitnessPoblation[idxWorst] = fitnessSecond;
      usado = true;
    }

    if (!usado && fitnessSecond > secondWorstFitness) {
      poblation[idxSecondWorst] = son[1];
      fitnessPoblation[idxSecondWorst] = fitnessSecond;
    }
// preguntar si , inserto un hijo y recalculo el peor , o inserto directamente los dos hijos si se puede.

  }
  return ResultMH(bestSolution, bestFitness, evals);
}
