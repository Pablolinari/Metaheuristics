#pragma once
#include "problem.h"
#include <cstddef>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Eigen>
#include <fstream>
#include <string>
class ProblemPort : public Problem<Eigen::VectorX<double>> {
  Eigen::MatrixXd sp_500;
  Eigen::MatrixXd
      sp_100; // preguntar : las empresas que estan en sp100 estan en sp500?
  Eigen::MatrixXd ib_35;
  Eigen::VectorXd solution;

public:
  ProblemPort(std::string sp500, std::string sp100, std::string ib35)
      : Problem<Eigen::VectorX<double>>() {
    this->sp_500 = loadCSV(sp500, 2765, 457);
    this->sp_100 = loadCSV(sp100, 2765, 97);
    this->ib_35 = loadCSV(sp100, 2765, 30);
  }

  virtual size_t getSolutionSize() override { return solution.size(); }
  virtual Eigen::VectorX<double> getSolutionDomainRange() override {
		Eigen::VectorX<double> v(this->sp_100.cols()*this->sp_500.cols()*this->ib_35.cols());
		v.setZero();
    return v;
  }

  tFitness fitness(const tSolution < std::vector<> & solution) override {
    tFitness count = 0;

    for (int i = 0; i < solution.size(); i++) {
      if (solution[i]) {
        if (i % 2 == 0) {
          count += 1;
        } else {
          count -= 1;
        }
      }
    }
    return count;
  }

  tSolution<int> createSolution() override {
    tSolution<int> solution(size);
    for (int i = 0; i < solution.size(); i++) {
      solution[i] = Random::get<bool>();
    }
    return solution;
  }

  virtual bool isValid(const tSolution<int> &solution) override { return true; }

  virtual void fix(tSolution<int> &solution) override {}
  /**
   * Loads a CSV file directly into an Eigen Matrix.
   * @param path The file system path to the .csv file.
   * @param rows The expected number of rows.
   * @param cols The expected number of columns.
   * @return An Eigen::MatrixXd populated with the CSV data.
   */

  static Eigen::MatrixXd loadCSV(const std::string &path, int rows, int cols) {
    std::ifstream file(path);
    if (!file.is_open()) {
      throw std::runtime_error("No se pudo abrir el archivo en: " + path);
    }
		int nrows = rows-1; // quito header de empresas
		int ncols = cols-1; //quito fechas
    Eigen::MatrixXd mat(nrows, ncols);
    std::string line;
    int currentRow = 0;
    std::string trash;
    getline(file, trash);
    while (getline(file, line) && currentRow < nrows) {
      std::stringstream ss(line);
      std::string cell;
      int currentCol = 0;
      getline(ss, trash, ',');
      while (getline(ss, cell, ',') && currentCol < ncols) {
        if (!cell.empty()) {
          mat(currentRow, currentCol) = stod(cell);
        } else {
          mat(currentRow, currentCol) = 0.0;
        }
        currentCol++;
      }
      currentRow++;
    }

    file.close();
    return mat;
  }
};
