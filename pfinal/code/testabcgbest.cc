extern "C" {
#include "cec17.h"
}
#include <iostream>
#include <vector>
#include <random>

using namespace std;

void clip(vector<double> &sol, double lower, double upper) {
  for (auto &val : sol) {
    if (val < lower) {
      val = lower;
    } else if (val > upper) {
      val = upper;
    }
  }
}

double fitness_transform(double obj) {
  if (obj >= 0.0) {
    return 1.0 / (1.0 + obj);
  }
  return 1.0 + (-obj);
}

int roulette_select(const vector<double> &probs, std::mt19937 &gen) {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  double r = dist(gen);
  double acc = 0.0;
  for (size_t i = 0; i < probs.size(); i++) {
    acc += probs[i];
    if (r <= acc) {
      return static_cast<int>(i);
    }
  }
  return static_cast<int>(probs.size() - 1);
}

// ---------------------------------------------------------------------------
// ABC guiado por la mejor solucion global (Gbest-guided ABC, GABC).
//   Zhu & Kwong, "Gbest-guided artificial bee colony algorithm for numerical
//   function optimization", Applied Mathematics and Computation, 2010.
//
// La unica diferencia con el ABC clasico es la ecuacion de generacion del
// candidato. En lugar de
//     v_ij = x_ij + phi_ij (x_ij - x_kj)
// se anade un termino que atrae la nueva fuente hacia la mejor solucion
// global encontrada hasta el momento (gbest):
//     v_ij = x_ij + phi_ij (x_ij - x_kj) + psi_ij (y_j - x_ij)
// con phi_ij en [-1, 1], psi_ij en [0, C] (C constante, 1.5 habitual) e y la
// mejor solucion global. El primer termino mantiene la exploracion y el
// segundo acelera la explotacion guiando el enjambre hacia gbest.
// ---------------------------------------------------------------------------
double artificial_bee_colony_gbest(int dim, int seed, int lower, int upper,
                                   int SN, int limit, int maxevals, double C) {
  std::uniform_real_distribution<double> dis(lower, upper);
  std::uniform_real_distribution<double> phi_dist(-1.0, 1.0);
  std::uniform_real_distribution<double> psi_dist(0.0, C);
  std::mt19937 gen(seed);

  int evals = 0;
  double bestfitness = -1;
  vector<double> bestsol(dim);
  vector<vector<double>> foods(SN, vector<double>(dim));
  vector<double> obj(SN);
  vector<double> fit(SN);
  vector<int> trial(SN, 0);

  for (int i = 0; i < SN; i++) {
    for (int j = 0; j < dim; j++) {
      foods[i][j] = dis(gen);
    }
    obj[i] = cec17_fitness(&foods[i][0]);
    evals += 1;
    if (bestfitness < 0 || obj[i] < bestfitness) {
      bestfitness = obj[i];
      bestsol = foods[i];
    }
  }

  std::uniform_int_distribution<int> index_dist(0, SN - 1);
  std::uniform_int_distribution<int> dim_dist(0, dim - 1);

  auto greedy_update = [&](int i) {
    int k = index_dist(gen);
    while (k == i) {
      k = index_dist(gen);
    }

    int j = dim_dist(gen);
    vector<double> cand = foods[i];
    cand[j] = foods[i][j] + phi_dist(gen) * (foods[i][j] - foods[k][j]) +
              psi_dist(gen) * (bestsol[j] - foods[i][j]);
    clip(cand, lower, upper);

    double candfit = cec17_fitness(&cand[0]);
    evals += 1;

    if (candfit < obj[i]) {
      foods[i] = cand;
      obj[i] = candfit;
      trial[i] = 0;
      if (candfit < bestfitness) {
        bestfitness = candfit;
        bestsol = foods[i];
      }
    } else {
      trial[i] += 1;
    }
  };

  while (evals < maxevals) {
    for (int i = 0; i < SN && evals < maxevals; i++) {
      greedy_update(i);
    }

    double fit_sum = 0.0;
    for (int i = 0; i < SN; i++) {
      fit[i] = fitness_transform(obj[i]);
      fit_sum += fit[i];
    }

    vector<double> probs(SN);
    for (int i = 0; i < SN; i++) {
      probs[i] = fit[i] / fit_sum;
    }

    int t = 0;
    while (t < SN && evals < maxevals) {
      int i = roulette_select(probs, gen);
      greedy_update(i);
      t += 1;
    }

    int max_trial = 0;
    for (int i = 1; i < SN; i++) {
      if (trial[i] > trial[max_trial]) {
        max_trial = i;
      }
    }

    if (trial[max_trial] >= limit && evals < maxevals) {
      for (int j = 0; j < dim; j++) {
        foods[max_trial][j] = dis(gen);
      }
      obj[max_trial] = cec17_fitness(&foods[max_trial][0]);
      evals += 1;
      trial[max_trial] = 0;
      if (obj[max_trial] < bestfitness) {
        bestfitness = obj[max_trial];
        bestsol = foods[max_trial];
      }
    }
  }

  return bestfitness;
}

int main() {
  int dim = 30;
  int seed = 42;
  int lower = -100;
  int upper = 100;
  int SN = 20;
  int limit = 100;
  double C = 1.5;  // limite superior de psi: peso de la atraccion hacia gbest
  const size_t maxtimes = 5;

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init("abc-gbest", funcid, dim);

    double bestfitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness = artificial_bee_colony_gbest(
          dim, seed, lower, upper, SN, limit, 10000 * dim / maxtimes, C);
      if (bestfitness < 0 || fitness < bestfitness) {
        bestfitness = fitness;
      }
    }

    cout << "Best ABC-Gbest[F" << funcid << "]: " << scientific
         << cec17_error(bestfitness) << endl;
  }

  return 0;
}
