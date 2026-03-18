#include "include/pportfolio.h"
#include "include/random.hpp"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>

using namespace std;
using Random = effolkronium::random_static;
int main() {

  ProblemPort problem("data/ibex_35.csv", 2813, 30, 0.005, 0.08);

	Eigen::VectorXd v= problem.createSolution();
  cout << endl;
  cout << v.sum()<< endl;


      return 0;
}
