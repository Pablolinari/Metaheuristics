#include <iostream>
#include "problem.h"
#include <string>
#include <istream>
#include <fstream>
// Real problem class
//#include "pportfolio.h"
#include <eigen3/Eigen/Eigen>
// All all algorithms
using namespace std;
Eigen::MatrixXd loadCSV(const string &path, int rows, int cols) {
	ifstream file(path);
    if (!file.is_open()) {
      throw runtime_error("Could not open file at: " + path);
    }

    Eigen::MatrixXd mat(rows, cols);

    string line;
    int currentRow = 0;

    // Read line by line until we reach the end of file or the expected row
    // count
    while (getline(file, line) /*&& currentRow < rows*/) {
      stringstream ss(line);
      string cell;
      int currentCol = 0;

      // Extract each cell separated by a comma
      while (getline(ss, cell, ',')  /*&&currentCol < cols*/) {
        try {
          // Convert string to double and insert directly into the matrix
          mat(currentRow, currentCol) = stod(cell);
        } catch (const invalid_argument &e) {
          // Handle non-numeric data (like headers or empty cells)
          mat(currentRow, currentCol) = 0.0;
        }
        currentCol++;
      }
      currentRow++;
    }

    file.close();
    return mat;
  }


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
  // Create the specific problem
  //ProblemPort problem = ProblemPort(10);


  return 0;
}
