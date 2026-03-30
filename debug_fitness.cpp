#include "include/pportfolio.h"
#include <eigen3/Eigen/Core>
#include <iostream>
#include <iomanip>

int main() {
    // Probar con ambos datasets
    std::cout << "=== PROBANDO CON DATOS 2024 (inc2025=false) ===" << std::endl;
    ProblemPort problem1 = ProblemPort("data/ibex_35.csv", 0.005, 0.08, false);
    
    // La solución del profesor
    tSolution<double> solution(30);
    solution << 0.0778231, 0.00810895, 0.0785671, 0.073502, 0.0768106, 
                0.00588811, 0.00562605, 0.0286347, 0.0767266, 0.00645015, 
                0.0796227, 0.0774488, 0.00593066, 0.00550847, 0.079162, 
                0.00756503, 0.0765881, 0.00562882, 0.00539433, 0.00672109, 
                0.00725571, 0.0797345, 0.0774675, 0.00808868, 0.00820176, 
                0.00547007, 0.00638785, 0.00525758, 0.00705178, 0.0073772;
    
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "Suma de pesos: " << solution.sum() << std::endl;
    std::cout << "¿Es válida?: " << std::boolalpha << problem1.isValid(solution) << std::endl;
    
    double profit1 = problem1.profit(solution);
    double risk1 = problem1.risk(solution);
    double fitness1 = problem1.fitness(solution);
    
    std::cout << "Beneficio: " << profit1 << std::endl;
    std::cout << "Riesgo: " << risk1 << std::endl;
    std::cout << "Fitness (lambda=500): " << fitness1 << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== PROBANDO CON DATOS 2025 (inc2025=true) ===" << std::endl;
    ProblemPort problem2 = ProblemPort("data/ibex_35.csv", 0.005, 0.08, true);
    
    double profit2 = problem2.profit(solution);
    double risk2 = problem2.risk(solution);
    double fitness2 = problem2.fitness(solution);
    
    std::cout << "Beneficio: " << profit2 << std::endl;
    std::cout << "Riesgo: " << risk2 << std::endl;
    std::cout << "Fitness (lambda=500): " << fitness2 << std::endl;
    
    std::cout << std::endl;
    std::cout << "=== RESULTADO ESPERADO DEL PROFESOR ===" << std::endl;
    std::cout << "Fitness del profesor: -4.789" << std::endl;
    std::cout << "Diferencia con 2024: " << std::abs(fitness1 - (-4.789)) << std::endl;
    std::cout << "Diferencia con 2025: " << std::abs(fitness2 - (-4.789)) << std::endl;
    
    return 0;
}
