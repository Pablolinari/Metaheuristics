extern "C" {
#include "cec17.h"
}
#include <iostream>
#include <vector>
#include <random>

using namespace std;

// reparo la solucion dejando cada componente dentro de [lower, upper]
void clip(vector<double> &sol, double lower, double upper) {
  for (auto &val : sol) {
    if (val < lower) {
      val = lower;
    } else if (val > upper) {
      val = upper;
    }
  }
}

// paso el objetivo (a minimizar) a una cantidad de nectar (a maximizar)
// por que si no la selección por ruleta esxogería las peores fuentes
double fitnessTransform(double obj) {
  if (obj >= 0.0) {
    return 1.0 / (1.0 + obj);
  }
  return 1.0 + (-obj);
}

// seleccion por ruleta: devuelve la fuente elegida con probabilidad probs[i]
int rouletteSelect(const vector<double> &probs, std::mt19937 &gen) {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  double r = dist(gen);
  double acc = 0.0;
  for (size_t i = 0; i < probs.size(); i++) {
    acc += probs[i];
    if (r <= acc) {
      return i;
    }
  }
  return probs.size() - 1;
}

// genero un candidato con el operador DE/current-to-best/1 + cruce binomial y
// me quedo con el si mejora (seleccion greedy)
void greedyUpdate(int i, vector<vector<double>> &foods, vector<double> &obj,
                  vector<int> &trial, double &bestFitness, int &evals,
                  int lower, int upper, double F, double CR,
                  std::mt19937 &gen) {
  int SN = foods.size();
  int dim = foods[0].size();
  std::uniform_int_distribution<int> indexDist(0, SN - 1);
  std::uniform_int_distribution<int> dimDist(0, dim - 1);
  std::uniform_real_distribution<double> unit(0.0, 1.0);

  // mejor fuente actual del enjambre: base de la mutacion current-to-best
  int best = 0;
  for (int b = 1; b < SN; b++) {
    if (obj[b] < obj[best]) {
      best = b;
    }
  }

  // dos indices distintos entre si, de i y de best (vector diferencia)
  int r1 = indexDist(gen);
  while (r1 == i || r1 == best) {
    r1 = indexDist(gen);
  }
  int r2 = indexDist(gen);
  while (r2 == i || r2 == best || r2 == r1) {
    r2 = indexDist(gen);
  }

  // cruce binomial: jrand asegura que al menos una dimension muta
  int jrand = dimDist(gen);
  vector<double> cand = foods[i];
  for (int j = 0; j < dim; j++) {
    if (j == jrand || unit(gen) < CR) {
      cand[j] = foods[i][j] + F * (foods[best][j] - foods[i][j]) +
                F * (foods[r1][j] - foods[r2][j]);
    }
  }
  clip(cand, lower, upper);
	//calculo su fitness
  double candFitness = cec17_fitness(&cand[0]);
  evals += 1;
	//si mejora me la quedo , si no la descarto y aumentan
	//los trials en 1
  if (candFitness < obj[i]) {
    foods[i] = cand;
    obj[i] = candFitness;
    trial[i] = 0;
    if (candFitness < bestFitness) {
      bestFitness = candFitness;
    }
  } else {
    trial[i] += 1;
  }
}

/*
 * ABC hibridado con Differential Evolution. La unica diferencia con el ABC
 * clasico esta en la regla de generacion del candidato. En lugar de perturbar
 * una sola dimension con phi*(x_i - x_k), construyo un mutante con el operador
 * DE/current-to-best/1:
 *
 *   v = x_i + F*(x_best - x_i) + F*(x_r1 - x_r2)
 *
 * y lo mezclo con la fuente actual mediante cruce binomial (CR), garantizando
 * que al menos una dimension venga del mutante (jrand). Asi cada candidato
 * combina la informacion diferencial de toda la poblacion en varias dimensiones
 * a la vez, dentro del esqueleto de fases del ABC.
 *
 *   F  : factor de escala de la mutacion diferencial (0.5 habitual).
 *   CR : probabilidad de cruce binomial (0.9 habitual).
 */
double artificialBeeColonyDe(int dim, int seed, int lower, int upper, int SN,
                             int limit, int maxevals, double F, double CR) {
  std::uniform_real_distribution<double> dis(lower, upper);
  std::mt19937 gen(seed);

  int evals = 0;
  double bestFitness = -1;
  vector<vector<double>> foods(SN, vector<double>(dim)); // fuentes de alimento (soluciones)
  vector<double> obj(SN);    // valor objetivo de cada fuente (a minimizar)
  vector<double> fit(SN);    // nectar de cada fuente (a maximizar, para la ruleta)
  vector<int> trial(SN, 0);  // intentos seguidos sin mejorar de cada fuente

  // inicializo SN fuentes de alimento aleatorias
  for (int i = 0; i < SN; i++) {
    for (int j = 0; j < dim; j++) {
      foods[i][j] = dis(gen);
    }
    obj[i] = cec17_fitness(&foods[i][0]);
    evals += 1;
    if (bestFitness < 0 || obj[i] < bestFitness) {
      bestFitness = obj[i];
    }
  }

  while (evals < maxevals) {
    // fase de abejas empleadas cada una explota su propia fuente
    for (int i = 0; i < SN && evals < maxevals; i++) {
      greedyUpdate(i, foods, obj, trial, bestFitness, evals, lower, upper, F, CR,
                   gen);
    }

    // probabilidad de cada fuente segun su nectar
    double fitSum = 0.0;
    for (int i = 0; i < SN; i++) {
			// uso finess transform para que el calculo de probabilidad
			// asigne más probabilidad a las mejores fuentes
      fit[i] = fitnessTransform(obj[i]);
      fitSum += fit[i];
    }

    vector<double> probs(SN); // vector de probabilidades de cada fuente
    for (int i = 0; i < SN; i++) {
      probs[i] = fit[i] / fitSum;
    }

    // fase de abejas observadoras: eligen fuente por ruleta y la explotan
    int t = 0;
    while (t < SN && evals < maxevals) {
      int i = rouletteSelect(probs, gen);
      greedyUpdate(i, foods, obj, trial, bestFitness, evals, lower, upper, F, CR,
                   gen);
      t += 1;
    }

    // fase de abeja exploradora: abandono la fuente mas estancada
    int maxTrial = 0;
    for (int i = 1; i < SN; i++) {
      if (trial[i] > trial[maxTrial]) {
        maxTrial = i;
      }
    }
		//genero la nueva fuente de alimento
    if (trial[maxTrial] >= limit && evals < maxevals) {
      for (int j = 0; j < dim; j++) {
        foods[maxTrial][j] = dis(gen);
      }
      obj[maxTrial] = cec17_fitness(&foods[maxTrial][0]);
      evals += 1;
      trial[maxTrial] = 0;
      if (obj[maxTrial] < bestFitness) {
        bestFitness = obj[maxTrial];
      }
    }
  }

  return bestFitness;
}

int main() {
  int dim = 30;
  int seed = 42;
  int lower = -100;
  int upper = 100;
  int SN = 20;
  int limit = 100;
  double F = 0.5;   // factor de escala de la mutacion diferencial
  double CR = 0.9;  // probabilidad de cruce binomial
  const size_t maxtimes = 5;

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init("abc-de", funcid, dim);

    double bestFitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness = artificialBeeColonyDe(
          dim, seed, lower, upper, SN, limit, 10000 * dim / maxtimes, F, CR);
      if (bestFitness < 0 || fitness < bestFitness) {
        bestFitness = fitness;
      }
    }

    cout << "Best ABC-DE[F" << funcid << "]: " << scientific
         << cec17_error(bestFitness) << endl;
  }

  return 0;
}
