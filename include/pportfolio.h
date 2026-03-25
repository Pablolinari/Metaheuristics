#pragma once
#include "problem.h"
#include "solution.h"
#include "utils.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <string>

class ProblemPort : public Problem<double> {
  int days;
  int ncompany;
  double low;
  double high;
  int lambda;
  Eigen::MatrixXd data;
  Eigen::MatrixXd covarianceMat;

public:
  ProblemPort(std::string path, double low, double high, bool inc2025,
              int lambda = 500)
      : Problem<double>(), low(low), high(high), lambda(lambda) {

    this->data = loadCSV(path, inc2025);
    this->covarianceMat = covarianceMatrix(this->data);
    this->days = data.rows();
    this->ncompany = data.cols();
  }

  size_t getSolutionSize() override { return this->data.cols(); }

  std::pair<double, double> getSolutionDomainRange() override {
    return std::make_pair(this->low, this->high);
  }
  double actionMean(int i) {
    Eigen::VectorX<double> column = data.col(i);
    return column.mean();
  }
  double covariance(int i, int j) {
    Eigen::VectorX<double> centerColumni = data.col(i).array() - actionMean(i);
    Eigen::VectorX<double> cenrterColumnj = data.col(j).array() - actionMean(j);
    Eigen::VectorX<double> product =
        centerColumni.array() * cenrterColumnj.array();

    return product.sum() / this->days;
  }

  Eigen::MatrixX<double> covarianceMatrix(Eigen::MatrixX<double> mat) {
    Eigen::VectorXd columnsMean = mat.colwise().mean();
    Eigen::MatrixX<double> centered = mat.rowwise() - columnsMean.transpose();
    return (centered.transpose() * centered) / (mat.rows());
  }
  double profit(const tSolution<double> &solution) {
    Eigen::MatrixXd logdata = data.array() + 1;
    logdata = logdata.array().log();
    Eigen::VectorXd logmeans = logdata.colwise().mean();
    return (solution.array() * logmeans.array()).sum();
  }
  double risk(const tSolution<double> &solution) {
    return (solution.transpose() * this->covarianceMat * solution).value();
  }

  tFitness fitness(const tSolution<double> &solution) override {
    return profit(solution) - this->lambda * risk(solution);
  }

  tSolution<double> createSolution() override {
    int size = this->getSolutionSize();
    tSolution<double> solution(size);

    for (int i = 0; i < size; i++) {
      solution(i) = Random::get(this->low, this->high);
    }

    int numberZeros = Random::get<int>(1, size - 1);

    std::vector<int> index(size);
    for (int i = 0; i < size; i++)
      index[i] = i;
    Random::shuffle(index);
    for (int i = 0; i < numberZeros; i++) {
      solution(index[i]) = 0;
    }
    if (isValid(solution)) {
      return solution;
    } else {
      this->fix(solution);

      return solution;
    }
  }

  bool isValid(const tSolution<double> &solution) override {
    bool valid = true;
    double error = 0.0003;
    if (std::fabs(solution.sum() - 1) >= error) {
      valid = false;
    }
    if (!(solution.array() <= this->high && solution.array() >= this->low)
             .all()) {
      valid = false;
    }
    return valid;
  }

  void fix(tSolution<double> &solution) override {
    solution = solution.array() / solution.sum();

    for (int i = 0; i < solution.size(); i++) {
      if (solution(i) < this->low) {
        solution(i) = 0.0;
      } else if (solution(i) > this->high) {
        solution(i) = this->high;
      }
    }
    if (solution.sum() < 1) {
      double waste = 1 - solution.sum();

      if (waste < this->low) {
        std::vector<int> validIndexs;
        for (int i = 0; i < solution.size(); i++) {
          if (solution(i) > this->low && solution(i) < this->high) {
            validIndexs.push_back(i);
          }
        }
        double spreadWaste = waste / validIndexs.size();
        for (int i : validIndexs) {
          solution(i) += spreadWaste;
        }

      } else {
        std::vector<int> zerosIndex;
        for (int i = 0; i < solution.size(); i++) {
          if (solution(i) == 0.0) {
            zerosIndex.push_back(i);
          }
        }
        if (!zerosIndex.empty()) {
          auto randomit = Random::get(zerosIndex);
          solution(*randomit) = waste;
        }
      }
    }
  }

  double heuristic(int i) override {
    Eigen::VectorX<double> vectorFormat = data.col(i).array() + 1;
    double logmean = (vectorFormat.array().log().sum()) / this->days;
    double sigmamean = this->covarianceMat.col(i).sum() / this->days;
    return logmean - (this->lambda * sigmamean);
  }

  void testclass() {

    std::cout << this->data.rows() << std::endl;
    std::cout << this->data.block(0, 0, 5, 20) << std::endl;
  }
};
