#include "include/mh.h"
#include "include/pportfolio.h"
#include "include/random.hpp"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>

// All all algorithms
#include "include/randomsearch.h"
#include "include/greedy.h"
#include "include/localsearch.h"
using namespace std;
int main(int argc, char *argv[]) {
  long int seed;
  // Seed the random number generator
  if (argc == 1) {
		cout<< "Modo de uso : \n ./program <nombre_archivo> <dias> <empresas> <numero de algoritmo> <seed>"<<endl;
    seed = 42;
  } else {
    seed = atoi(argv[1]);
  }

  // Create the algorithms
  RandomSearch ralg = RandomSearch();
  Greedy greedyalg = Greedy();
	LocalSearch localalg = LocalSearch();
  // Create the specific problem path ,days , company , low, high
  ProblemPort ib35problem = ProblemPort("data/ibex_35.csv", 2813, 30, 0.005, 0.08);
  ProblemPort sp100problem = ProblemPort("data/syp_100.csv", 2813, 30, 0.005, 0.05);
  ProblemPort sp500problem = ProblemPort("data/syp_500.csv", 2813, 30, 0.005, 0.02);
	
  ProblemPort problem = ProblemPort(argv[2], 2813, 97, 0.005, 0.05);
	
  /*
    // Solve using evaluations
    vector<pair<string, MH<double> *>> algoritmos = {
        make_pair("RandomSearch", &ralg), make_pair("Greedy", &rgreedy)};

    for (int i = 0; i < algoritmos.size(); i++) {
      Random::seed(seed);
      cout << algoritmos[i].first << endl;
      auto mh = algoritmos[i].second;
      ResultMH result = mh->optimize(pproblem, 1000);
      cout << "Best solution: " << result.solution << endl;
      cout << "Best fitness: " << result.fitness << endl;
      cout << "Evaluations: " << result.evaluations << endl;
    }
    */
  Random::seed(seed);
  ResultMH ralgresult = ralg.optimize(problem,10);
  ResultMH greedyresult = greedyalg.optimize(problem,1);
  ResultMH localresult = localalg.optimize(problem,200);
	cout << "Random Search"<<endl;
  cout << "Best solution: " << ralgresult.solution.transpose() << endl;
  cout << "Best fitness: " << ralgresult.fitness << endl;
  cout << "Evaluations: " << ralgresult.evaluations << endl;
	cout << "Greedy"<<endl;
  cout << "Best solution: " << greedyresult.solution.transpose() << endl;
  cout << "Best fitness: " << greedyresult.fitness << endl;
  cout << "Evaluations: " << greedyresult.evaluations << endl;
	cout << "Local Search"<<endl;
  cout << "Best solution: " << localresult.solution.transpose() << endl;
  cout << "Best fitness: " << localresult.fitness << endl;
  cout << "Evaluations: " << localresult.evaluations << endl;
  return 0;
}
