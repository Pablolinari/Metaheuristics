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
En esta seccion veremos cuales han sido los algoritmos que se han implemetado para solucionar el problema y 
cual es la estructura que se ha llevado a cabo en la definicion del prblema y de los algoritmos. 



= Estructura de los algoritmos 
== Random Search
#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
```py
// Pseudocódigo del algoritmo Random Search
Función optimizar(problema , evaluaciones):a
  temporalSolution 
  bestSolution
  
  for (i < maxevals)
       temporalSolution = randomSolution
    if temporalSolution es mejor que bestSolution :
      bestSolution =temporalSolution

  return bestSolution

```]
== Greedy
#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
```py
// Pseudocódigo del algoritmo
Función optimizar(problema , evaluaciones):a
  temporalSolution 
  bestSolution
  
  for (i < maxevals)
       temporalSolution = randomSolution
    if temporalSolution es mejor que bestSolution :
      bestSolution =temporalSolution

  return bestSolution

```]
== Local Search
#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
```cpp
// Pseudocódigo del algoritmo
Función MaximizarRetorno(datos):
    Si datos es nulo:
        Retornar 0
    Para cada empresa en lista:
        Calcular peso ideal
    Retornar portafolio
```]


== Metodología
Aquí va tu explicación técnica detallada.

= Conclusión
El documento está listo para crecer con un estilo profesional y académico.

