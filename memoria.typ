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
  hyphenate: true // Activa la partición de palabras (silabeo)
)

// Justificar el texto (típico de LaTeX)
#set par(
  justify: true,
  leading: 0.65em,     // Espacio entre líneas
  first-line-indent: 1.5em // Indentación de la primera línea
)

// Regla para que el primer párrafo tras un título SÍ tenga sangría
// (En LaTeX estándar el primero no tiene, pero muchos lo prefieren así)
#show heading: it =>  {
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

  #block(text(weight: "bold", 2.5em)[Práctica 1])
  #v(0.5em)
  #block(text(weight: "bold", 1.8em)[Problema del portfolio])
  #v(1cm)
  #block(text(weight: "bold", 1.5em)[Pablo Linari Pérez])

  #v(3cm)

  #text(size: 1.2em)[
    #underline[DNI:] 75571079W \
    #underline[Grupo de prácticas]: 3 (Martes 15:30-17:30) \ 
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

= Descripción del problema

El problema que se ha escogido es el problema del portfolio, en el cual se nos da una lista de empresas de las cuales sabemos su retorno obtenido diario desde principios de 2015 hasta finales de 2025. Con estos datos necesitamos elaborar una solución indicando qué porcentaje de nuestra cartera deberíamos haber invertido en cada empresa para maximizar el beneficio de las inversiones. 

En este contexto, el problema no solo busca el máximo retorno, sino que también suele considera la correlaicion 
entre activos teniendo esto en cuenta en las ecuaciones que se usan para maximizar el beneficio. 

De esta manera podemos ajustar el codigo ajustando el parametro $lambda$ de la ecuacion a maximizar para asi tener mayor o menor aversion al riesgo.

#pagebreak()
= Algoritmos empleados y estructura del problema

== Estructura del problema 
Para modelar el problema del portfolio se ha usado la clase Pportfolio la cual es hija de la clase Problem que viene ya predefinnida con varios métodos virtuales , los cuales se implementan en la clase hija. Para representar el problema usando estas clases se tienen en cuenta los siguientes atributos:

- *days* : número entero de días de los que se tienen datos. 
- *ncompany* : número entero  de empresas de las que se tienen datos.
- *low* : mínimo porcentaje de inversión en una empresa (double).
- *high* : máximo porcentaje de inversión en una empresa (double).
- *lambda* : parámetro entero  usado en el fitness.
- *data* : matriz de la libreria Eigen con valores double que contienen los beneficios diarios de cada empresa, en las columnas se representan las empresas y en las filas los dias.
- *covarianceMat* : matriz de la libreria Eigen con valores double que contiene la  covarianza de las empresas.
- *ERROR* : Error permitido en la suma total del vector solución (double).

Podemos observar que se ha optado por usar la libreria Eigen de C++ para poder manejar las operaciones de matrices y vectores como la suma total , la media total , la media por columnas o por filas , multiplicación escalar a escalar, etc... . Esto hace que el código sea más compacto y fácil de seguir a la vez que más preciso y cómodo de utilizar. 

== Representación de las soluciones

Para este problema las soluciones se representan por vectores de la biblioteca Eigen con tipo de dato double donde cada posición del vector representa la i-ésima empresa del fichero de datos de entrada , todos estos vectores deben cumplir las siguientes restricciones: 

- El tamaño de la solución debe ser el mismo que el número de empresas en los datos de entrada. 
- La suma de todos sus valores debe ser igual a 1.
- Cada posición del vector tiene que tener un valor entre *low* y *high*
- Puede haber posiciones del vector con valor 0(representando que en esa empresa no se invierte).

== Funciones comunes usadas por los algoritmos

En primer lugar la función que permite cargar los datos en nuestra clase Pportfolio es una función implementada en un archivo auxiliar ya que no debe pertenecer  necesariamente al problema ---revisar

Las principales funciones que se usan de la clase *Pportfolio* para implementar los algoritmos  son las siguientes : 

La función *createSolution* Se usa en dos algoritmos , en la Búsqueda Aleatoria para generar soluciones aleatorias y escoger finalmente la que mejor fitness tenga y en la Búsqueda Local para generar la solución desde la cual se comienza a buscar la mejor solución posible.
#pagebreak()

 #block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
 ```py
  funcion createSolution():
    vector solucion;

    for i<solucion.size : 
      solucion(i) = Aleatorio entre high y low
    

    numeroceros = Aleatorio entre 1 y solucion.size-1
    vectorindices = {0,1,2, ......,solucion.size-1}
    desordenar(vectorindices)
     ## Seleccionamos valores aleatorios que ponemos a 0
    for i<solucion.size:
      solucion(vectorindices[i])=0
    
    if la solucion no es valida
      fix(solucion)
    
    return solution;
 ```]

 #block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
 ```py
 # Pseudocódigo del calculo del beneficio
  funcion profit(solution): 
    
    logdata = log((data+1)) # la operación se aplica a todos las componentes
    return (solution * logdata.colwise.sum).sum; #multiplicación componente a componente
 
 ```]
 #block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
 ```py
 # Pseudocódigo del calculo driesgo
  funcion risk(solution): 
    return (solution.transpose * covarianceMat * solution);
 #devuelve un número resultado de las dimensiones  1xn * nxn *nx1
 ```]

 #block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
 ```py
 # Pseudocódigo del calculo del fitness
  funcion fitness(solution): 
    return (profit - lambda*risk);
 #devuelve un número resultado de las dimensiones  1xn * nxn *nx1
 ```]

El principal objetivo de los algoritmos es maximizar la función fitness , la cual trata de maximizar el beneficio y minimizar el riesgo en una sola fórmula.



= Estructura de los algoritmos 
== Random Search
Explora el espacio de soluciones de forma uniforme para obtener un punto de partida robusto sin asumir estructura. Cada candidato se genera al azar, se normaliza y se acepta como mejor si supera el valor actual.
 #block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
 ```py
 # Pseudocódigo del algoritmo Random Search
 función RandomSearch(max_evals):
     mejor = solucion_aleatoria_factible()
     repetir max_evals veces:
         candidata = solucion_aleatoria_factible()
         si f(candidata) > f(mejor):
             mejor = candidata
     devolver mejor
 ```]

== Greedy
Construye la cartera de forma incremental asignando peso primero a los activos con mejor cociente retorno-riesgo estimado. Se usa un tamaño de paso fijo para repartir el presupuesto y se renormaliza al final para cumplir las restricciones.
#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
```py
# Pseudocódigo del algoritmo Greedy
función Greedy(paso):
    pesos = vector_ceros()
    presupuesto = 1.0
    orden = activos_ordenados_por_ratio(mu, varianza)
    para activo en orden mientras presupuesto > 0:
        asignacion = min(paso, presupuesto)
        pesos[activo] += asignacion
        presupuesto -= asignacion
    normalizar(pesos)
    devolver pesos
```]

== Local Search
Parte de una solución inicial (Greedy o Random Search) y mejora localmente moviendo pequeñas cantidades de peso entre dos activos. Solo se acepta el movimiento si aumenta el fitness; el proceso se detiene cuando no hay mejoras o se agota el presupuesto de evaluaciones.
#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
```py
# Pseudocódigo del algoritmo de búsqueda local
función LocalSearch(sol_inicial, max_evals, delta):
    actual = sol_inicial
    mejor = sol_inicial
    evals = 0
    mientras evals < max_evals:
        i, j = elegir_dos_activos()
        candidata = mover_peso(actual, i, j, delta)
        evals += 1
        si candidata es factible y f(candidata) > f(mejor):
            mejor = candidata
            actual = candidata
        si no hay mejora en varias iteraciones:
            romper
    devolver mejor
```
]

== Metodología
- Preparación de datos: cálculo de $mu$ y $Sigma$ con retornos diarios, limpieza de valores faltantes y normalización temporal para evitar sesgos por escalas.
- Configuración experimental: mismo número máximo de evaluaciones por heurística, $lambda$ fijado tras pruebas preliminares y tamaño de paso `paso` o `delta` calibrados para equilibrar exploración y rapidez.
- Métricas de calidad: valor de la función objetivo, retorno medio anualizado y volatilidad anualizada; se revisa además el reparto de pesos para asegurar diversificación.
- Validación: división temporal (train 2015-2023, validación 2024, test 2025) para comprobar si las carteras generalizan y no sobreajustan al histórico.

= Conclusión
El documento está listo para crecer con un estilo profesional y académico.
