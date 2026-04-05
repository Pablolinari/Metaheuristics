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

La mejor solución será aquella que consiga maximizar la función objetivo.

== Funciones comunes usadas por los algoritmos

En primer lugar la función que permite cargar los datos en nuestra clase Pportfolio es una función implementada en un archivo auxiliar ya que no tiene por qué pertenecer  necesariamente al problema que se plantea.Esta función lee datos de un fichero en formato CSV. Tras abrir el archivo la función descarta la primera linea con los nombre de las empresas y comienza a leer las siguientes lineas, una vez escogida una linea , se salta el string que contiene la fecha y comienza a insertar datos al vector usando como separador la coma.
El código comprueba los valores de la fecha leida , si el parametro only2025 es True , solo se leen datos de 2025 , de lo contrario se leen todos los datos hasta el último día del 2024. Una vez leidos todos los datos, la matriz obtenida se convierte a una matriz Eigen para poder manejarla con más facilidad.

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
 ```py
  funcion LoadCSV(archivo, only2025):
    matrix data
    leer(primera_linea)
    abrir archivo
    while(leer(linea_archivo)):
      if es fecha el primer string :
        continue
      if(es fecha el primer string and not only2025):
        solo leer valores de 2014-2024
      if(es fecha el primer string and only2025):
        solo leer valores de 2025
      vector valores

      while(leer(dato)):
        if(dato no vacio):
          valores.pushback(dato)
        else:
          valores.pushback(0.0)
        
      data.pushback(valores)

    cerrar archivo
    
    if(data no vacio):
      matriz_eigen = data
    return matriz_eigen
 ```]

Las principales funciones que se usan de la clase *Pportfolio* para implementar los algoritmos  son las siguientes : 

La función *createSolution* Se usa en dos algoritmos , en la Búsqueda Aleatoria para generar soluciones aleatorias y escoger finalmente la que mejor fitness tenga y en la Búsqueda Local para generar la solución desde la cual se comienza a buscar la mejor solución posible.


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
    return (solution * logdata.colwise.sum).sum #multiplicación componente a componente
 
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


 #block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
 ```py
 # Pseudocódigo del calculo de la heurística
  funcion heuristic(i): 
    datalogaritmo = (data.columna(i)+1).log().sum()
    mediacovarianza = matrizcovarianza.columna(i).sum()/ncompany
    return datalogaritmo - (lambda * mediacovarianza);
  

 ```]


El principal objetivo de los algoritmos es maximizar la función fitness , la cual trata de maximizar el beneficio y minimizar el riesgo en una sola fórmula.El parámetro lambda se usará a modo de ajuste , cuando más alto sea su valor el algoritmo valorará mejor soluciones con poco beneficio pero que sean seguras , es decir que tengan poco riesgo , mientras que cuanto más cercano a 0 sea lambda , el algoritmo valorará mejor las soluciones con máximo beneficio posible , ignorando el riesgo.


#pagebreak()
= Estructura de los algoritmos 
== Random Search

El algoritmo de Búsqueda aleatoria consiste en generar un número determinado de soluciones válidas de forma aleatoria y seleccionar la solución que maximice o minimice la función del problema, en este caso en concreto , se selecciona la solución que maximice el fitness del problema.

 #block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
 ```py
 # Pseudocódigo del algoritmo Random Search
 función RandomSearch(max_evals):
    mejorfitnes = -infinito
     for 0 hasta max_evals:
      soluciontemporal = solucion_aleatoria
      if fitnes_soluciontemporal > mejorfitness:
             solucion = mejorsolucion
            fitness =mejorfitness
    return (solucion,fitness,max_evals)
 ```]
Como se puede observar , el algoritmo de Búsqueda aleatoria dependerá de la suerte que tengamos a la hora de generar soluciones aleatorias ya que el espacio de búsqueda de la solución son todas las soluciones posibles a generar en el intervalo dado de número de evaluaciones por tanto no podemos esperar que obtenga una solución que sea mejor que la de otros algoritmos más avanzados , en caso de que esto ocurriese sería por una cuestión de suerte.

#pagebreak()
== Greedy

El algorítmo Greedy trata de construir una solución ordenando las empresas de mayor a menor según la heurística establecida en el guión . Una vez obtenidas las empresas ordenadas selecciona por orden descendente y construye una solución seleccionado en cada iteración la mejor opción disponible según la heuristica hasta completar el total de la inversión (llegar a suma 1).Como en cada iteración se ha seleccionado la mejor opción posible, el algoritmo intentará asignar el mayor porcentaje posible a esta empresa siempre que le sea posible , de lo contrario , le asignará el porcentaje restante para llegar al cien por cien .

La heurística usada en el problema es la siguiente :
$ h e u r(i) = (sum_t log(1 + mu_(i t))) - lambda dot (sum_j Sigma_(i j)) / (|N|) $



 #block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
 ```py
 # Pseudocódigo del calculo de la heurística
  funcion heuristic(i): 
    datalogaritmo = (data.columna(i)+1).log.sum
    mediacovarianza = matrizcovarianza.columna(i).sum/ncompany
    return datalogaritmo - (lambda * mediacovarianza);
  

 ```]
#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
```py
# Pseudocódigo del algoritmo Greedy
función greedy(maxevals):
  mejorescompanias = vector{0,1,2,....,ncompany-1} 
  ordenar por heuristica de mayor a menor -> mejorescompanias 
  mejorsolucion = vector{0,0...0}
  while mejorescompanias.sum<1:
    compania = mejorescompanias.front
    mejorescompanias = mejorescompanias.delete(compania)
    mejorsolucion[compania] = minimo entre high y (1-mejorsolucion.sum)
     
  return (mejorsolucion , fitness(mejorsolucion),1)
```]
Podemos ver que el algoritmo Greedy va seleccionando las mejores empresas según la heurística y les asigna el mayor porcentaje de la cartera posible , sin la opción de poder comprobar si verdaderamente esta empresa es la que mejor conviene a la cartera ya que el orden de la elección se hace mediante la heurística lo que hace que el algorítmo sea muy rápido pero no sea capaz de corregir en caso de que existiese una solución mejor.

Como último detalle vemos que por la naturaleza de los valores low y high propuestos en elguión la solución que genere el algoritmo Greedy siempre será valida ya que una vez que se rellenen todos los pesos posibles con el valor high , el valor restante de *1-sumatotal* será un valor mayor que low , por tanto podremos asignarlo a una empresa y los pesos de la solución generada por el algoritmo sumarán 1.


== Local Search

El algortimo de búsqueda local (el primero el mejor) parte de una solución generada aleatoriamente y genera un vecindario alrededor de esta solución , una vez generado el vecindario , se comienza a recorrer cada vecino  y se evalúa su fitness. Si se encuentra un fitness mejor que el de la solución actual ,se escoge ese vecino como mejor solución y se genera un nuevo vecindario alrededor de el.El proceso termina cuando se llega al máximo de evaluaciones permitidas o cuando no se ha encontrado ningún vecino del vecindario actual generado que mejore la solución.

#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
```py
# Pseudocódigo del algoritmo de búsqueda local
función LocalSearch(max_evals):
    double ratio
    solucioninicial = solucionAleatoria
    bestfitnes = fitness(solucionAleatoria)
  
    vecinos = Generar vector de todas las parejas (i,j) posibles con i,j en [0,ncompany-1]
    # excepto en el caso i=j;
    
    mezclar vecinos
    while evals < maxevals and haymejora:
      for v in vecinos:
        cantidadmover = ratio*solucioninicial[v.primero]
        
        solucioninicial[v.primero]-=cantidadmover
        solucioninicial[v.segundo]+=cantidadmover
        
        if es valida solucioninicial:
          evals = evals+1
          
          if fitness(solucioninicial) > mejorfitness:
            haymejora=true;
            mejorfitness=fitnes de la solucion inicial modificada
            #si hay una mejora salimos del bucle for 
            #ya que es una busqueda del primero el mejor
            salir del bucle for
        # si no cambiamos de solucion inicial , hay que
        # devolverla a su estado original
        devolver solucioninicial a su estado original
        if evals==maxevals:
          salir del bucle for
          #para no pasarnos de evaluaciones 
    return solucioninicial ,mejorfitness,evals
```
]
#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
```py
  #Generación del vecindario
  for i,j in ncompany:
    vectorparejas.insertar(pareja(i,j))

```]
#block(fill: luma(240), inset: 8pt, radius: 4pt, width: 100%)[
```py
  #Generación del vecindario
  for i,j in ncompany:
    vectorparejas.insertar(pareja(i,j))

```]
La implementación del algoritmo de Búsqueda local se divide en dos partes , primero se crea el vector vecinos , en este caso se usa un vector de parejas , donde cada vecino es la pareja (i,j) la cual nos indicará los índices que debemos modificar en el vector solución.
Una vez creado el vector , lo mezclamos aleatoriamente , y este será el que nos indique qué vecinos debemos de probar. Si no fuese una busqueda de el primero el mejor y se escogiese el mejor vecino no haría falta mezclar las parejas ya que el orden no importaría. 

Una vez disponemos del vecindario , debemos generar las nuevas soluciones a partir de las parejas que vamos escogiendo del vector , si tenemos la pareja (i,j) quitaremos un porcentaje del peso a la posición i y se la sumaremos a la posición j. Esto nos genera una nueva solución la cual comprobaremos si es valida y en caso de ser valida si supera el fitness de nuestra mejor solución hasta el momento , este nuevo vector solución con las nuevas modificaciones en los indices (i,j) se convertirá en nuestro nueva solución. Dejaremos de recorrer el vector de vecinos y de nuevo a recorrerlo desde el principio para ver si a partir de esta nueva solución generada , conseguimos una mejor.

Si la solución generada no es valida o no mejora la solución actual , restauraremos las modificaciones hechas al vector solución , de esta manera podemos usar un solo vector y modificar dos posiciones en vez de hacer copias de vectores completos.

Mientras que no se superen las restricciones de las evaluaciones y se sigan encontrando vecinos mejores en cada vecindario seguiremos intentando encontrar la mejor solución . En caso de recorrer todo el vecindario y no encontrar una solución mejor o que se superen las evaluciones permitidas , se detendrá la búsqueda.

== Estructura del código


= Conclusión
El documento está listo para crecer con un estilo profesional y académico.
