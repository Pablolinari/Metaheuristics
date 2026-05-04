#include "../include/operator.h"
#include <algorithm>
#include <utility>

std::pair<tSolution<double>, tSolution<double>>
Operator::arithmetic(const tSolution<double> &p1, const tSolution<double> &p2) {

  tSolution<double> son1(p1.size());
  tSolution<double> son2(p1.size());

  for (int i = 0; i < p1.size(); i++) {
    if (p1(i) == p2(i)) {
      son1(i) = p1(i);
      son2(i) = p1(i);
    } else {
      double sigma = Random::get<double>(0.0, 1.0);
      son1(i) = sigma * p1(i) + (1 - sigma) * p2(i);
      son2(i) = sigma * p2(i) + (1 - sigma) * p1(i);
    }
  }
  return std::make_pair(son1, son2);
}

std::pair<tSolution<double>, tSolution<double>>
Operator::blxAlpha(const tSolution<double> &p1, const tSolution<double> &p2,
                   double alpha) {

  tSolution<double> son1(p1.size());
  tSolution<double> son2(p1.size());

  for (int i = 0; i < p1.size(); i++) {
    if (p1(i) == p2(i)) {
      son1(i) = p1(i);
      son2(i) = p1(i);
    } else {
      double cmax = std::max(p1(i), p2(i));
      double cmin = std::min(p1(i), p2(i));
      double gap = cmax - cmin;

      son1(i) = Random::get<double>(cmin - gap * alpha, cmax - gap * alpha);
      son2(i) = Random::get<double>(cmin - gap * alpha, cmax - gap * alpha);
    }
  }
  return std::make_pair(son1, son2);
}

bool Operator::mutation(tSolution<double> &mutSol, Problem<double> &problem,
                        double mutationRate, int maxAttempts) {
  int idxGene = -1;
  double amount = 0.0;
  std::pair<double, double> problembounds = problem.getSolutionDomainRange();

  for (int attempt = 0; attempt < maxAttempts; ++attempt) {
    int candidateFirst = Random::get<int>(0, problem.getSolutionSize() - 1);
    double candidateAmount = mutSol(candidateFirst) * mutationRate;
    if (candidateAmount <= 0.0) {
      continue;
    }

    double firstAfter = mutSol(candidateFirst) - candidateAmount;
    bool firstValid = firstAfter == 0.0 || (firstAfter >= problembounds.first &&
                                            firstAfter <= problembounds.second);

    if (firstValid) {
      idxGene = candidateFirst;
      amount = candidateAmount;
      break;
    }
  }

  if (idxGene == -1) {
    return false;
  }

  int idxNextGene = -1;
  for (int attempt = 0; attempt < maxAttempts; ++attempt) {
    int candidateSecond = Random::get<int>(0, problem.getSolutionSize() - 1);
    if (candidateSecond == idxGene) {
      continue;
    }

    double secondAfter = mutSol(candidateSecond) + amount;
    bool secondValid =
        secondAfter == 0.0 || (secondAfter >= problembounds.first &&
                               secondAfter <= problembounds.second);

    if (secondValid) {
      idxNextGene = candidateSecond;

      break;
    }
  }

  if (idxNextGene == -1) {
    return false;
  }

  mutSol(idxGene) -= amount;
  mutSol(idxNextGene) += amount;
	return true;
}
