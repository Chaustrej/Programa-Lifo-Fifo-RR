#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>   // Libreria para medir el tiempo de ejecucion
#include <iomanip>  // Libreria para dar formato a la tabla

using namespace std;
using namespace std::chrono;

// Estructura de Datos para las Actividades
struct Actividad {
    string id;
    int ti;       // Tiempo de llegada
    int t;        // Tiempo de duracion
    int tf;       // Tiempo final
    int T;        // Tiempo de estancia (tf - ti)
    long long E;  // Eficacia (T * t) -> long long para evitar desbordamiento
    double I;     // Indice de rendimiento (t / T)
};

// Funcion para cargar datos desde el CSV 
void cargarActividades(vector<Actividad> &lista) {
    ifstream archivo("data/Datos.csv");
    if (!archivo) {
        cerr << "Error: No se pudo abrir el archivo data/Datos.csv" << endl;
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
