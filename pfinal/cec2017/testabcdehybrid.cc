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


int differentialEvolution(vector<vector<double>> &foods, vector<double> &obj,
                          vector<int> &trial, double &bestFitness, double F,
                          double CR, int maxevals, int lower, int upper,
                          std::mt19937 &gen) {
  const int SN = foods.size();
  const int dim = foods[0].size();

  // hacen falta al menos 3 fuentes distintas de i para DE/rand/1
  if (SN < 4) {
    return 0;
  }

  std::uniform_int_distribution<int> indexDist(0, SN - 1);
  std::uniform_int_distribution<int> dimDist(0, dim - 1);
  std::uniform_real_distribution<double> unit(0.0, 1.0);

  int evals = 0;

  while (evals < maxevals) {
    for (int i = 0; i < SN && evals < maxevals; i++) {
      // tres indices distintos entre si y distintos de i
      int r1 = indexDist(gen);
      while (r1 == i) {
        r1 = indexDist(gen);
      }
      int r2 = indexDist(gen);
      while (r2 == i || r2 == r1) {
        r2 = indexDist(gen);
      }
      int r3 = indexDist(gen);
      while (r3 == i || r3 == r1 || r3 == r2) {
        r3 = indexDist(gen);
      }

      // mutacion DE/rand/1 + cruce binomial (jrand garantiza una mutacion)
      int jrand = dimDist(gen);
      vector<double> cand = foods[i];
      for (int j = 0; j < dim; j++) {
        if (j == jrand || unit(gen) < CR) {
          cand[j] = foods[r1][j] + F * (foods[r2][j] - foods[r3][j]);
        }
      }
      clip(cand, lower, upper);

      double candFitness = cec17_fitness(&cand[0]);
      evals += 1;

      // seleccion greedy propia del DE
      if (candFitness < obj[i]) {
        foods[i] = cand;
        obj[i] = candFitness;
        trial[i] = 0;
        if (candFitness < bestFitness) {
          bestFitness = candFitness;
        }
      }
    }
  }

  return evals;
}

// genero un candidato perturbando una dimension de la fuente i y me quedo con
// el si mejora 
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

double artificialBeeColony(int dim, int seed, int lower, int upper, int SN,
                           int limit, int maxevals, bool useDe, int deFreq,
                           int deBudget, double F, double CR) {
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

    // hibridacion: intercalo el algoritmo DE sobre la poblacion
    cycle += 1;
    if (useDe && cycle % deFreq == 0 && evals < maxevals) {
      int budget = min(deBudget, maxevals - evals);
      evals += differentialEvolution(foods, obj, trial, bestFitness, F, CR,
                                     budget, lower, upper, gen);
    }
  }

  return bestFitness;
}

int main(int argc, char *argv[]) {
  int dim = 30;
  int seed = 42;
  int lower = -100;
  int upper = 100;
  int SN = 20;
  int limit = 100;
  const size_t maxtimes = 5;

  // por defecto corre el hibrido. paso "abc" como argumento para el ABC puro
  bool useDe = true;
  string name = "abc-de-hyb";
  if (argc > 1 && string(argv[1]) == "abc") {
    useDe = false;
    name = "abc";
  }

  int deFreq = 50;     // ejecuta el DE cada 50 ciclos del ABC
  int deBudget = 500;  // evaluaciones por ejecucion del DE
  double F = 0.5;      // factor de escala del DE
  double CR = 0.9;     // probabilidad de cruce del DE

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init(name.c_str(), funcid, dim);

    double bestFitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness =
          artificialBeeColony(dim, seed, lower, upper, SN, limit,
                              10000 * dim / maxtimes, useDe, deFreq, deBudget, F,
                              CR);
      if (bestFitness < 0 || fitness < bestFitness) {
        bestFitness = fitness;
      }
    }

    cout << "Best " << name << "[F" << funcid << "]: " << scientific
         << cec17_error(bestFitness) << endl;
  }

  return 0;
}
