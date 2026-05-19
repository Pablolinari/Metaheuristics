#include "../include/ils.h"
#include <algorithm>
#include <cmath>
#include <limits>

ResultMH<double> Ils::localSearch(Problem<double> &problem, int maxevals,
                                  tSolution<double> solInit) {
  assert(maxevals >= 1);
  double ratio = 0.15;
  // primera solución aleatoria
  tSolution<double> startSolution = solInit;
  double bestFitness = problem.fitness(startSolution);
  bool improved = true;
  std::vector<std::pair<int, int>> neighbors;
  // genero el vector vecindario
  for (int i = 0; i < problem.getSolutionSize(); i++) {
    for (int j = 0; j < problem.getSolutionSize(); j++) {
      if (i != j) {
        neighbors.emplace_back(std::make_pair(i, j));
      }
    }
  }
  int evals = 0;
  // mientras no gaste las evaluaciones o no haya recorrido
  // el vecindario entero sigo probando
  while (evals < maxevals && improved) {
    improved = false;
    std::pair<double, double> bounds = problem.getSolutionDomainRange();
    // genero un vecindario
    Random::shuffle(neighbors);
    for (std::pair<int, int> p : neighbors) {
      // guardo los valores originales para poder restaurar la solucion
      double amount = startSolution(p.first) * ratio;
      double originalFirst = startSolution(p.first);
      double originalSecond = startSolution(p.second);

      // cambio el valor en las posiciones indicadas
      startSolution(p.first) -= amount;
      startSolution(p.second) += amount;
      // si la solución es valida calculo su fitness
      // si el fitness es mejor , me quedo con la modificación hecha
      // y me salgo del for para generar un nuevo vecindario para
      // la nueva solución escogida
      bool validFirst =
          startSolution(p.first) >= bounds.first || startSolution(p.first) == 0;
      bool validSecond = (startSolution(p.second) >= bounds.first &&
                          startSolution(p.second) <= bounds.second) ||
                         (startSolution(p.second) == 0);
      if (validSecond && validFirst) {
        double currentFitness = problem.fitness(startSolution);
        evals++;
        if (currentFitness > bestFitness) {

          bestFitness = currentFitness;

          improved = true;
          break;
        }
      }
      // si la solución no mejora el fitness deshago el cambio
      startSolution(p.first) = originalFirst;
      startSolution(p.second) = originalSecond;

      if (evals == maxevals) {
        break;
      }
    }
  }
  return ResultMH(startSolution, bestFitness, evals);
}

namespace {
int pickDifferentCluster(int current, int minCluster, int maxCluster) {
  if (minCluster == maxCluster) {
    return current;
  }
  int candidate = current;
  int attempts = 0;
  while (candidate == current && attempts < 10) {
    candidate = Random::get<int>(minCluster, maxCluster);
    attempts++;
  }
  if (candidate == current) {
    if (current == minCluster) {
      return minCluster + 1;
    }
    return minCluster;
  }
  return candidate;
}
}

tSolution<double> Ils::mutation(Problem<double> &problem,
                                tSolution<double> mutSol, double ratio) {
  tSolution<double> retSol = mutSol;
  int solutionSize = static_cast<int>(problem.getSolutionSize());
  if (solutionSize <= 0) {
    return retSol;
  }
  int nMut = std::max(1, static_cast<int>(std::ceil(solutionSize * ratio)));

  std::pair<double, double> bounds = problem.getSolutionDomainRange();
  int minCluster = static_cast<int>(bounds.first);
  int maxCluster = static_cast<int>(bounds.second);
  if (minCluster == maxCluster) {
    return retSol;
  }

  std::vector<int> indices(solutionSize);
  for (int i = 0; i < solutionSize; i++) {
    indices[i] = i;
  }
  Random::shuffle(indices);

  int changed = 0;
  int idx = 0;
  while (changed < nMut && idx < solutionSize) {
    int pos = indices[idx];
    int currentCluster = static_cast<int>(std::round(retSol(pos)));
    int newCluster = pickDifferentCluster(currentCluster, minCluster, maxCluster);
    if (newCluster != currentCluster) {
      retSol(pos) = static_cast<double>(newCluster);
      changed++;
    }
    idx++;
  }

  if (!problem.isValid(retSol)) {
    problem.fix(retSol);
  }
  return retSol;
}
ResultMH<double> Ils::optimize(ProblemDouble &problem, int maxevals) {
  assert(maxevals > 0);
  int localsearchevals = 2000;
  int evals = 0;
	int niter = 4;

  tSolution<double> currentSolution = problem.createSolution();
  double currentFitness = problem.fitness(currentSolution);
  evals++;

  ResultMH<double> resultLocal =
      localSearch(problem, localsearchevals, currentSolution);
  evals += resultLocal.evaluations;
  tSolution<double> bestSolution = resultLocal.solution;
  double bestFitness = resultLocal.fitness;

  for(int i=0; i< niter;i++) {

    tSolution<double> mutSol = mutation(problem, bestSolution, 0.2);
    ResultMH<double> iterLocal = localSearch(problem, localsearchevals, mutSol);
    evals += iterLocal.evaluations;
    if (iterLocal.fitness > bestFitness) {
      bestFitness = iterLocal.fitness;
      bestSolution = iterLocal.solution;
    }
  }
  return ResultMH(bestSolution, bestFitness, evals);
}
