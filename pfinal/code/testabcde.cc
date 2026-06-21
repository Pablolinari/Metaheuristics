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
// ABC hibridado con Differential Evolution.
//
// La unica diferencia con el ABC clasico esta en la regla de generacion del
// candidato. En lugar de perturbar una sola dimension con phi*(x_i - x_k), se
// construye un mutante con el operador DE/current-to-best/1:
//
//   v = x_i + F*(x_best - x_i) + F*(x_r1 - x_r2)
//
// y se mezcla con la fuente actual mediante cruce binomial (CR), garantizando
// que al menos una dimension provenga del mutante (jrand). De este modo cada
// candidato combina la informacion diferencial de toda la poblacion en varias
// dimensiones a la vez (DE) dentro del esqueleto de fases del ABC (empleadas,
// observadoras por ruleta y exploradoras).
//
//   F  : factor de escala de la mutacion diferencial (0.5 habitual).
//   CR : probabilidad de cruce binomial (0.9 habitual).
// ---------------------------------------------------------------------------
double artificial_bee_colony_de(int dim, int seed, int lower, int upper, int SN,
                                int limit, int maxevals, double F, double CR) {
  std::uniform_real_distribution<double> dis(lower, upper);
  std::uniform_real_distribution<double> unit(0.0, 1.0);
  std::mt19937 gen(seed);

  int evals = 0;
  double bestfitness = -1;
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
    }
  }

  std::uniform_int_distribution<int> index_dist(0, SN - 1);
  std::uniform_int_distribution<int> dim_dist(0, dim - 1);

  auto greedy_update = [&](int i) {
    // Mejor fuente actual del enjambre: base de la mutacion DE/current-to-best.
    int best = 0;
    for (int b = 1; b < SN; b++) {
      if (obj[b] < obj[best]) {
        best = b;
      }
    }

    // Dos indices distintos entre si, de i y de best (vector diferencia).
    int r1 = index_dist(gen);
    while (r1 == i || r1 == best) {
      r1 = index_dist(gen);
    }
    int r2 = index_dist(gen);
    while (r2 == i || r2 == best || r2 == r1) {
      r2 = index_dist(gen);
    }

    // Cruce binomial: jrand asegura que al menos una dimension muta.
    int jrand = dim_dist(gen);
    vector<double> cand = foods[i];
    for (int j = 0; j < dim; j++) {
      if (j == jrand || unit(gen) < CR) {
        cand[j] = foods[i][j] + F * (foods[best][j] - foods[i][j]) +
                  F * (foods[r1][j] - foods[r2][j]);
      }
    }
    clip(cand, lower, upper);

    double candfit = cec17_fitness(&cand[0]);
    evals += 1;

    if (candfit < obj[i]) {
      foods[i] = cand;
      obj[i] = candfit;
      trial[i] = 0;
      if (candfit < bestfitness) {
        bestfitness = candfit;
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
  double F = 0.5;    // factor de escala de la mutacion diferencial
  double CR = 0.9;   // probabilidad de cruce binomial
  const size_t maxtimes = 5;

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init("abc-de", funcid, dim);

    double bestfitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness = artificial_bee_colony_de(
          dim, seed, lower, upper, SN, limit, 10000 * dim / maxtimes, F, CR);
      if (bestfitness < 0 || fitness < bestfitness) {
        bestfitness = fitness;
      }
    }

    cout << "Best ABC-DE[F" << funcid << "]: " << scientific
         << cec17_error(bestfitness) << endl;
  }

  return 0;
}
