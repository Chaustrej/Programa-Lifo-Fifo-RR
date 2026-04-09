#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>   // Para medir el tiempo (exigencia del proyecto)
#include <iomanip>  // Para dar formato a la tabla en consola

using namespace std;
using namespace std::chrono;

// 1. ESTRUCTURA DE DATOS: 
struct Actividad {
    string id;
    int ti;       // Tiempo de llegada (Input)
    int t;        // Tiempo de duración (Input)
    int tf;       // Tiempo final (Calculado)
    int T;        // Tiempo de Estancia (Calculado)
    long long E;  // Eficacia (T * t)
    double I;     // Índice de Rendimiento (t / T)
};

// 2. CARGA DE DATOS: Lee el archivo CSV línea por línea
void cargarActividades(vector<Actividad> &lista) {
    ifstream archivo("data/Datos.csv");
    if (!archivo) {
        cerr << "ERROR: No se encontro data/Datos.csv" << endl;
        exit(1);
    }
    string id, temp;
    while (getline(archivo, id, ',')) {
        Actividad a;
        a.id = id;
        getline(archivo, temp, ','); a.ti = stoi(temp);
        getline(archivo, temp);      a.t = stoi(temp);
        lista.push_back(a);
    }
    archivo.close();
}

