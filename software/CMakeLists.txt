cmake_minimum_required(VERSION 3.10)
project(mh_p1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Crear una opción clara para el usuario
# Si no se especifica, por defecto será Debug para que los errores de Eigen no pasen desapercibidos
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Elija el tipo de build: Debug o Release" FORCE)
endif()

# 2. Configurar Flags de optimización
# DEBUG: Sin optimizar (-O0) y con símbolos de depuración (-g)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall")

# RELEASE: Máxima optimización (-O3) y arquitectura nativa (-march=native)
# IMPORTANTE: Se añade -DNDEBUG para que Eigen desactive las comprobaciones de aserción y vuele.
set(CMAKE_CXX_FLAGS_RELEASE "-O3  -DNDEBUG")

message(STATUS "Configurando proyecto en modo: ${CMAKE_BUILD_TYPE}")

# 3. Archivos y Dependencias
file(GLOB SOURCES "src/*.cpp")
#find_package(Eigen3 REQUIRED)

add_executable(program main.cpp ${SOURCES})
target_include_directories(program PRIVATE "include")
##target_link_libraries(program PRIVATE Eigen3::Eigen)

# --- COMANDO "make run" ---
add_custom_target(run
    COMMAND ./program
    DEPENDS program
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Ejecutando el programa..."
)
