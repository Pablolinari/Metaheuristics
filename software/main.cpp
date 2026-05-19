#include "include/Eigen/Core"
#include "include/mh.h"
#include "include/pportfolio.h"
#include "include/problem.h"
#include "include/random.hpp"
#include <algorithm>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

// All all algorithms
#include "include/agearit.h"
#include "include/ageblx.h"
#include "include/aggarit.h"
#include "include/aggblx.h"
#include "include/amall.h"
#include "include/ambest.h"
#include "include/amrand.h"
#include "include/greedy.h"
#include "include/localsearch.h"
#include "include/randomsearch.h"
#include "include/solution.h"
#include "include/es.h"
#include "include/ils.h"
#include "include/bmb.h"

using namespace std;
using namespace std::chrono;
int main(int argc, char *argv[]) {
  long int seed;
  long int totalevaluations;
  double low, high;
  string path = "";
  string output_csv = "";

  if (argc == 4) {
    path = argv[1];
    low = stod(argv[2]);
    high = stod(argv[3]);
    seed = 42;
    totalevaluations = 1;

    ProblemPort problem = ProblemPort(path, low, high, false);
    AgeArit agearit(100, 0.1, 3);
    AgeBLX ageblx(100, 0.1, 0.3, 3);

    ResultMH<double> result = agearit.optimize(problem, 10000);
    ResultMH<double> result2 = ageblx.optimize(problem, 10000);
    cout << "arit" << endl;
    cout << "Fitness :" << result.fitness << endl;
    cout << "Evals :" << result.evaluations << endl;
    cout << "blx" << endl;
    cout << "Fitness :" << result2.fitness << endl;
    cout << "Evals :" << result2.evaluations << endl;

    return 0;
  } else if (argc == 6) {
    path = argv[1];
    low = stod(argv[2]);
    high = stod(argv[3]);
    totalevaluations = stoi(argv[4]);
    seed = stoi(argv[5]);
  } else if (argc == 7) {
    path = argv[1];
    low = stod(argv[2]);
    high = stod(argv[3]);
    totalevaluations = stoi(argv[4]);
    seed = stoi(argv[5]);
    output_csv = argv[6];
  } else {

    cout << "Modo de uso : \n ./program <path archivo csv> <low> <high> "
            "<evaluaciones> "
            "<seed> <output_file_name>"
         << endl;
    cout << "El <low> y <high> son las restricciones de  porcentaje "
            "que se debe cumplir"
         << endl;
    cout << "El numero de evaluaciones es opcional , por defecto es 1" << endl;
    cout << "El seed es opcional , por defecto es 42" << endl;
    cout << "Para cada evaluacion de los algoritmos el seed se incrementa 10 "
            "unidades"
         << endl;
    cout << "Si se proporciona un sexto parametro, se genera un CSV con los "
            "resultados"
         << endl;
    return 0;
  }

  // Create the algorithms
  RandomSearch ralg = RandomSearch();
  Greedy greedyalg = Greedy();
  LocalSearch localalg = LocalSearch();

  AggArit aggarit(100, 0.8, 0.1, 3);
  AggBLX aggblx(100, 0.8, 0.1, 0.3, 3);

  AgeArit agearit(100, 0.1, 3);
  AgeBLX ageblx(100, 0.1, 0.3, 3);

  AmAll amall(100, 0.8, 0.1, 3);
  AmRand amrand(100, 0.8, 0.1, 3);
  AmBest ambest(100, 0.8, 0.1, 3);
	Ils ils = Ils();
	Bmb bmb = Bmb();
	Es enfriamiento =Es();
  // Create the specific problem
  ProblemPort problem = ProblemPort(path, low, high, false);
  ProblemPort problem25 = ProblemPort(path, low, high, true);
  std::vector<pair<std::string, MH<double> *>> algorithms = {
      make_pair("Greedy", &greedyalg),      make_pair("Random", &ralg),
      make_pair("Local Search", &localalg), make_pair("AM-BEST", &ambest),make_pair("BMB", &bmb),make_pair("ILS", &ils),make_pair("ES", &enfriamiento)};
  Eigen::MatrixX<double> fitness(totalevaluations, algorithms.size());
  Eigen::MatrixX<double> profit2025(totalevaluations, algorithms.size());
  Eigen::MatrixX<double> fitness2025(totalevaluations, algorithms.size());
  Eigen::MatrixX<double> evals(totalevaluations, algorithms.size());
  Eigen::MatrixX<double> times(totalevaluations, algorithms.size());
  vector<tSolution<double>> solutions;
  cout << endl << "..........Ejecutando ..........." << endl;
  for (int j = 0; j < algorithms.size(); j++) {
    tSolution<double> sol;
    for (int i = 0; i < totalevaluations; i++) {
      Random::seed(seed);
      auto mh = algorithms[j].second;
      auto start = high_resolution_clock::now();
      ResultMH<double> result = mh->optimize(problem, 10000);
      auto end = high_resolution_clock::now();
      sol = result.solution;

      times(i, j) =
          duration_cast<std::chrono::milliseconds>(end - start).count();
      fitness(i, j) = result.fitness;
      fitness2025(i, j) = problem25.fitness(result.solution);
      profit2025(i, j) = problem25.profit(result.solution);
      evals(i, j) = result.evaluations;

      seed += 10;
    }
    solutions.push_back(sol);
    cout << "Termina " << algorithms[j].first << endl;
  }

  cout << "----------Termina evaluacion-----------";
  for (int i = 0; i < algorithms.size(); i++) {
    cout << endl;
    cout << algorithms[i].first << endl;
    cout << "Mean fitness of " << totalevaluations
         << " evaluations : " << fitness.col(i).mean() << endl;
    cout << "Fitness 2025 :" << fitness2025.col(i).mean();
    cout << " | Profit 2025 :" << profit2025.col(i).mean() << endl;
    cout << "Mean evaluations " << evals.col(i).mean();
    cout << " | Mean time (miliseconds): " << times.col(i).mean() << endl;
  }
  if (argc == 7) {
    // Generate CSV if requested
    ofstream csv_file(output_csv);
    if (csv_file.is_open()) {
      // Write header
      csv_file << "alg,fitness" << endl;

      // Write data for each algorithm and iteration
      for (int j = 0; j < algorithms.size(); j++) {
        for (int i = 0; i < totalevaluations; i++) {
          csv_file << algorithms[j].first.substr(0, 6) << "," << fitness(i, j)
                   << endl;
        }
      }

      csv_file.close();
      cout << endl << "CSV generado en: " << output_csv << endl;
    } else {
      cerr << "Error: No se pudo crear el archivo CSV " << output_csv << endl;
    }
  }
  return 0;
}
