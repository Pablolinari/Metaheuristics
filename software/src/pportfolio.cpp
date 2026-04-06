#include "../include/pportfolio.h"
#include "../include/utils.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

/*
 * @brief Constructor del problema de portfolio
 * @params path: ruta al archivo CSV con datos de retornos
 * @params low: límite inferior para los pesos del portfolio
 * @params high: límite superior para los pesos del portfolio
 * @params inc2025: flag para incluir datos del 2025
 * @params lambda: parámetro de aversión al riesgo
 * */
ProblemPort::ProblemPort(std::string path, double low, double high,
                         bool inc2025, int lambda)
    : Problem<double>(), low(low), high(high), lambda(lambda) {
  this->data = loadCSV(path, inc2025);
  this->covarianceMat = this->covarianceMatrix();
  this->days = data.rows();
  this->ncompany = data.cols();
  this->logSums = logSumsVector();
}

/*
 * @brief Obtiene el tamaño de la solución
 * @params ninguno
 * @returns número de columnas de la matriz de datos (número de compañías)
 * */
size_t ProblemPort::getSolutionSize() { return this->data.cols(); }

/*
 * @brief Obtiene el rango del dominio de la solución
 * @params ninguno
 * @returns par con los límites inferior y superior de los pesos
 * */
std::pair<double, double> ProblemPort::getSolutionDomainRange() {
  return std::make_pair(this->low, this->high);
}

/*
 * @brief Calcula el vector de sumas logarítmicas de los retornos
 * @params ninguno
 * @returns vector con la suma de log(retorno+1) para cada compañía
 * */
Eigen::VectorXd ProblemPort::logSumsVector() {
  Eigen::MatrixXd logdata = (this->data.array() + 1).log();
  return logdata.colwise().sum();
}

/*
 * @brief Calcula la matriz de covarianza de los retornos
 * @params ninguno
 * @returns matriz de covarianza de los datos centrados
 * */
Eigen::MatrixXd ProblemPort::covarianceMatrix() {
  Eigen::VectorXd columnsMean = this->data.colwise().mean();
  Eigen::MatrixX<double> centered =
      this->data.rowwise() - columnsMean.transpose();
  return (centered.transpose() * centered) / (this->data.rows());
}

/*
 * @brief Calcula el beneficio esperado de una solución
 * @params solution: vector de pesos del portfolio
 * @returns valor del beneficio esperado calculado
 * */
double ProblemPort::profit(const tSolution<double> &solution) {
  return (solution.array() * this->logSums.array()).sum();
}

/*
 * @brief Calcula el riesgo de una solución
 * @params solution: vector de pesos del portfolio
 * @returns valor del riesgo calculado usando la matriz de covarianza
 * */
double ProblemPort::risk(const tSolution<double> &solution) {
  return (solution.transpose() * this->covarianceMat * solution).value();
}
/*
 * @brief Calcula la función fitness del problema
 * @params solución del problema
 * @returns double del fitness calculado para solucion
 * */
tFitness ProblemPort::fitness(const tSolution<double> &solution) {
  return profit(solution) - this->lambda * risk(solution);
}

/*
 * @brief Crea una solución aleatoria válida para el problema
 * @params ninguno
 * @returns solución aleatoria normalizada con algunos pesos en cero
 * */
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

/*
 * @brief Verifica si una solución es válida
 * @params solution: vector de pesos del portfolio a verificar
 * @returns true si la solución cumple todas las restricciones, false en caso contrario
 * */
bool ProblemPort::isValid(const tSolution<double> &solution) {
  bool valid = true;

  // La suma puede tener tolerancia ERROR
  if (std::abs(solution.sum() - 1.0) > this->ERROR) {
    valid = false;
  }

  // Los valores low y high deben ser exactos (sin tolerancia)
  for (int i = 0; i < solution.size(); i++) {
    double val = solution(i);
    // Valor debe ser 0 (con tolerancia) o estar EXACTAMENTE en [low, high]
    if (std::abs(val)==0) {
      // Es efectivamente cero, OK
      continue;
    } else if (val >= this->low && val <= this->high) {
      continue;
    } else {
      // Fuera de rango
      valid = false;
      break;
    }
  }
  
  return valid;
}

/*
 * @brief Repara una solución no válida para que cumpla las restricciones
 * @params solution: referencia al vector de pesos del portfolio a reparar
 * @returns ninguno (modifica la solución in-place)
 * */
void ProblemPort::fix(tSolution<double> &solution) {
  // Paso 1: Normalizar
  double suma = solution.sum();
  if (suma > 0) {
    solution = solution.array() / suma;
  }
  
  // Paso 2: Aplicar restricciones [low, high] o 0
  for (int i = 0; i < this->ncompany; i++) {
    if (solution(i) < this->low) {
      solution(i) = 0.0;
    } else if (solution(i) > this->high) {
      solution(i) = this->high;
    }
  }
  
  // Paso 3: Ajustar para que sume 1.0
  double diff = 1.0 - solution.sum();
  int max_iter = 100;
  int iter = 0;
  
  while (std::abs(diff) > this->ERROR && iter < max_iter) {
    iter++;
    
    if (diff > 0) {
      // Falta peso: aumentar valores
      bool ajustado = false;
      int i=0;
      // Intentar ajustar valores existentes en [low, high)
      while (i < this->ncompany && !ajustado) {
        if (solution(i) >= this->low && solution(i) < this->high) {
          double espacio = this->high - solution(i);
          if (espacio >= diff) {
            solution(i) += diff;
            ajustado = true;
          } else {
            solution(i) = this->high;
            diff -= espacio;
          }
        }
				i++;
      }
      
      // Si no pudimos ajustar, activar un cero
      if (!ajustado && diff >= this->low) {
        for (int i = 0; i < this->ncompany && !ajustado; i++) {
          if (solution(i) == 0.0) {
            solution(i) = std::min(diff, this->high);
            ajustado = true;
          }
        }
      }
      
      // Si diff < low, distribuir entre valores no en límite
      if (!ajustado && diff < this->low) {
        for (int i = 0; i < this->ncompany && !ajustado; i++) {
          if (solution(i) > this->low && solution(i) < this->high) {
            solution(i) += diff;
            ajustado = true;
          }
        }
      }
      
      if (!ajustado) break;
      
    } else {
      /*// Sobra peso: reducir valores
      bool ajustado = false;
      double falta = -diff;
      
      // Reducir valores mayores que low
      for (int i = 0; i < this->ncompany && !ajustado; i++) {
        if (solution(i) > this->low) {
          double margen = solution(i) - this->low;
          if (margen >= falta) {
            solution(i) -= falta;
            ajustado = true;
          } else {
            solution(i) = this->low;
            falta -= margen;
          }
        }
      }
      
      // Si no fue suficiente, poner valores a 0
      if (!ajustado) {
        for (int i = 0; i < this->ncompany && !ajustado; i++) {
          if (solution(i) >= this->low) {
            solution(i) = 0.0;
            ajustado = true;
          }
        }
      }
      
      if (!ajustado) break;
			*/
    }
    
    diff = 1.0 - solution.sum();
  }
}

/*
 * @brief Calcula el valor heurístico de una compañía para greedy
 * @params i: índice de la compañía
 * @returns valor heurístico basado en beneficio y riesgo de la compañía i
 * */
double ProblemPort::heuristic(int i) {
  double logmean = this->logSums(i);
  double sigmamean = this->covarianceMat.col(i).sum() / this->ncompany;
  return logmean - (this->lambda * sigmamean);
}

/*
 * @brief Función de testeo para verificar la validez de las soluciones generadas
 * @params ninguno
 * @returns ninguno (imprime resultados de las pruebas por consola)
 * */
void ProblemPort::testclass() {
  int notvalid = 0;
  for (int i = 0; i < 100000; i++) {
    tSolution<double> sol = this->createSolution();
    if (!this->isValid(sol)) {
      notvalid++;
    }
  }
  std::cout << "total no validas : " << notvalid << std::endl;
}
