#pragma once
#include "problem.h"
#include "solution.h"
#include "utils.hpp"
#include <eigen3/Eigen/Eigen>
#include <string>

class ProblemPort : public Problem<double> {
  Eigen::MatrixXd data;
  int days;
  int ncompany;
  double low;
  double high;
  int lambda;

public:
  ProblemPort(std::string path, int days,int ncompany, double low, double high,
              int lambda = 500);

  virtual size_t getSolutionSize() override;
  virtual std::pair<double, double> getSolutionDomainRange() override;
  double actionMean(int i);

  double covariance(int i, int j);

  double profit(const tSolution<double> &solution);
  double risk(const tSolution<double> &solution);

  tFitness fitness(const tSolution<double> &solution) override;

  tSolution<double> createSolution() override;

  virtual bool isValid(const tSolution<double> &solution) override;

  virtual void fix(tSolution<double> &solution) override;
	void testclass();
};
