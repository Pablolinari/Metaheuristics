#include "../include/age.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>
Age::Age(int populationSize, double mutationProb,
         int tournamentSize)
    : populationSize(populationSize),
      mutationProb(mutationProb), tournamentSize(tournamentSize) {}

ResultMH<double> Age::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  int evals = 0;
  int problemSize = problem.getSolutionSize();
  std::pair<double, double> problembounds = problem.getSolutionDomainRange();
  double bestFitness = -std::numeric_limits<double>::infinity();
  double worstFitness = std::numeric_limits<double>::infinity();
  double secondWorstFitness = std::numeric_limits<double>::infinity();
  int idxWorst = 0;
  int idxSecondWorst = 0;
  tSolution<double> bestSolution(problemSize);
  std::vector<tSolution<double>> poblation;
  std::vector<double> fitnessPoblation;
  std::pair<int, int> idxparents;
  for (int i = 0; i < this->populationSize; i++) {
    poblation.push_back(problem.createSolution());
  }
  for (int i = 0; i < this->populationSize; i++) {
    double tempFitness = problem.fitness(poblation[i]);
    fitnessPoblation.push_back(tempFitness);
    if (tempFitness > bestFitness) {
      bestFitness = tempFitness;
      bestSolution = poblation[i];
    }
  }
  while (evals < maxevals) {
    for (int i = 0; i < this->populationSize; i++) {
      double tempFitness = problem.fitness(poblation[i]);
      fitnessPoblation.push_back(tempFitness);
			evals++;
      if (tempFitness < worstFitness) {
        secondWorstFitness = worstFitness;
        worstFitness = tempFitness;
        idxSecondWorst = idxWorst;
        idxWorst = i;
      }
    }

    std::vector<int> idx;
    // torneo para npob veces
    for (int i = 0; i < 2; i++) {
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
      idx.clear();
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
    for (int i = 0; i < 2; i++) {

      if (Random::get<bool>(mutationProb)) {
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
    }
    // reemplazo generacional
    //  busco el peor de la generación nueva y lo elimino
    //  busco el mejor de la generación nueva y compruebo si es mejor que el
    fitnessPoblation.clear();

    double fitnessFirst = problem.fitness(son[0]);
    double fitnessSecond = problem.fitness(son[1]);
		evals+=2;
    double min = std::min(fitnessFirst, fitnessSecond);
    if (min == fitnessFirst) {
      if (fitnessFirst > worstFitness) {
        poblation[idxWorst] = son[0];
      }
      if (fitnessSecond > secondWorstFitness) {
        poblation[idxSecondWorst] = son[1];
      }
      if (fitnessSecond > bestFitness) {
        bestFitness = fitnessSecond;
        bestSolution = son[1];
      }
    } else {
      if (fitnessSecond > worstFitness) {
        poblation[idxWorst] = son[1];
      }
      if (fitnessSecond > secondWorstFitness) {
        poblation[idxSecondWorst] = son[0];
      }
      if (fitnessFirst > bestFitness) {
        bestFitness = fitnessFirst;
        bestSolution = son[0];
      }
    }
  }
  return ResultMH(bestSolution, bestFitness, evals);
}
