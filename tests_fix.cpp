#include "include/pportfolio.h"
#include "include/random.hpp"
#include <eigen3/Eigen/Core>
#include <iostream>
#include <vector>

int main() {
  // Config básico
  ProblemPort problem("data/ibex_35.csv", 0.01, 0.08, false);

  auto show = [&](const std::string &title, tSolution<double> sol) {
    std::cout << "--- " << title << " ---\n";
    std::cout << "antes: sum=" << sol.sum() << " valid="
              << std::boolalpha << problem.isValid(sol) << "\n";
    problem.fix(sol);
    std::cout << "despues: sum=" << sol.sum() << " valid="
              << std::boolalpha << problem.isValid(sol) << "\n";
    std::cout << sol.transpose() << "\n\n";
  };

  // Caso 1: todos a cero
  tSolution<double> s1 = tSolution<double>::Zero(problem.getSolutionSize());
  show("todos cero", s1);

  // Caso 2: suma > 1 y varios por encima de high
  tSolution<double> s2(problem.getSolutionSize());
  s2.setConstant(0.2);
  show("suma 6, recorte", s2);

  // Caso 3: suma < 1, algunos por debajo de low y otros cero
  tSolution<double> s3(problem.getSolutionSize());
  for (int i = 0; i < s3.size(); ++i) {
    s3(i) = (i % 3 == 0) ? 0.0 : 0.002; // 0 o muy por debajo de low
  }
  show("faltante con ceros", s3);

  // Caso 4: todos en high pero suma < 1 (pocos elementos)
  tSolution<double> s4(problem.getSolutionSize());
  s4.setZero();
  s4(0) = s4(1) = problem.getSolutionDomainRange().second;
  show("capacidad limitada <1", s4);

  // Caso 5: todos en low pero suma > 1
  tSolution<double> s5(problem.getSolutionSize());
  s5.setConstant(problem.getSolutionDomainRange().first);
  s5(0) = s5(1) = 0.2; // provocar exceso claro
  show("exceso con low", s5);

  return 0;
}
