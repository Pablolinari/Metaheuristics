#include "../include/gwo.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

Gwo::Gwo(int populationSize, double aMax)
    : populationSize(populationSize), aMax(aMax) {}

ResultMHDouble Gwo::optimize(ProblemDouble &problem, int maxevals) {
  assert(maxevals > 0);
  int evals = 0;
  int problemSize = static_cast<int>(problem.getSolutionSize());
  std::pair<double, double> bounds = problem.getSolutionDomainRange();

  std::vector<tSolution<double>> wolves;
  std::vector<double> fitness;
  wolves.reserve(this->populationSize);
  fitness.reserve(this->populationSize);

  tSolution<double> bestSolution(problemSize);
  double bestFitness = -std::numeric_limits<double>::infinity();

  for (int i = 0; i < this->populationSize; i++) {
    tSolution<double> sol = problem.createSolution();
    double fit = problem.fitness(sol);
    evals++;
    wolves.push_back(sol);
    fitness.push_back(fit);
    if (fit > bestFitness) {
      bestFitness = fit;
      bestSolution = sol;
    }
    if (evals >= maxevals) {
      return ResultMH(bestSolution, bestFitness, evals);
    }
  }

  while (evals < maxevals) {
    int idxAlpha = 0;
    int idxBeta = 0;
    int idxDelta = 0;
    double alphaFit = -std::numeric_limits<double>::infinity();
    double betaFit = -std::numeric_limits<double>::infinity();
    double deltaFit = -std::numeric_limits<double>::infinity();

    for (int i = 0; i < this->populationSize; i++) {
      double fit = fitness[i];
      if (fit > alphaFit) {
        deltaFit = betaFit;
        idxDelta = idxBeta;
        betaFit = alphaFit;
        idxBeta = idxAlpha;
        alphaFit = fit;
        idxAlpha = i;
      } else if (fit > betaFit) {
        deltaFit = betaFit;
        idxDelta = idxBeta;
        betaFit = fit;
        idxBeta = i;
      } else if (fit > deltaFit) {
        deltaFit = fit;
        idxDelta = i;
      }
    }

    double progress = static_cast<double>(evals) / maxevals;
    double a = this->aMax - progress * this->aMax;

    for (int i = 0; i < this->populationSize; i++) {
      tSolution<double> newSol(problemSize);
      for (int d = 0; d < problemSize; d++) {
        double r1 = Random::get<double>(0.0, 1.0);
        double r2 = Random::get<double>(0.0, 1.0);
        double A1 = 2.0 * a * r1 - a;
        double C1 = 2.0 * r2;
        double Dalpha = std::abs(C1 * wolves[idxAlpha](d) - wolves[i](d));
        double X1 = wolves[idxAlpha](d) - A1 * Dalpha;

        r1 = Random::get<double>(0.0, 1.0);
        r2 = Random::get<double>(0.0, 1.0);
        double A2 = 2.0 * a * r1 - a;
        double C2 = 2.0 * r2;
        double Dbeta = std::abs(C2 * wolves[idxBeta](d) - wolves[i](d));
        double X2 = wolves[idxBeta](d) - A2 * Dbeta;

        r1 = Random::get<double>(0.0, 1.0);
        r2 = Random::get<double>(0.0, 1.0);
        double A3 = 2.0 * a * r1 - a;
        double C3 = 2.0 * r2;
        double Ddelta = std::abs(C3 * wolves[idxDelta](d) - wolves[i](d));
        double X3 = wolves[idxDelta](d) - A3 * Ddelta;

        double newVal = (X1 + X2 + X3) / 3.0;
        if (newVal < bounds.first) {
          newVal = bounds.first;
        } else if (newVal > bounds.second) {
          newVal = bounds.second;
        }
        newSol(d) = newVal;
      }

      if (!problem.isValid(newSol)) {
        problem.fix(newSol);
      }

      double newFit = problem.fitness(newSol);
      evals++;

      wolves[i] = newSol;
      fitness[i] = newFit;

      if (newFit > bestFitness) {
        bestFitness = newFit;
        bestSolution = newSol;
      }

      if (evals >= maxevals) {
        break;
      }
    }
  }

  return ResultMH(bestSolution, bestFitness, evals);
}
