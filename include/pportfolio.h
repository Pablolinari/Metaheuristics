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

void fix(tSolution<double>& solution) override {
    int n = solution.size();
    if (n <= 0) return;

    // --- PASO 1: Normalización inicial ---
    double sum = std::accumulate(solution.begin(), solution.end(), 0.0);
    if (sum > 0) {
        for (double& w : solution) w /= sum;
    } else {
        // Si la suma es 0, inicializamos equitativamente para evitar errores
        for (double& w : solution) w = 1.0 / n;
    }

    // --- PASO 2: Ajuste estricto al rango [lo, hi] ---
    for (double& w : solution) {
        if (w < this->low) w = 0.0;
        else if (w > this->high) w = this->high;
    }

    // --- PASO 3: Corrección iterativa de la suma ---
    // Aumentamos MAX_ITER y refinamos la lógica de reparto
    const int MAX_ITER = 20; 
    for (int iter = 0; iter < MAX_ITER; ++iter) {
        double current_sum = std::accumulate(solution.begin(), solution.end(), 0.0);
        double diff = 1.0 - current_sum;

        if (std::abs(diff) < 1e-12) break; 

        if (diff > 0) { // Falta para llegar a 1
            std::vector<int> can_grow;
            for (int i = 0; i < n; ++i) {
                if (solution[i] < this->high) can_grow.push_back(i);
            }
            
            if (can_grow.empty()) {
                // Si nadie puede crecer y falta suma, forzamos a los que son 0 a ser 'low'
                for (int i = 0; i < n && diff > 0; ++i) {
                    if (solution[i] == 0) {
                        solution[i] = std::min(this->low, diff);
                        diff -= solution[i];
                    }
                }
                break; 
            }
            
            double share = diff / can_grow.size();
            for (int idx : can_grow) {
                double added = std::min(share, this->high - solution[idx]);
                solution[idx] += added;
            }
        } 
        else { // Sobra (diff es negativo)
            std::vector<int> can_shrink;
            for (int i = 0; i < n; ++i) {
                if (solution[i] > this->low) can_shrink.push_back(i);
                else if (solution[i] > 0 && solution[i] <= this->low) can_shrink.push_back(i);
            }
            
            if (can_shrink.empty()) break;
            
            double amount_to_take = std::abs(diff);
            double share = amount_to_take / can_shrink.size();
            for (int idx : can_shrink) {
                // No permitimos que baje de 'low' a menos que lo mandemos a 0
                double can_take = solution[idx] - this->low;
                if (can_take > 0) {
                    double taken = std::min(share, can_take);
                    solution[idx] -= taken;
                } else {
                    // Si ya está en 'low' o cerca, lo mandamos a 0 si hace falta
                    solution[idx] = 0.0;
                }
            }
        }
    }

    // --- PASO 4: El "Seguro de Vida" (Ajuste Final de Precisión) ---
    // Forzamos que los valores estén en {0} U [low, high]
    double final_sum = 0;
    for (double& w : solution) {
        if (w > 0 && w < this->low) w = this->low;
        if (w > this->high) w = this->high;
        final_sum += w;
    }

    // Si después de limpiar el rango la suma no es 1, ajustamos el primer elemento disponible
    double final_diff = 1.0 - final_sum;
    if (std::abs(final_diff) > 1e-12) {
        for (double& w : solution) {
            double newValue = w + final_diff;
            if ((newValue >= this->low && newValue <= this->high) || std::abs(newValue) < 1e-12) {
                w = (std::abs(newValue) < 1e-12) ? 0.0 : newValue;
                break;
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
