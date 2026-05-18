#include "../include/bco.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>

Bco::Bco(int colonySize, int trialLimit)
    : colonySize(colonySize), trialLimit(trialLimit) {}

tSolution<double> perturbSolution(const tSolution<double> &solution,
                                  Problem<double> &problem) {
  tSolution<double> candidate = solution;
  int size = static_cast<int>(candidate.size());
  if (size < 2) {
    return candidate;
  }

  int i = Random::get<int>(0, size - 1);
  int j = Random::get<int>(0, size - 2);
  if (j >= i) {
    j += 1;
  }

  double ratio = Random::get<double>(0.05, 0.3);
  double amount = candidate(i) * ratio;
  if (amount <= 0.0) {
    return candidate;
  }

  candidate(i) -= amount;
  candidate(j) += amount;
  if (!problem.isValid(candidate)) {
    problem.fix(candidate);
  }
  return candidate;
}

std::vector<double> buildProbabilities(const std::vector<double> &fitnesses) {
  std::vector<double> probs(fitnesses.size(), 0.0);
  if (fitnesses.empty()) {
    return probs;
  }
  double minFitness = *std::min_element(fitnesses.begin(), fitnesses.end());
  double shift = 0.0;
  if (minFitness <= 0.0) {
    shift = -minFitness + 1e-9;
  }

  double total = 0.0;
  for (size_t i = 0; i < fitnesses.size(); i++) {
    probs[i] = fitnesses[i] + shift;
    total += probs[i];
  }

  if (total <= 0.0) {
    double uniform = 1.0 / static_cast<double>(fitnesses.size());
    std::fill(probs.begin(), probs.end(), uniform);
    return probs;
  }

  for (size_t i = 0; i < probs.size(); i++) {
    probs[i] /= total;
  }
  return probs;
}

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

ResultMH<double> Bco::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
  int evals = 0;
  int problemSize = problem.getSolutionSize();

  int nFoodSources = colonySize/2;
  std::vector<tSolution<double>> foodSources(nFoodSources);
  std::vector<double> fitnesses(nFoodSources);
  std::vector<int> trials(nFoodSources, 0);

  tSolution<double> bestSolution(problemSize);
  double bestFitness = -std::numeric_limits<double>::infinity();

  for (int i = 0; i < nFoodSources; i++) {
    foodSources[i] = problem.createSolution();
    fitnesses[i] = problem.fitness(foodSources[i]);
    evals++;
    if (fitnesses[i] > bestFitness) {
      bestFitness = fitnesses[i];
      bestSolution = foodSources[i];
    }
    if (evals >= maxevals) {
      return ResultMH(bestSolution, bestFitness, evals);
    }
  }

  while (evals < maxevals) {
    // fase empleadas
    for (int i = 0; i < nFoodSources && evals < maxevals; i++) {
      tSolution<double> candidate = perturbSolution(foodSources[i], problem);
      double candidateFitness = problem.fitness(candidate);
      evals++;
      if (candidateFitness > fitnesses[i]) {
        foodSources[i] = candidate;
        fitnesses[i] = candidateFitness;
        trials[i] = 0;
      } else {
        trials[i] += 1;
      }
      if (candidateFitness > bestFitness) {
        bestFitness = candidateFitness;
        bestSolution = candidate;
      }
    }

    if (evals >= maxevals) {
      break;
    }

    // fase observadoras 
    std::vector<double> probabilities = buildProbabilities(fitnesses);
    int onlookers = nFoodSources;
    for (int k = 0; k < onlookers && evals < maxevals; k++) {
      int idx = rouletteSelect(probabilities);
      tSolution<double> candidate = perturbSolution(foodSources[idx], problem);
      double candidateFitness = problem.fitness(candidate);
      evals++;
      if (candidateFitness > fitnesses[idx]) {
        foodSources[idx] = candidate;
        fitnesses[idx] = candidateFitness;
        trials[idx] = 0;
      } else {
        trials[idx] += 1;
      }
      if (candidateFitness > bestFitness) {
        bestFitness = candidateFitness;
        bestSolution = candidate;
      }
    }

    if (evals >= maxevals) {
      break;
    }

    // Scout bees phase
    for (int i = 0; i < nFoodSources && evals < maxevals; i++) {
      if (trials[i] >= trialLimit) {
        foodSources[i] = problem.createSolution();
        fitnesses[i] = problem.fitness(foodSources[i]);
        evals++;
        trials[i] = 0;
        if (fitnesses[i] > bestFitness) {
          bestFitness = fitnesses[i];
          bestSolution = foodSources[i];
        }
      }
    }
  }

  return ResultMH(bestSolution, bestFitness, evals);
}
