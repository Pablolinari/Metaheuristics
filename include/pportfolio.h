#pragma once
#include "problem.h"
#include "solution.h"
#include "utils.hpp"
#include <cstddef>
#include <cstdlib>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Eigen>
#include <fstream>
#include <numeric>
#include <string>
#include <utility>
class ProblemPort : public Problem<double> {
  Eigen::MatrixXd data;
  int days;
  int ncompany;
  double low;
  double high;
  int lambda;

public:
  ProblemPort(std::string path, int days, double low, double high,
              int lambda = 500)
      : Problem<double>(), days(days), low(low), high(high), lambda(lambda) {

    data = loadCSV(path, days, ncompany);
  }

  virtual size_t getSolutionSize() override { return this->ncompany; }
  virtual std::pair<double, double> getSolutionDomainRange() override {
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

  double profit(const tSolution<double> &solution) {
    Eigen::MatrixXd logdata = data.array().log() + 1;
    Eigen::VectorXd logmeans = logdata.colwise().mean();
    return (solution.array() * logmeans.array()).sum();
  }
  double risk(const tSolution<double> &solution) { return 0; }

  tFitness fitness(const tSolution<double> &solution) override {
    return profit(solution) - this->lambda * risk(solution);
  }

  tSolution<double> createSolution() override {
    int size = this->getSolutionSize();
    tSolution<double> solution(size);

    for (int i = 0; i < size; i++) {
      solution[i] = Random::get(this->low, this->high);
    }

    int numberZeros = Random::get<int>(1, size - 1);
		Eigen::VectorXd indexZeros = Eigen::VectorXd::LinSpaced(size,size-1,0);
    Random::shuffle(indexZeros);
    for (int i = 0; i < numberZeros; i++) {
      solution[indexZeros[i]] = 0;
    }

    this->fix(solution);
    return solution;
  }

  virtual bool isValid(const tSolution<double> &solution) override {
    double error = 0.003;
    return std::abs(solution.sum() - 1) <= error; 
  }

  virtual void fix(tSolution<double> &solution) override {
		solution = solution.array()/solution.sum();

		for(int i = 0; i<solution.size();i++){
			if(solution(i)< this->low ){
				solution(i)=0.0;
			}
			else if (solution(i)>this->high){
				solution(i)=this->high;
			}
		}
		if (solution.sum()<1){
			double waste = 1-solution.sum();
			if(waste >this->low){}

		}

  }
};
