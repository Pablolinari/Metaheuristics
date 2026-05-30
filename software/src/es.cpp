#include "../include/es.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

ResultMH<double> Es::optimize(Problem<double> &problem,
                              const tSolution<double> &current,
                              tFitness fitness, int maxevals) {
  assert(maxevals > 0);
  // limites de evaluaciones y dominio del problema
  const int maxEvals = maxevals;
  int evals = 0;
  int problemSize = problem.getSolutionSize();
  std::pair<double, double> bounds = problem.getSolutionDomainRange();
  double lower = bounds.first;
  double upper = bounds.second;

  // solucion inicial y su fitness
  tSolution<double> currentSolution = current;
  double currentFitness = fitness;
  evals++;

  // mejor solucion encontrada hasta el momento
  tSolution<double> bestSolution = currentSolution;
  double bestFitness = currentFitness;

  // parametros del enfriamiento simulado
  int maxVecinos = 10 * problemSize;
  int maxExitos = 1 * problemSize;
  int nEnfriamientos = std::max(1, maxEvals / maxVecinos);
  double phi = 0.3;
  double mu = 0.2;
  double tempFinal = 1e-3;
  // temperatura inicial dependiente del fitness
  double tempInicial = (mu * std::abs(currentFitness)) / -std::log(phi);
  // si la temperatura inicial es demasiado baja, reinicio con soluciones
  // aleatorias
  while (tempInicial <= tempFinal && evals < maxEvals) {
    currentSolution = problem.createSolution();
    currentFitness = problem.fitness(currentSolution);
    evals++;
    bestSolution = currentSolution;
    bestFitness = currentFitness;
    tempInicial = (mu * std::abs(currentFitness)) / -std::log(phi);
  }
  // aseguro una temperatura inicial util
  if (tempInicial <= tempFinal) {
    tempInicial = tempFinal * 10.0;
  }

  // factor de enfriamiento
  double beta =
      (tempInicial - tempFinal) / (nEnfriamientos * tempInicial * tempFinal);
  double temp = tempInicial;

  // precomputo el vecindario de pares (i, j)
  std::vector<std::pair<int, int>> neighbors;
  neighbors.reserve(problemSize * problemSize);
  for (int i = 0; i < problemSize; i++) {
    for (int j = 0; j < problemSize; j++) {
      neighbors.emplace_back(i, j);
    }
  }

  int enfriamientos = 0;
  bool hayExito = true;
  // bucle principal de enfriamiento
  while (evals < maxEvals && hayExito) {
    int vecinosGenerados = 0;
    int exitos = 0;
    Random::shuffle(neighbors);
    int pos = 0;
    int totalPos = static_cast<int>(neighbors.size());

    while (vecinosGenerados < maxVecinos && exitos < maxExitos &&
           evals < maxEvals && pos < totalPos) {
      int i = neighbors[pos].first;
      int j = neighbors[pos].second;
      // descarto movimientos invalidos o sin cambio
      if (i == j) {
        pos++;
        continue;
      }

      double valorI = currentSolution[i];
      double valorJ = currentSolution[j];
      // compruebo limites antes de mover masa entre posiciones
      if (valorI <= 0.0 || valorI <= lower || valorJ >= upper) {
        pos++;
        continue;
      }

      // calculo el ratio maximo permitido para el intercambio
      double tope1 = 1.0 - (lower / valorI);
      double tope2 = (upper - valorJ) / valorI;
      double tope3 = 0.3;
      double ratio = std::min({tope1, tope2, tope3});
      if (!(ratio > 0.0)) {
        pos++;
        continue;
      }

      vecinosGenerados++;
      // aplico el movimiento al vecino
      currentSolution[i] = (1.0 - ratio) * valorI;
      currentSolution[j] = valorJ + ratio * valorI;
      if (problem.isValid(currentSolution)) {
        double candidateFitness = problem.fitness(currentSolution);
        evals++;
        double incremento = currentFitness - candidateFitness;
        double u = Random::get<double>(0.0, 1.0);
        // acepto si mejora o por criterio de Metropolis
        if (incremento < 0.0 || (u <= std::exp(-incremento / temp))) {
          currentFitness = candidateFitness;
          exitos++;
          Random::shuffle(neighbors);
          pos = 0;
          // actualizo mejor solucion
          if (currentFitness > bestFitness) {
            bestFitness = currentFitness;
            bestSolution = currentSolution;
          }
        } else {
          // deshago el cambio si no se acepta
          currentSolution[i] = valorI;
          currentSolution[j] = valorJ;
          pos++;
        }
      } else {
        // deshago el cambio si la solucion no es valida
        currentSolution[i] = valorI;
        currentSolution[j] = valorJ;
        pos++;
      }
    }

    // actualizo la temperatura y criterio de parada por exitos
    temp = temp / (1.0 + beta * temp);
    hayExito = exitos > 0;
    enfriamientos++;
  }
  return ResultMH(bestSolution, bestFitness, evals);
}
