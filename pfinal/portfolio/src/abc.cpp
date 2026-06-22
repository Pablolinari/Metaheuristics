#include "../include/abc.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

Abc::Abc(int colonySize, int trialLimit)
    : MH(), colonySize(colonySize), trialLimit(trialLimit) {}


// Genera una solución candidata v_i a partir de la fuente x_i
tSolution<double> produceCandidate(const std::vector<tSolution<double>> &foods,
                                   int i, Problem<double> &problem) {
  int SN = foods.size();
  tSolution<double> candidate = foods[i];
  int D = candidate.size();
  if (D == 0) {
    return candidate;
  }

  // dimensión j a perturbar
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

 
  candidate(j) = foods[i](j) + phi * (foods[i](j) - foods[k](j));

  // si es necesario se repara la solución
  if (!problem.isValid(candidate)) {
    problem.fix(candidate);
  }
  return candidate;
}

 //Calcula la probabilidad de que cada fuente sea elegida por una observadora,
 
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


int rouletteSelect(const std::vector<double> &probabilities) {
  double r = Random::get<double>(0.0, 1.0);
  double acc = 0.0;
  for (size_t i = 0; i < probabilities.size(); i++) {
    acc += probabilities[i];
    if (r <= acc) {
      return i;
    }
  }
  return probabilities.size() - 1;
}


ResultMH<double> Abc::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  int evals = 0;
  int problemSize = problem.getSolutionSize();
  if (problemSize == 0) {
    tSolution<double> emptySolution(0);
    return ResultMH(emptySolution, -std::numeric_limits<double>::infinity(), 0);
  }


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

  for (int i = 0; i < nFoodSources; i++) {
    foodSources[i] = problem.createSolution();
    fitnesses[i] = problem.fitness(foodSources[i]);
    evals++;
    updateBest(foodSources[i], fitnesses[i]);
    if (evals >= maxevals) {
      return ResultMH(bestSolution, bestFitness, evals);
    }
  }

  while (evals < maxevals) {
    // Fase de abejas obreras 
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
		//fase de observadoras 
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

    // fase de exploradoras
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
