#include "../include/agg.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <set>
#include <utility>
#include <vector>
Agg::Agg(int populationSize, double crossoverProb, double mutationProb,
         int tournamentSize)
    : populationSize(populationSize), crossoverProb(crossoverProb),
      mutationProb(mutationProb), tournamentSize(tournamentSize) {}

ResultMH<double> Agg::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  int evals = 0;
  int problemSize = problem.getSolutionSize();
  std::pair<double, double> problembounds = problem.getSolutionDomainRange();
  double bestFitness = -std::numeric_limits<double>::infinity();
  tSolution<double> bestSolution(problemSize);
  std::vector<tSolution<double>> poblation;
  std::vector<double> fitnessPoblation;
  std::vector<tSolution<double>> parents;
  std::vector<tSolution<double>> sons;
  std::vector<int> idxparents;
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

    std::vector<int> idx;
    // torneo para npob veces
    for (int i = 0; i < this->populationSize; i++) {
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
      idxparents.push_back(idxBestSolution);
      idx.clear();
    }

    // genero los padres de la siguiente generación
    for (auto it = idxparents.begin(); it != idxparents.end(); it++) {
      parents.push_back(poblation[*it]);
    }
    // cruce
		
		//esta variable indica el numero de padres que tienen que cruzarse , no el 
		//numero de cruces , es lo mismo que el guión pero sin dividir por 2.
    int expectednParentsCrossover =
        std::ceil(this->crossoverProb * (parents.size()));
    if (expectednParentsCrossover % 2 != 0) {
      expectednParentsCrossover--;
    }
    for (int i = 0; i < expectednParentsCrossover; i += 2) {

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
    for (int i = expectednParentsCrossover; i < this->populationSize; i++) {
      sons.push_back(parents[i]);
    }

    // mutación
    int expectedMutation =
        std::ceil(this->mutationProb * this->populationSize * problemSize);

    for (int i = 0; i < expectedMutation; i++) {

      int idxSolution = Random::get(0, this->populationSize - 1);
      tSolution<double> mutSol = sons[idxSolution];

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
			fitnessPoblation.push_back(tempFitness);
      if (tempFitness > bestFitnessSons) {
        bestFitnessSons = tempFitness;
        idxbest = i;
      }

      if (tempFitness < worstFitnessSons) {
        worstFitnessSons = tempFitness;
        idxworst = i;
      }
    }
    sons[idxworst] = bestSolution;
		fitnessPoblation[idxworst]=bestFitness;
    if (bestFitnessSons > bestFitness) {
      bestSolution = sons[idxbest];
      bestFitness = bestFitnessSons;
    }
    poblation = sons;
		
    sons.clear();
    parents.clear();
    idxparents.clear();
  }
  return ResultMH(bestSolution, bestFitness, evals);
}
