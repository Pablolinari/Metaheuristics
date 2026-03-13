cmake_minimum_required(VERSION 3.10) # 4.0 es muy reciente, 3.10 es más estándar

project(mh_p1 CXX)

# Configurar el estándar de C++ (necesario para stod y Eigen)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar archivos fuentes
file(GLOB C_SOURCES "src/*.cpp")

# Incluir directorios (Asegúrate de tener Eigen instalado en el sistema)
include_directories("src")
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Crear el ejecutable
add_executable(program "main.cpp" ${C_SOURCES})

# --- ESTO CREA EL COMANDO "make run" ---
add_custom_target(run
    COMMAND ./program
    DEPENDS program
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Ejecutando el programa..."
)
