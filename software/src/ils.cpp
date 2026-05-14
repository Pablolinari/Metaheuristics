#include "../include/ils.h"
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

void Ils::mutation(tSolution<double>&mutSol,int poblationSize,double ratio){
	int nMut = std::ceil(poblationSize*ratio);
	int idx =Random::get(0,poblationSize);
	int idxend =idx+nMut;
	if(idxend>poblationSize){
		idxend = idx-nMut;
	}
	Random::shuffle(mutSol.begin()+idx,mutSol.begin()+idxend);
}
ResultMH<double> Ils::optimize(ProblemDouble &problem, int maxevals) {
  assert(maxevals > 0);
  tSolution<double> bestSolution;
  double bestFitness = -std::numeric_limits<double>::infinity();
  int numsol = 5;
  int localsearchevals = 2000;
  int evals = 0;
  tSolution<double> startSolution = problem.createSolution();

  for (int i = 0; i < numsol; i++) {
    ResultMH<double> localsearchSolution =
        localSearch(problem, localsearchevals, startSolution);
    evals += localsearchSolution.evaluations;
		
    if (localsearchSolution.fitness > bestFitness) {
      bestSolution = localsearchSolution.solution;
      bestFitness = localsearchSolution.fitness;
    }
  }

  return ResultMH(bestSolution, bestFitness, evals);
}
