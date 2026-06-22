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

// busqueda local Solis-Wets (tomada de testsolis.cc)
void incremBias(vector<double> &bias, vector<double> dif) {
  for (unsigned i = 0; i < bias.size(); i++) {
    bias[i] = 0.2 * bias[i] + 0.4 * (dif[i] + bias[i]);
  }
}

void decrementBias(vector<double> &bias, vector<double> dif) {
  for (unsigned i = 0; i < bias.size(); i++) {
    bias[i] = bias[i] - 0.4 * (dif[i] + bias[i]);
  }
}

// devuelve el numero de evaluaciones realmente consumidas por la BL
template <class Random>
int soliswets(vector<double> &sol, double &fitness, double delta, int maxevals,
              int lower, int upper, Random &random) {
  const size_t dim = sol.size();
  vector<double> bias(dim), dif(dim), newSol(dim);
  double newFit;
  size_t i;

  int evals = 0;
  int numExitos = 0;
  int numFallos = 0;

  while (evals < maxevals) {
    std::uniform_real_distribution<double> distribution(0.0, delta);

    for (i = 0; i < dim; i++) {
      dif[i] = distribution(random);
      newSol[i] = sol[i] + dif[i] + bias[i];
    }

    clip(newSol, lower, upper);
    newFit = cec17_fitness(&newSol[0]);
    evals += 1;

    if (newFit < fitness) {
      sol = newSol;
      fitness = newFit;
      incremBias(bias, dif);
      numExitos += 1;
      numFallos = 0;
    } else if (evals < maxevals) {
      for (i = 0; i < dim; i++) {
        newSol[i] = sol[i] - dif[i] - bias[i];
      }

      clip(newSol, lower, upper);
      newFit = cec17_fitness(&newSol[0]);
      evals += 1;

      if (newFit < fitness) {
        sol = newSol;
        fitness = newFit;
        decrementBias(bias, dif);
        numExitos += 1;
        numFallos = 0;
      } else {
        for (i = 0; i < dim; i++) {
          bias[i] /= 2;
        }
        numExitos = 0;
        numFallos += 1;
      }
    }

    if (numExitos >= 5) {
      numExitos = 0;
      delta *= 2;
    } else if (numFallos >= 3) {
      numFallos = 0;
      delta /= 2;
    }
  }

  return evals;
}

// genero un candidato perturbando una dimension de la fuente i y me quedo con
// el si mejora (seleccion greedy)
void greedyUpdate(int i, vector<vector<double>> &foods, vector<double> &obj,
                  vector<int> &trial, double &bestFitness, int &evals,
                  int lower, int upper, std::mt19937 &gen) {
  int SN = foods.size();
  int dim = foods[0].size();
  std::uniform_int_distribution<int> indexDist(0, SN - 1);
  std::uniform_int_distribution<int> dimDist(0, dim - 1);
  std::uniform_real_distribution<double> phiDist(-1.0, 1.0);
	//elijo una fuente y me aseguro de que no sea la misma
  int k = indexDist(gen);
  while (k == i) {
    k = indexDist(gen);
  }
	//elijo una dimensión
  int j = dimDist(gen);
  vector<double> cand = foods[i];
	//aplico la fórmula vista en el paper
  cand[j] = foods[i][j] + phiDist(gen) * (foods[i][j] - foods[k][j]);
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
 * ABC memetico: ABC + Solis-Wets como busqueda local.
 *   lsFreq   : aplica la BL cada lsFreq ciclos del ABC.
 *   lsBudget : evaluaciones que se le dan a cada aplicacion de la BL.
 *   lsDelta  : paso inicial de la BL.
 */
double artificialBeeColony(int dim, int seed, int lower, int upper, int SN,
                           int limit, int maxevals, int lsFreq, int lsBudget,
                           double lsDelta) {
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

  int cycle = 0;

  while (evals < maxevals) {
    // fase de abejas empleadas cada una explota su propia fuente
    for (int i = 0; i < SN && evals < maxevals; i++) {
      greedyUpdate(i, foods, obj, trial, bestFitness, evals, lower, upper, gen);
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
      greedyUpdate(i, foods, obj, trial, bestFitness, evals, lower, upper, gen);
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

    // hibridacion: intensifico la mejor fuente con Solis-Wets
    cycle += 1;
    if (cycle % lsFreq == 0 && evals < maxevals) {
      int best = 0;
      for (int i = 1; i < SN; i++) {
        if (obj[i] < obj[best]) {
          best = i;
        }
      }

      int budget = min(lsBudget, maxevals - evals);
      evals += soliswets(foods[best], obj[best], lsDelta, budget, lower, upper,
                         gen);
      trial[best] = 0;
      if (obj[best] < bestFitness) {
        bestFitness = obj[best];
      }
    }
  }

  return bestFitness;
}

int main() {
  int dim = 100;
  int seed = 42;
  int lower = -100;
  int upper = 100;
  int SN = 20;
  int limit = 100;
  const size_t maxtimes = 5;

  int lsFreq = 50;       // aplica la BL cada 50 ciclos
  int lsBudget = 500;    // evaluaciones por aplicacion de la BL
  double lsDelta = 0.2;  // paso inicial de Solis-Wets

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init("abc-ls", funcid, dim);

    double bestFitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness =
          artificialBeeColony(dim, seed, lower, upper, SN, limit,
                              10000 * dim / maxtimes, lsFreq, lsBudget, lsDelta);
      if (bestFitness < 0 || fitness < bestFitness) {
        bestFitness = fitness;
      }
    }

    cout << "Best ABC-LS[F" << funcid << "]: " << scientific
         << cec17_error(bestFitness) << endl;
  }

  return 0;
}
