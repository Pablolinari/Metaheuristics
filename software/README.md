# ESTRUCTURA

- carpeta src: 
 - pportfolio.cpp : implementación de la clase pportfolio.
 - randomsearch.cpp: implementación de la búsqueda aleatoria
 - localsearch.cpp: implementación de la búsqueda local
 - greedy.cpp: implementación de greedy
- carpeta include: 
 - Eigen: carpeta con la libreria eigen
 - pportfolio.h : cabezera de la clase pportfolio
 - randomsearch.h : cabezera de la clase randomsearch
 - localsearch.h : cabezera de la clase localsearch 
 - greedy.h : cabezera de la clase greedy 
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
