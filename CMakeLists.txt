cmake_minimum_required(VERSION 3.10)

project(mh_p1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")

find_package(Eigen3 REQUIRED)

add_executable(program main.cpp ${SOURCES})

target_include_directories(program PRIVATE "include")

target_link_libraries(program PRIVATE Eigen3::Eigen)

# --- COMANDO "make run" ---
add_custom_target(run
    COMMAND ./program
    DEPENDS program
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Ejecutando el programa..."
)
