#include "../include/pportfolio.h"
#include "../include/random.hpp"
#include "../include/utils.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

ProblemPort::ProblemPort(std::string path, double low, double high,
                         bool inc2025, int lambda)
    : Problem<double>(), low(low), high(high), lambda(lambda) {
  this->data = loadCSV(path, inc2025);
  this->covarianceMat = covarianceMatrix(this->data);
  this->days = data.rows();
  this->ncompany = data.cols();
}

size_t ProblemPort::getSolutionSize() { return this->data.cols(); }

std::pair<double, double> ProblemPort::getSolutionDomainRange() {
  return std::make_pair(this->low, this->high);
}

double ProblemPort::actionMean(int i) {
  Eigen::VectorX<double> column = data.col(i);
  return column.mean();
}

double ProblemPort::covariance(int i, int j) {
  Eigen::VectorX<double> centerColumni = data.col(i).array() - actionMean(i);
  Eigen::VectorX<double> cenrterColumnj = data.col(j).array() - actionMean(j);
  Eigen::VectorX<double> product =
      centerColumni.array() * cenrterColumnj.array();
  return product.sum() / this->days;
}

Eigen::MatrixX<double>
ProblemPort::covarianceMatrix(Eigen::MatrixX<double> mat) {
  Eigen::VectorXd columnsMean = mat.colwise().mean();
  Eigen::MatrixX<double> centered = mat.rowwise() - columnsMean.transpose();
  return (centered.transpose() * centered) / (mat.rows());
}

double ProblemPort::profit(const tSolution<double> &solution) {
  Eigen::MatrixXd logdata = (data.array() + 1).log();
  Eigen::VectorXd logmeans = logdata.colwise().sum();
  return (solution.array() * logmeans.array()).sum();
}

double ProblemPort::risk(const tSolution<double> &solution) {
  return (solution.transpose() * this->covarianceMat * solution).value();
}

tFitness ProblemPort::fitness(const tSolution<double> &solution) {
  return profit(solution) - this->lambda * risk(solution);
}

tSolution<double> ProblemPort::createSolution() {
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
  solution.array() /= solution.sum();
  if (!isValid(solution)) {
    this->fix(solution);
  }
  return solution;
}

bool ProblemPort::isValid(const tSolution<double> &solution) {
  bool valid = true;

  if (std::abs(solution.sum() - 1.0) > this->ERROR) {
    valid = false;
  }

  // Permitir pesos exactamente 0 o dentro de [low, high]
  if (!(((solution.array() == 0.0) ||
         ((solution.array() >= this->low) && (solution.array() <= this->high)))
            .all())) {
    valid = false;
  }
  return valid;
}

void ProblemPort::fix(tSolution<double> &solution) {
  // Normalizar solo si la suma es positiva
  if (solution.sum() > 0) {
    solution = solution.array() / solution.sum();
  }

  std::vector<int> modificables;
  std::vector<int> zeros;

  // Clasificar componentes según su valor
  for (int i = 0; i < this->ncompany; i++) {
    if (solution(i) < this->low) {
      // Valores menores que low los ponemos a cero
      solution(i) = 0.0;
      zeros.push_back(i);
    } else if (solution(i) > this->high) {
      solution(i) = this->high;
      modificables.push_back(i);
    } else {
      // Valores válidos en [low, high]
      modificables.push_back(i);
    }
  }

  if (solution.sum() < 1) {
    // Calculamos cuánto falta para llegar a 1.0
    double faltante = 1.0 - solution.sum();

    // Evitar división por cero
    if (modificables.empty() && zeros.empty()) {
      // Caso extremo: distribuir equitativamente
      double valor = 1.0 / this->ncompany;
      for (int i = 0; i < this->ncompany; i++) {
        solution(i) = valor;
      }
      return;
    }

    double sobra = 0.0;

    // Intentar repartir entre modificables
    if (!modificables.empty()) {
      double reparto = faltante / modificables.size();

      // Actualizar modificables eliminando los que lleguen al tope
      std::vector<int> nuevos_modificables;
      for (int i : modificables) {
        solution(i) += reparto;
        if (solution(i) > this->high) {
          sobra += solution(i) - this->high;
          solution(i) = this->high;
        } else {
          nuevos_modificables.push_back(i);
        }
      }
      modificables = nuevos_modificables;
    } else {
      // Si no hay modificables, todo el faltante va a sobra para distribuir
      // en zeros
      sobra = faltante;
    }

    // Redistribuir el excedente (sobra) entre modificables que no estén al
    // tope
    while (sobra > this->ERROR && !modificables.empty()) {
      double reparto = sobra / modificables.size();
      std::vector<int> nuevos_modificables;

      for (int idx : modificables) {
        if (solution(idx) < this->high) {
          double espacio = this->high - solution(idx);
          double a_sumar = std::min(sobra / modificables.size(), espacio);

          solution(idx) += a_sumar;
          sobra -= a_sumar;

          if (solution(idx) < this->high - this->ERROR) {
            nuevos_modificables.push_back(idx);
          }
        }
      }

      // Si no pudimos redistribuir, salir del bucle
      if (nuevos_modificables.size() == modificables.size()) {
        break;
      }
      modificables = nuevos_modificables;
    }

    // Si aún queda sobra y está en rango válido, activar un cero
    if (sobra >= this->low && sobra <= this->high && !zeros.empty()) {
      solution(zeros[0]) = sobra;
      zeros.erase(zeros.begin());
      sobra = 0;
    } else if (sobra > this->high && !zeros.empty()) {
      // Si sobra es mayor que high, distribuir en múltiples ceros
      while (sobra > this->ERROR && !zeros.empty()) {
        if (sobra >= this->high) {
          solution(zeros[0]) = this->high;
          zeros.erase(zeros.begin());
          sobra -= this->high;
        } else if (sobra >= this->low) {
          solution(zeros[0]) = sobra;
          zeros.erase(zeros.begin());
          sobra = 0;
        } else {
          // El sobra es muy pequeño, repartir entre modificables
          break;
        }
      }
    }

    // Corrección final: si queda un residuo muy pequeño, ajustar
    if (std::abs(solution.sum() - 1.0) > this->ERROR) {
      double ajuste = 1.0 - solution.sum();

      // Buscar el primer valor que podamos ajustar
      for (int i = 0; i < this->ncompany; i++) {
        if (solution(i) > 0 && solution(i) < this->high) {
          double nuevo_valor = solution(i) + ajuste;
          if (nuevo_valor >= this->low && nuevo_valor <= this->high) {
            solution(i) = nuevo_valor;
            break;
          }
        }
      }
    }
  }
}

double ProblemPort::heuristic(int i) {
  Eigen::VectorX<double> vectorFormat = data.col(i).array() + 1;
  double logmean = (vectorFormat.array().log().sum());
  double sigmamean = this->covarianceMat.col(i).sum() / this->ncompany;
  return logmean - (this->lambda * sigmamean);
}

void ProblemPort::testclass() {
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
