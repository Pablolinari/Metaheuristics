extern "C" {
#include "cec17.h"
}
#include <iostream>
#include <vector>
#include <random>
#include <cmath>

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

// diversidad del enjambre: media de las desviaciones tipicas por dimension,
// normalizada por el tamano del dominio 
double swarmDiversity(const vector<vector<double>> &foods, int lower,
                      int upper) {
  int SN = foods.size();
  int dim = foods[0].size();
  double total = 0.0;
  for (int j = 0; j < dim; j++) {
    // media de la dimension j
    double mean = 0.0;
    for (int i = 0; i < SN; i++) {
      mean += foods[i][j];
    }
    mean /= SN;
    // desviacion tipica de la dimension j
    double var = 0.0;
    for (int i = 0; i < SN; i++) {
      double d = foods[i][j] - mean;
      var += d * d;
    }
    var /= SN;
    total += std::sqrt(var);
  }
  double avgStd = total / dim;
  return avgStd / (upper - lower);
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
 * ABC guiado por gbest con LIMIT DINAMICO para equilibrar diversidad y
 * convergencia. La base es el Gbest-guided ABC (Zhu & Kwong, 2010): el termino
 * gbest esta siempre activo y acelera la convergencia.
 *
 * El problema del gbest puro es que converge demasiado y el enjambre se colapsa.
 * Aqui el limite de la abeja exploradora (cuantos ciclos sin mejorar aguanta una
 * fuente antes de reiniciarse) deja de ser fijo y se adapta a la diversidad:
 *   - poca diversidad (enjambre colapsado) -> limit pequeno: las fuentes se
 *     reinician antes, inyectando diversidad con saltos aleatorios grandes.
 *   - mucha diversidad (enjambre disperso)  -> limit grande: las fuentes
 *     explotan mas tiempo hacia gbest.
 * Como reiniciar una fuente es un salto aleatorio en todo el dominio, el limit
 * dinamico si reabre el enjambre de verdad (a diferencia de apagar gbest).
 *
 *   C        : limite superior de psi (peso de la atraccion hacia gbest).
 *   limitMin : limit cuando la diversidad es minima (mas exploracion).
 *   limitMax : limit cuando la diversidad es maxima (mas explotacion).
 */
double artificialBeeColonyGbestDl(int dim, int seed, int lower, int upper,
                                  int SN, int maxevals, double C, int limitMin,
                                  int limitMax) {
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

  // diversidad de referencia: la de la poblacion inicial (la mas dispersa)
  double divRef = swarmDiversity(foods, lower, upper);
  if (divRef <= 0.0) {
    divRef = 1.0;  // evito dividir por cero en casos degenerados
  }

  while (evals < maxevals) {
    // limit dinamico segun la diversidad actual respecto a la inicial
    double frac = swarmDiversity(foods, lower, upper) / divRef;
    if (frac > 1.0) {
      frac = 1.0;
    } else if (frac < 0.0) {
      frac = 0.0;
    }
    int curLimit = limitMin + (int)(frac * (limitMax - limitMin));

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

    // fase de abeja exploradora: abandono la fuente mas estancada usando el
    // limit dinamico de este ciclo
    int maxTrial = 0;
    for (int i = 1; i < SN; i++) {
      if (trial[i] > trial[maxTrial]) {
        maxTrial = i;
      }
    }
		//genero la nueva fuente de alimento
    if (trial[maxTrial] >= curLimit && evals < maxevals) {
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
  int dim = 50;
  int seed = 42;
  int lower = -100;
  int upper = 100;
  int SN = 20;
  double C = 1.5;     // limite superior de psi: peso de la atraccion a gbest
  int limitMin = 20;  // limit con diversidad minima (reinicia fuentes antes)
  int limitMax = 200; // limit con diversidad maxima (explota mas)
  const size_t maxtimes = 5;

  for (int funcid = 1; funcid <= 30; funcid++) {
    cec17_init("abc-gbest-dl", funcid, dim);

    double bestFitness = -1;
    for (size_t times = 0; times < maxtimes; times++) {
      double fitness = artificialBeeColonyGbestDl(
          dim, seed, lower, upper, SN, 10000 * dim / maxtimes, C, limitMin,
          limitMax);
      if (bestFitness < 0 || fitness < bestFitness) {
        bestFitness = fitness;
      }
    }

    cout << "Best ABC-Gbest-DL[F" << funcid << "]: " << scientific
         << cec17_error(bestFitness) << endl;
  }

  return 0;
}
