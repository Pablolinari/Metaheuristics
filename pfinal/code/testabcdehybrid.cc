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
// Differential Evolution (DE/rand/1/bin) ejecutado como ALGORITMO COMPLETO
// sobre la poblacion que recibe. No es solo un operador: tiene su propio bucle
// generacional (para cada individuo: mutacion diferencial + cruce binomial +
// seleccion greedy) y se repite hasta agotar el presupuesto de evaluaciones.
//
// Es el analogo a soliswets() en testabchybrid.cc: una busqueda que opera sobre
// las fuentes del ABC y devuelve el numero de evaluaciones realmente consumidas
// para que el ABC las descuente de su presupuesto global.
//
//   foods/obj/trial : poblacion compartida con el ABC (se actualiza in place).
//   bestfitness     : mejor fitness global (se actualiza si DE mejora).
//   F  : factor de escala de la mutacion diferencial.
//   CR : probabilidad de cruce binomial.
// ---------------------------------------------------------------------------
int differential_evolution(vector<vector<double>> &foods, vector<double> &obj,
                           vector<int> &trial, double &bestfitness, double F,
                           double CR, int maxevals, int lower, int upper,
                           std::mt19937 &gen) {
  const int SN = static_cast<int>(foods.size());
  const int dim = static_cast<int>(foods[0].size());

  // Hacen falta al menos 3 fuentes distintas de i para DE/rand/1.
  if (SN < 4) {
    return 0;
  }

  std::uniform_int_distribution<int> index_dist(0, SN - 1);
  std::uniform_int_distribution<int> dim_dist(0, dim - 1);
  std::uniform_real_distribution<double> unit(0.0, 1.0);

  int evals = 0;

  while (evals < maxevals) {
    for (int i = 0; i < SN && evals < maxevals; i++) {
      // Tres indices distintos entre si y distintos de i.
      int r1 = index_dist(gen);
      while (r1 == i) {
        r1 = index_dist(gen);
      }
      int r2 = index_dist(gen);
      while (r2 == i || r2 == r1) {
        r2 = index_dist(gen);
      }
      int r3 = index_dist(gen);
      while (r3 == i || r3 == r1 || r3 == r2) {
        r3 = index_dist(gen);
      }

      // Mutacion DE/rand/1 + cruce binomial (jrand garantiza una mutacion).
      int jrand = dim_dist(gen);
      vector<double> cand = foods[i];
      for (int j = 0; j < dim; j++) {
        if (j == jrand || unit(gen) < CR) {
          cand[j] = foods[r1][j] + F * (foods[r2][j] - foods[r3][j]);
        }
      }
      clip(cand, lower, upper);

      double candfit = cec17_fitness(&cand[0]);
      evals += 1;

      // Seleccion greedy propia del DE.
      if (candfit < obj[i]) {
        foods[i] = cand;
        obj[i] = candfit;
        trial[i] = 0;
        if (candfit < bestfitness) {
          bestfitness = candfit;
        }
      }
    }
  }

  return evals;
}

// ---------------------------------------------------------------------------
// ABC. Si use_de == true se convierte en hibrido (ABC + Differential Evolution
// intercalado), con la misma estructura que el hibrido ABC + Solis-Wets:
//   de_freq   : ejecuta el DE cada de_freq ciclos del ABC.
//   de_budget : evaluaciones que se le dan a cada ejecucion del DE.
//   F, CR     : parametros del DE.
// ---------------------------------------------------------------------------
double artificial_bee_colony(int dim, int seed, int lower, int upper, int SN,
                             int limit, int maxevals, bool use_de, int de_freq,
                             int de_budget, double F, double CR) {
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

    // ----- Hibridacion: intercalar el algoritmo DE sobre la poblacion -----
    cycle += 1;
    if (use_de && cycle % de_freq == 0 && evals < maxevals) {
      int budget = min(de_budget, maxevals - evals);
      evals += differential_evolution(foods, obj, trial, bestfitness, F, CR,
                                      budget, lower, upper, gen);
    }
  }

  return bestfitness;
}

int main(int argc, char *argv[]) {
  int dim = 30;
  int seed = 42;
  int lower = -100;
  int upper = 100;
  int SN = 20;
  int limit = 100;
  const size_t maxtimes = 5;

  // Por defecto corre el hibrido. Pasa "abc" como argumento para el ABC puro.
  bool use_de = true;
  string name = "abc-de-hyb";
  if (argc > 1 && string(argv[1]) == "abc") {
    use_de = false;
    name = "abc";
  }

  int de_freq = 50;     // ejecuta el DE cada 50 ciclos del ABC
  int de_budget = 500;  // evaluaciones por ejecucion del DE
  double F = 0.5;       // factor de escala del DE
  double CR = 0.9;      // probabilidad de cruce del DE

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init(name.c_str(), funcid, dim);

    double bestfitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness = artificial_bee_colony(
          dim, seed, lower, upper, SN, limit, 10000 * dim / maxtimes, use_de,
          de_freq, de_budget, F, CR);
      if (bestfitness < 0 || fitness < bestfitness) {
        bestfitness = fitness;
      }
    }

    cout << "Best " << name << "[F" << funcid << "]: " << scientific
         << cec17_error(bestfitness) << endl;
  }

  return 0;
}
