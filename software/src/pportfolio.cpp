#include "../include/pportfolio.h"
#include "../include/utils.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

ProblemPort::ProblemPort(std::string path, double low, double high,
                         bool inc2025, int lambda)
    : Problem<double>(), low(low), high(high), lambda(lambda) {
  this->data = loadCSV(path, inc2025);
  this->covarianceMat = this->covarianceMatrix();
  this->days = data.rows();
  this->ncompany = data.cols();
	this->logSums=logSumsVector();
}

size_t ProblemPort::getSolutionSize() { return this->data.cols(); }

std::pair<double, double> ProblemPort::getSolutionDomainRange() {
  return std::make_pair(this->low, this->high);
}

Eigen::VectorXd ProblemPort::logSumsVector() {
  Eigen::MatrixXd logdata = (this->data.array() + 1).log();
  return logdata.colwise().sum();
}

Eigen::MatrixXd ProblemPort::covarianceMatrix() {
  Eigen::VectorXd columnsMean = this->data.colwise().mean();
  Eigen::MatrixX<double> centered = this->data.rowwise() - columnsMean.transpose();
  return (centered.transpose() * centered) / (this->data.rows());
}

double ProblemPort::profit(const tSolution<double> &solution) {
  return (solution.array() * this->logSums.array()).sum();
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
  double suma = solution.sum();
  if (suma > 0) {
    solution = solution.array() / suma;
  }

  // Pre-alocar vectores con capacidad estimada para evitar realocaciones
  std::vector<int> modificables;
  std::vector<bool> es_modificable(this->ncompany, false);
  std::vector<int> zeros;
  modificables.reserve(this->ncompany);
  zeros.reserve(this->ncompany);

  // Clasificar componentes según su valor
  for (int i = 0; i < this->ncompany; i++) {
    if (solution(i) < this->low) {
      // Valores menores que low los ponemos a cero
      solution(i) = 0.0;
      zeros.push_back(i);
    } else if (solution(i) > this->high) {
      solution(i) = this->high;
      modificables.push_back(i);
      es_modificable[i] = true;
    } else {
      // Valores válidos en [low, high]
      modificables.push_back(i);
      es_modificable[i] = true;
    }
  }

  suma = solution.sum();
  if (suma < 1.0) {
    // Calculamos cuánto falta para llegar a 1.0
    double faltante = 1.0 - suma;

    // Evitar división por cero
    if (modificables.empty() && zeros.empty()) {
      // Caso extremo: distribuir equitativamente
      double valor = 1.0 / this->ncompany;
      solution.setConstant(valor);
      return;
    }

    double sobra = 0.0;

    // Intentar repartir entre modificables
    if (!modificables.empty()) {
      double reparto = faltante / modificables.size();

      // Marcar elementos que lleguen al tope usando el vector booleano
      for (int i : modificables) {
        solution(i) += reparto;
        if (solution(i) > this->high) {
          sobra += solution(i) - this->high;
          solution(i) = this->high;
          es_modificable[i] = false; // Ya no es modificable
        }
      }
    } else {
      // Si no hay modificables, todo el faltante va a sobra para distribuir
      // en zeros
      sobra = faltante;
    }

    // Redistribuir el excedente (sobra) entre modificables que no estén al tope
    // Usar un límite de iteraciones para evitar bucles infinitos
    int max_iter = modificables.size() + 1;
    int iter = 0;
    while (sobra > this->ERROR && iter < max_iter) {
      // Contar cuántos modificables quedan
      int count_modificables = 0;
      for (int idx : modificables) {
        if (es_modificable[idx]) {
          count_modificables++;
        }
      }

      if (count_modificables == 0)
        break;

      double reparto = sobra / count_modificables;
      double sobra_anterior = sobra;

      for (int idx : modificables) {
        if (es_modificable[idx] && solution(idx) < this->high) {
          double espacio = this->high - solution(idx);
          double a_sumar = std::min(reparto, espacio);

          solution(idx) += a_sumar;
          sobra -= a_sumar;

          if (solution(idx) >= this->high - this->ERROR) {
            es_modificable[idx] = false;
          }
        }
      }

      // Si no pudimos redistribuir nada, salir del bucle
      if (std::abs(sobra - sobra_anterior) < this->ERROR) {
        break;
      }
      iter++;
    }

    // Si aún queda sobra, distribuir en zeros usando índice en lugar de erase
    int zero_idx = 0;
    if (sobra >= this->low && sobra <= this->high && zero_idx < zeros.size()) {
      solution(zeros[zero_idx]) = sobra;
      zero_idx++;
      sobra = 0;
    } else if (sobra > this->high) {
      // Si sobra es mayor que high, distribuir en múltiples ceros
      while (sobra > this->ERROR && zero_idx < zeros.size()) {
        if (sobra >= this->high) {
          solution(zeros[zero_idx]) = this->high;
          zero_idx++;
          sobra -= this->high;
        } else if (sobra >= this->low) {
          solution(zeros[zero_idx]) = sobra;
          zero_idx++;
          sobra = 0;
        } else {
          // El sobra es muy pequeño, salir
          break;
        }
      }
    }

    // Corrección final: si queda un residuo muy pequeño, ajustar
    double diff = 1.0 - solution.sum();
    if (std::abs(diff) > this->ERROR) {
      // Buscar el primer valor que podamos ajustar
      for (int i = 0; i < this->ncompany; i++) {
        if (solution(i) > 0 && solution(i) < this->high) {
          double nuevo_valor = solution(i) + diff;
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
  double logmean = this->logSums(i);
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
