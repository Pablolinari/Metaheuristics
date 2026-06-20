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

== Mejoras del algoritmo

=== Hibridación con Local Search

Com primera opción de mejora se opta por hibridarlo con la Búsqueda Local, el enfoque que se usa puede ser de dos maneras distintas.

- Como primera prueba se ha usado la búsqueda local para intensificar la búsqueda sobre la mejor solución. Esta búsqueda local no se ejecuta sobre todas las posibles mejores soluciones, sino que se tiene un contador y cada 





