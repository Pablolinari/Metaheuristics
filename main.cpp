#include "include/mh.h"
#include "include/pportfolio.h"
#include "include/random.hpp"
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
  long int totalevaluations;
  double low, high;
  string path = "";

  // Seed the random number generator

  if (argc == 4) {
    path = argv[1];
    low = stod(argv[2]);
    high = stod(argv[3]);
    seed = 42;
    totalevaluations = 1;
  } else if (argc == 6) {
    path = argv[1];
    low = stod(argv[2]);
    high = stod(argv[3]);
    totalevaluations = stoi(argv[4]);
    seed = stoi(argv[5]);
  } else {
    cout << "Modo de uso : \n ./program <path archivo csv> <low> <high> "
            "<evaluaciones> "
            "<seed> "
         << endl;
    cout << "El <low> y <high> son las restricciones de  porcentaje cada "
            "mercado que se deben cumplir"
         << endl;
    cout << "El numero de evaluaciones es opcional , por defecto es 1" << endl;
    cout << "El seed es opcional , por defecto es 42" << endl;
    cout << "Para cada evaluacion de los algoritmos el seed se incrementa 10 "
            "unidades"
         << endl;
    return 0;
  }


  // Create the algorithms
  RandomSearch ralg = RandomSearch();
  Greedy greedyalg = Greedy();
  LocalSearch localalg = LocalSearch();

  // Create the specific problem
  ProblemPort problem = ProblemPort(path, low, high, false);
  ProblemPort problem25 = ProblemPort(path, low, high, true);
  std::vector<pair<std::string, MH<double> *>> algorithms = {
      make_pair("Random", &ralg), make_pair("Greedy", &greedyalg),
      make_pair("Local Search", &localalg)};
  Eigen::MatrixX<double> fitness(totalevaluations, algorithms.size());
  Eigen::MatrixX<double> evals(totalevaluations, algorithms.size());
  Eigen::MatrixX<double> times(totalevaluations, algorithms.size());
  cout << endl << "..........Ejecutando ..........." << endl;
  for (int j = 0; j < algorithms.size(); j++) {
    for (int i = 0; i < totalevaluations; i++) {
      Random::seed(seed);
      auto mh = algorithms[j].second;
      auto start = high_resolution_clock::now();
      ResultMH<double> result = mh->optimize(problem, 10000);
      auto end = high_resolution_clock::now();

      times(i, j) =
          duration_cast<std::chrono::milliseconds>(end - start).count();
      fitness(i, j) = result.fitness;
      evals(i, j) = result.evaluations;
      seed += 10;
    }

    cout << "Termina " << algorithms[j].first << endl;
  }

  cout << endl << "----------Termina evaluacion-----------" << endl;
  cout << endl;
  for (int i = 0; i < algorithms.size(); i++) {
    cout << endl;
    cout << algorithms[i].first << endl;
    cout << "Mean fitness of " << totalevaluations
         << " evaluations : " << fitness.col(i).mean() << endl;
    cout << "Mean evaluations " << evals.col(i).mean() << endl;
    cout << "Mean time : " << times.col(i).mean() << endl;
  }
  return 0;
}
