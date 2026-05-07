#include "../include/operator.h"
#include <algorithm>
#include <iostream>
#include <istream>
#include <ostream>
#include <utility>
#include <vector>

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

      son1(i) = Random::get<double>(cmin - gap * alpha, cmax + gap * alpha);
      son2(i) = Random::get<double>(cmin - gap * alpha, cmax + gap * alpha);
    }
  }
  return std::make_pair(son1, son2);
}

bool Operator::mutation(tSolution<double> &mutSol, Problem<double> &problem,
                        double mutationRate, int maxAttempts) {
  std::pair<double, double> problembounds = problem.getSolutionDomainRange();
  int solutionsize = problem.getSolutionSize();

  std::vector<int> validDonors;
  for (int i = 0; i < solutionsize; ++i) {
    double amount = mutSol(i) * mutationRate;
    if (amount <= 0.0) {
      continue;
    }
    double firstAfter = mutSol(i) - amount;
    bool firstValid = firstAfter == 0.0 || (firstAfter >= problembounds.first &&
                                            firstAfter <= problembounds.second);
    if (firstValid) {
      validDonors.push_back(i);
    }
  }

  if (validDonors.empty()) {
    std::cout << "no hay donantes" << std::endl;
    return false;
  }

  for (int attempt = 0; attempt < maxAttempts; ++attempt) {
    int pickDonor = Random::get<int>(0, (validDonors.size()) - 1);
    int candidateFirst = validDonors[pickDonor];
    double amount = mutSol(candidateFirst) * mutationRate;

    double firstAfter = mutSol(candidateFirst) - amount;
    bool firstValid = firstAfter == 0.0 || (firstAfter >= problembounds.first &&
                                            firstAfter <= problembounds.second);
    if (!firstValid) {
      continue;
    }

    std::vector<int> validReceivers;
    validReceivers.reserve(solutionsize - 1);
    for (int i = 0; i < solutionsize; ++i) {
      if (i == candidateFirst) {
        continue;
      }
      double secondAfter = mutSol(i) + amount;
      bool secondValid =
          secondAfter == 0.0 || (secondAfter >= problembounds.first &&
                                 secondAfter <= problembounds.second);
      if (secondValid) {
        validReceivers.push_back(i);
      }
    }

    if (validReceivers.empty()) {
			std::cout<<"no receptores"<<std::endl;
      continue;
    }

    int pick = Random::get<int>(0, static_cast<int>(validReceivers.size()) - 1);
    int candidateSecond = validReceivers[pick];
    double secondAfter = mutSol(candidateSecond) + amount;

    mutSol(candidateFirst) -= amount;
    mutSol(candidateSecond) += amount;
    return true;
  }

  return false;
}

std::vector<int>
Operator::tournament(int tournamentsize, int populationsize,int selectionsize,
                     const std::vector<double> &fitnesspoblation) {

  std::vector<int> idxparents;

  for (int i = 0; i < selectionsize; i++) {

    double fitnessTournament = -std::numeric_limits<double>::infinity();
    int idxBestSolution = 0;
    int idx;
    for (int j = 0; j < tournamentsize; j++) {
      idx = Random::get(0, populationsize - 1);
      if (fitnesspoblation[idx] > fitnessTournament) {
        fitnessTournament = fitnesspoblation[idx];
        idxBestSolution = idx;
      }
    }
    idxparents.push_back(idxBestSolution);
  }

  return idxparents;
}
