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
