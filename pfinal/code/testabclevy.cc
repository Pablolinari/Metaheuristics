extern "C" {
#include "cec17.h"
}
#include <iostream>
#include <vector>
#include <random>
#include <cmath>

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
// Paso de vuelo de Levy mediante el algoritmo de Mantegna.
//   sigma_u se calcula una sola vez (es constante para un beta fijo) y se
//   pasa precalculado. El paso resultante tiene cola pesada: la mayoria de
//   los valores son pequenos, con saltos largos ocasionales.
// ---------------------------------------------------------------------------
double mantegna_sigma_u(double beta) {
  double num = tgamma(1.0 + beta) * sin(M_PI * beta / 2.0);
  double den = tgamma((1.0 + beta) / 2.0) * beta * pow(2.0, (beta - 1.0) / 2.0);
  return pow(num / den, 1.0 / beta);
}

double levy_step(double beta, double sigma_u, std::mt19937 &gen) {
  std::normal_distribution<double> normal_u(0.0, sigma_u);
  std::normal_distribution<double> normal_v(0.0, 1.0);
  double u = normal_u(gen);
  double v = normal_v(gen);
  return u / pow(fabs(v), 1.0 / beta);
}

// ---------------------------------------------------------------------------
// ABC con vuelos de Levy. La unica diferencia con el ABC clasico es la regla
// de generacion del candidato: en lugar de phi*(x_i - x_k) se usa un paso de
// Levy dirigido hacia la mejor fuente del enjambre.
//   beta  : exponente de la distribucion de Levy (1.5 habitual).
//   alpha : factor de escala del salto (evita salirse del dominio).
// ---------------------------------------------------------------------------
double artificial_bee_colony_levy(int dim, int seed, int lower, int upper,
                                  int SN, int limit, int maxevals, double beta,
                                  double alpha) {
  std::uniform_real_distribution<double> dis(lower, upper);
  std::mt19937 gen(seed);

  double sigma_u = mantegna_sigma_u(beta);

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

  std::uniform_int_distribution<int> dim_dist(0, dim - 1);

  auto greedy_update = [&](int i) {
    // Mejor fuente actual del enjambre: orienta el salto de Levy.
    int best = 0;
    for (int b = 1; b < SN; b++) {
      if (obj[b] < obj[best]) {
        best = b;
      }
    }

    int j = dim_dist(gen);
    vector<double> cand = foods[i];
    double step = levy_step(beta, sigma_u, gen);
    cand[j] = foods[i][j] + alpha * step * (foods[i][j] - foods[best][j]);
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
  int dim = 100;
  int seed = 42;
  int lower = -100;
  int upper = 100;
  int SN = 20;
  int limit = 100;
  double beta = 1.5;    // exponente del vuelo de Levy
  double alpha = 0.01;  // factor de escala del salto
  const size_t maxtimes = 5;

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init("abc-levy", funcid, dim);

    double bestfitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness = artificial_bee_colony_levy(
          dim, seed, lower, upper, SN, limit, 10000 * dim / maxtimes, beta,
          alpha);
      if (bestfitness < 0 || fitness < bestfitness) {
        bestfitness = fitness;
      }
    }

    cout << "Best ABC-Levy[F" << funcid << "]: " << scientific
         << cec17_error(bestfitness) << endl;
  }

  return 0;
}
