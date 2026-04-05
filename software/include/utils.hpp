#pragma once
#include "../include/Eigen/Core"
#include <fstream>
/**
 * Loads a CSV file directly into an Eigen Matrix.
 * @param path The file system path to the .csv file.
 * @param rows The expected number of rows.
 * @param cols The expected number of columns.
 * @return An Eigen::MatrixXd populated with the CSV data.
 */

inline Eigen::MatrixXd loadCSV(const std::string &path, bool only2025) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("No se pudo abrir el archivo en: " + path);
  }

  std::vector<std::vector<double>> data;
  std::string line;

  // Leer cabecera
  if (!getline(file, line)) {
    return Eigen::MatrixXd(0, 0);
  }

  while (getline(file, line)) {
    if (line.empty())
      continue;

    std::stringstream ss(line);
    std::string dateCell;

    // 1. Extraer la primera columna (la fecha: DD-MM-YYYY)
    if (!getline(ss, dateCell, ','))
      continue;

    // 2. Lógica de filtrado por año
    // Buscamos si la fecha contiene "2025"
    bool is2025 = (dateCell.find("2025") != std::string::npos);

    // Si 'only2025' es true, solo pasamos si es 2025.
    // Si 'only2025' es false, solo pasamos si NO es 2025 (2024 hacia atrás).
    if (only2025 && !is2025)
      continue;
    if (!only2025 && is2025)
      continue;

    // 3. Procesar las columnas de datos (Empresas)
    std::vector<double> rowValues;
    std::string cell;
    while (getline(ss, cell, ',')) {
        rowValues.push_back(cell.empty() ? 0.0 : std::stod(cell));
    }

    if (!rowValues.empty()) {
      data.push_back(rowValues);
    }
  }
  file.close();

  if (data.empty())
    return Eigen::MatrixXd(0, 0);

  int rows = data.size();
  int cols = data[0].size();
  Eigen::MatrixXd mat(rows, cols);

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      mat(i, j) = data[i][j];
    }
  }

  return mat;
}

/**
 * Normaliza un vector para que la suma sea 1 y todos los valores estén
 * en el intervalo [low, high] o sean 0.
 * 
 * @param vec Vector Eigen con valores entre 0 y 1
 * @param low Límite inferior del intervalo (debe estar entre 0 y 1)
 * @param high Límite superior del intervalo (debe estar entre 0 y 1)
 * @return Vector normalizado donde la suma es 1 y todos los valores son 0 o están en [low, high]
 */
inline Eigen::VectorXd normalizeVector(const Eigen::VectorXd &vec, double low, double high) {
  // Validar parámetros
  if (low < 0.0 || low > 1.0 || high < 0.0 || high > 1.0 || low > high) {
    throw std::invalid_argument("Los límites deben cumplir: 0 <= low <= high <= 1");
  }
  
  int n = vec.size();
  Eigen::VectorXd result = vec;
  
  // Paso 1: Ajustar valores al intervalo [low, high] o 0
  for (int i = 0; i < n; ++i) {
    if (result(i) < low) {
      result(i) = 0.0;  // Valores menores que low se ponen a 0
    } else if (result(i) > high) {
      result(i) = high;  // Valores mayores que high se limitan a high
    }
  }
  
  // Paso 2: Normalizar para que la suma sea 1
  double sum = result.sum();
  
  if (sum == 0.0) {
    // Si todos los valores son 0, distribuir uniformemente
    result = Eigen::VectorXd::Constant(n, 1.0 / n);
    
    // Ajustar al intervalo [low, high] si es necesario
    if (1.0 / n < low) {
      // No es posible distribuir uniformemente dentro del intervalo
      throw std::runtime_error("Imposible normalizar: 1/n < low");
    } else if (1.0 / n > high) {
      result = Eigen::VectorXd::Constant(n, high);
      sum = result.sum();
      result /= sum;
    }
  } else {
    // Normalizar por la suma
    result /= sum;
    
    // Paso 3: Verificar que después de normalizar seguimos en [low, high]
    // Si algún valor sale del intervalo, necesitamos reajustar
    bool needsAdjustment = false;
    for (int i = 0; i < n; ++i) {
      if (result(i) > 0.0 && (result(i) < low || result(i) > high)) {
        needsAdjustment = true;
        break;
      }
    }
    
    if (needsAdjustment) {
      // Reajuste iterativo para cumplir restricciones
      int maxIterations = 100;
      for (int iter = 0; iter < maxIterations; ++iter) {
        bool changed = false;
        
        // Clampear valores fuera del intervalo
        for (int i = 0; i < n; ++i) {
          if (result(i) > 0.0 && result(i) < low) {
            result(i) = low;
            changed = true;
          } else if (result(i) > high) {
            result(i) = high;
            changed = true;
          }
        }
        
        // Renormalizar
        sum = result.sum();
        if (sum > 0.0) {
          result /= sum;
        }
        
        if (!changed) break;
      }
    }
  }
  
  return result;
}
