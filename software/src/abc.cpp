#include "../include/abc.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

Abc::Abc(int colonySize, int trialLimit)
    : MH(), colonySize(colonySize), trialLimit(trialLimit) {}

namespace {

/*
 * Genera una solución candidata v_i a partir de la fuente x_i aplicando la
 * ecuación (2.2) del artículo:
 *
 *      v_{ij} = x_{ij} + phi_{ij} * (x_{ij} - x_{kj})
 *
 * donde:
 *   - j es una dimensión (componente) elegida al azar,
 *   - k != i es otra fuente vecina elegida al azar,
 *   - phi_{ij} es un número aleatorio en [-1, 1].
 *
 * Sólo se modifica la componente j; el resto se copian de x_i. Como el
 * movimiento rompe las restricciones del problema (los pesos deben sumar 1 y
 * estar en [low, high] o ser 0), se repara la solución con problem.fix().
 */
tSolution<double> produceCandidate(const std::vector<tSolution<double>> &foods,
                                   int i, Problem<double> &problem) {
  int SN = static_cast<int>(foods.size());
  tSolution<double> candidate = foods[i]; // v_i parte de x_i
  int D = static_cast<int>(candidate.size());
  if (D == 0) {
    return candidate;
  }

  // dimensión j a perturbar, aleatoria en {0, ..., D-1}
  int j = Random::get<int>(0, D - 1);

  // fuente vecina k aleatoria y distinta de i (si hay al menos dos fuentes)
  int k = i;
  if (SN > 1) {
    k = Random::get<int>(0, SN - 2);
    if (k >= i) {
      k += 1; // se salta i para que k != i con probabilidad uniforme
    }
  }

  // factor de escala phi en [-1, 1]
  double phi = Random::get<double>(-1.0, 1.0);

  // ecuación (2.2): se perturba únicamente la componente j
  candidate(j) = foods[i](j) + phi * (foods[i](j) - foods[k](j));

  // el movimiento deja la solución fuera de la región factible: la reparamos
  if (!problem.isValid(candidate)) {
    problem.fix(candidate);
  }
  return candidate;
}

/*
 * Calcula la probabilidad de que cada fuente sea elegida por una observadora,
 * según la ecuación (2.1):
 *
 *      p_i = fit_i / sum_{n=1}^{SN} fit_n
 *
 * La ecuación supone cantidades de néctar (fitness) no negativas. Como en el
 * problema del portfolio el fitness (beneficio - lambda * riesgo) puede ser
 * negativo, se desplazan todos los valores para que el peor quede ligeramente
 * por encima de 0, conservando las diferencias relativas entre fuentes.
 */
std::vector<double> computeProbabilities(const std::vector<double> &fitnesses) {
  std::vector<double> probs(fitnesses.size(), 0.0);
  if (fitnesses.empty()) {
    return probs;
  }

  double minFit = *std::min_element(fitnesses.begin(), fitnesses.end());
  double shift = (minFit <= 0.0) ? (-minFit + 1e-9) : 0.0;

  double total = 0.0;
  for (size_t i = 0; i < fitnesses.size(); i++) {
    probs[i] = fitnesses[i] + shift;
    total += probs[i];
  }

  if (total <= 0.0) { // todas las fuentes iguales: reparto uniforme
    double uniform = 1.0 / static_cast<double>(fitnesses.size());
    std::fill(probs.begin(), probs.end(), uniform);
    return probs;
  }

  for (double &p : probs) {
    p /= total;
  }
  return probs;
}

/*
 * Selección por ruleta: devuelve el índice de una fuente elegida con
 * probabilidad proporcional a probabilities[i].
 */
int rouletteSelect(const std::vector<double> &probabilities) {
  double r = Random::get<double>(0.0, 1.0);
  double acc = 0.0;
  for (size_t i = 0; i < probabilities.size(); i++) {
    acc += probabilities[i];
    if (r <= acc) {
      return static_cast<int>(i);
    }
  }
  return static_cast<int>(probabilities.size() - 1);
}

} // namespace

/*
 * Algoritmo Artificial Bee Colony.
 *
 * Estructura por ciclos (mientras queden evaluaciones):
 *   (a) Fase de abejas empleadas.
 *   (b) Fase de abejas observadoras.
 *   (c) Fase de abeja exploradora (como máximo una por ciclo).
 *
 * En todas las fases se aplica selección greedy entre la fuente y su candidata,
 * y se memoriza la mejor solución encontrada hasta el momento.
 */
ResultMH<double> Abc::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  int evals = 0;
  int problemSize = static_cast<int>(problem.getSolutionSize());
  if (problemSize == 0) {
    tSolution<double> emptySolution(0);
    return ResultMH(emptySolution, -std::numeric_limits<double>::infinity(), 0);
  }

  // SN = número de fuentes de alimento = número de empleadas = número de
  // observadoras = mitad del tamaño de la colonia.
  int nFoodSources = std::max(1, colonySize / 2);
  std::vector<tSolution<double>> foodSources(nFoodSources);
  std::vector<double> fitnesses(nFoodSources);
  std::vector<int> trials(nFoodSources, 0); // ciclos sin mejora de cada fuente

  tSolution<double> bestSolution(problemSize);
  double bestFitness = -std::numeric_limits<double>::infinity();

  // memoriza la mejor solución global encontrada
  auto updateBest = [&](const tSolution<double> &sol, double fit) {
    if (fit > bestFitness) {
      bestFitness = fit;
      bestSolution = sol;
    }
  };

  // ---- Inicialización: SN fuentes de alimento aleatorias ----
  for (int i = 0; i < nFoodSources; i++) {
    foodSources[i] = problem.createSolution();
    fitnesses[i] = problem.fitness(foodSources[i]);
    evals++;
    updateBest(foodSources[i], fitnesses[i]);
    if (evals >= maxevals) {
      return ResultMH(bestSolution, bestFitness, evals);
    }
  }

  // ---- Ciclos de búsqueda ----
  while (evals < maxevals) {
    // (a) Fase de abejas empleadas: cada empleada explota su propia fuente.
    for (int i = 0; i < nFoodSources && evals < maxevals; i++) {
      tSolution<double> candidate = produceCandidate(foodSources, i, problem);
      double candidateFitness = problem.fitness(candidate);
      evals++;
      if (candidateFitness > fitnesses[i]) { // selección greedy
        foodSources[i] = candidate;
        fitnesses[i] = candidateFitness;
        trials[i] = 0;
      } else {
        trials[i]++;
      }
      updateBest(candidate, candidateFitness);
    }

    // (b) Fase de abejas observadoras: eligen fuente por ruleta (ec. 2.1) y la
    // explotan igual que las empleadas (ec. 2.2). Se reparten SN observadoras.
    std::vector<double> probabilities = computeProbabilities(fitnesses);
    for (int t = 0; t < nFoodSources && evals < maxevals; t++) {
      int i = rouletteSelect(probabilities);
      tSolution<double> candidate = produceCandidate(foodSources, i, problem);
      double candidateFitness = problem.fitness(candidate);
      evals++;
      if (candidateFitness > fitnesses[i]) { // selección greedy
        foodSources[i] = candidate;
        fitnesses[i] = candidateFitness;
        trials[i] = 0;
      } else {
        trials[i]++;
      }
      updateBest(candidate, candidateFitness);
    }

    // (c) Fase de abeja exploradora: como máximo una por ciclo. Se abandona la
    // fuente con más ciclos sin mejora si supera el límite, sustituyéndola por
    // una fuente aleatoria nueva.
    if (evals < maxevals) {
      int worst = 0;
      for (int i = 1; i < nFoodSources; i++) {
        if (trials[i] > trials[worst]) {
          worst = i;
        }
      }
      if (trials[worst] >= trialLimit) {
        foodSources[worst] = problem.createSolution();
        fitnesses[worst] = problem.fitness(foodSources[worst]);
        evals++;
        trials[worst] = 0;
        updateBest(foodSources[worst], fitnesses[worst]);
      }
    }
  }

  return ResultMH(bestSolution, bestFitness, evals);
}
