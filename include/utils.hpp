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

inline Eigen::MatrixXd loadCSV(const std::string &path, int rows, int cols) {
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
