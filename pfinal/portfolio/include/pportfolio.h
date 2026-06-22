#pragma once
#include "../include/Eigen/Core"
#include "problem.h"
#include "solution.h"
#include <string>

class ProblemPort : public Problem<double> {
private:
  int days;
  int ncompany;
  double low;
  double high;
  int lambda;
  const double ERROR = 1e-14;
  Eigen::MatrixXd data;
  Eigen::MatrixXd covarianceMat;
  Eigen::VectorXd logSums; 

public:
  ProblemPort(std::string path, double low, double high, bool inc2025, int lambda = 500);

  size_t getSolutionSize() override;
  std::pair<double, double> getSolutionDomainRange() override;
  
	Eigen::VectorXd logSumsVector();

  Eigen::MatrixXd covarianceMatrix();
  
  double profit(const tSolution<double> &solution);
  double risk(const tSolution<double> &solution);
  
  tFitness fitness(const tSolution<double> &solution) override;
  tSolution<double> createSolution() override;
  bool isValid(const tSolution<double> &solution) override;
  void fix(tSolution<double> &solution) override;
  double heuristic(int i) override;
  
  void testclass();
};
