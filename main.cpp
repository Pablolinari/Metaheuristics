#include "include/mh.h"
#include "include/pportfolio.h"
#include "include/problem.h"
#include "include/random.hpp"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <utility>
#include <vector>

// All all algorithms
#include "include/randomsearch.h"
#include "include/greedy.h"
#include "include/localsearch.h"
using namespace std;
int main(int argc, char *argv[]) {
  long int seed;
	long int evaluations=1;
	string path="";

  // Seed the random number generator
  if (argc == 1) {
		cout<< "Modo de uso : \n ./program <path archivo csv> <evaluaciones> <seed>"<<endl;
		cout<< "El seed es opcional , por defecto es 42"<<endl;
		cout<< "Para cada evaluacion de los algoritmos el seed se incrementa 10 unidades"<<endl;
    seed = 42;
  } 
  if(argc == 3){

	}

  // Create the algorithms
  RandomSearch ralg = RandomSearch();
  Greedy greedyalg = Greedy();
	LocalSearch localalg = LocalSearch();
  // Create the specific problem
	
  ProblemPort problem = ProblemPort(argv[2], 0.005, 0.05,false);
  ProblemPort problem25 = ProblemPort(argv[2], 0.005, 0.05,true);
	std::vector<pair<string,MH<double>*>> algorithms = {make_pair("random",&ralg),make_pair("greedy",&greedyalg),make_pair("localsearch",&localalg)};
	for(pair<string,MH<double>*> p: algorithms){
		for(int i=0;i<evaluations;i++){
		Random::seed(seed);
			p.second->optimize(problem, 10000);
		}
	}


  Random::seed(seed);
  ResultMH ralgresult = ralg.optimize(problem,10);
  ResultMH greedyresult = greedyalg.optimize(problem,1);
  ResultMH localresult = localalg.optimize(problem,8000);
	cout << "Random Search"<<endl;
  //cout << "Best solution: " << ralgresult.solution.transpose() << endl;
  cout << "Best fitness: " << ralgresult.fitness << endl;
  cout << "Evaluations: " << ralgresult.evaluations << endl;
	cout << "Profit :"<<problem.profit(ralgresult.solution)<<endl;
	cout << "Greedy"<<endl;
  //cout << "Best solution: " << greedyresult.solution.transpose() << endl;
  cout << "Best fitness: " << greedyresult.fitness << endl;
  cout << "Evaluations: " << greedyresult.evaluations << endl;
	cout << "Profit :"<<problem.profit(greedyresult.solution)<<endl;
	cout << "Local Search"<<endl;
  //cout << "Best solution: " << localresult.solution.transpose() << endl;
  cout << "Best fitness: " << localresult.fitness << endl;
  cout << "Evaluations: " << localresult.evaluations << endl;
	cout << "Profit :"<<problem.profit(localresult.solution)<<endl;
  return 0;
}
