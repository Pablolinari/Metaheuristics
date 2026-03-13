#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <sstream>

using namespace std;
int main() {
    try {
        // Asegúrate de que la ruta sea correcta respecto a donde ejecutas el binario
        Eigen::MatrixXd mt = loadCSV("data/ibex_35.csv", 2813, 30);
        
        cout << "Matriz cargada con éxito." << endl;
        cout << "Dimensiones: " << mt.rows() << "x" << mt.cols() << endl;
        // Imprimir una pequeña parte para verificar
        cout << "Primer elemento: " << mt(2,1) << endl;
				cout << mt.block(0,0,10,8)<<endl;

    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
