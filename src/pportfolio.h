#pragma once
#include "problem.h"
#include "solution.h"
#include <cstddef>
#include <cstdlib>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Eigen>
#include <fstream>
#include <numeric>
#include <string>
#include <utility>
class ProblemPort : public Problem<double> {
  Eigen::MatrixXd sp_500;
  Eigen::MatrixXd
      sp_100; // preguntar : las empresas que estan en sp100 estan en sp500?
  Eigen::MatrixXd ib_35;
  int n100 = 97;
  int n500 = 457;
  int n35 = 30;
  double low = 0.005;
  double hi500 = 0.02;
  double hi100 = 0.05;
  double hi35 = 0.08;
	int lambda = 500;

public:
  ProblemPort(std::string sp500, std::string sp100, std::string ib35)
      : Problem<double>() {
    this->sp_500 = loadCSV(sp500, 2765, this->n500);
    this->sp_100 = loadCSV(sp100, 2765, this->n100);
    this->ib_35 = loadCSV(sp100, 2765, this->n35);
  }

  virtual size_t getSolutionSize() override {
    return this->n500 * this->n100 * this->n35;
  }
  virtual std::pair<double, double> getSolutionDomainRange() override {
    return std::pair<double, double>(this->low, this->hi35);
  }

  tFitness fitness(const std::vector<double> &solution) override {

    return ;
  }

  tSolution<double> createSolution() override {
    int size = this->getSolutionSize();
    tSolution<double> solution(size);

    for (int i = 0; i < this->n35; i++) {
      solution[i] = Random::get(this->low, this->hi35);
    }

    for (int i = this->n35; i < this->n100; i++) {
      solution[i] = Random::get(this->low, this->hi100);
    }

    for (int i = this->n100; i < this->n500; i++) {
      solution[i] = Random::get(this->low, this->hi500);
    }

    int numberZeros = Random::get<int>(1, size - 1);
		std::vector<int> indexZeros = std::vector<int>(size);
		std::iota(indexZeros.begin(),indexZeros.end(),0);
		Random::shuffle(indexZeros);
		for (int i=0; i<numberZeros;i++){
			solution[indexZeros[i]]=0;
		}

    this->fix(solution);
    return solution;
  }

  virtual bool isValid(const tSolution<double> &solution) override {
    double total_sum = std::accumulate(solution.begin(),solution.end(),0.0);
		double error = 0.003;
		if(std::abs(total_sum-1)<=error){
			return true;
		}
		else{
			return false;
		}
  }

  virtual void fix(tSolution<double> &solution) override {
		double total_sum = std::accumulate(solution.begin(),solution.end(),0.0);
	}
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
    int nrows = rows - 1; // quito header de empresas
    int ncols = cols - 1; // quito fechas
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
