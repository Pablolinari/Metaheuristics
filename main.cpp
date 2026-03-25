#include "include/mh.h"
#include "include/pportfolio.h"
#include "include/problem.h"
#include "include/random.hpp"
#include <complex>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

// All all algorithms
#include "include/greedy.h"
#include "include/localsearch.h"
#include "include/randomsearch.h"
using namespace std;
using namespace std::chrono;
int main(int argc, char *argv[]) {
  long int seed;
  long int evaluations = 1;
  string path = "";

  // Seed the random number generator
  if (argc == 1) {
    cout << "Modo de uso : \n ./program <path archivo csv>  <evaluaciones> "
            "<seed> "
         << endl;
    cout << "El numero de evaluaciones es opcional , por defecto es 1" << endl;
    cout << "El seed es opcional , por defecto es 42" << endl;
    cout << "Para cada evaluacion de los algoritmos el seed se incrementa 10 "
            "unidades"
         << endl;
		return 0;
  }
	if(argc == 2){
		path = argv[1];
		seed = 42;
		evaluations =1;
	}
  if (argc == 4) {
		path = argv[1];
		evaluations =stoi(argv[2]);
		seed = stoi(argv[3]);
  }

  // Create the algorithms
  RandomSearch ralg = RandomSearch();
  Greedy greedyalg = Greedy();
  LocalSearch localalg = LocalSearch();
  // Create the specific problem

  ProblemPort problem = ProblemPort(argv[2], 0.005, 0.05, false);
  ProblemPort problem25 = ProblemPort(argv[2], 0.005, 0.05, true);
  std::vector<pair<std::string, MH<double> *>> algorithms = {
      make_pair("Random", &ralg), make_pair("Greedy", &greedyalg),
      make_pair("Local Search", &localalg)};
  Eigen::MatrixX<double> fitness(evaluations, algorithms.size(), 0);
  Eigen::MatrixX<double> evals(evaluations, algorithms.size(), 0);
  Eigen::MatrixX<double> times(evaluations, algorithms.size(), 0);
  int maxevals = 1;
  for (int i = 0; i < algorithms.size(); i++) {
    for (int j = 0; j < evaluations; j++) {
      Random::seed(seed);
      auto mh = algorithms[i].second;
      auto start = high_resolution_clock::now();
      ResultMH<double> result = mh->optimize(problem, 10000);
      auto end = high_resolution_clock::now();

      times(i, j) =
          duration_cast<std::chrono::milliseconds>(start - end).count();
      fitness(i, j) = result.fitness;
      evals(i, j) = result.evaluations;
      seed += 10;
    }
  }
  for (int i = 0; i < algorithms.size(); i++) {
    cout << algorithms[i].first << endl;
    cout << "Mean fitness of " << evaluations
         << " evaluations : " << fitness.col(i).mean() << endl;
    cout << "Mean evaluations " << evals.col(i).mean() << endl;
    cout << "Mean time : " << times.col(i).mean() << endl;
  }

  return 0;
}
