#pragma once
#include "../include/random.hpp"
#include "problem.h"
#include "solution.h"
#include "utils.hpp"
#include <algorithm>
#include <cmath>
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <numeric>
#include <ostream>
#include <string>
#include <vector>

class ProblemPort : public Problem<double> {
  int days;
  int ncompany;
  double low;
  double high;
  int lambda;
  const double ERROR = 0.0000000005;
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
		solution.array()/=solution.sum();
    if (!isValid(solution)) {
      this->fix(solution);
    }
    return solution;
  }

  bool isValid(const tSolution<double> &solution) override {
    bool valid = true;

    if (std::abs(solution.sum() - 1.0) > this->ERROR) {
      valid = false;
    }

    // Permitir pesos exactamente 0 o dentro de [low, high]
    if (!(((solution.array() == 0.0) || ((solution.array() >= this->low) && (solution.array() <= this->high))).all())) {
      valid = false;
    }
    return valid;
  }

  void fix(tSolution<double> &solution) override {
    const double EPS = 1e-12;
    int n = solution.size();
    if (n <= 0)
      return;

    double sum = std::accumulate(solution.begin(), solution.end(), 0.0);
    if (sum > 0) {
      solution /= sum;
    } else {
      solution.setConstant(1.0 / n);
    }

    for (int i = 0; i < n; ++i) {
      if (solution(i) < this->low)
        solution(i) = 0.0;
      else if (solution(i) > this->high)
        solution(i) = this->high;
    }

    auto add_mass = [&](double deficit, bool only_positive) {
      if (deficit <= EPS)
        return 0.0;

      std::vector<int> idx;
      idx.reserve(n);
      for (int i = 0; i < n; ++i) {
        if (solution(i) < this->high - EPS && (!only_positive || solution(i) > 0))
          idx.push_back(i);
      }

      if (idx.empty())
        return deficit;

      double capacity = 0.0;
      for (int i : idx)
        capacity += (this->high - solution(i));

      if (capacity <= EPS)
        return deficit;

      double used = std::min(deficit, capacity);
      for (int i : idx) {
        double share = used * (this->high - solution(i)) / capacity;
        solution(i) += share;
        deficit -= share;
      }
      return deficit;
    };

    auto remove_mass = [&](double surplus, bool keep_low) {
      if (surplus <= EPS)
        return 0.0;

      std::vector<int> idx;
      idx.reserve(n);
      for (int i = 0; i < n; ++i) {
        double limit = keep_low ? this->low : 0.0;
        if (solution(i) > limit + EPS)
          idx.push_back(i);
      }

      if (idx.empty())
        return surplus;

      double capacity = 0.0;
      for (int i : idx) {
        double limit = keep_low ? this->low : 0.0;
        capacity += (solution(i) - limit);
      }

      if (capacity <= EPS)
        return surplus;

      double used = std::min(surplus, capacity);
      for (int i : idx) {
        double limit = keep_low ? this->low : 0.0;
        double take = used * (solution(i) - limit) / capacity;
        solution(i) = std::max(limit, solution(i) - take);
        surplus -= take;
      }
      return surplus;
    };

    sum = solution.sum();
    if (sum < 1.0 - EPS) {
      double deficit = 1.0 - sum;
      deficit = add_mass(deficit, true);
      if (deficit > EPS) {
        deficit = add_mass(deficit, false);
      }
    } else if (sum > 1.0 + EPS) {
      double surplus = sum - 1.0;
      surplus = remove_mass(surplus, true);
      if (surplus > EPS) {
        surplus = remove_mass(surplus, false);
      }
    }

    for (int i = 0; i < n; ++i) {
      if (solution(i) < this->low - EPS && solution(i) > 0)
        solution(i) = this->low;
      if (solution(i) < EPS)
        solution(i) = 0.0;
      if (solution(i) > this->high)
        solution(i) = this->high;
    }

    double final_sum = solution.sum();
    double diff = 1.0 - final_sum;

    if (std::abs(diff) > EPS) {
      if (diff > 0) {
        for (int i = 0; i < n && diff > EPS; ++i) {
          if (solution(i) < this->high - EPS) {
            double add = std::min(this->high - solution(i), diff);
            solution(i) += add;
            diff -= add;
          }
        }
      } else {
        double surplus = -diff;
        for (int i = 0; i < n && surplus > EPS; ++i) {
          double limit = (solution(i) > this->low + EPS) ? this->low : 0.0;
          if (solution(i) > limit + EPS) {
            double take = std::min(solution(i) - limit, surplus);
            solution(i) -= take;
            surplus -= take;
          }
        }
      }
    }

    for (int i = 0; i < n; ++i) {
      if (solution(i) < EPS)
        solution(i) = 0.0;
      if (solution(i) > this->high)
        solution(i) = this->high;
      if (solution(i) > 0 && solution(i) < this->low - EPS)
        solution(i) = this->low;
    }

    double final_sum_check = solution.sum();
    if (std::abs(final_sum_check - 1.0) > this->ERROR) {
      double adjust = 1.0 - final_sum_check;
      for (int i = 0; i < n; ++i) {
        if (adjust > 0 && solution(i) < this->high - EPS) {
          double add = std::min(this->high - solution(i), adjust);
          solution(i) += add;
          adjust -= add;
        } else if (adjust < 0 && solution(i) > this->low + EPS) {
          double take = std::min(solution(i) - this->low, -adjust);
          solution(i) -= take;
          adjust += take;
        }
        if (std::abs(adjust) <= this->ERROR)
          break;
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
