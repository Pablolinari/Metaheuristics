#include "../include/agg.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <set>
#include <utility>
#include <vector>

ResultMH<double> Agg::optimize(Problem<double> &problem, int maxevals) {
  assert(maxevals > 0);
	int evals;
  int npob = 100;
  double probcruce = 0.8;
  double probmut = 0.8;
  int k = 3;
	double bestFitness = -std::numeric_limits<double>::infinity();
	tSolution<double> bestSolution(problem.getSolutionSize());
  std::vector<tSolution<double>> poblation;
  std::vector<tSolution<double>> parents;
  std::vector<tSolution<double>> sons;
  std::set<int> idxparents;
  for (int i = 0; i < npob; i++) {
    poblation.push_back(problem.createSolution());
  }
	for(int i=0;i<npob;i++){
		double tempFitness = problem.fitness(poblation[i]);
		if(tempFitness>bestFitness){
			bestFitness = tempFitness;
			bestSolution = poblation[i];
		}
	}
  std::vector<int> idx;
  // torneo para npob veces
  for (int i = 0; i < npob; i++) {
    for (int j = 0; j < k; j++) {
      idx.push_back(Random::get(0, npob - 1));
    }
    double fitnessTournament = -std::numeric_limits<double>::infinity();
    int idxBestSolution = 0;
    for (int j = 0; j < k; j++) {
      double actualFitness = problem.fitness(poblation[idx[j]]);
      if (actualFitness > fitnessTournament) {
        fitnessTournament = actualFitness;
        idxBestSolution = idx[j];
      }
    }
    idxparents.insert(idxBestSolution);
  }

  // genero los padres de la siguiente generación
  for (auto it = idxparents.begin(); it != idxparents.end(); it++) {
    parents.push_back(poblation[*it]);
  }
	//cruce
  for (int i = 0; i <= npob - 2; i += 2) {
    if (Random::get<bool>(this->crossoverProb)) {

      std::pair<tSolution<double>, tSolution<double>> son =
          crossover(parents[i], parents[i + 1]);
    }else{
			sons.push_back(parents[i]);
			sons.push_back(parents[i+1]);
		}
  }

	//mutación
	bool mutado =false;
	while(!mutado){

	}

	
	return ResultMH(bestSolution,bestFitness,evals);
}
