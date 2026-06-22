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

// genero un candidato con el termino de atraccion hacia gbest (bestSolution) y
// me quedo con el si mejora (seleccion greedy)
void greedyUpdate(int i, vector<vector<double>> &foods, vector<double> &obj,
                  vector<int> &trial, vector<double> &bestSolution,
                  double &bestFitness, int &evals, int lower, int upper,
                  double C, std::mt19937 &gen) {
  int SN = foods.size();
  int dim = foods[0].size();
  std::uniform_int_distribution<int> indexDist(0, SN - 1);
  std::uniform_int_distribution<int> dimDist(0, dim - 1);
  std::uniform_real_distribution<double> phiDist(-1.0, 1.0);
  std::uniform_real_distribution<double> psiDist(0.0, C);
	//elijo una fuente y me aseguro de que no sea la misma
  int k = indexDist(gen);
  while (k == i) {
    k = indexDist(gen);
  }
	//elijo una dimensión
  int j = dimDist(gen);
  vector<double> cand = foods[i];
	//aplico la fórmula de gbest
  cand[j] = foods[i][j] + phiDist(gen) * (foods[i][j] - foods[k][j]) +
            psiDist(gen) * (bestSolution[j] - foods[i][j]);
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
      bestSolution = foods[i];
    }
  } else {
    trial[i] += 1;
  }
}

/*
 * ABC guiado por la mejor solucion global (Gbest-guided ABC, GABC).
 *   Zhu & Kwong, "Gbest-guided artificial bee colony algorithm for numerical
 *   function optimization", Applied Mathematics and Computation, 2010.
 *
 * La unica diferencia con el ABC clasico es la ecuacion de generacion del
 * candidato. En lugar de
 *     v_ij = x_ij + phi_ij (x_ij - x_kj)
 * anado un termino que atrae la nueva fuente hacia la mejor solucion global
 * encontrada hasta el momento (gbest):
 *     v_ij = x_ij + phi_ij (x_ij - x_kj) + psi_ij (y_j - x_ij)
 * con phi_ij en [-1, 1], psi_ij en [0, C] (C constante, 1.5 habitual) e y la
 * mejor solucion global. El primer termino mantiene la exploracion y el segundo
 * acelera la explotacion guiando el enjambre hacia gbest.
 */
double artificialBeeColonyGbest(int dim, int seed, int lower, int upper, int SN,
                                int limit, int maxevals, double C) {
  std::uniform_real_distribution<double> dis(lower, upper);
  std::mt19937 gen(seed);

  int evals = 0;
  double bestFitness = -1;
  vector<double> bestSolution(dim);
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
      bestSolution = foods[i];
    }
  }

  while (evals < maxevals) {
    // fase de abejas empleadas cada una explota su propia fuente
    for (int i = 0; i < SN && evals < maxevals; i++) {
      greedyUpdate(i, foods, obj, trial, bestSolution, bestFitness, evals, lower,
                   upper, C, gen);
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
      greedyUpdate(i, foods, obj, trial, bestSolution, bestFitness, evals, lower,
                   upper, C, gen);
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
        bestSolution = foods[maxTrial];
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
  double C = 1.5;  // limite superior de psi: peso de la atraccion hacia gbest
  const size_t maxtimes = 5;

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init("abc-gbest", funcid, dim);

    double bestFitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness = artificialBeeColonyGbest(
          dim, seed, lower, upper, SN, limit, 10000 * dim / maxtimes, C);
      if (bestFitness < 0 || fitness < bestFitness) {
        bestFitness = fitness;
      }
    }

    cout << "Best ABC-Gbest[F" << funcid << "]: " << scientific
         << cec17_error(bestFitness) << endl;
  }

  return 0;
}
