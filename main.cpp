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
  vector<int> v = {1, 2, 3, 4};
  Random::shuffle(v);
	cout <<endl;
		cout<< std::accumulate(v.begin(), v.end(), 0);
	cout << endl;

  return 0;
}
