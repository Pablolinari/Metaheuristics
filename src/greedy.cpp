#include "../include/greedy.h"

/**
 * Create random solutions until maxevals has been achieved, and returns the
 * best one.
 *
 * @param problem The problem to be optimized
 * @param maxevals Maximum number of evaluations allowed
 * @return A pair containing the best solution found and its fitness
 *
ResultMHInt Greedy::optimize(ProblemDouble &problem, int maxevals) {
  assert(maxevals > 0);
  vector<bool> values;
  ProblemIncrem realproblem = dynamic_cast<ProblemIncrem &>(problem);
  tSolution<int> sol(problem.getSolutionSize());
  print_vector("sol_initial", sol);

  auto size = problem.getSolutionSize();

  for (int i = 0; i < size; i++) {
    values.push_back(i);
  }

  for (int r = 0; r < size / 2; r++) {
    vector<float> heuristics;

    for (auto option : values) {
      tHeuristic value = ((option % 2) == 1 ? sol.size() - option : sol.size());
      heuristics.push_back(value);
    }
    // print_vector("heuristics", heuristics);

    auto posi = min_element(heuristics.begin(), heuristics.end());
    int posi_int = posi - heuristics.begin();

    sol[values[posi_int]] = 1;
    values[posi_int] = values.back();
    values.pop_back();
  }
  
  tFitness fitness = problem.fitness(sol);
  return ResultMH(sol, fitness, 1);
}
*/
