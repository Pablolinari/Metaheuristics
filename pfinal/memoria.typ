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
  #image("../logo.jpg", width: 60%)
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

= Funcionamiento del algoritmo

El algoritmo *Artificial Bee Colony* (ABC), propuesto por Karaboga y Basturk en 2007, es una metaheurística poblacional inspirada en el comportamiento de búsqueda de alimento de las colonias de abejas. La idea central es que un conjunto de abejas explora el espacio de búsqueda en torno a una serie de *fuentes de alimento*, intensificando la exploración alrededor de las fuentes más prometedoras y abandonando aquellas que se agotan.

En el contexto de este problema cada *fuente de alimento* representa una solución candidata (un vector de pesos del portfolio), y la *cantidad de néctar* de una fuente se corresponde con el valor de fitness de esa solución. El objetivo del algoritmo es, por tanto, encontrar la fuente con mayor cantidad de néctar, es decir, la solución que maximiza la función objetivo.

== La metáfora de la colonia

La colonia se divide en tres tipos de abejas, cada uno con un papel distinto dentro de la búsqueda:

- *Abejas empleadas (employed bees)*: cada una está asociada a una fuente de alimento concreta y se encarga de explotarla, buscando soluciones mejores en su vecindario.
- *Abejas observadoras (onlooker bees)*: esperan en la colmena y eligen qué fuentes explotar en función de la información que comparten las empleadas. Las fuentes con más néctar tienen más probabilidad de ser elegidas, lo que concentra el esfuerzo en las zonas prometedoras (intensificación).
- *Abejas exploradoras (scout bees)*: cuando una fuente se agota (no mejora durante muchos ciclos) se abandona, y la abeja empleada asociada se convierte en exploradora, buscando una fuente completamente nueva de forma aleatoria (diversificación).

El número de fuentes de alimento $S N$ es la mitad del tamaño total de la colonia, y coincide tanto con el número de abejas empleadas como con el de abejas observadoras (cada fuente tiene una empleada asociada).

== Parámetros del algoritmo

El comportamiento del algoritmo se controla mediante los siguientes parámetros:

- *colonySize*: tamaño total de la colonia. El número de fuentes de alimento se calcula como $S N = "colonySize" / 2$. En la implementación se usa un valor de 50, por lo que $S N = 25$.
- *trialLimit* (parámetro _limit_ del artículo): número máximo de ciclos que una fuente puede permanecer sin mejorar antes de ser abandonada. Controla cuándo entra en juego la fase exploradora.
- *maxevals*: número máximo de evaluaciones de la función objetivo, que actúa como criterio de parada.

== Ecuaciones fundamentales

El algoritmo se apoya en dos ecuaciones clave. La primera genera una *solución candidata* $v_i$ a partir de una fuente existente $x_i$, perturbando una única componente $j$ elegida al azar:

$ v_(i j) = x_(i j) + phi.alt_(i j) dot (x_(i j) - x_(k j)) $

donde $k != i$ es otra fuente vecina elegida aleatoriamente y $phi.alt_(i j)$ es un número aleatorio en el intervalo $[-1, 1]$. El resto de componentes de $v_i$ se copian directamente de $x_i$. Intuitivamente, la nueva solución se acerca o se aleja de una fuente vecina en una sola dimensión.

La segunda ecuación calcula la *probabilidad* de que cada fuente sea seleccionada por una observadora, de forma proporcional a su néctar:

$ p_i = "fit"_i / (sum_(n=1)^(S N) "fit"_n) $

Cuanto mayor es el fitness de una fuente, mayor es su probabilidad de ser explotada por las observadoras.

== Etapas del algoritmo

La ejecución del algoritmo se organiza en una etapa inicial de inicialización seguida de un bucle de ciclos. Cada ciclo se compone de tres fases consecutivas (empleadas, observadoras y exploradoras), que se repiten hasta agotar el número máximo de evaluaciones.

=== Etapa 1: Inicialización

Se generan $S N$ fuentes de alimento de forma aleatoria mediante `createSolution()`, asegurando que cada una cumple las restricciones del problema (pesos en $["low", "high"]$ que suman 1). Se evalúa el fitness de cada una y se memoriza la mejor solución encontrada hasta el momento. Además, se inicializa a 0 un contador de _trials_ por cada fuente, que registra cuántos ciclos lleva sin mejorar.

=== Etapa 2: Fase de abejas empleadas

Cada abeja empleada explota su propia fuente $x_i$. Para ello genera una solución candidata $v_i$ aplicando la ecuación (2.2) y la evalúa. A continuación se aplica una *selección greedy*: si la candidata es mejor que la fuente original, la sustituye y el contador de _trials_ de esa fuente se reinicia a 0; en caso contrario, la fuente se mantiene y su contador de _trials_ se incrementa. Esta fase explora el vecindario de todas las fuentes existentes.

=== Etapa 3: Fase de abejas observadoras

Primero se calculan las probabilidades de selección de cada fuente con la ecuación (2.1). Después, se reparten $S N$ observadoras: cada una elige una fuente mediante *selección por ruleta* (las fuentes con más néctar tienen más probabilidad) y la explota igual que una empleada, generando una candidata con la ecuación (2.2) y aplicando de nuevo la selección greedy. Como las mejores fuentes tienden a ser elegidas varias veces, esta fase intensifica la búsqueda en las regiones más prometedoras del espacio.

=== Etapa 4: Fase de abeja exploradora

Al final de cada ciclo se comprueba si alguna fuente se ha agotado. Se localiza la fuente con mayor número de _trials_ acumulados y, si este supera el `trialLimit`, dicha fuente se abandona: se sustituye por una nueva solución aleatoria generada con `createSolution()`, se evalúa y su contador de _trials_ vuelve a 0. Se permite *como máximo una* abeja exploradora por ciclo. Esta fase introduce diversificación y evita que el algoritmo quede estancado en óptimos locales.

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

- *Reparación de soluciones*: la ecuación (2.2) perturba una componente y, en general, rompe las restricciones del problema (los pesos deben sumar 1 y estar en $["low", "high"]$ o ser 0). Por ello, tras generar cada candidata se comprueba su validez y, si no es factible, se repara con `problem.fix()` antes de evaluarla.
- *Desplazamiento de las probabilidades*: la ecuación (2.1) asume cantidades de néctar no negativas, pero el fitness del portfolio (beneficio menos $lambda$ por riesgo) puede ser negativo. Para poder aplicar la ruleta, se desplazan todos los valores de fitness sumándoles una constante que deja el peor de ellos ligeramente por encima de 0, conservando las diferencias relativas entre fuentes. Si todas las fuentes tienen el mismo fitness se reparte la probabilidad de forma uniforme.
