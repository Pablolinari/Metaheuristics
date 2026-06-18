
#pragma once

#include "mh.h"
#include "problem.h"
#include "solution.h"
#include <vector>

using namespace std;

// Instanciamos la plantilla con el tipo que nos interese
using MHDouble = MH<double>;
using ProblemDouble = Problem<double>;
using ResultMDouble = ResultMH<double>;

/**
 * Artificial Bee Colony (ABC).
 *
 * Implementación del algoritmo de colonia de abejas artificiales descrito en
 * Karaboga & Basturk, "A powerful and efficient algorithm for numerical
 * function optimization: artificial bee colony (ABC) algorithm" (2007).
 *
 * La colonia se divide en tres grupos de abejas:
 *  - Empleadas: explotan las fuentes de alimento actuales (ecuación 2.2).
 *  - Observadoras: eligen fuentes por ruleta según su néctar (ecuación 2.1).
 *  - Exploradoras (scouts): abandonan una fuente agotada y la sustituyen por
 *    otra aleatoria.
 *
 * El número de fuentes de alimento (SN) es la mitad del tamaño de la colonia, y
 * coincide con el número de empleadas y de observadoras.
 */
class Abc : public MHDouble {

public:
  /**
   * @param colonySize Tamaño total de la colonia. El número de fuentes de
   *                   alimento (SN) es colonySize / 2.
   * @param trialLimit Número máximo de ciclos sin mejora de una fuente antes de
   *                   abandonarla (parámetro "limit" del artículo).
   */
  Abc(int colonySize = 50, int trialLimit = 50);
  ~Abc() {}

  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

private:
  int colonySize;
  int trialLimit;
};
