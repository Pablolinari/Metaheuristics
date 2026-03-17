#include "src/random.hpp"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using Random = effolkronium::random_static;
int main() {
  int numberZeros = Random::get<int>(1, 10 - 1);
  Eigen::VectorXd indexZeros = Eigen::VectorXd::LinSpaced(10, 10 - 1, 0);
  Random::shuffle(indexZeros);
  cout << indexZeros;
  return 0;
}
