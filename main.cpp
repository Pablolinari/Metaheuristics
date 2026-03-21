#include "include/greedy.h"
#include "include/mh.h"
#include "include/pportfolio.h"
#include "include/random.hpp"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>

// All all algorithms
#include "include/randomsearch.h"

using namespace std;
int main(int argc, char *argv[]) {
  long int seed;
  // Seed the random number generator
  if (argc == 1) {
    // Use a fixed seed for reproducibility
    seed = 42;
  } else {
    seed = atoi(argv[1]);
  }

  // Create the algorithms
  RandomSearch ralg = RandomSearch();
  Greedy rgreedy = Greedy();
  // Create the specific problem path ,days , company , low, high
  ProblemPort pproblem = ProblemPort("data/ibex_35.csv", 2813, 30, 0.005, 0.08);
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
  ResultMH result = rgreedy.optimize(pproblem, 1);
  cout << "Best solution: " << result.solution << endl;
  cout << "Best fitness: " << result.fitness << endl;
  cout << "Evaluations: " << result.evaluations << endl;

  return 0;
}
