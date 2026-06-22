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

El algoritmo *Artificial Bee Colony* (ABC), propuesto por Dervis Karaboga en 2007, es una metaheurística de swarm intelligence inspirada en el comportamiento de búsqueda de alimento de las colonias de abejas. La idea del algoritmo es que un conjunto de abejas explora el espacio de búsqueda  en el que hay una serie de fuentes de alimentos, intensificando la exploración alrededor de las fuentes más prometedoras y abandonando aquellas que se agotan.

En el contexto de este problema cada *fuente de alimento* representa una solución candidata, y la cantidad de néctar de una fuente se corresponde con el valor de fitness de esa solución. El objetivo del algoritmo es  encontrar la fuente con mayor cantidad de néctar, es decir, la solución que maximiza la función objetivo.

== Explicación de los componentes del algoritmo

La colonia se divide en tres tipos de abejas, cada uno con un papel distinto dentro de la búsqueda:

- *Abejas obreras (worker bees)*: cada una está asociada a una fuente de alimento concreta y se encarga de explotarla buscando soluciones mejores en su vecindario.
- *Abejas observadoras (onlooker bees)*: esperan en la colmena y eligen qué fuentes explotar en función de la información que comparten las obreras. Las fuentes con más néctar tienen más probabilidad de ser elegidas, esto hace que se concentre la búsqueda en las funetes de alimento más prometedoras.Conceptualmente podríamos decir que se convierten en obreras cuando llegan a la fuente de alimento.
- *Abejas exploradoras (scout bees)*: cuando una fuente se agota se abandona, y la abeja obrera que trabajaba sobre esta se convierte en exploradora, buscando una fuente de alimento nueva.La abeja obrera principal se transforma en exploradora 

Des esta manera las observadoras se transforman en obreras cuando llegan a una fuente de alimento y cuando el alimento se acabaa , la obrera que había sido la primera en llegar se convierte en exploradora.

El número de fuentes de alimento, según se indica en el artículo, es la mitad del tamaño total de la colonia, y coincide tanto con el número de abejas trabajadoras como con el de abejas observadoras ya que cada fuente tiene una trabajadora asociada.

Esta forma de inicializar la población es la propuesta en el artículo , no obstante veremos que se puede modificar este parámetro para cambiar el comportamiento del algoritmo aunque no sea el parámetro que más influya.

== Funcionamiento  del algoritmo
== Parámetros del algoritmo

El comportamiento del algoritmo se controla mediante los siguientes parámetros:

- *colonySize*: tamaño total de la colonia. El número de fuentes de alimento se calcula como $S N = "colonySize" / 2$. En la implementación se usa un valor de 50, por lo que $S N = 25$.

- *Limit* : número máximo de ciclos que una fuente puede permanecer sin mejorar antes de ser abandonada. Controla cuanto se pueden explotar las soluciones.Es el parámetro que más influye en el algoritmo.
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

La ejecución se divide en 4 fases  donde se identifica el comportamiento de cada abeja con su rol.

=== Etapa 1: Inicialización

Se generan $S N$ fuentes de alimento de forma aleatoria, asegurando que cada una cumple las restricciones del problema (en el caso del portfolio se comprueban las restriciones necesarias). Se evalúa el fitness de cada una y se memoriza la mejor solución encontrada hasta el momento. Además, se inicializa a 0 un contador de _trials_ por cada fuente, que registra cuántos ciclos lleva sin mejorar.

=== Etapa 2: Fase de abejas empleadas

Cada abeja empleada explota su propia fuente $x_i$. Para ello genera una solución candidata $v_i$ aplicando la  ecuación y la evalúa. A continuación se aplica una selección greedy: si la candidata es mejor que la fuente original, la sustituye y el contador de _trials_ de esa fuente se reinicia a 0, en caso contrario, la fuente se mantiene y su contador de _trials_ se incrementa. Esta fase genera por cada fuente de alimento una posible solución mejor usando la ecuación previamente vista, si esta solución es mejor será la que se use como base para la explotación.

=== Etapa 3: Fase de abejas observadoras

Esta etapa consiste en el "baile de las obreras" donde cada obrera vuelve a la colmena e intentará convencer a las observadoras de elegir su fuente de alimento , las observadoras eligirán con mayor probabilidad la fuente que más nectar tenga.Los pasos que se siguen una vez la obrera vuelve a la colmena son los siguientes. Primero se calculan las probabilidades de selección de cada fuente con la segunda ecuación . Después, se reparten $S N$ observadoras: cada una elige una fuente mediante selección por ruleta ,(las fuentes con más néctar tienen más probabilidad) y la explota igual que una obrera, usando la misma ecuación y aplicando de nuevo la selección greedy. Como las mejores fuentes tienden a ser elegidas varias veces, esta fase intensifica la búsqueda en las regiones más prometedoras del espacio. Es decir que podemos tener varias abejas observadoras en la misma fuente de alimento, esto ocurrirá sobre todo en las fuente más prometedoras donde como es lógico, interesa intensificar la búsqueda.

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

      # Fase empleadas
      para i en 0..SN:
        candidata = produceCandidate(fuentes, i)   
        si fitness(candidata) > fitness[i]:         # selección greedy
          fuentes[i] = candidata;  trials[i] = 0
        sino:
          trials[i] += 1
        actualizar_mejor(candidata)

      # Fase observadoras
      probs = computeProbabilities(fitness)         
      para t en 0..SN:
        i = rouletteSelect(probs)
        candidata = produceCandidate(fuentes, i)    
        si fitness(candidata) > fitness[i]:         # selección greedy
          fuentes[i] = candidata;  trials[i] = 0
        sino:
          trials[i] += 1
        actualizar_mejor(candidata)

      # Fase exploradora ( una por ciclo)
      peor = indice con mayor trials
      si trials[peor] >= trialLimit:
        fuentes[peor] = problema.createSolution()
        trials[peor]  = 0
        actualizar_mejor(fuentes[peor])

    devolver mejor_solucion
  ```
]

#pagebreak()
= Uso del software

El código se reparte en dos carpetas independientes según el problema sobre el que se trabaja: `portfolio`, con la aplicación al problema del portfolio, y `cec2017`, con la experimentación sobre el benchmark CEC2017.

== Carpeta portfolio

El código se organiza en tres bloques: la carpeta `src`, que contiene los archivos `.cpp` con la implementación de cada algoritmo, la carpeta `include`, con las cabeceras (`.h`/`.hpp`) correspondientes y la librería Eigen y el fichero `main.cpp`, que es el programa principal, se encarga de leer los argumentos y lanzar los algoritmos. La implementación del ABC se encuentra en `src/abc.cpp` (con su cabecera en `include/abc.h`), y el resto de algoritmos usados en la comparación siguen el mismo patrón (`randomsearch.cpp`, `localsearch.cpp`, `greedy.cpp`, `ambest.cpp`, `bmb.cpp`, `es.cpp`, etc.).

Para compilar se ejecuta `./script.sh release`, que configura el proyecto con CMake en modo *release* y genera el ejecutable `program`. La ejecución recibe la ruta del CSV de datos, los pesos mínimo y máximo (`low` y `high`), el número de ejecuciones, la semilla y, opcionalmente, el archivo CSV de salida:

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
  ```sh
  ./program data/ibex_35.csv 0.005 0.08 50 42 ib35.csv
  ```
]

Por terminal se muestra la media de las ejecuciones y, en el CSV de salida, todos los fitness obtenidos. Para reproducir de una sola vez los experimentos sobre los tres conjuntos de datos (IBEX-35, S&P 100 y S&P 500) se proporciona el script `execute_all.sh`, que lanza `program` sobre cada uno de ellos con sus parámetros y escribe los resultados en `salida.txt`:

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
  ```sh
  ./execute_all.sh
  ```
]

== Carpeta `cec2017`

Esta carpeta contiene el código del benchmark CEC2017 junto con un fichero de prueba por cada variante del algoritmo: `testabc.cc` (ABC base), `testabchybrid.cc` (hibridación con Solis-Wets), `testabcde.cc`(hibridación con el operador de DE) , `testabcdehybrid.cc` (hibridacion completa con DE ), `testabcgbest.cc` (ABC guiado por la mejor solución global) y `testabcgbestdl.cc` (gbest con límite dinámico). Todos ellos se compilan desde el `CMakeLists.txt`, que genera un ejecutable por archivo.

Cada ejecutable recorre las 30 funciones del benchmark y escribe sus resultados en la carpeta `results` correspondiente que hay que crear previamente. Por ejemplo:

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
  ```sh
  cmake CMakeLists.txt
  make
  ./testabc
  ```
]

Los scripts de python restantes son los usados para generar las gráficas y los xlsx con los resultados.

= Adaptacion al problema del portfolio

La formulación original de ABC está pensada para la optimización de funciones numéricas sin restricciones, por lo que ha sido necesario realizar dos adaptaciones:

- *Reparación de soluciones*: la ecuación que se usa para explorar nuevas soluciones modifica una componente y, en general, rompe las restricciones del problema (los pesos deben sumar 1 y estar en $["low", "high"]$ o ser 0). Por ello, tras generar cada candidata se comprueba su validez y, si no es factible, se repara con `problem.fix()` antes de evaluarla.
- *Desplazamiento de las probabilidades*: la ecuación de cálculo de probabilidades  asume cantidades de néctar no negativas, pero el fitness del portfolio  puede ser negativo, de hecho en la práctica es siempre negativo. Para poder aplicar la ruleta, se desplazan todos los valores de fitness sumándoles una constante que deja el peor de ellos ligeramente por encima de 0, conservando las diferencias relativas entre fuentes.

Por lo que respecta a la implementación , la  diferencia entre la implementación para el problema del portfolio y para el *cec2017* son las mencionadas previamente y el uso de las funciones de la clase *pPortfolio* , principalmente usadas para poder arreglar y generar soluciones con las restricciones del problema.

== Comparación de resultados

En esta sección veremos los resultados que obtiene el ABC implementado sin ninguna modificación en el problema del portfolio, los algoritmos que se usan para la comparación serán: *Random Search , Greedy ,Local Search, AM-BEST , BMB y Enfriamiento simulado*.


#figure(
  table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 7 { (bottom: 0.8pt) } else { none },
    inset: 8pt,

    [*Algoritmo*],
    [*2015-2024*\ *Fitness*],
    [*2025*\ *Fitness*],
    [*Beneficio*],
    [*Evaluaciones*],
    [*Tiempo*\ *(segundos)*],

    [Greedy], [-4.558], [-3.254], [0.040], [1], [0.001],
    [Random], [-4.980], [-3.378], [0.052], [10000], [0.005],
    [BL], [-4.608], [-3.115], [0.020], [1517.36], [0.001],
    [AM-BEST], [-4.494], [-3.084], [0.032], [10000], [0.005],
    [BMB], [-4.566], [-3.104], [0.022], [6990.3], [0.001],
    [ES], [-4.594], [-3.113], [0.022], [10000], [0.003],
    [ABC], [-4.519], [-3.139], [0.025], [10000], [0.002],
  ),
  caption: [Resultados de Ibex 35 (Medias)],
)
#figure(
  table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 7 { (bottom: 0.8pt) } else { none },
    inset: 8pt,

    [*Algoritmo*],
    [*2015-2024*\ *Fitness*],
    [*2025*\ *Fitness*],
    [*Beneficio*],
    [*Evaluaciones*],
    [*Tiempo*\ *(segundos)*],

    [Greedy], [-2.835], [-3.691], [0.092], [1], [0.001],
    [Random], [-3.408], [-4.174], [0.088], [10000], [0.02],
    [BL], [-3.118], [-3.888], [0.096], [8984.76], [0.01],
    [AM-BEST], [-2.814], [-3.755], [0.101], [10000], [0.044],
    [BMB], [-3.023], [-3.837], [0.092], [10000], [0.022],
    [ES], [-3.105], [-3.895], [0.095], [10000], [0.048],
    [ABC], [-3.036], [-3.871], [0.091], [10000], [0.011],
  ),
  caption: [Resultados de S&P100 (Medias)],
)
#figure(
  table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 7 { (bottom: 0.8pt) } else { none },
    inset: 8pt,

    [*Algoritmo*],
    [*2015-2024*\ *Fitness*],
    [*2025*\ *Fitness*],
    [*Beneficio*],
    [*Evaluaciones*],
    [*Tiempo*\ *(segundos)*],

    [Greedy], [-2.749], [-3.776], [0.013], [1], [0.001],
    [Random], [-3.638], [-4.389], [0.026], [10000], [0.253],
    [BL], [-3.517], [-4.324], [0.010], [10000], [0.220],
    [AM-BEST], [-3.220], [-4.156], [0.005], [10000], [0.456],
    [BMB], [-3.663], [-4.450], [0.013], [10000], [0.238],
    [ES], [-3.440], [-4.343], [0.012], [10000], [2.606],
    [ABC], [-3.584], [-4.363], [0.012], [10000], [0.180],
  ),

  caption: [Resultados de S&P500 (Medias)],
)
#figure(
  image("cec2017/graficas/ib35.png", width: 80%),
  caption: [Boxplots Ibex35],
)

#figure(
  image("cec2017/graficas/syp100.png", width: 80%),
  caption: [Boxplots S&P100],
)
#figure(
  image("cec2017/graficas/syp500.png", width: 80%),
  caption: [Boxplots S&P500],
)
Como se puede ver , el ABC consigue estar a la altura de los mejores algoritmos de la
práctica (AM-BEST y GREEDY) en dimensiones bajas (IBEX35) pero a medida que van subiendo las dimensiones el algoritmo va perdiendo fuerza y se termina quedando en las posiciones 3 y 4. Esto se debe a la mala convergencia que tiene y a que el número de iteraciones no sea lo suficientemente grande para que pueda sacar buenos resultados. También podemos fijarnos en los boxplots del ABC , en el S&P100 su caja se ensancha de una manera esperada ya que el espacio es másgrande que el del IBEX por tanto la población inicial le afecta más, pero al pasar al S&P500 su boxplot presenta muchos outliers por el rango de arriba, esto nos indica que en varias ejecuciones el resultado ha sido mejor que la media , y también presenta outliers por abajo representando todo lo contrario. Esto nos indica que el algoritmo depende mucho de las fuentes de alimento 
que se toman inicialmente, por tanto si una vez seleccionada la población inicial , las demás fuentes se tomasen en una determinada dirección en vez de hacerlo aleatoriamente el algoritmo podría mejorar considerablemente. Este tipo de mejoras lo veremos en las sigueintes secciones.

Podemos concluir que el algoritmo ABC aplicado a la práctica se encuentra entre el tercer y cuarto puesto de los mejores algoritmos para este escenario de práctico donde se han usado 10000 evaluaciones.

#pagebreak()

= Mejoras del algoritmo
La mejoras del algoritmo se han implementado usando las funciones de *cec2017* para que la comparación pueda ser más efectiva y se pueda comparar en más problemas , no solo en el de prácticas 

Veremos a lo largo de esta sección las diferentes mejoras que se han hecho sobre el algoritmo , se han planteado mejoras que hybridan el algoritmo de ABC con otros como el Solis-Wets o el Diferential evolution , y también se han hecho mejoras que solo modifican el operador de selección de nuevas soluciones.De esta forma podemos ver que cambios tienen más impacto sobre el algoritmo.


== Hibridación con búsqueda local (Solis-Wets)

El ABC clásico es un buen explorador del espacio de búsqueda, pero su capacidad de *refinamiento local* es limitada: la ecuación de selección solo modifica una componente por candidata y depende de las diferencias entre fuentes, por lo que cuando la población converge esas diferencias se hacen pequeñas y el avance se ralentiza. La idea de esta hibridación es convertir el algoritmo en un algoritmo memético, combinando la exploración global del ABC con una búsqueda local que intente refinar con más detalle las mejores soluciones

=== Búsqueda local: Solis-Wets

Solis-Wets es una búsqueda local estocástica de paso adaptativo que opera sobre *una única solución*. En lugar de explorar todo el espacio, da pequeños pasos aleatorios alrededor de la solución actual y aprende en qué dirección conviene moverse mediante dos elementos:

- Un *paso* $delta$ que controla el tamaño de las perturbaciones y que se ajusta de forma dinámica según el algoritmo vaya teniendo éxito o no.
- Un vector de *sesgo* que acumula la dirección de los movimientos que han funcionado, de modo que la búsqueda tiende a continuar avanzando hacia donde ya ha mejorado.

En cada iteración se genera una perturbación aleatoria  y se construye una solución candidata sumando el paso y el sesgo:

$ x' = x + d + "bias" $

Si la candidata mejora, se acepta y se refuerza el sesgo en esa dirección. Si no mejora, se prueba el movimiento opuesto $x' = x - d - "bias"$ ,si este mejora, se acepta y el sesgo se ajusta . Si ninguno de los dos mejora, se considera un fracaso y el sesgo se reduce a la mitad para no volver a tomar una dirección que ya no es útil.

El paso $delta$ se adapta para equilibrar exploración y precisión: tras 5 éxitos consecutivos se duplica  y tras 3 fracasos consecutivos se divide entre dos . Esto  permite que la BL dé pasos grandes cuando está lejos del óptimo local y pasos finos cuando está cerca de él.

=== Integración en el ABC

La búsqueda local no sustituye a ninguna fase del ABC, sino que se intercala dentro del bucle principal. La hibridación se controla con tres parámetros:

- *ls_freq*: cada cuántos ciclos del ABC se aplica la búsqueda local.
- *ls_budget*: número de evaluaciones que se usan en cada aplicación de la BL.
- *ls_delta*: paso inicial $delta$ de Solis-Wets.

Al final de cada ciclo, y solo una vez cada ls_freq ciclos, se localiza la mejor fuente de la población actual y se le aplica Solis-Wets con unas determinadas evaluaciones. Después se reinicia su contador de trials ya que como acaba de ser mejorada , no tiene sentido considerarla agotada y queremos que se intente explotar todavía más. Las evaluaciones que consume la BL se descuentan del presupuesto global del algoritmo, de forma que ABC y BL comparten el mismo límite de evaluaciones.
#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
  ```py
  # dentro del bucle principal del ABC, al final de cada ciclo:
  cycle += 1
  si use_ls y cycle % ls_freq == 0 y evals < maxevals:
    best = indice de la fuente con menor obj    # la mejor
    budget = min(ls_budget, maxevals - evals)   # no pasarse del limite
    evals += soliswets(foods[best], obj[best], ls_delta, budget, ...)
    trial[best] = 0                             # permito seguir explotando
    actualizar_mejor(foods[best])
  ```
]

== Hibridación con Differential Evolution

Differential Evolution es una metaheurística poblacional que genera nuevas soluciones combinando las diferencias entre individuos de la población. Su operador básico es la mutación diferencial: a un vector base se le suma la diferencia escalada entre otros dos individuos, $F dot (x_(r 2) - x_(r 3))$, donde $F$ es un factor de escala. Después, un cruce binomial mezcla el mutante con la solución original copiando cada componente con probabilidad $C R$, garantizando con un índice aleatorio que al menos una componente provenga del mutante. Finalmente se aplica una selección _greedy_ entre la candidata y la original.
El operador de DE combina información de toda la población en varias dimensiones a la vez, lo que suele dar un mejor poder de búsqueda. Se han probado dos formas distintas de incorporar DE al ABC.


=== Hibridación completa con DE 

Esta hibridación sigue exactamente el mismo esquema que la hibridación con Solis-Wets:intercala el algoritmo DE completo dentro del bucle del ABC. El DE Recorre toda la población aplicando a cada individuo mutación, cruce y selección greedy y repite el proceso hasta agotar su presupuesto de evaluaciones. La mutación que se usa  parte de un vector base aleatorio :

$ v_(i j) = x_(r 1 j) + F dot (x_(r 2 j) - x_(r 3 j)) $

con $r 1, r 2, r 3$ , $i$ tres índices distintos entre sí.

El DE  se ejecuta sobre las fuentes del ABC, las modifica in place y devuelve el número de evaluaciones consumidas para que el ABC las tenga en cuenta. Se controla con los mismos tres parámetros:

- *de_freq*: cada cuántos ciclos del ABC se lanza el DE.
- *de_budget*: evaluaciones concedidas a cada ejecución del DE.
- *F, CR*: parámetros del DE.

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

=== Uso del operador de DE para generación de candidatos

En esta variante DE no se añade como una fase nueva, sino que reemplaza la regla de movimiento de las abejas, ese decir usaremos su operador de mutación para encontrar nuevas soluciones. Donde el ABC clásico usaba la ecuación vista anteriormente para generar nuevas soluciones, ahora se usa el operador del Diferential Evolution con cruce binomial.En este caso para que tenga más efecto se usa en vez de una solución aleatoria la mejor solución, para intentar crear soluciones en esa dirección. Para cada componente usamos el cruce:

$
  v_(i j) = x_(i j) + F dot (x_("best" j) - x_(i j)) + F dot (x_(r 1 j) - x_(r 2 j))
$

donde $x_"best"$ es la mejor fuente actual del enjambre y $r 1, r 2$ son dos índices aleatorios distintos entre sí, de $i$ y de _best_. El término $F dot (x_"best" - x_i)$ mueve la solucion escogida hacia la mejor solución, a diferencia del ABC normal que se mueve solo en la dirección de un vecino escogido aleatoriamente que puede no ser la mejor solución,  y $F dot (x_(r 1) - x_(r 2))$ añade una perturbación diferencial que mantiene la diversidad. El cruce binomial con probabilidad $C R$ decide qué componentes se mutan y  asegura que al menos una cambie.

Este operador se usa la fase de las abejas obreras y en la de las abejas observadoras  del ABC de esta manera el esquema del algoritmo se mantiene y la única diferencia con el ABC original es cómo se genera cada candidata. Los parámetros usados son $F = 0.5$ y $C R = 0.9$, valores habituales en DE.

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
-----
== Comparación de las hibridaciones

#figure(
  image("cec2017/graficas/comparativa_dim30_barrashyb.png", width: 100%),
  caption: [Comparación error para dimensión 30],
)

#figure(
  image("cec2017/graficas/victorias_dim30hyb.png", width: 80%),
  caption: [Número de victorias dimensión 30],
)
#figure(
  image("cec2017/graficas/comparativa_dim50_barrashyb.png", width: 100%),
  caption: [Comparación error para dimensión 50],
)

#figure(
  image("cec2017/graficas/victorias_dim50hyb.png", width: 80%),
  caption: [Número de victorias dimensión 50],
)
#figure(
  image("cec2017/graficas/comparativa_dim100_barrashyb.png", width: 100%),
  caption: [Comparación error para dimensión 100],
)
#figure(
  image("cec2017/graficas/victorias_dim100hyb.png", width: 80%),
  caption: [Número de victorias dimensión 100],
)
= Comparación de hibridaciones

En este apartado se comparan las tres hibridaciones propuestas frente al ABC clásico: *ABC-LS* (hibridación con la búsqueda local de Solis-Wets), *ABC-DE* (cambio del operador de generación por el de DE) y *ABC-DE-HYB* (hibridación completa con DE). Los gráficos de barras multicolores muestran el error relativo al mejor de cada función (cuanto más alta es la barra, peor es el resultado) y los gráficos de barras verdes indican cuántas veces gana cada algoritmo.

Observando los gráficos de victorias vemos que las dos hibridaciones que de verdad mejoran al ABC son ABC-DE-HYB y ABC-LS, que se reparten el primer puesto según la dimensión. En dimensión 30 y 50 la mejor es la hibridación completa con DE (ABC-DE-HYB), que gana en 12 y 15 funciones respectivamente, por delante del ABC-LS (9 y 12). Sin embargo, en dimensión 100 la situación se invierte y el ABC-LS pasa a ser claramente el mejor con 12 victorias frente a las 8 del ABC-DE-HYB. Esto tiene sentido ya que la búsqueda local de Solis-Wets refina las soluciones de forma muy efectiva y escala mejor a dimensiones altas, mientras que el DE completo, al recorrer toda la población dentro del bucle del ABC, pierde algo de eficiencia cuando el espacio de búsqueda crece.

El caso del ABC-DE (cambio únicamente del operador)  es el  peor de todas las variantes en número de victorias e incluso queda por debajo del ABC clásico. Además, si nos fijamos en los gráficos multicolores, es con diferencia el algoritmo que comete los errores más grandes . Esto nos indica que cuando pierde, pierde por mucho, y que el simple reemplazo del operador de generación por el de DE desestabiliza al algoritmo en lugar de mejorarlo.

El ABC clásico, por su parte, se comporta razonablemente bien en dimensión 100 (7 victorias, por delante incluso del ABC-DE-HYB en algunas funciones), lo que confirma que el ABC base es un buen explorador en espacios grandes.

Como conclusión, las dos hibridaciones recomendables son ABC-DE-HYB y ABC-LS. La hibridación completa con DE destaca en dimensiones bajas y medias, pero el ABC-LS es la opción más robusta y la que mejor escala, por lo que la tomamos como la mejor hibridación para la comparación posterior con el resto de modificaciones del ABC.

#pagebreak()
= Propuestas de mejora

Uno de los puntos débiles del ABC es la forma en las que se explotan las funetes de alimento ya que al tomar vecinos aleatoriamente para generar nuevas soluciones , podemos estar tomando soluciones malas y estar moviendo nuestra solución hacia soluciones peores.

Esto hace que la convergencia del algoritmo sea lenta y que no se puedan alcanzar las mejores soluciones dentro de las evaluaciones permitidas.

== ABC guiado por la mejor solución global Gbest-guided ABC

Una de las mejoras que se pueden aplicar consiste en cambiar la forma en la que las abejas generan nuevas soluciones. En el ABC clásico la dirección en la que se modifican las soluciones es local ya que  la perturbación $phi.alt_(i j) dot (x_(i j) - x_(k j))$ desplaza la fuente $i$ en la dirección de otra fuente $k$ elegida al azar, sin tener en cuenta dónde se encuentra la mejor solución conocida. Esto favorece la exploración, pero ralentiza la convergencia ya que la información de la mejor solución encontrada no se aprovecha para guiar la búsqueda.

El Gbest-guided ABC , corrige esto  inspirándose en la actualización de velocidad del PSO,  añade a la ecuación de movimiento un término adicional que atrae cada nueva fuente hacia la *mejor solución global* encontrada hasta el momento.Así el algoritmo conserva la componente exploratoria del ABC y aumenta la  presión de explotación que acelera la convergencia hacia las regiones prometedoras del espacio de búsqueda.

=== Regla de generación del candidato

La regla de generación del candidato  se reemplaza por una expresión con dos términos de perturbación. Sobre la componente $j$ elegida al azar:

$ v_(i j) = x_(i j) + phi.alt_(i j) dot (x_(i j) - x_(k j)) + psi_(i j) dot (y_j - x_(i j)) $

donde $y$ es la mejor solución global encontrada hasta el momento y cada término hace lo siguiente:

- El término $phi.alt_(i j) dot (x_(i j) - x_(k j))$, con $phi.alt_(i j) in [-1, 1]$ y $k != i$ una fuente aleatoria, es el movimiento clásico del ABC y aporta la exploración: genera diversidad usando las soluciones vecinas.
- El término $psi_(i j) dot (y_j - x_(i j))$ es la parte que arrastra la fuente $i$ hacia la mejor solución global, de forma proporcional a la distancia que las separa, lo que acelera la convergencia.
- El factor $psi_(i j)$ es un número aleatorio en $[0, C]$, donde $C$ es una constante positiva  que controla el peso de la atracción hacia la mejor solución. Un valor de $C$ demasiado grande provoca una convergencia prematura, mientras que uno demasiado pequeño desaprovecha la guía global.

Lo mejor de esta modificación es que mantiene el mismo esquema que el ABC original y solo tenemos que cambiar la función del operador que genera nuevas soluciones.

== ABC guiado por gbest con límite de  dinámico

El término gbest acelera mucho la convergencia, pero tiene un efecto secundario: como todas las fuentes son atraídas hacia la misma solución global, el enjambre tiende a _colapsar_ en un único punto. Cuando esto ocurre, el ABC deja de progresar, porque la perturbación clásica $phi.alt_(i j) dot (x_(i j) - x_(k j))$ es proporcional a la distancia entre fuentes, si todas las fuentes están casi en el mismo sitio, los movimientos que generan son pequeños y la búsqueda se estanca.

El único mecanismo del ABC que introduce diversidad real es la abeja exploradora, que reinicia una fuente con un salto aleatorio en todo el dominio. Sin embargo, en el ABC clásico esa fase depende de un parámetro $"limit"$ fijo, una fuente solo se abandona tras $"limit"$ ciclos sin mejorar. Si $"limit"$ es muy alto, las fuentes estancadas tardan demasiado en reiniciarse y el enjambre permanece colapsado, si es muy bajo, se reinician fuentes que todavía eran útiles y se pierde capacidad de explotación.

La propuesta consiste en hacer que ese límite *deje de ser fijo y se adapte a la diversidad del enjambre*. Como base para esta modificación voy a seleccionar el ABC que usa Gbest ya que es el que mejor funciona y  lo único que cambio es cuándo se reinicia una fuente.

=== Medida de diversidad

En cada ciclo se mide la dispersión de las fuentes calculando, para cada dimensión $j$, la desviación típica $sigma_j$ de las $S N$ fuentes, promediando sobre las $n$ dimensiones y normalizando por el tamaño del dominio:

$ D = 1/n sum_(j=1)^(n) sigma_j / ("upper" - "lower") $

El valor $D$ es cercano a $0$ cuando el enjambre está colapsado y máximo cuando está disperso. Esta medida no consume evaluaciones, ya que solo usa las coordenadas de las fuentes que ya tenemos.

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
  ```py
  funcion swarmDiversity(fuentes, lower, upper):
    SN  = numero de fuentes
    n   = numero de dimensiones
    total = 0
    para j en 0..n:                       # recorro cada dimensión
      # media de la dimensión j sobre las SN fuentes
      media = 0
      para i en 0..SN:
        media += fuentes[i][j]
      media /= SN
      # varianza de la dimensión j
      var = 0
      para i en 0..SN:
        var += (fuentes[i][j] - media)^2
      var /= SN
      total += sqrt(var)                   # desviación típica sigma_j
    avgStd = total / n                      # promedio de las sigma_j
    devolver avgStd / (upper - lower)       # normalizado por el dominio
  ```
]

=== Límite dinámico

El límite de abandono se interpola entre un mínimo y un máximo en función de la diversidad actual respecto a la inicial $D_0$ (la de la población aleatoria de partida, que es la más dispersa). Definiendo $"frac" = D \/ D_0$ recortado a $[0, 1]$, el límite del ciclo es:

$ "limit" = "limit"_"min" + "frac" dot ("limit"_"max" - "limit"_"min") $

El comportamiento resultante es:

- *Poca diversidad* (enjambre colapsado, $D approx 0$): el límite baja hacia $"limit"_"min"$, de modo que las fuentes estancadas se reinician antes y se inyecta diversidad mediante saltos aleatorios.
- *Mucha diversidad* (enjambre disperso, $D approx D_0$): el límite sube hacia $"limit"_"max"$, dejando que las fuentes exploten más tiempo hacia la mejor solución global.

De esta forma  cuando el enjambre converge demasiado, el propio algoritmo aumenta el ritmo de reinicios para reabrir la búsqueda, y cuando hay diversidad suficiente, relaja los reinicios para afinar la solución. A diferencia de una estrategia que simplemente desactivase la guía global cuando baja la diversidad, aquí la diversidad se recupera con el reinicio aleatorio de la exploradora, cuyo salto es de magnitud grande e independiente de lo colapsado que esté el enjambre, por lo que sí consigue reabrir la búsqueda de manera efectiva.

Los parámetros usados son $"limit"_"min" = 20$ y $"limit"_"max" = 200$, manteniendo $C = 1.5$ para el término gbest.

= Comparación de resultados de distintos ABC

En este apartado veremos los resultados de las distintas modificaciones que se le han hecho al ABC y veremos cuales destacan. En las siguiente sección se compararán las mejores con los resultados obtenidos en CEC2017 por otros algoritmos


Los gráficos de barras multicolores indican el error que comete cada algoritmo en cada función. Cuanto más alta es la barra peor es el resultado del algoritmo. Los gráfics de barras verdes indican cuantas veces ha ganado un algoritmo.

#figure(
  image("cec2017/graficas/comparativa_dim30_barras.png", width: 100%),
  caption: [Comparación error para dimensión 30],
)

#figure(
  image("cec2017/graficas/victorias_dim30.png", width: 80%),
  caption: [Número de victorias dimensión 30],
)
#figure(
  image("cec2017/graficas/comparativa_dim50_barras.png", width: 100%),
  caption: [Comparación error para dimensión 50],
)

#figure(
  image("cec2017/graficas/victorias_dim50.png", width: 80%),
  caption: [Número de victorias dimensión 50],
)
#figure(
  image("cec2017/graficas/comparativa_dim100_barras.png", width: 100%),
  caption: [Comparación error para dimensión 100],
)
#figure(
  image("cec2017/graficas/victorias_dim100.png", width: 80%),
  caption: [Número de victorias dimensión 100],
)

Observando los gráficos donde se evalua el número de funciones ganadas vemos que el algoritmo que mejor actua independientemente de la dimensión es el ABC-LS. Esto es lógico ya que el punto débil del ABC es la convergencia , y si lo hibridamos con BL , conseguimos intensificar la explotación de las mejores soluciones en  dirección a una solución mejor y no de manera más ineficiente como hace el ABC. La hibridación completa con DE ABC-DE-HYB también consigue buenos resultados en casi todas las dimensiones pero peores que el ABC-LS , esto se debe a que cuando la dimensión es mayor como DE tiene que recorrer toda la población dentro del bucle del ABC pierde algo de eficiencia cuando el espacio de búsqueda crece, por tanto la mejor hibridación que podemos tener de ABC en esta prueba es la que se hace con la búsqueda local. 

Si nos fijamos en la hibridación con DE hecha  solo en el operador de búsqueda de nuevas soluciones ABC-DE vemos que el comportamiento es mejor que el del ABC en casi todos los casos menos en el de dimensión 100 donde el ABC clásico lo supera.Por tanto esto nos indica que el cambio en el operador influye bastante en la mejora de el algoritmo base pero no lo suficiente como para superar al algoritmo hibridado.También se puede observar en el gráfico multicolor  que es el algoritmo que comete los errores más grandes , es decir que cuando pierde pierde por mucho, esto nos indica entonces que es bastante inestable en sus resultados y que  las veces que gana puede ser por que la función a evaluar le favorezca.


Por último vemos que la modificación ABC-GBEST ha tenido muy buenos resultados ya que permite al algoritmo acelerar su convergencia, la diferencia de victorias entre ABC-GBEST y ABC-GBEST-DL (usando límite dinámico) es pequeña y ambos algoritmos consiguen ganar consistentemente en casi todas las dimensiones. El límite dinámico junto al GBEST ayuda a acelerar la convergencia y a  dedicar mas o menos evaluaciones a las fuentes de alimento dependiendo de su diversidad ajustando el parámetro limit, por eso ABC-GBEST-DL tiene resultados un poco mejores al que solo usa GBEST 


Como conclusión final podemos ver que el ABC por si solo es un algoritmo que se comporta bien en  ya que consigue algunas victorias en dimensión 100, pero el usar generadores de soluciones un poco más complejos le ayuda a mejorar drásticamente. En cuanto a las hibridaciones vemos que la mejor es la que usa Solis-Wets pero la que usa DE también es una buena opción si pudiesemos tener más evaluaciones.

#pagebreak()
= Comparación de resultados CEC2017

En esta sección se van a compara los resultados obtenidos por las mejores implementaciones del ABC con los siguientes algoritmos: *DE*, *PSO *, *RB-IPOP-CMA-ES* y *jSO*. La comparación se realiza sobre las 30 funciones del benchmark *CEC2017* en dimensión 30,50 y 100 mostrando el error medio obtenido por cada algoritmo. La fila Best indica en cuántas funciones cada algoritmo obtiene el mejor resultado.

Los algoritmos que compararemos serán : *ABC* , *ABC-LS* (mejor hibridación), *ABC-GBESTDL* (mejor modificación).

== ABC
#figure(
  text(size: 10pt)[#table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 31 { (bottom: 0.8pt) } else { none },
    inset: 6pt,

    [*Función*],
    [*ABC*],
    [*DE*],
    [*PSO*],
    [*RB-IPOP-CMA-ES*],
    [*jSO*],

    [F01], [3.665e+03], [4.909e+04], [4.175e+09], [3.150e-08], [#text(fill: red)[0.000e+00]],
    [F02], [6.747e+10], [1.309e+19], [1.000e+00], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F03], [1.192e+05], [3.481e+03], [5.453e+04], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F04], [#text(fill: red)[7.786e+00]], [8.430e+01], [1.183e+03], [5.528e+01], [5.867e+01],
    [F05], [1.156e+02], [2.015e+02], [2.170e+02], [#text(fill: red)[1.649e+00]], [8.557e+00],
    [F06], [1.717e-03], [6.320e+00], [3.694e+01], [1.208e-07], [#text(fill: red)[6.000e-09]],
    [F07], [1.146e+02], [2.334e+02], [3.596e+02], [#text(fill: red)[3.433e+01]], [3.893e+01],
    [F08], [1.148e+02], [1.894e+02], [1.745e+02], [#text(fill: red)[1.756e+00]], [9.092e+00],
    [F09], [2.578e+03], [6.530e+01], [2.842e+03], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F10], [2.933e+03], [3.764e+03], [6.938e+03], [#text(fill: red)[1.443e+03]], [1.528e+03],
    [F11], [7.757e+02], [7.958e+01], [1.207e+03], [4.107e+01], [#text(fill: red)[3.037e+00]],
    [F12], [3.230e+06], [3.258e+05], [3.586e+08], [1.093e+03], [#text(fill: red)[1.704e+02]],
    [F13], [1.537e+05], [1.536e+02], [4.508e+07], [1.187e+02], [#text(fill: red)[1.484e+01]],
    [F14], [2.171e+05], [7.100e+01], [3.059e+05], [9.083e+01], [#text(fill: red)[2.183e+01]],
    [F15], [2.336e+04], [6.256e+01], [2.737e+05], [2.177e+02], [#text(fill: red)[1.088e+00]],
    [F16], [6.607e+02], [1.319e+03], [1.568e+03], [5.016e+02], [#text(fill: red)[7.892e+01]],
    [F17], [3.390e+02], [4.809e+02], [4.725e+02], [1.323e+02], [#text(fill: red)[3.292e+01]],
    [F18], [7.763e+05], [6.122e+01], [2.170e+06], [1.601e+02], [#text(fill: red)[2.041e+01]],
    [F19], [8.593e+03], [3.572e+01], [1.260e+06], [1.146e+02], [#text(fill: red)[4.503e+00]],
    [F20], [9.940e+01], [2.751e+02], [4.621e+02], [2.965e+02], [#text(fill: red)[2.937e+01]],
    [F21], [3.097e+02], [3.255e+02], [4.113e+02], [#text(fill: red)[2.086e+02]], [2.093e+02],
    [F22], [1.369e+02], [1.002e+02], [1.027e+03], [6.716e+02], [#text(fill: red)[1.000e+02]],
    [F23], [4.790e+02], [5.346e+02], [6.404e+02], [#text(fill: red)[3.393e+02]], [3.507e+02],
    [F24], [7.046e+02], [6.059e+02], [7.095e+02], [#text(fill: red)[4.194e+02]], [4.265e+02],
    [F25], [3.900e+02], [3.870e+02], [6.864e+02], [#text(fill: red)[3.867e+02]], [#text(fill: red)[3.867e+02]],
    [F26], [#text(fill: red)[3.139e+02]], [4.038e+02], [3.369e+03], [3.939e+02], [9.202e+02],
    [F27], [5.230e+02], [#text(fill: red)[4.925e+02]], [8.068e+02], [5.118e+02], [4.974e+02],
    [F28], [4.272e+02], [3.943e+02], [1.105e+03], [3.089e+02], [#text(fill: red)[3.087e+02]],
    [F29], [6.453e+02], [1.025e+03], [1.409e+03], [4.933e+02], [#text(fill: red)[4.337e+02]],
    [F30], [2.470e+04], [3.657e+03], [1.359e+07], [2.844e+03], [#text(fill: red)[1.971e+03]],
    [*Best*], [*2*], [*1*], [*0*], [*10*], [*20*],
  )],
  caption: [Resultados CEC2017 dimensión 30 (Medias)],
)

#figure(
  text(size: 10pt)[#table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 31 { (bottom: 0.8pt) } else { none },
    inset: 6pt,

    [*Función*],
    [*ABC*],
    [*DE*],
    [*PSO*],
    [*RB-IPOP-CMA-ES*],
    [*jSO*],

    [F01], [1.039e+04], [2.039e+08], [1.823e+10], [1.131e-07], [#text(fill: red)[0.000e+00]],
    [F02], [7.357e+26], [5.158e+52], [1.000e+00], [2.767e+05], [#text(fill: red)[0.000e+00]],
    [F03], [2.942e+05], [7.597e+04], [1.008e+05], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F04], [7.455e+01], [2.322e+02], [4.091e+03], [#text(fill: red)[2.959e+01]], [5.621e+01],
    [F05], [2.610e+02], [3.978e+02], [4.328e+02], [#text(fill: red)[2.790e+00]], [1.641e+01],
    [F06], [9.876e-04], [1.180e+01], [5.450e+01], [#text(fill: red)[1.632e-07]], [1.093e-06],
    [F07], [2.863e+02], [4.781e+02], [8.197e+02], [#text(fill: red)[5.663e+01]], [6.650e+01],
    [F08], [2.801e+02], [4.070e+02], [4.077e+02], [#text(fill: red)[2.575e+00]], [1.696e+01],
    [F09], [1.006e+04], [3.756e+03], [1.620e+04], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F10], [5.599e+03], [1.127e+04], [1.295e+04], [#text(fill: red)[1.726e+03]], [3.140e+03],
    [F11], [4.434e+03], [2.173e+02], [3.151e+03], [1.832e+02], [#text(fill: red)[2.794e+01]],
    [F12], [8.735e+06], [1.382e+08], [6.062e+09], [2.438e+06], [#text(fill: red)[1.681e+03]],
    [F13], [2.067e+04], [2.726e+04], [5.262e+08], [1.652e+03], [#text(fill: red)[3.060e+01]],
    [F14], [1.664e+06], [1.683e+02], [1.558e+06], [2.416e+02], [#text(fill: red)[2.496e+01]],
    [F15], [2.469e+04], [5.090e+02], [5.790e+06], [5.290e+02], [#text(fill: red)[2.386e+01]],
    [F16], [1.267e+03], [3.047e+03], [2.745e+03], [8.899e+02], [#text(fill: red)[4.505e+02]],
    [F17], [1.360e+03], [1.827e+03], [1.652e+03], [3.978e+02], [#text(fill: red)[2.829e+02]],
    [F18], [5.569e+06], [2.420e+04], [1.203e+07], [3.573e+02], [#text(fill: red)[2.428e+01]],
    [F19], [9.648e+04], [1.322e+02], [1.336e+07], [1.394e+02], [#text(fill: red)[1.414e+01]],
    [F20], [6.477e+02], [9.384e+02], [1.282e+03], [5.468e+02], [#text(fill: red)[1.401e+02]],
    [F21], [4.418e+02], [6.177e+02], [6.655e+02], [#text(fill: red)[2.062e+02]], [2.192e+02],
    [F22], [6.390e+03], [#text(fill: red)[1.707e+02]], [1.258e+04], [2.051e+03], [1.487e+03],
    [F23], [6.928e+02], [8.435e+02], [1.098e+03], [#text(fill: red)[4.227e+02]], [4.301e+02],
    [F24], [1.116e+03], [8.850e+02], [1.206e+03], [#text(fill: red)[4.912e+02]], [5.075e+02],
    [F25], [5.699e+02], [5.667e+02], [3.069e+03], [#text(fill: red)[4.807e+02]], [4.809e+02],
    [F26], [4.209e+03], [#text(fill: red)[5.745e+02]], [7.890e+03], [6.548e+02], [1.129e+03],
    [F27], [6.340e+02], [5.765e+02], [1.744e+03], [6.081e+02], [#text(fill: red)[5.113e+02]],
    [F28], [5.187e+02], [5.013e+02], [3.468e+03], [4.700e+02], [#text(fill: red)[4.598e+02]],
    [F29], [1.523e+03], [2.218e+03], [3.566e+03], [6.690e+02], [#text(fill: red)[3.629e+02]],
    [F30], [1.079e+06], [2.386e+06], [3.374e+08], [6.460e+06], [#text(fill: red)[6.011e+05]],
    [*Best*], [*0*], [*2*], [*0*], [*12*], [*18*],
  )],
  caption: [Resultados CEC2017 dimensión 50 (Medias)],
)

#figure(
  text(size: 10pt)[#table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 31 { (bottom: 0.8pt) } else { none },
    inset: 6pt,

    [*Función*],
    [*ABC*],
    [*DE*],
    [*PSO*],
    [*RB-IPOP-CMA-ES*],
    [*jSO*],

    [F01], [8.187e+04], [1.030e+10], [1.097e+11], [2.759e-07], [#text(fill: red)[0.000e+00]],
    [F02], [1.875e+76], [7.019e+135], [1.000e+00], [#text(fill: red)[0.000e+00]], [8.940e+00],
    [F03], [6.030e+05], [2.283e+05], [2.920e+05], [#text(fill: red)[0.000e+00]], [2.391e-06],
    [F04], [2.724e+02], [1.492e+03], [2.257e+04], [#text(fill: red)[1.560e+02]], [1.896e+02],
    [F05], [8.017e+02], [9.967e+02], [1.165e+03], [#text(fill: red)[1.100e+01]], [4.391e+01],
    [F06], [1.208e-03], [3.598e+01], [7.538e+01], [#text(fill: red)[1.331e-07]], [2.024e-04],
    [F07], [9.114e+02], [1.185e+03], [2.369e+03], [#text(fill: red)[1.265e+02]], [1.449e+02],
    [F08], [8.093e+02], [9.973e+02], [1.228e+03], [#text(fill: red)[1.220e+01]], [4.215e+01],
    [F09], [5.497e+04], [3.090e+04], [5.577e+04], [#text(fill: red)[0.000e+00]], [4.590e-02],
    [F10], [1.313e+04], [2.600e+04], [2.966e+04], [#text(fill: red)[5.586e+03]], [9.704e+03],
    [F11], [9.957e+04], [2.782e+04], [6.682e+04], [1.047e+03], [#text(fill: red)[1.132e+02]],
    [F12], [4.234e+07], [4.396e+09], [3.663e+10], [5.790e+04], [#text(fill: red)[1.843e+04]],
    [F13], [5.781e+04], [3.037e+07], [3.724e+09], [4.511e+03], [#text(fill: red)[1.449e+02]],
    [F14], [1.517e+07], [2.134e+05], [1.884e+07], [5.785e+02], [#text(fill: red)[6.434e+01]],
    [F15], [3.469e+04], [7.510e+05], [9.414e+07], [7.564e+02], [#text(fill: red)[1.621e+02]],
    [F16], [3.777e+03], [8.579e+03], [8.692e+03], [#text(fill: red)[1.695e+03]], [1.859e+03],
    [F17], [3.367e+03], [5.528e+03], [5.963e+03], [1.380e+03], [#text(fill: red)[1.278e+03]],
    [F18], [1.514e+07], [4.452e+06], [1.157e+07], [3.160e+02], [#text(fill: red)[1.673e+02]],
    [F19], [5.774e+05], [4.166e+06], [5.261e+08], [5.937e+02], [#text(fill: red)[1.049e+02]],
    [F20], [3.347e+03], [3.941e+03], [4.932e+03], [1.905e+03], [#text(fill: red)[1.376e+03]],
    [F21], [9.582e+02], [1.259e+03], [1.680e+03], [#text(fill: red)[2.450e+02]], [2.638e+02],
    [F22], [1.373e+04], [#text(fill: red)[1.978e+03]], [3.152e+04], [6.324e+03], [1.021e+04],
    [F23], [9.729e+02], [1.554e+03], [2.368e+03], [5.840e+02], [#text(fill: red)[5.712e+02]],
    [F24], [1.535e+03], [1.898e+03], [3.456e+03], [#text(fill: red)[8.932e+02]], [9.022e+02],
    [F25], [7.015e+02], [2.281e+03], [9.639e+03], [#text(fill: red)[6.775e+02]], [7.361e+02],
    [F26], [1.097e+04], [1.355e+04], [2.540e+04], [#text(fill: red)[2.893e+03]], [3.267e+03],
    [F27], [8.112e+02], [1.112e+03], [3.206e+03], [6.729e+02], [#text(fill: red)[5.855e+02]],
    [F28], [6.403e+02], [1.694e+03], [1.266e+04], [#text(fill: red)[4.976e+02]], [5.268e+02],
    [F29], [3.798e+03], [6.958e+03], [1.073e+04], [2.028e+03], [#text(fill: red)[1.257e+03]],
    [F30], [6.374e+04], [8.920e+06], [3.022e+09], [1.191e+04], [#text(fill: red)[2.325e+03]],
    [*Best*], [*0*], [*1*], [*0*], [*15*], [*14*],
  )],
  caption: [Resultados CEC2017 dimensión 100 (Medias)],
)

Lo primero que llama la atención es la diferencia que hay entre los algoritmos especializados (*jSO* y *RB-IPOP-CMA-ES*) y el resto. El jSO domina claramente en dimensión 30 ganando en 20 de las 30 funciones, mientras que el CMA-ES se queda con 10 victorias, y a medida que sube la dimensión el CMA-ES va igualando e incluso superando al jSO (15 frente a 14 en dimensión 100). Esto tiene sentido ya que ambos son algoritmos del estado del arte muy afinados para este benchmark, y en las funciones unimodales y simples (F01, F03, F06, F09) consiguen errores prácticamente nulos, algo que el ABC no es capaz de alcanzar.

El ABC consigue 2 victorias en dimensión 30 (F04 con 7.786e+00 y F26 con 3.139e+02) pero pierde todas sus victorias en dimensión 50 y 100. Esto confirma el principal problema del algoritmo  su mala convergencia. Cuando el espacio crece, el número de evaluaciones no le da margen para refinar las soluciones y los errores se disparan, perdiendo cualquier opción de competir con los algoritmos especializados.

Podemos ver que si comparamos el ABC con el PSO y el DE vemos que el ABC se comporta mucho mejor que el PSO en prácticamente todas las funciones, ya que el PSO comete errores   muy grandes (F01 con 4.175e+09 o F12 con 3.586e+08) y no gana en ninguna función en ninguna dimensión. Si lo comparamos con el DE el ABC también gana en varias funciones (F16, F20, F29), aunque el DE le supera con claridad en las funciones (F13, F14, F15, F18, F19) donde el ABC se estanca y genera errores enormes (F18 con 7.763e+05 frente a 6.122e+01 del DE). Esto nos vuelve a indicar que el punto débil del ABC es el refinamiento local de las soluciones.

Por último cabe destacar el comportamiento raro de la función F02, donde el ABC alcanza valores que crecen de forma descontrolada con la dimensión (desde 6.747e+10 en dimensión 30 hasta 1.875e+76 en dimensión 100).Como hemos visto en gráficas de comparaciones anteriores esta función es muy inestable y hace que los algoritmos sean muy inestables.

== ABC-LS (Hibridación con Wet-Solis)

#figure(
  text(size: 10pt)[#table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 31 { (bottom: 0.8pt) } else { none },
    inset: 6pt,

    [*Función*],
    [*ABC-LS*],
    [*DE*],
    [*PSO*],
    [*RB-IPOP-CMA-ES*],
    [*jSO*],

    [F01], [1.105e+03], [4.909e+04], [4.175e+09], [3.150e-08], [#text(fill: red)[0.000e+00]],
    [F02], [9.573e+13], [1.309e+19], [1.000e+00], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F03], [1.350e+05], [3.481e+03], [5.453e+04], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F04], [6.934e+01], [8.430e+01], [1.183e+03], [#text(fill: red)[5.528e+01]], [5.867e+01],
    [F05], [1.324e+02], [2.015e+02], [2.170e+02], [#text(fill: red)[1.649e+00]], [8.557e+00],
    [F06], [1.766e-02], [6.320e+00], [3.694e+01], [1.208e-07], [#text(fill: red)[6.000e-09]],
    [F07], [1.195e+02], [2.334e+02], [3.596e+02], [#text(fill: red)[3.433e+01]], [3.893e+01],
    [F08], [1.243e+02], [1.894e+02], [1.745e+02], [#text(fill: red)[1.756e+00]], [9.092e+00],
    [F09], [2.761e+03], [6.530e+01], [2.842e+03], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F10], [3.155e+03], [3.764e+03], [6.938e+03], [#text(fill: red)[1.443e+03]], [1.528e+03],
    [F11], [6.318e+01], [7.958e+01], [1.207e+03], [4.107e+01], [#text(fill: red)[3.037e+00]],
    [F12], [2.451e+06], [3.258e+05], [3.586e+08], [1.093e+03], [#text(fill: red)[1.704e+02]],
    [F13], [4.668e+02], [1.536e+02], [4.508e+07], [1.187e+02], [#text(fill: red)[1.484e+01]],
    [F14], [8.978e+04], [7.100e+01], [3.059e+05], [9.083e+01], [#text(fill: red)[2.183e+01]],
    [F15], [2.837e+02], [6.256e+01], [2.737e+05], [2.177e+02], [#text(fill: red)[1.088e+00]],
    [F16], [8.451e+02], [1.319e+03], [1.568e+03], [5.016e+02], [#text(fill: red)[7.892e+01]],
    [F17], [2.544e+02], [4.809e+02], [4.725e+02], [1.323e+02], [#text(fill: red)[3.292e+01]],
    [F18], [2.723e+05], [6.122e+01], [2.170e+06], [1.601e+02], [#text(fill: red)[2.041e+01]],
    [F19], [1.067e+02], [3.572e+01], [1.260e+06], [1.146e+02], [#text(fill: red)[4.503e+00]],
    [F20], [1.850e+02], [2.751e+02], [4.621e+02], [2.965e+02], [#text(fill: red)[2.937e+01]],
    [F21], [2.930e+02], [3.255e+02], [4.113e+02], [#text(fill: red)[2.086e+02]], [2.093e+02],
    [F22], [1.147e+02], [1.002e+02], [1.027e+03], [6.716e+02], [#text(fill: red)[1.000e+02]],
    [F23], [4.823e+02], [5.346e+02], [6.404e+02], [#text(fill: red)[3.393e+02]], [3.507e+02],
    [F24], [6.547e+02], [6.059e+02], [7.095e+02], [#text(fill: red)[4.194e+02]], [4.265e+02],
    [F25], [3.891e+02], [3.870e+02], [6.864e+02], [#text(fill: red)[3.867e+02]], [#text(fill: red)[3.867e+02]],
    [F26], [#text(fill: red)[2.274e+02]], [4.038e+02], [3.369e+03], [3.939e+02], [9.202e+02],
    [F27], [5.266e+02], [#text(fill: red)[4.925e+02]], [8.068e+02], [5.118e+02], [4.974e+02],
    [F28], [4.154e+02], [3.943e+02], [1.105e+03], [3.089e+02], [#text(fill: red)[3.087e+02]],
    [F29], [7.550e+02], [1.025e+03], [1.409e+03], [4.933e+02], [#text(fill: red)[4.337e+02]],
    [F30], [2.317e+04], [3.657e+03], [1.359e+07], [2.844e+03], [#text(fill: red)[1.971e+03]],
    [*Best*], [*1*], [*1*], [*0*], [*11*], [*20*],
  )],
  caption: [Resultados ABC-LS CEC2017 dimensión 30 (Medias)],
)

#figure(
  text(size: 10pt)[#table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 31 { (bottom: 0.8pt) } else { none },
    inset: 6pt,

    [*Función*],
    [*ABC-LS*],
    [*DE*],
    [*PSO*],
    [*RB-IPOP-CMA-ES*],
    [*jSO*],

    [F01], [4.356e+02], [2.039e+08], [1.823e+10], [1.131e-07], [#text(fill: red)[0.000e+00]],
    [F02], [5.723e+26], [5.158e+52], [1.000e+00], [2.767e+05], [#text(fill: red)[0.000e+00]],
    [F03], [2.272e+05], [7.597e+04], [1.008e+05], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F04], [5.898e+01], [2.322e+02], [4.091e+03], [#text(fill: red)[2.959e+01]], [5.621e+01],
    [F05], [2.641e+02], [3.978e+02], [4.328e+02], [#text(fill: red)[2.790e+00]], [1.641e+01],
    [F06], [1.078e-02], [1.180e+01], [5.450e+01], [#text(fill: red)[1.632e-07]], [1.093e-06],
    [F07], [2.246e+02], [4.781e+02], [8.197e+02], [#text(fill: red)[5.663e+01]], [6.650e+01],
    [F08], [2.372e+02], [4.070e+02], [4.077e+02], [#text(fill: red)[2.575e+00]], [1.696e+01],
    [F09], [1.259e+04], [3.756e+03], [1.620e+04], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F10], [5.697e+03], [1.127e+04], [1.295e+04], [#text(fill: red)[1.726e+03]], [3.140e+03],
    [F11], [1.936e+02], [2.173e+02], [3.151e+03], [1.832e+02], [#text(fill: red)[2.794e+01]],
    [F12], [1.150e+07], [1.382e+08], [6.062e+09], [2.438e+06], [#text(fill: red)[1.681e+03]],
    [F13], [5.271e+02], [2.726e+04], [5.262e+08], [1.652e+03], [#text(fill: red)[3.060e+01]],
    [F14], [8.626e+05], [1.683e+02], [1.558e+06], [2.416e+02], [#text(fill: red)[2.496e+01]],
    [F15], [1.413e+02], [5.090e+02], [5.790e+06], [5.290e+02], [#text(fill: red)[2.386e+01]],
    [F16], [1.799e+03], [3.047e+03], [2.745e+03], [8.899e+02], [#text(fill: red)[4.505e+02]],
    [F17], [8.765e+02], [1.827e+03], [1.652e+03], [3.978e+02], [#text(fill: red)[2.829e+02]],
    [F18], [2.272e+06], [2.420e+04], [1.203e+07], [3.573e+02], [#text(fill: red)[2.428e+01]],
    [F19], [1.074e+02], [1.322e+02], [1.336e+07], [1.394e+02], [#text(fill: red)[1.414e+01]],
    [F20], [3.789e+02], [9.384e+02], [1.282e+03], [5.468e+02], [#text(fill: red)[1.401e+02]],
    [F21], [4.519e+02], [6.177e+02], [6.655e+02], [#text(fill: red)[2.062e+02]], [2.192e+02],
    [F22], [6.104e+03], [#text(fill: red)[1.707e+02]], [1.258e+04], [2.051e+03], [1.487e+03],
    [F23], [7.289e+02], [8.435e+02], [1.098e+03], [#text(fill: red)[4.227e+02]], [4.301e+02],
    [F24], [1.050e+03], [8.850e+02], [1.206e+03], [#text(fill: red)[4.912e+02]], [5.075e+02],
    [F25], [5.471e+02], [5.667e+02], [3.069e+03], [#text(fill: red)[4.807e+02]], [4.809e+02],
    [F26], [3.337e+03], [#text(fill: red)[5.745e+02]], [7.890e+03], [6.548e+02], [1.129e+03],
    [F27], [7.700e+02], [5.765e+02], [1.744e+03], [6.081e+02], [#text(fill: red)[5.113e+02]],
    [F28], [4.941e+02], [5.013e+02], [3.468e+03], [4.700e+02], [#text(fill: red)[4.598e+02]],
    [F29], [1.275e+03], [2.218e+03], [3.566e+03], [6.690e+02], [#text(fill: red)[3.629e+02]],
    [F30], [7.870e+05], [2.386e+06], [3.374e+08], [6.460e+06], [#text(fill: red)[6.011e+05]],
    [*Best*], [*0*], [*2*], [*0*], [*12*], [*18*],
  )],
  caption: [Resultados ABC-LS CEC2017 dimensión 50 (Medias)],
)

#figure(
  text(size: 10pt)[#table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 31 { (bottom: 0.8pt) } else { none },
    inset: 6pt,

    [*Función*],
    [*ABC-LS*],
    [*DE*],
    [*PSO*],
    [*RB-IPOP-CMA-ES*],
    [*jSO*],

    [F01], [4.221e+03], [1.030e+10], [1.097e+11], [2.759e-07], [#text(fill: red)[0.000e+00]],
    [F02], [3.610e+79], [7.019e+135], [1.000e+00], [#text(fill: red)[0.000e+00]], [8.940e+00],
    [F03], [5.706e+05], [2.283e+05], [2.920e+05], [#text(fill: red)[0.000e+00]], [2.391e-06],
    [F04], [2.750e+02], [1.492e+03], [2.257e+04], [#text(fill: red)[1.560e+02]], [1.896e+02],
    [F05], [8.097e+02], [9.967e+02], [1.165e+03], [#text(fill: red)[1.100e+01]], [4.391e+01],
    [F06], [1.296e-02], [3.598e+01], [7.538e+01], [#text(fill: red)[1.331e-07]], [2.024e-04],
    [F07], [7.095e+02], [1.185e+03], [2.369e+03], [#text(fill: red)[1.265e+02]], [1.449e+02],
    [F08], [7.469e+02], [9.973e+02], [1.228e+03], [#text(fill: red)[1.220e+01]], [4.215e+01],
    [F09], [4.194e+04], [3.090e+04], [5.577e+04], [#text(fill: red)[0.000e+00]], [4.590e-02],
    [F10], [1.175e+04], [2.600e+04], [2.966e+04], [#text(fill: red)[5.586e+03]], [9.704e+03],
    [F11], [1.604e+04], [2.782e+04], [6.682e+04], [1.047e+03], [#text(fill: red)[1.132e+02]],
    [F12], [4.738e+07], [4.396e+09], [3.663e+10], [5.790e+04], [#text(fill: red)[1.843e+04]],
    [F13], [2.157e+03], [3.037e+07], [3.724e+09], [4.511e+03], [#text(fill: red)[1.449e+02]],
    [F14], [2.755e+06], [2.134e+05], [1.884e+07], [5.785e+02], [#text(fill: red)[6.434e+01]],
    [F15], [5.210e+02], [7.510e+05], [9.414e+07], [7.564e+02], [#text(fill: red)[1.621e+02]],
    [F16], [3.894e+03], [8.579e+03], [8.692e+03], [#text(fill: red)[1.695e+03]], [1.859e+03],
    [F17], [2.814e+03], [5.528e+03], [5.963e+03], [1.380e+03], [#text(fill: red)[1.278e+03]],
    [F18], [2.667e+06], [4.452e+06], [1.157e+07], [3.160e+02], [#text(fill: red)[1.673e+02]],
    [F19], [4.278e+02], [4.166e+06], [5.261e+08], [5.937e+02], [#text(fill: red)[1.049e+02]],
    [F20], [2.242e+03], [3.941e+03], [4.932e+03], [1.905e+03], [#text(fill: red)[1.376e+03]],
    [F21], [8.571e+02], [1.259e+03], [1.680e+03], [#text(fill: red)[2.450e+02]], [2.638e+02],
    [F22], [1.435e+04], [#text(fill: red)[1.978e+03]], [3.152e+04], [6.324e+03], [1.021e+04],
    [F23], [9.217e+02], [1.554e+03], [2.368e+03], [5.840e+02], [#text(fill: red)[5.712e+02]],
    [F24], [1.488e+03], [1.898e+03], [3.456e+03], [#text(fill: red)[8.932e+02]], [9.022e+02],
    [F25], [7.071e+02], [2.281e+03], [9.639e+03], [#text(fill: red)[6.775e+02]], [7.361e+02],
    [F26], [8.860e+03], [1.355e+04], [2.540e+04], [#text(fill: red)[2.893e+03]], [3.267e+03],
    [F27], [7.977e+02], [1.112e+03], [3.206e+03], [6.729e+02], [#text(fill: red)[5.855e+02]],
    [F28], [7.309e+02], [1.694e+03], [1.266e+04], [#text(fill: red)[4.976e+02]], [5.268e+02],
    [F29], [4.055e+03], [6.958e+03], [1.073e+04], [2.028e+03], [#text(fill: red)[1.257e+03]],
    [F30], [3.291e+04], [8.920e+06], [3.022e+09], [1.191e+04], [#text(fill: red)[2.325e+03]],
    [*Best*], [*0*], [*1*], [*0*], [*15*], [*14*],
  )],
  caption: [Resultados ABC-LS CEC2017 dimensión 100 (Medias)],
)

El efecto de hibridar el ABC con la búsqueda local de Solis-Wets se nota de forma muy clara en las funciones donde el ABC original se estancaba. Si comparamos las tablas con las del ABC base, vemos que las funciones híbridas mejoran mucho el comportamiento: en dimensión 30 la F13 pasa de 1.537e+05 a 4.668e+02, la F15 de 2.336e+04 a 2.837e+02, la F19 de 8.593e+03 a 1.067e+02 y la F11 de 7.757e+02 a 6.318e+01. Esta mejora se mantiene en las dimensiones altas . Esto confirma que la búsqueda local ayuda a mejorar el punto débil del ABC, que es el refinamiento de las soluciones, intensificando la explotación en la dirección correcta en lugar de hacerlo de forma aleatoria.

Sin embargo, aunque hay una reducción del error el número de victorias apenas cambia (1 en dimensión 30 y 0 en las demás), ya que los algoritmos  jSO y RB-IPOP-CMA-ES siguen estando por delante, sobre todo en las funciones  donde alcanzan errores cercanos a cero. Lo que sí consigue el ABC-LS es separarse  del DE y del PSO, superándolos en la gran mayoría de funciones. También se observa que en algún caso puntual la búsqueda local no aporta nada e incluso empeora  (la F04 en dimensión 30 pasa de 7.786e+00 a 6.934e+01, perdiendo la victoria que tenía el ABC), pero son casos aislados frente a la mejora global.

En resumen, el ABC-LS no llega a competir con los mejores algoritmos , pero recorta muchísimo la distancia que había, demostrando que el problema del ABC no era tanto la exploración del espacio como la falta de un mecanismo de explotación más exacto.

== ABC-GBESTDL (usando GBEST y limit dinámico)

#figure(
  text(size: 10pt)[#table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 31 { (bottom: 0.8pt) } else { none },
    inset: 6pt,

    [*Función*],
    [*ABC-GBESTDL*],
    [*DE*],
    [*PSO*],
    [*RB-IPOP-CMA-ES*],
    [*jSO*],

    [F01], [1.102e+04], [4.909e+04], [4.175e+09], [3.150e-08], [#text(fill: red)[0.000e+00]],
    [F02], [5.391e+12], [1.309e+19], [1.000e+00], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F03], [1.418e+05], [3.481e+03], [5.453e+04], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F04], [7.181e+01], [8.430e+01], [1.183e+03], [#text(fill: red)[5.528e+01]], [5.867e+01],
    [F05], [8.523e+01], [2.015e+02], [2.170e+02], [#text(fill: red)[1.649e+00]], [8.557e+00],
    [F06], [2.104e-06], [6.320e+00], [3.694e+01], [1.208e-07], [#text(fill: red)[6.000e-09]],
    [F07], [7.502e+01], [2.334e+02], [3.596e+02], [#text(fill: red)[3.433e+01]], [3.893e+01],
    [F08], [6.975e+01], [1.894e+02], [1.745e+02], [#text(fill: red)[1.756e+00]], [9.092e+00],
    [F09], [6.128e+02], [6.530e+01], [2.842e+03], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F10], [2.989e+03], [3.764e+03], [6.938e+03], [#text(fill: red)[1.443e+03]], [1.528e+03],
    [F11], [4.612e+02], [7.958e+01], [1.207e+03], [4.107e+01], [#text(fill: red)[3.037e+00]],
    [F12], [1.193e+06], [3.258e+05], [3.586e+08], [1.093e+03], [#text(fill: red)[1.704e+02]],
    [F13], [9.089e+04], [1.536e+02], [4.508e+07], [1.187e+02], [#text(fill: red)[1.484e+01]],
    [F14], [4.661e+04], [7.100e+01], [3.059e+05], [9.083e+01], [#text(fill: red)[2.183e+01]],
    [F15], [2.842e+04], [6.256e+01], [2.737e+05], [2.177e+02], [#text(fill: red)[1.088e+00]],
    [F16], [7.885e+02], [1.319e+03], [1.568e+03], [5.016e+02], [#text(fill: red)[7.892e+01]],
    [F17], [2.356e+02], [4.809e+02], [4.725e+02], [1.323e+02], [#text(fill: red)[3.292e+01]],
    [F18], [4.019e+05], [6.122e+01], [2.170e+06], [1.601e+02], [#text(fill: red)[2.041e+01]],
    [F19], [5.498e+04], [3.572e+01], [1.260e+06], [1.146e+02], [#text(fill: red)[4.503e+00]],
    [F20], [4.198e+02], [2.751e+02], [4.621e+02], [2.965e+02], [#text(fill: red)[2.937e+01]],
    [F21], [2.727e+02], [3.255e+02], [4.113e+02], [#text(fill: red)[2.086e+02]], [2.093e+02],
    [F22], [1.002e+02], [1.002e+02], [1.027e+03], [6.716e+02], [#text(fill: red)[1.000e+02]],
    [F23], [4.285e+02], [5.346e+02], [6.404e+02], [#text(fill: red)[3.393e+02]], [3.507e+02],
    [F24], [5.811e+02], [6.059e+02], [7.095e+02], [#text(fill: red)[4.194e+02]], [4.265e+02],
    [F25], [3.872e+02], [3.870e+02], [6.864e+02], [#text(fill: red)[3.867e+02]], [#text(fill: red)[3.867e+02]],
    [F26], [#text(fill: red)[3.005e+02]], [4.038e+02], [3.369e+03], [3.939e+02], [9.202e+02],
    [F27], [5.214e+02], [#text(fill: red)[4.925e+02]], [8.068e+02], [5.118e+02], [4.974e+02],
    [F28], [4.193e+02], [3.943e+02], [1.105e+03], [3.089e+02], [#text(fill: red)[3.087e+02]],
    [F29], [6.657e+02], [1.025e+03], [1.409e+03], [4.933e+02], [#text(fill: red)[4.337e+02]],
    [F30], [1.800e+04], [3.657e+03], [1.359e+07], [2.844e+03], [#text(fill: red)[1.971e+03]],
    [*Best*], [*1*], [*1*], [*0*], [*11*], [*20*],
  )],
  caption: [Resultados ABC-GBESTDL CEC2017 dimensión 30 (Medias)],
)

#figure(
  text(size: 10pt)[#table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 31 { (bottom: 0.8pt) } else { none },
    inset: 6pt,

    [*Función*],
    [*ABC-GBESTDL*],
    [*DE*],
    [*PSO*],
    [*RB-IPOP-CMA-ES*],
    [*jSO*],

    [F01], [4.485e+05], [2.039e+08], [1.823e+10], [1.131e-07], [#text(fill: red)[0.000e+00]],
    [F02], [3.536e+23], [5.158e+52], [1.000e+00], [2.767e+05], [#text(fill: red)[0.000e+00]],
    [F03], [2.337e+05], [7.597e+04], [1.008e+05], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F04], [1.057e+02], [2.322e+02], [4.091e+03], [#text(fill: red)[2.959e+01]], [5.621e+01],
    [F05], [1.382e+02], [3.978e+02], [4.328e+02], [#text(fill: red)[2.790e+00]], [1.641e+01],
    [F06], [2.916e-06], [1.180e+01], [5.450e+01], [#text(fill: red)[1.632e-07]], [1.093e-06],
    [F07], [2.038e+02], [4.781e+02], [8.197e+02], [#text(fill: red)[5.663e+01]], [6.650e+01],
    [F08], [1.314e+02], [4.070e+02], [4.077e+02], [#text(fill: red)[2.575e+00]], [1.696e+01],
    [F09], [1.984e+03], [3.756e+03], [1.620e+04], [#text(fill: red)[0.000e+00]], [#text(fill: red)[0.000e+00]],
    [F10], [4.392e+03], [1.127e+04], [1.295e+04], [#text(fill: red)[1.726e+03]], [3.140e+03],
    [F11], [1.052e+03], [2.173e+02], [3.151e+03], [1.832e+02], [#text(fill: red)[2.794e+01]],
    [F12], [5.916e+06], [1.382e+08], [6.062e+09], [2.438e+06], [#text(fill: red)[1.681e+03]],
    [F13], [5.892e+04], [2.726e+04], [5.262e+08], [1.652e+03], [#text(fill: red)[3.060e+01]],
    [F14], [1.065e+06], [1.683e+02], [1.558e+06], [2.416e+02], [#text(fill: red)[2.496e+01]],
    [F15], [1.791e+04], [5.090e+02], [5.790e+06], [5.290e+02], [#text(fill: red)[2.386e+01]],
    [F16], [1.465e+03], [3.047e+03], [2.745e+03], [8.899e+02], [#text(fill: red)[4.505e+02]],
    [F17], [1.154e+03], [1.827e+03], [1.652e+03], [3.978e+02], [#text(fill: red)[2.829e+02]],
    [F18], [2.448e+06], [2.420e+04], [1.203e+07], [3.573e+02], [#text(fill: red)[2.428e+01]],
    [F19], [5.626e+04], [1.322e+02], [1.336e+07], [1.394e+02], [#text(fill: red)[1.414e+01]],
    [F20], [1.008e+03], [9.384e+02], [1.282e+03], [5.468e+02], [#text(fill: red)[1.401e+02]],
    [F21], [3.213e+02], [6.177e+02], [6.655e+02], [#text(fill: red)[2.062e+02]], [2.192e+02],
    [F22], [5.903e+03], [#text(fill: red)[1.707e+02]], [1.258e+04], [2.051e+03], [1.487e+03],
    [F23], [5.723e+02], [8.435e+02], [1.098e+03], [#text(fill: red)[4.227e+02]], [4.301e+02],
    [F24], [9.495e+02], [8.850e+02], [1.206e+03], [#text(fill: red)[4.912e+02]], [5.075e+02],
    [F25], [5.258e+02], [5.667e+02], [3.069e+03], [#text(fill: red)[4.807e+02]], [4.809e+02],
    [F26], [2.465e+03], [#text(fill: red)[5.745e+02]], [7.890e+03], [6.548e+02], [1.129e+03],
    [F27], [6.764e+02], [5.765e+02], [1.744e+03], [6.081e+02], [#text(fill: red)[5.113e+02]],
    [F28], [5.042e+02], [5.013e+02], [3.468e+03], [4.700e+02], [#text(fill: red)[4.598e+02]],
    [F29], [1.170e+03], [2.218e+03], [3.566e+03], [6.690e+02], [#text(fill: red)[3.629e+02]],
    [F30], [8.785e+05], [2.386e+06], [3.374e+08], [6.460e+06], [#text(fill: red)[6.011e+05]],
    [*Best*], [*0*], [*2*], [*0*], [*12*], [*18*],
  )],
  caption: [Resultados ABC-GBESTDL CEC2017 dimensión 50 (Medias)],
)

#figure(
  text(size: 10pt)[#table(
    columns: 6,
    align: center,
    stroke: (x, y) => if y == 0 or y == 31 { (bottom: 0.8pt) } else { none },
    inset: 6pt,

    [*Función*],
    [*ABC-GBESTDL*],
    [*DE*],
    [*PSO*],
    [*RB-IPOP-CMA-ES*],
    [*jSO*],

    [F01], [4.867e+05], [1.030e+10], [1.097e+11], [2.759e-07], [#text(fill: red)[0.000e+00]],
    [F02], [1.095e+72], [7.019e+135], [1.000e+00], [#text(fill: red)[0.000e+00]], [8.940e+00],
    [F03], [5.659e+05], [2.283e+05], [2.920e+05], [#text(fill: red)[0.000e+00]], [2.391e-06],
    [F04], [2.543e+02], [1.492e+03], [2.257e+04], [#text(fill: red)[1.560e+02]], [1.896e+02],
    [F05], [5.629e+02], [9.967e+02], [1.165e+03], [#text(fill: red)[1.100e+01]], [4.391e+01],
    [F06], [2.199e-05], [3.598e+01], [7.538e+01], [#text(fill: red)[1.331e-07]], [2.024e-04],
    [F07], [7.001e+02], [1.185e+03], [2.369e+03], [#text(fill: red)[1.265e+02]], [1.449e+02],
    [F08], [5.170e+02], [9.973e+02], [1.228e+03], [#text(fill: red)[1.220e+01]], [4.215e+01],
    [F09], [2.142e+04], [3.090e+04], [5.577e+04], [#text(fill: red)[0.000e+00]], [4.590e-02],
    [F10], [1.209e+04], [2.600e+04], [2.966e+04], [#text(fill: red)[5.586e+03]], [9.704e+03],
    [F11], [7.796e+04], [2.782e+04], [6.682e+04], [1.047e+03], [#text(fill: red)[1.132e+02]],
    [F12], [4.422e+07], [4.396e+09], [3.663e+10], [5.790e+04], [#text(fill: red)[1.843e+04]],
    [F13], [9.054e+04], [3.037e+07], [3.724e+09], [4.511e+03], [#text(fill: red)[1.449e+02]],
    [F14], [9.202e+06], [2.134e+05], [1.884e+07], [5.785e+02], [#text(fill: red)[6.434e+01]],
    [F15], [1.062e+05], [7.510e+05], [9.414e+07], [7.564e+02], [#text(fill: red)[1.621e+02]],
    [F16], [3.912e+03], [8.579e+03], [8.692e+03], [#text(fill: red)[1.695e+03]], [1.859e+03],
    [F17], [3.292e+03], [5.528e+03], [5.963e+03], [1.380e+03], [#text(fill: red)[1.278e+03]],
    [F18], [6.422e+06], [4.452e+06], [1.157e+07], [3.160e+02], [#text(fill: red)[1.673e+02]],
    [F19], [5.517e+04], [4.166e+06], [5.261e+08], [5.937e+02], [#text(fill: red)[1.049e+02]],
    [F20], [2.727e+03], [3.941e+03], [4.932e+03], [1.905e+03], [#text(fill: red)[1.376e+03]],
    [F21], [7.006e+02], [1.259e+03], [1.680e+03], [#text(fill: red)[2.450e+02]], [2.638e+02],
    [F22], [1.501e+04], [#text(fill: red)[1.978e+03]], [3.152e+04], [6.324e+03], [1.021e+04],
    [F23], [7.811e+02], [1.554e+03], [2.368e+03], [5.840e+02], [#text(fill: red)[5.712e+02]],
    [F24], [1.284e+03], [1.898e+03], [3.456e+03], [#text(fill: red)[8.932e+02]], [9.022e+02],
    [F25], [7.783e+02], [2.281e+03], [9.639e+03], [#text(fill: red)[6.775e+02]], [7.361e+02],
    [F26], [8.379e+03], [1.355e+04], [2.540e+04], [#text(fill: red)[2.893e+03]], [3.267e+03],
    [F27], [7.333e+02], [1.112e+03], [3.206e+03], [6.729e+02], [#text(fill: red)[5.855e+02]],
    [F28], [6.540e+02], [1.694e+03], [1.266e+04], [#text(fill: red)[4.976e+02]], [5.268e+02],
    [F29], [4.149e+03], [6.958e+03], [1.073e+04], [2.028e+03], [#text(fill: red)[1.257e+03]],
    [F30], [3.808e+04], [8.920e+06], [3.022e+09], [1.191e+04], [#text(fill: red)[2.325e+03]],
    [*Best*], [*0*], [*1*], [*0*], [*15*], [*14*],
  )],
  caption: [Resultados ABC-GBESTDL CEC2017 dimensión 100 (Medias)],
)

El ABC-GBESTDL, al guiar la búsqueda hacia la mejor solución global, acelera la convergencia y esto se traduce en una clara mejora sobre las funciones. Comparando con el ABC base en dimensión 30, la F05 baja de 1.156e+02 a 8.523e+01, la F07 de 1.146e+02 a 7.502e+01, la F08 de 1.148e+02 a 6.975e+01, la F09 de 2.578e+03 a 6.128e+02 y la F06 de 1.717e-03 a 2.104e-06. Incluso en la F22 alcanza un 1.002e+02, prácticamente igualando al DE y quedándose muy cerca del jSO (1.000e+02). Esta tendencia se mantiene en dimensión 50 y 100 (la F08 en dimensión 100 baja de 8.093e+02 a 5.170e+02), lo que demuestra que la atracción hacia la mejor solución  junto al límite dinámico ayuda al algoritmo a converger más rápido sobre estas funciones.

En cambio, en otras funciones el ABC-GBESTDL no mejora tanto como lo hacía el ABC-LS. Por ejemplo, en la F13 en dimensión 30 el ABC-GBESTDL se queda en 9.089e+04 mientras que el ABC-LS conseguía 4.668e+02. Esto tiene sentido ya que el gbest acelera la convergencia global pero no aporta el refinamiento local fino que sí da la búsqueda local. Por tanto cada modificación destaca en un tipo de función distinto: el ABC-LS en las híbridas y el ABC-GBESTDL en las multimodales y simples.

En cuanto a victorias, el ABC-GBESTDL obtiene resultados muy parecidos al ABC-LS (1 victoria en dimensión 30 con la F26 y 0 en el resto), y al igual que él tampoco consigue superar al jSO ni al RB-IPOP-CMA-ES.

== Conclusión

Como conclusión final de la comparación, podemos ver que ninguna de las modificaciones del ABC llega a alcanzar a los algoritmos especializados del CEC2017, que están muy afinados para este benchmark y son muy buenos  en las funciones básicas. No obstante, tanto el ABC-LS como el ABC-GBESTDL logran convertir un algoritmo que perdía contra el DE en varias funciones en uno que lo supera de forma consistente y que reduce mucho la distancia con los mejores, cada uno reforzando una capacidad diferente del algoritmo: la explotación local en el caso del ABC-LS y la velocidad de convergencia en el del ABC-GBESTDL.


