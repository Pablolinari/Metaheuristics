#include "include/pportfolio.h"
#include <eigen3/Eigen/Core>
#include <iostream>
#include <iomanip>

int main() {
    // Crear el problema - Probando con S&P 500
    std::cout << "=== Probando con S&P 500 ===" << std::endl;
    ProblemPort problem1 = ProblemPort("data/syp_500.csv", 0.005, 0.08, false);
    
    // La solución del profesor
    tSolution<double> solution(30);
    solution << 0.0778231, 0.00810895, 0.0785671, 0.073502, 0.0768106, 
                0.00588811, 0.00562605, 0.0286347, 0.0767266, 0.00645015, 
                0.0796227, 0.0774488, 0.00593066, 0.00550847, 0.079162, 
                0.00756503, 0.0765881, 0.00562882, 0.00539433, 0.00672109, 
                0.00725571, 0.0797345, 0.0774675, 0.00808868, 0.00820176, 
                0.00547007, 0.00638785, 0.00525758, 0.00705178, 0.0073772;
    
    // Calcular componentes
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "Suma de pesos: " << solution.sum() << std::endl;
    std::cout << "¿Es válida?: " << std::boolalpha << problem1.isValid(solution) << std::endl;
    std::cout << std::endl;
    
    double profit1 = problem1.profit(solution);
    double risk1 = problem1.risk(solution);
    double fitness1 = problem1.fitness(solution);
    
    std::cout << "Beneficio: " << profit1 << std::endl;
    std::cout << "Riesgo: " << risk1 << std::endl;
    std::cout << "Fitness (lambda=500): " << fitness1 << std::endl;
    std::cout << "  = " << profit1 << " - 500 * " << risk1 << std::endl;
    std::cout << "  = " << profit1 << " - " << (500 * risk1) << std::endl;
    std::cout << std::endl;
    
    // Probar con IBEX 35
    std::cout << "=== Probando con IBEX 35 ===" << std::endl;
    ProblemPort problem2 = ProblemPort("data/ibex_35.csv", 0.005, 0.08, false);
    
    double profit2 = problem2.profit(solution);
    double risk2 = problem2.risk(solution);
    double fitness2 = problem2.fitness(solution);
    
    std::cout << "Beneficio: " << profit2 << std::endl;
    std::cout << "Riesgo: " << risk2 << std::endl;
    std::cout << "Fitness (lambda=500): " << fitness2 << std::endl;
    
    return 0;
}
