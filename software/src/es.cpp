#include "../include/es.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

ResultMH<double> Es::optimize(Problem<double> &problem, const tSolution<double>&current,tFitness fitness ,int maxevals) {
  assert(maxevals > 0);
  const int max_evals = maxevals;
  int evals = 0;
  int size = problem.getSolutionSize();
  auto range = problem.getSolutionDomainRange();
  double lo = range.first;
  double hi = range.second;

  tSolution<double> currentSolution = current;
  double currentFitness = fitness;
  evals++;

  tSolution<double> bestSolution = currentSolution;
  double bestFitness = currentFitness;

  int max_vecinos = 10 * size;
  int max_exitos = 1 * size;
  int nEnfriamientos = std::max(1, max_evals / max_vecinos);
  double phi = 0.3;
  double mu = 0.2;
  double finalTemp = 1e-3;
  double inicialTemp = (mu * std::abs(currentFitness)) / -std::log(phi);
  while (inicialTemp <= finalTemp) {
    currentSolution = problem.createSolution();
    currentFitness = problem.fitness(currentSolution);
    evals++;
    bestSolution = currentSolution;
    bestFitness = currentFitness;
    inicialTemp = (mu * std::abs(currentFitness)) / -std::log(phi);
  }

  double beta =
      (inicialTemp - finalTemp) / (nEnfriamientos * inicialTemp * finalTemp);
  double temp = inicialTemp;

  std::vector<std::pair<int, int>> neighbors;
  neighbors.reserve(size * size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      neighbors.emplace_back(i, j);
    }
  }

  int enfriamientos = 0;
  bool hay_exito = true;
  while (evals < max_evals && hay_exito) {
    int vecinos_generados = 0;
    int exitos = 0;
    Random::shuffle(neighbors);
    int pos = 0;
    int total_pos = static_cast<int>(neighbors.size());

    while (vecinos_generados < max_vecinos && exitos < max_exitos &&
           evals < max_evals && pos < total_pos) {
      int i = neighbors[pos].first;
      int j = neighbors[pos].second;
      if (i != j) {
        double valor_i = currentSolution[i];
        double valor_j = currentSolution[j];
        if (valor_i > lo && valor_j < hi) {
          double tope_1 = 1.0 - (lo / valor_i);
          double tope_2 = (hi - valor_j) / valor_i;
          double tope_3 = 0.3;
          double ratio = std::min({tope_1, tope_2, tope_3});
          if (ratio == 0.0) {
            pos++;
            continue;
          }
          vecinos_generados++;
          currentSolution[i] = (1.0 - ratio) * valor_i;
          currentSolution[j] = valor_j + ratio * valor_i;
          if (problem.isValid(currentSolution)) {
            double candidateFitness = problem.fitness(currentSolution);
            evals++;
            double incremento = currentFitness - candidateFitness;
            double u = Random::get<double>(0.0, 1.0);
            if (incremento < 0.0 || (u <= std::exp(-incremento / temp))) {
              currentFitness = candidateFitness;
              exitos++;
              Random::shuffle(neighbors);
              pos = 0;
              if (currentFitness > bestFitness) {
                bestFitness = currentFitness;
                bestSolution = currentSolution;
              }
            } else {
              currentSolution[i] = valor_i;
              currentSolution[j] = valor_j;
              pos++;
            }
          } else {
            currentSolution[i] = valor_i;
            currentSolution[j] = valor_j;
            pos++;
          }
        } else {
          pos++;
        }
      } else {
        pos++;
      }
    }

    temp = temp / (1.0 + beta * temp);
    hay_exito = exitos > 0;
    enfriamientos++;
  }
  return ResultMH(bestSolution, bestFitness, evals);
}
