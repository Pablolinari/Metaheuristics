#pragma once
#include <eigen3/Eigen/Eigen>
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
      try {
        rowValues.push_back(cell.empty() ? 0.0 : std::stod(cell));
      } catch (...) {
        rowValues.push_back(0.0);
      }
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
