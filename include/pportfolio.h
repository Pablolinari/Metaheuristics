#pragma once
#include "../include/random.hpp"
#include "problem.h"
#include "solution.h"
#include "utils.hpp"
#include <cmath>
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

class ProblemPort : public Problem<double> {
  int days;
  int ncompany;
  double low;
  double high;
  int lambda;
  const double ERROR = 0.00005;
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
    Eigen::MatrixXd logdata = (data.array() + 1).log();
    Eigen::VectorXd logmeans = logdata.colwise().sum();
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
    if (!isValid(solution)) {
      this->fix(solution);
    }
    return solution;
  }

  bool isValid(const tSolution<double> &solution) override {
    bool valid = true;
    if (std::fabs(solution.sum() - 1) >= this->ERROR) {
      valid = false;
    }
    if (!(solution.array() <= this->high).all() &&
        !(solution.array() >= this->low).all()) {
      valid = false;
    }
    return valid;
  }

  void fix(tSolution<double> &solution) override {
    int n = solution.size();
    if (n == 0)
      return;

    // 1. Normalizar inicialmente para que la suma sea 1
    double current_sum = std::accumulate(solution.begin(), solution.end(), 0.0);
    if (current_sum > 0) {
      for (double &w : solution) {
        w /= current_sum;
      }
    }

    // 2. Asegurar que estén en el rango [lo, hi]
    // Nota: La imagen dice "w_i <- 0 si w_i < lo", esto implica un posible
    // incumplimiento del rango inferior a menos que lo sea 0.
    for (double &w : solution) {
      if (w < this->low)
        w = 0.0;
      else if (w > this->high)
        w = this->high;
    }

    // 3. Comprobar la suma de nuevo
    current_sum = std::accumulate(solution.begin(), solution.end(), 0.0);

    // 4. Si la suma es menor que 1
    if (current_sum < 1.0) {
      double diff = 1.0 - current_sum;

      // Identificar candidatos: pesos > 0 y < hi
      std::vector<int> candidates;
      for (int i = 0; i < n; ++i) {
        if (solution[i] > 0 && solution[i] < this->high) {
          candidates.push_back(i);
        }
      }

      if (!candidates.empty()) {
        double share = diff / candidates.size();
        for (int idx : candidates) {
          double can_add = this->high - solution[idx];
          double to_add = std::min(share, can_add);
          solution[idx] += to_add;
          diff -= to_add;
        }
      }

      // Si aún falta (diff > 0), incrementar pesos que son 0 (aleatorios)
      if (diff > 0.000001) { // Margen de error de punto flotante
        for (int i = 0; i < n && diff > 0; ++i) {
          if (solution[i] == 0) {
            double to_add = std::min(diff, this->high);
            solution[i] = to_add;
            diff -= to_add;
          }
        }
      }
    }
    // 5. Si la suma es mayor que 1
    else if (current_sum > 1.0) {
      double diff = current_sum - 1.0;

      // Identificar candidatos: pesos > lo
      std::vector<int> candidates;
      for (int i = 0; i < n; ++i) {
        if (solution[i] > this->low) {
          candidates.push_back(i);
        }
      }

      if (!candidates.empty()) {
        double share = diff / candidates.size();
        for (int idx : candidates) {
          double can_subtract = solution[idx] - this->low;
          double to_subtract = std::min(share, can_subtract);
          solution[idx] -= to_subtract;
          diff -= to_subtract;
        }
      }
    }
  }

  /*
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
    if (solution(i) > this->low && solution(i)< this->high) {
      validIndexs.push_back(i);
    }
  }
  double spreadWaste = waste / validIndexs.size();
  for (int i : validIndexs) {
                                  if(solution(i)+spreadWaste<=high){
                                          solution(i)+=spreadWaste;
                                  }
  }

} else{
  std::vector<int> zerosIndex;
                          double waste = 1- solution.sum();
  for (int i = 0; i < solution.size(); i++) {
    if (solution(i) == 0.0) {
      zerosIndex.push_back(i);
    }
  }
  while(!(std::fabs(solution.sum()-1)<= 0.00003)||!zerosIndex.empty()) {
    auto randomidx = Random::get(zerosIndex);
    solution(*randomidx) = waste;
                                  zerosIndex.erase(randomidx);
  }
}
}
}
*/
  double heuristic(int i) override {
    Eigen::VectorX<double> vectorFormat = data.col(i).array() + 1;
    double logmean = (vectorFormat.array().log().sum());
    double sigmamean = this->covarianceMat.col(i).sum() / this->ncompany;
    return logmean - (this->lambda * sigmamean);
  }

  void testclass() {
    int notvalid = 0;
    for (int i = 0; i < 10000; i++) {
      tSolution<double> sol = this->createSolution();
      if (!this->isValid(sol)) {
        std::cout << "No es valida" << std::endl;
        double error = 0.0003;
        if (std::fabs(sol.sum() - 1) >= error) {

          std::cout << "no cumple suma : " << 1 - sol.sum() << std::endl;
        }
        // std::cout << sol.transpose() << std::endl;
        notvalid++;
      }
    }
    std::cout << "total no validas : " << notvalid << std::endl;
  }
};
