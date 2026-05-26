#include "../include/ils.h"
#include "../include/localsearch.h"
#include <cmath>

tSolution<double> Ils::mutation(Problem<double> &problem,
                                tSolution<double> mutSol, double ratio) {

	tSolution<double> retSolution = mutSol;
	int nselection = std::ceil(ratio*problem.getSolutionSize());

	int index = Random::get<int>(0,problem.getSolutionSize()-1);
	int endindex;

	if(index + nselection > problem.getSolutionSize()){
		endindex = index;
		index = endindex-nselection;
	}else{
		endindex = index+nselection;
	}

	Random::shuffle(retSolution.begin()+index,retSolution.begin()+endindex);

	return retSolution;
}

ResultMH<double> Ils::optimize(ProblemDouble &problem, int maxevals) {
  assert(maxevals > 0);
  int localsearchevals = 2000;
  int evals = 0;
  int niter = 4;

  tSolution<double> currentSolution = problem.createSolution();
  double currentFitness = problem.fitness(currentSolution);
  evals++;
	LocalSearch localsearch  = LocalSearch(0.4);

  ResultMH<double> resultLocal =
      localsearch.optimize(problem, currentSolution,currentFitness,localsearchevals);
  evals += resultLocal.evaluations;
  tSolution<double> bestSolution = resultLocal.solution;
  double bestFitness = resultLocal.fitness;

  for (int i = 0; i < niter; i++) {

    tSolution<double> mutSol = mutation(problem, bestSolution, 0.2);
    ResultMH<double> iterLocal = localsearch.optimize(problem, mutSol, problem.fitness(mutSol),localsearchevals);
		
    evals += iterLocal.evaluations;
    if (iterLocal.fitness > bestFitness) {
      bestFitness = iterLocal.fitness;
      bestSolution = iterLocal.solution;
    }
  }
  return ResultMH(bestSolution, bestFitness, evals);
}
