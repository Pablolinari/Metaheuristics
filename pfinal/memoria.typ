// --- CONFIGURACIÓN ESTILO LATEX ---
#set page(
  paper: "a4",
  margin: (x: 3cm, y: 2.5cm),
  numbering: "1",
)

#set text(
  // "New Computer Modern" es la versión moderna de la fuente de LaTeX
  font: "New Computer Modern",
  size: 11pt,
  lang: "es",
  region: "es",
  hyphenate: true, // Activa la partición de palabras (silabeo)
)

// Justificar el texto (típico de LaTeX)
#set par(
  justify: true,
  leading: 0.65em, // Espacio entre líneas
  first-line-indent: 1.5em, // Indentación de la primera línea
)

// Regla para que el primer párrafo tras un título SÍ tenga sangría
// (En LaTeX estándar el primero no tiene, pero muchos lo prefieren así)

#show heading: it => {
  it
  par(text(size: 0pt, ""))
}

// Configuración de encabezados
#set heading(numbering: "1.1")
#show heading: set block(above: 1.4em, below: 1em)

// --- PORTADA ---
#align(center)[
  #v(1cm)
  #image("logo.jpg", width: 60%)
  #v(3cm)

  #block(text(weight: "bold", 2.5em)[Artificial Bee Colony])
  #v(0.5em)
  #block(text(weight: "bold", 1.8em)[Práctica opcional])
  #v(1cm)
  #block(text(weight: "bold", 1.5em)[Pablo Linari Pérez])

  #v(3cm)

  #text(size: 1.2em)[
    #underline[DNI:] 75571079W \
    #underline[Curso]: 25/26 \
    #underline[Correo]: #link("mailto:pablolinari@correo.ugr.es")[pablolinari\@correo.ugr.es]
  ]
  #v(1fr)
]

#pagebreak()

// --- ÍNDICE ---
#outline(
  title: [Índice de contenidos],
  indent: auto,
)

#pagebreak()

// --- CONTENIDO ---

= Descripción del algoritmo

El algoritmo *Artificial Bee Colony* (ABC), propuesto por Dervis Karaboga en 2007, es una metaheurística poblacional inspirada en el comportamiento de búsqueda de alimento de las colonias de abejas. La idea del algoritmo es que un conjunto de abejas explora el espacio de búsqueda  en el que hay una serie de fuentes de alimentos, intensificando la exploración alrededor de las fuentes más prometedoras y abandonando aquellas que se agotan.

En el contexto de este problema cada *fuente de alimento* representa una solución candidata, y la cantidad de néctar de una fuente se corresponde con el valor de fitness de esa solución. El objetivo del algoritmo es, por tanto, encontrar la fuente con mayor cantidad de néctar, es decir, la solución que maximiza la función objetivo.

==Explicación de los componentes del algoritmo

La colonia se divide en tres tipos de abejas, cada uno con un papel distinto dentro de la búsqueda:

- *Abejas obreras (worker bees)*: cada una está asociada a una fuente de alimento concreta y se encarga de explotarla buscando soluciones mejores en su vecindario.
- *Abejas observadoras (onlooker bees)*: esperan en la colmena y eligen qué fuentes explotar en función de la información que comparten las obreras. Las fuentes con más néctar tienen más probabilidad de ser elegidas, esto hace que se concentre la búsqueda en las funetes de alimento más prometedoras.
- *Abejas exploradoras (scout bees)*: cuando una fuente se agota se abandona, y la abeja obrera que trabajaba sobre esta se convierte en exploradora, buscando una fuente de alimento nueva.

El número de fuentes de alimento, según se indica en el artículo, es la mitad del tamaño total de la colonia, y coincide tanto con el número de abejas trabajadoras como con el de abejas observadoras ya que cada fuente tiene una trabajadora asociada.

Esta forma de inicializar la población es la propuesta en el artículo , no obstante veremos que se puede modificar este parámetro para cambiar el comportamiento del algoritmo aunque no sea el parámetro que más influya.

== Funcionamiento  del algoritmo
== Parámetros del algoritmo

El comportamiento del algoritmo se controla mediante los siguientes parámetros:

- *colonySize*: tamaño total de la colonia. El número de fuentes de alimento se calcula como $S N = "colonySize" / 2$. En la implementación se usa un valor de 50, por lo que $S N = 25$.

- *Limit* : número máximo de ciclos que una fuente puede permanecer sin mejorar antes de ser abandonada. Controla cuanto se pueden explotar las soluciones.
- *maxevals*: número máximo de evaluaciones de la función objetivo, que actúa como criterio de parada.

== Ecuaciones

El algoritmo usa dos ecuaciones . La primera genera una solución candidata $v_i$ a partir de una fuente existente $x_i$, perturbando una única componente $j$ elegida al azar:

$ v_(i j) = x_(i j) + phi.alt_(i j) dot (x_(i j) - x_(k j)) $

donde $k != i$ es otra fuente vecina elegida aleatoriamente y $phi.alt_(i j)$ es un número aleatorio en el intervalo $[-1, 1]$. El resto de componentes de $v_i$ se copian directamente de $x_i$.La nueva solución se acerca o se aleja de una fuente vecina en una sola dimensión.

Para entenderlo mejor , este movimiento en el problema del portfolio equivale a tomar una solución $i$ , y sumar o resar a una empresa de dicha solución un porcentaje de la misma empresa pero de la otra solución escogida $k$.De esta forma nos acercaríamos o alejaríamos de la solución escogida en dicha componente.

La segunda ecuación calcula la probabilidad de que cada fuente sea seleccionada por una observadora, de forma proporcional a su néctar:

$ p_i = "fit"_i / (sum_(n=1)^(S N) "fit"_n) $

Cuanto mayor es el fitness de una fuente, mayor es su probabilidad de ser explotada por las observadoras.

== Etapas del algoritmo

La ejecución se divide en 4 fases claras donde se identifica el comportamiento de cada abeja con su rol.

=== Etapa 1: Inicialización

Se generan $S N$ fuentes de alimento de forma aleatoria, asegurando que cada una cumple las restricciones del problema (en el caso del portfolio se comprueban las restriciones necesarias). Se evalúa el fitness de cada una y se memoriza la mejor solución encontrada hasta el momento. Además, se inicializa a 0 un contador de _trials_ por cada fuente, que registra cuántos ciclos lleva sin mejorar.

=== Etapa 2: Fase de abejas empleadas

Cada abeja empleada explota su propia fuente $x_i$. Para ello genera una solución candidata $v_i$ aplicando la ecuación (2.2) y la evalúa. A continuación se aplica una selección greedy: si la candidata es mejor que la fuente original, la sustituye y el contador de _trials_ de esa fuente se reinicia a 0, en caso contrario, la fuente se mantiene y su contador de _trials_ se incrementa. Esta fase genera por cada fuente de alimento una posible solución mejor usando la ecuación previamente vista, si esta solución es mejor será la que se use como base para la explotación.

=== Etapa 3: Fase de abejas observadoras

Esta etapa consiste en el "baile de las obreras" donde cada obrera vuelve a la colmena e intentará convencer a las observadoras de elegir su fuente de alimento , las observadoras eligirán con mayor probabilidad la fuente que más nectar tenga.Los pasos que se siguen una vez la obrera vuelve a la colmena son los siguientes. Primero se calculan las probabilidades de selección de cada fuente con la segunda ecuación . Después, se reparten $S N$ observadoras: cada una elige una fuente mediante selección por ruleta ,(las fuentes con más néctar tienen más probabilidad) y la explota igual que una empleada, usando la misma ecuación y aplicando de nuevo la selección greedy. Como las mejores fuentes tienden a ser elegidas varias veces, esta fase intensifica la búsqueda en las regiones más prometedoras del espacio. Es decir que podemos tener varias abejas observadoras en la misma fuente de alimento, esto ocurrirá sobre todo en las fuente más prometedoras donde como es lógico, interesa intensificar la búsqueda.

=== Etapa 4: Fase de abeja exploradora

Al final de cada ciclo se comprueba si alguna fuente se ha agotado. Se localiza la fuente con mayor número de _trials_ acumulados y, si este supera el _Limit_, dicha fuente se abandona, se sustituye y la abeja exploradora buscaría una nueva, que a modo práctico consiste en crear una nueva solución aleatoria,se evalúa la nueva solución y su contador de _trials_ vuelve a 0. Se permite como máximo una abeja exploradora por ciclo. Esta fase introduce diversificación y evita que el algoritmo quede estancado en óptimos locales.

En las tres fases se actualiza siempre la mejor solución global encontrada, que es la que finalmente devuelve el algoritmo.

== Pseudocódigo

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
  ```py
  funcion ABC(problema, maxevals):
    SN = colonySize / 2
    # --- Inicialización ---
    para i en 0..SN:
      fuentes[i]   = problema.createSolution()
      fitness[i]   = problema.fitness(fuentes[i])
      trials[i]    = 0
      actualizar_mejor(fuentes[i], fitness[i])

    # --- Ciclos de búsqueda ---
    mientras evals < maxevals:

      # (a) Fase empleadas
      para i en 0..SN:
        candidata = produceCandidate(fuentes, i)   # ec. 2.2
        si fitness(candidata) > fitness[i]:         # selección greedy
          fuentes[i] = candidata;  trials[i] = 0
        sino:
          trials[i] += 1
        actualizar_mejor(candidata)

      # (b) Fase observadoras
      probs = computeProbabilities(fitness)         # ec. 2.1
      para t en 0..SN:
        i = rouletteSelect(probs)
        candidata = produceCandidate(fuentes, i)    # ec. 2.2
        si fitness(candidata) > fitness[i]:         # selección greedy
          fuentes[i] = candidata;  trials[i] = 0
        sino:
          trials[i] += 1
        actualizar_mejor(candidata)

      # (c) Fase exploradora (máx. una por ciclo)
      peor = indice con mayor trials
      si trials[peor] >= trialLimit:
        fuentes[peor] = problema.createSolution()
        trials[peor]  = 0
        actualizar_mejor(fuentes[peor])

    devolver mejor_solucion
  ```
]

== Adaptaciones al problema del portfolio

La formulación original de ABC está pensada para la optimización de funciones numéricas sin restricciones, por lo que ha sido necesario realizar dos adaptaciones:

- *Reparación de soluciones*: la ecuación que se usa para explorar nuevas soluciones modifica una componente y, en general, rompe las restricciones del problema (los pesos deben sumar 1 y estar en $["low", "high"]$ o ser 0). Por ello, tras generar cada candidata se comprueba su validez y, si no es factible, se repara con `problem.fix()` antes de evaluarla.
- *Desplazamiento de las probabilidades*: la ecuación (2.1) asume cantidades de néctar no negativas, pero el fitness del portfolio  puede ser negativo, de hecho en la práctica es siempre negativo. Para poder aplicar la ruleta, se desplazan todos los valores de fitness sumándoles una constante que deja el peor de ellos ligeramente por encima de 0, conservando las diferencias relativas entre fuentes.

Por lo que respecta a la implementación , la  diferencia entre la implementación para el problema del portfolio y para el *cec2017* son las mencionadas previamente y el uso de las funciones de la clase *pPortfolio* , principalmente usadas para poder arreglar y generar soluciones con las restricciones del problema.


#pagebreak()

= Mejoras del algoritmo

== Hibridación con búsqueda local (Solis-Wets)

El ABC clásico es un buen explorador del espacio de búsqueda, pero su capacidad de *refinamiento local* es limitada: la ecuación (2.2) solo modifica una componente por candidata y depende de las diferencias entre fuentes, por lo que cuando la población converge esas diferencias se hacen pequeñas y el avance se ralentiza. La idea de esta hibridación es convertir el algoritmo en un *algoritmo memético*, combinando la exploración global del ABC con una *búsqueda local* que pula con detalle las soluciones más prometedoras.

=== La búsqueda local: Solis-Wets

Solis-Wets es una búsqueda local estocástica de paso adaptativo que opera sobre *una única solución*. En lugar de explorar todo el espacio, da pequeños pasos aleatorios alrededor de la solución actual y aprende en qué dirección conviene moverse mediante dos elementos:

- Un *paso* $delta$ que controla el tamaño de las perturbaciones y que se ajusta de forma dinámica según el algoritmo vaya teniendo éxito o no.
- Un vector de *sesgo* (_bias_) que acumula la dirección de los movimientos que han funcionado, de modo que la búsqueda tiende a continuar avanzando hacia donde ya ha mejorado.

En cada iteración se genera una perturbación aleatoria $d tilde U(0, delta)$ y se construye una solución candidata sumando el paso y el sesgo:

$ x' = x + d + "bias" $

Si la candidata mejora, se acepta y se *refuerza* el sesgo en esa dirección. Si no mejora, se prueba el *movimiento opuesto* $x' = x - d - "bias"$ (aprovechando la misma perturbación en sentido contrario); si este mejora, se acepta y el sesgo se ajusta en consecuencia. Si ninguno de los dos mejora, se considera un fracaso y el sesgo se reduce a la mitad para "olvidar" una dirección que ya no es útil.

El paso $delta$ se adapta para equilibrar exploración y precisión: tras *5 éxitos consecutivos* se duplica (la búsqueda va bien, conviene avanzar más rápido) y tras *3 fracasos consecutivos* se divide entre dos (hay que afinar con pasos más cortos). Este mecanismo permite que la BL dé pasos grandes cuando está lejos del óptimo local y pasos finos cuando está cerca de él.

=== Integración en el ABC

La búsqueda local no sustituye a ninguna fase del ABC, sino que se *intercala* dentro del bucle principal. La hibridación se controla con tres parámetros:

- *ls_freq*: cada cuántos ciclos del ABC se lanza la búsqueda local (se usa $50$).
- *ls_budget*: número de evaluaciones que se concede a cada aplicación de la BL (se usa $500$).
- *ls_delta*: paso inicial $delta$ de Solis-Wets (se usa $0.2$).

Al final de cada ciclo, y solo una vez cada `ls_freq` ciclos, se localiza la *mejor fuente* de la población actual y se le aplica Solis-Wets con un presupuesto de `ls_budget` evaluaciones. Después se reinicia su contador de _trials_ (acaba de ser intensificada, no tiene sentido considerarla agotada). Las evaluaciones que consume la BL se descuentan del presupuesto global del algoritmo, de forma que ABC y BL *comparten el mismo límite de evaluaciones*: la búsqueda local no es "gratis", sino que se le roban evaluaciones a la fase poblacional.

De este modo el ABC se encarga de la *diversificación* (encontrar regiones prometedoras del espacio) y Solis-Wets de la *intensificación* (exprimir al máximo la mejor solución encontrada), repartiendo el esfuerzo entre ambos a lo largo de la ejecución.

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
  ```py
  # dentro del bucle principal del ABC, al final de cada ciclo:
  cycle += 1
  si use_ls y cycle % ls_freq == 0 y evals < maxevals:
    best = indice de la fuente con menor obj    # la mejor
    budget = min(ls_budget, maxevals - evals)   # no pasarse del limite
    evals += soliswets(foods[best], obj[best], ls_delta, budget, ...)
    trial[best] = 0                             # recien intensificada
    actualizar_mejor(foods[best])
  ```
]

== Hibridación con Differential Evolution

*Differential Evolution* (DE) es una metaheurística poblacional que genera nuevas soluciones combinando las *diferencias entre individuos* de la población. Su operador básico es la *mutación diferencial*: a un vector base se le suma la diferencia escalada entre otros dos individuos, $F dot (x_(r 2) - x_(r 3))$, donde $F$ es un factor de escala. Después, un *cruce binomial* mezcla el mutante con la solución original copiando cada componente con probabilidad $C R$, garantizando con un índice aleatorio (`jrand`) que al menos una componente provenga del mutante. Finalmente se aplica una selección _greedy_ entre la candidata y la original.

A diferencia de la ecuación (2.2) del ABC, que solo modifica una componente y usa una única fuente vecina, el operador de DE *combina información de toda la población en varias dimensiones a la vez*, lo que suele dar un mejor poder de búsqueda. Se han probado *dos formas distintas* de incorporar DE al ABC.

=== Tipo 1: DE como operador de generación de candidatos

En esta primera variante DE *no se añade como una fase nueva*, sino que *reemplaza la regla de movimiento* de las abejas. Donde el ABC clásico usaba la ecuación (2.2), ahora se usa el operador *DE/current-to-best/1* con cruce binomial. Para cada componente seleccionada por el cruce:

$
  v_(i j) = x_(i j) + F dot (x_("best" j) - x_(i j)) + F dot (x_(r 1 j) - x_(r 2 j))
$

donde $x_"best"$ es la mejor fuente actual del enjambre y $r 1, r 2$ son dos índices aleatorios distintos entre sí, de $i$ y de _best_. El término $F dot (x_"best" - x_i)$ atrae la candidata hacia la mejor solución (intensificación dirigida), mientras que $F dot (x_(r 1) - x_(r 2))$ añade una perturbación diferencial que mantiene la diversidad. El cruce binomial con probabilidad $C R$ decide qué componentes se mutan, y `jrand` asegura que al menos una cambie.

Este operador se usa *en las dos fases de explotación* del ABC (abejas empleadas y observadoras), de modo que el esqueleto del algoritmo (ruleta de las observadoras, exploradoras según el _Limit_, criterio de parada) se mantiene intacto y la única diferencia con el ABC original es *cómo se genera cada candidata*. Los parámetros usados son $F = 0.5$ y $C R = 0.9$, valores habituales en DE.

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
  ```py
  # reemplaza a produceCandidate (ec. 2.2) en empleadas y observadoras:
  funcion produceCandidate_DE(foods, i):
    best = indice de la mejor fuente
    r1, r2 = dos indices aleatorios distintos de i, best y entre si
    jrand = dimension aleatoria
    cand = copia(foods[i])
    para j en 0..dim:
      si j == jrand o rand() < CR:                       # cruce binomial
        cand[j] = foods[i][j] + F*(foods[best][j]-foods[i][j])
                              + F*(foods[r1][j]-foods[r2][j])
    devolver clip(cand)
  ```
]

=== Tipo 2: DE como algoritmo completo intercalado

La segunda variante sigue *exactamente el mismo esquema que la hibridación con Solis-Wets*: en lugar de modificar la generación de candidatos, se *intercala un DE completo* dentro del bucle del ABC. Aquí DE no es un operador, sino un *algoritmo con su propio bucle generacional*: recorre toda la población aplicando a cada individuo mutación, cruce y selección _greedy_, y repite el proceso hasta agotar su presupuesto de evaluaciones. La variante usada es *DE/rand/1/bin*, cuya mutación parte de un vector base aleatorio en lugar de la mejor fuente:

$ v_(i j) = x_(r 1 j) + F dot (x_(r 2 j) - x_(r 3 j)) $

con $r 1, r 2, r 3$ tres índices distintos entre sí y de $i$. Al partir de un individuo aleatorio en vez del mejor, esta versión es *más exploradora* que la del Tipo 1.

DE cumple aquí el papel que tenía Solis-Wets en la hibridación memética: opera sobre las fuentes del ABC, las modifica _in place_ y devuelve el número de evaluaciones consumidas para que el ABC las descuente de su presupuesto. Se controla con los mismos tres parámetros:

- *de_freq*: cada cuántos ciclos del ABC se lanza el DE (se usa $50$).
- *de_budget*: evaluaciones concedidas a cada ejecución del DE (se usa $500$).
- *F, CR*: parámetros del DE ($F = 0.5$, $C R = 0.9$).

La diferencia conceptual entre las dos variantes es clara: en el *Tipo 1* el comportamiento de DE está presente *en todo momento* (sustituye al movimiento de las abejas), mientras que en el *Tipo 2* el ABC y el DE se *alternan por fases*, dejando que cada uno trabaje sobre la población durante un número de evaluaciones antes de devolver el control al otro. El Tipo 2 permite además aislar el efecto de DE, ya que activarlo o desactivarlo no cambia el resto del algoritmo.

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
  ```py
  # algoritmo DE completo, ejecutado sobre la poblacion del ABC:
  funcion differential_evolution(foods, obj, trial, F, CR, maxevals):
    evals = 0
    mientras evals < maxevals:
      para i en 0..SN:
        r1, r2, r3 = tres indices distintos de i y entre si
        jrand = dimension aleatoria
        cand = copia(foods[i])
        para j en 0..dim:
          si j == jrand o rand() < CR:                  # cruce binomial
            cand[j] = foods[r1][j] + F*(foods[r2][j]-foods[r3][j])
        candfit = fitness(clip(cand));  evals += 1
        si candfit < obj[i]:                            # seleccion greedy
          foods[i] = cand;  obj[i] = candfit;  trial[i] = 0
    devolver evals

  # dentro del bucle principal del ABC, al final de cada ciclo:
  cycle += 1
  si use_de y cycle % de_freq == 0 y evals < maxevals:
    budget = min(de_budget, maxevals - evals)
    evals += differential_evolution(foods, obj, trial, F, CR, budget)
  ```
]

= Propuestas de mejora
== ABC guiado por la mejor solución global (Gbest-guided ABC)

Una de las mejoras que se pueden aplicar consiste en cambiar la forma en la que las abejas generan nuevas soluciones. En el ABC clásico el movimiento que define la ecuación (2.2) es puramente *local y no informado*: la perturbación $phi.alt_(i j) dot (x_(i j) - x_(k j))$ desplaza la fuente $i$ en la dirección de otra fuente $k$ elegida al azar, sin tener en cuenta dónde se encuentra la mejor solución conocida. Esto favorece la exploración, pero ralentiza la convergencia, ya que la información de la mejor solución encontrada no se aprovecha para guiar la búsqueda.

El *Gbest-guided ABC* (GABC), propuesto por Zhu y Kwong (2010), corrige esta carencia inspirándose en la actualización de velocidad de la optimización por enjambre de partículas (PSO): añade a la ecuación de movimiento un término adicional que atrae cada nueva fuente hacia la *mejor solución global* encontrada hasta el momento. De este modo el algoritmo conserva la componente exploratoria del ABC y, al mismo tiempo, incorpora una presión de explotación que acelera la convergencia hacia las regiones prometedoras del espacio de búsqueda.

=== Regla de generación del candidato

La regla de generación del candidato (ecuación 2.2) se reemplaza por una expresión con dos términos de perturbación. Sobre la componente $j$ elegida al azar:

$ v_(i j) = x_(i j) + phi.alt_(i j) dot (x_(i j) - x_(k j)) + psi_(i j) dot (y_j - x_(i j)) $

donde $y$ es la mejor solución global encontrada hasta el momento y cada término cumple un papel concreto:

- El término $phi.alt_(i j) dot (x_(i j) - x_(k j))$, con $phi.alt_(i j) in [-1, 1]$ y $k != i$ una fuente aleatoria, es el *movimiento clásico del ABC* y aporta la *exploración*: genera diversidad muestreando en la vecindad de otras fuentes de la población.
- El término $psi_(i j) dot (y_j - x_(i j))$ es la *aportación del GABC* y proporciona la *explotación*: arrastra la fuente $i$ hacia la mejor solución global, de forma proporcional a la distancia que las separa, lo que acelera la convergencia.
- El factor $psi_(i j)$ es un número aleatorio uniforme en $[0, C]$, donde $C$ es una constante positiva (se usa $C = 1.5$, valor recomendado por los autores) que controla el *peso de la atracción* hacia la mejor solución. Un valor de $C$ demasiado grande provoca una convergencia prematura, mientras que uno demasiado pequeño desaprovecha la guía global.

Al igual que en el ABC clásico, tras generar la candidata se recorta al dominio $["lower", "upper"]$ antes de evaluarla. El resto de fases del algoritmo (observadoras con ruleta, exploradoras según el _Limit_ y criterio de parada) se mantienen idénticas, de forma que la única diferencia con el ABC original es la regla de movimiento, lo que permite atribuir cualquier cambio en el rendimiento exclusivamente a la guía por la mejor solución global. La única estructura de datos adicional que requiere esta variante es el vector $y$ con la mejor solución global, que se actualiza cada vez que se encuentra una fuente con mejor _fitness_.


== Comparación de resultados



#figure(
  image("code/graficas/comparativa_dim30_barras.png", width: 100%),
  caption: [Comparación error para dimensión 30],
)

#figure(
  image("code/graficas/victorias_dim30.png", width: 80%),
  caption: [Número de victorias dimensión 30],
)
#figure(
  image("code/graficas/comparativa_dim50_barras.png", width: 100%),
  caption: [Comparación error para dimensión 50],
)

#figure(
  image("code/graficas/victorias_dim50.png", width: 80%),
  caption: [Número de victorias dimensión 50],
)
#figure(
  image("code/graficas/comparativa_dim100_barras.png", width: 100%),
  caption: [Comparación error para dimensión 100],
)
#figure(
  image("code/graficas/victorias_dim100.png", width: 80%),
  caption: [Número de victorias dimensión 100],
)
