# ESTRUCTURA

- carpeta src: 
 - pportfolio.cpp : implementación de la clase pportfolio.
 - randomsearch.cpp: implementación de la búsqueda aleatoria
 - localsearch.cpp: implementación de la búsqueda local
 - greedy.cpp: implementación de greedy
 - agg.cpp: implementación del algoritmo genético generacional (AGG)
 - age.cpp: implementación del algoritmo genético estacionario (AGE)
 - am.cpp: implementación de la memética base (AM)
 - amall.cpp: implementación de AM con búsqueda local en toda la población
 - amrand.cpp: implementación de AM con búsqueda local sobre soluciones aleatorias
 - ambest.cpp: implementación de AM con búsqueda local sobre el 10% mejor
 - operator.cpp: implementación de operadores 
 - main.cpp: punto de entrada del programa
- carpeta include: 
 - Eigen: carpeta con la libreria eigen
 - pportfolio.h : cabezera de la clase pportfolio
 - randomsearch.h : cabezera de la clase randomsearch
 - localsearch.h : cabezera de la clase localsearch 
 - greedy.h : cabezera de la clase greedy 
 - agg.h : cabezera del AGG
 - age.h : cabezera del AGE
 - am.h : cabezera del AM
 - amall.h : cabezera del AM con busqueda local completa
 - amrand.h : cabezera del AM con busqueda local aleatoria
 - ambest.h : cabezera del AM con busqueda local en los mejores
 - aggarit.h : AGG con cruce aritmetico
 - aggblx.h : AGG con cruce BLX
 - agearit.h : AGE con cruce aritmetico
 - ageblx.h : AGE con cruce BLX
 - operator.h : cabecera de los operadores 
 - utils.hpp : implementación de la función loadCsv
 - random.hpp : archivo usado para todo lo relacionado con aleatoriedad
- carpeta data: archivos con datos en formato .csv dados en la práctica

- script.sh : script para facilitar la creacion del archivo make , recibe un parametro (debug o release)
 para configurar el modo de compilación , el resultado de las prácticas se ha hecho en modo release


# COMO EJECUTAR 


1. ejecutar <./script.sh release> esto ejecuta el archivo CMakeLists.txt en modo release con optimizaciones
2. se genera un ejecutable llamado program 
3. diferentes opciones para ejecutar : 
 <./program data/ibex\_35.csv 0.005 0.08 50 42 ibex35.csv>
 <./program data/syp\_500.csv 0.005 0.05 50 42 syp100.csv>
 <./program data/syp\_500.csv 0.005 0.02 50 42 syp500.csv>
 4. Salida por terminal de la media de las ejecuciones , y en el archivo csv todos los fitness de las ejecuciones
