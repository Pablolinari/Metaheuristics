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
// Busqueda local Solis-Wets (tomada de testsolis.cc)
// ---------------------------------------------------------------------------
void increm_bias(vector<double> &bias, vector<double> dif) {
  for (unsigned i = 0; i < bias.size(); i++) {
    bias[i] = 0.2 * bias[i] + 0.4 * (dif[i] + bias[i]);
  }
}

void decrement_bias(vector<double> &bias, vector<double> dif) {
  for (unsigned i = 0; i < bias.size(); i++) {
    bias[i] = bias[i] - 0.4 * (dif[i] + bias[i]);
  }
}

// Devuelve el numero de evaluaciones realmente consumidas por la BL.
template <class Random>
int soliswets(vector<double> &sol, double &fitness, double delta, int maxevals,
              int lower, int upper, Random &random) {
  const size_t dim = sol.size();
  vector<double> bias(dim), dif(dim), newsol(dim);
  double newfit;
  size_t i;

  int evals = 0;
  int num_success = 0;
  int num_failed = 0;

  while (evals < maxevals) {
    std::uniform_real_distribution<double> distribution(0.0, delta);

    for (i = 0; i < dim; i++) {
      dif[i] = distribution(random);
      newsol[i] = sol[i] + dif[i] + bias[i];
    }

    clip(newsol, lower, upper);
    newfit = cec17_fitness(&newsol[0]);
    evals += 1;

    if (newfit < fitness) {
      sol = newsol;
      fitness = newfit;
      increm_bias(bias, dif);
      num_success += 1;
      num_failed = 0;
    } else if (evals < maxevals) {
      for (i = 0; i < dim; i++) {
        newsol[i] = sol[i] - dif[i] - bias[i];
      }

      clip(newsol, lower, upper);
      newfit = cec17_fitness(&newsol[0]);
      evals += 1;

      if (newfit < fitness) {
        sol = newsol;
        fitness = newfit;
        decrement_bias(bias, dif);
        num_success += 1;
        num_failed = 0;
      } else {
        for (i = 0; i < dim; i++) {
          bias[i] /= 2;
        }
        num_success = 0;
        num_failed += 1;
      }
    }

    if (num_success >= 5) {
      num_success = 0;
      delta *= 2;
    } else if (num_failed >= 3) {
      num_failed = 0;
      delta /= 2;
    }
  }

  return evals;
}

// ---------------------------------------------------------------------------
// ABC. Si use_ls == true se convierte en memetico (ABC + Solis-Wets).
//   ls_freq   : aplica la BL cada ls_freq ciclos del ABC.
//   ls_budget : evaluaciones que se le dan a cada aplicacion de la BL.
//   ls_delta  : paso inicial de la BL.
// ---------------------------------------------------------------------------
double artificial_bee_colony(int dim, int seed, int lower, int upper, int SN,
                             int limit, int maxevals, bool use_ls,
                             int ls_freq, int ls_budget, double ls_delta) {
  std::uniform_real_distribution<double> dis(lower, upper);
  std::uniform_real_distribution<double> phi_dist(-1.0, 1.0);
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
    int k = index_dist(gen);
    while (k == i) {
      k = index_dist(gen);
    }

    int j = dim_dist(gen);
    vector<double> cand = foods[i];
    cand[j] = foods[i][j] + phi_dist(gen) * (foods[i][j] - foods[k][j]);
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

  int cycle = 0;

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

    // ----- Hibridacion: intensificar la mejor fuente con Solis-Wets -----
    cycle += 1;
    if (use_ls && cycle % ls_freq == 0 && evals < maxevals) {
      int best = 0;
      for (int i = 1; i < SN; i++) {
        if (obj[i] < obj[best]) {
          best = i;
        }
      }

      int budget = min(ls_budget, maxevals - evals);
      evals += soliswets(foods[best], obj[best], ls_delta, budget, lower,
                         upper, gen);
      trial[best] = 0;
      if (obj[best] < bestfitness) {
        bestfitness = obj[best];
      }
    }
  }

  return bestfitness;
}

int main(int argc, char *argv[]) {
  int dim = 100;
  int seed = 42;
  int lower = -100;
  int upper = 100;
  int SN = 20;
  int limit = 100;
  const size_t maxtimes = 5;

  // Por defecto corre el hibrido. Pasa "abc" como argumento para el ABC puro.
  bool use_ls = true;
  string name = "abc-ls";
  if (argc > 1 && string(argv[1]) == "abc") {
    use_ls = false;
    name = "abc";
  }

  int ls_freq = 50;       // aplica la BL cada 50 ciclos
  int ls_budget = 500;    // evaluaciones por aplicacion de la BL
  double ls_delta = 0.2;  // paso inicial de Solis-Wets

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init(name.c_str(), funcid, dim);

    double bestfitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness = artificial_bee_colony(
          dim, seed, lower, upper, SN, limit, 10000 * dim / maxtimes, use_ls,
          ls_freq, ls_budget, ls_delta);
      if (bestfitness < 0 || fitness < bestfitness) {
        bestfitness = fitness;
      }
    }

    cout << "Best " << name << "[F" << funcid << "]: " << scientific
         << cec17_error(bestfitness) << endl;
  }

  return 0;
}
