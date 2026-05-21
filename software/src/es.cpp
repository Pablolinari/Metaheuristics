#include "../include/es.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

ResultMH<double> Es::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  const int max_evals = 100000;
  int evals = 0;
  int succes = 0;
  int size = problem.getSolutionSize();

  tSolution<double> currentSolution = problem.createSolution();
  double currentFitness = problem.fitness(currentSolution);
  evals++;

  tSolution<double> bestSolution = currentSolution;
  double bestFitness = currentFitness;

  int max_vecinos = 10 * size;
  int max_exitos = 1 * size;
  int nEnfriamientos = max_evals / max_vecinos;
  double phi = 0.3;
  double mu = 0.2;
  double inicialTemp = (mu * currentFitness) / -std::log(phi);
  double finalTemp = 1e-3;
  if (finalTemp >= inicialTemp) {
    finalTemp = inicialTemp * 0.5;
  }

  double beta =
      (inicialTemp - finalTemp) / (nEnfriamientos * inicialTemp * finalTemp);
  double temp = inicialTemp;

  std::vector<std::pair<int, int>> neighbors;
  neighbors.reserve(size * (size - 1));
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i != j) {
        neighbors.emplace_back(i, j);
      }
    }
  }

  int enfriamientos = 0;
  while (evals < max_evals) {
    succes = 0;
    int vecinos_generados = 0;
    Random::shuffle(neighbors);
    for (const std::pair<int, int> &move : neighbors) {
      if (vecinos_generados >= max_vecinos || succes >= max_exitos ||
          evals >= max_evals) {
        break;
      }
      int i = move.first;
      int j = move.second;
      tSolution<double> candidate = currentSolution;
      double ratio = Random::get<double>(0.05, 0.3);
      double amount = candidate(i) * ratio;
      if (amount <= 0.0) {
        vecinos_generados++;
        continue;
      }

      candidate(i) -= amount;
      candidate(j) += amount;
      if (!problem.isValid(candidate)) {
        problem.fix(candidate);
      }

      double candidateFitness = problem.fitness(candidate);
      evals++;
      double delta = candidateFitness - currentFitness;
      bool accepted = false;
      if (delta > 0.0) {
        accepted = true;
      } else {
        double prob = std::exp(delta / temp);
        double r = Random::get<double>(0.0, 1.0);
        if (r < prob) {
          accepted = true;
        }
      }

      if (accepted) {
        currentSolution = candidate;
        currentFitness = candidateFitness;
        succes++;
        if (currentFitness > bestFitness) {
          bestFitness = currentFitness;
          bestSolution = currentSolution;
        }
      }
      vecinos_generados++;
    }

    if (succes == 0) {
      break;
    }
    if (enfriamientos < nEnfriamientos) {
      temp = temp / (1.0 + beta * temp);
      if (temp < finalTemp) {
        temp = finalTemp;
      }
      enfriamientos++;
    } else {
      temp = finalTemp;
    }
  }
  return ResultMH(bestSolution, bestFitness, evals);
}
