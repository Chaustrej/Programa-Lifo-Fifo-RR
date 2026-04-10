#include <iostream> 
#include <vector>
#include <string>
#include <fstream> // Para manejo de archivos
#include <sstream> // Para manejo de archivos y cadenas
#include <algorithm> // Para std::min y std::max
#include <iomanip> // Para formatear la salida
#include <chrono> // Para medir tiempos
#include <queue> // Para std::queue

using namespace std;
using namespace std::chrono;

struct Actividad {
    string id;
    int ti, t, tf, T, t_restante;
    long long E; 
    double I;
};

// -FUNCIÓN DE CARGA-
void cargarDatos(string ruta, vector<Actividad> &base) {
    ifstream archivo(ruta);
    if (!archivo) { 
        cerr << "ERROR: No se pudo abrir " << ruta << endl; 
        exit(1); 
    }
    string linea;
    bool primera = true;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        stringstream ss(linea);
        string id_v, ti_v, t_v;
        if (getline(ss, id_v, ',') && getline(ss, ti_v, ',') && getline(ss, t_v)) {
            if (primera && (ti_v.find_first_not_of("0123456789") != string::npos)) {
                primera = false; continue;
            }
            base.push_back({id_v, stoi(ti_v), stoi(t_v), 0, 0, stoi(t_v), 0, 0});
        }
        primera = false;
    }
    archivo.close();
}

// - FUNCIÓN PARA PROCESAR Y ESCRIBIR RESULTADOS -
double procesarYGrabar(string nombre, vector<Actividad> &res, double micro, ofstream &salida) {
    int n = res.size();
    double sumT = 0, sumI = 0; long long sumE = 0;

    // Preparamos el encabezado para consola y archivo
    string header = "\n================= " + nombre + " =================\n";
    header += "ID | ti | t  | tf | T  |      E       |    I\n";
    header += "----------------------------------------------------------\n";
    
    cout << header;
    salida << header;

    for (auto &a : res) {
        a.T = a.tf - a.ti;
        a.E = (long long)a.T * a.t;
        a.I = (double)a.t / a.T;

        // Formateo de línea
        stringstream ss;
        ss << left << setw(6) << a.id << "| " << setw(3) << a.ti << "| " << setw(3) << a.t 
           << "| " << setw(3) << a.tf << "| " << setw(3) << a.T << "| " << setw(13) << a.E 
           << "| " << fixed << setprecision(4) << a.I << "\n";
        
        cout << ss.str();
        salida << ss.str();

        sumT += a.T; sumE += a.E; sumI += a.I;
    }

    double pT = sumT / n;
    string footer = "----------------------------------------------------------\n";
    footer += "PROMEDIOS: pT=" + to_string(pT) + " | pE=" + to_string(sumE/n) + " | pI=" + to_string(sumI/n) + "\n";
    footer += "TIEMPO DE CALCULO: " + to_string(micro) + " us\n";

    cout << footer;
    salida << footer;
    return pT;
}

// -ALGORITMOS-

double ejecutarFIFO(vector<Actividad> lista, ofstream &salida) {
    auto start = high_resolution_clock::now();
    int n = lista.size(), reloj = 0, hechos = 0;
    vector<bool> done(n, false);
    vector<Actividad> resultado;
    while (hechos < n) {
        int idx = -1;
        for (int i = 0; i < n; i++) {
            if (!done[i] && lista[i].ti <= reloj) {
                if (idx == -1 || lista[i].ti < lista[idx].ti) idx = i;
            }
        }
        if (idx == -1) { reloj++; continue; }
        reloj += lista[idx].t;
        lista[idx].tf = reloj;
        done[idx] = true;
        resultado.push_back(lista[idx]);
        hechos++;
    }
    return procesarYGrabar("FIFO", resultado, duration_cast<microseconds>(high_resolution_clock::now()-start).count(), salida);
}

double ejecutarLIFO(vector<Actividad> lista, ofstream &salida) {
    auto start = high_resolution_clock::now();
    int n = lista.size(), reloj = 0, hechos = 0;
    vector<bool> done(n, false);
    vector<Actividad> resultado;
    while (hechos < n) {
        int idx = -1;
        for (int i = 0; i < n; i++) {
            if (!done[i] && lista[i].ti <= reloj) {
                if (idx == -1 || lista[i].ti > lista[idx].ti) idx = i;
            }
        }
        if (idx == -1) { reloj++; continue; }
        reloj += lista[idx].t;
        lista[idx].tf = reloj;
        done[idx] = true;
        resultado.push_back(lista[idx]);
        hechos++;
    }
    return procesarYGrabar("LIFO", resultado, duration_cast<microseconds>(high_resolution_clock::now()-start).count(), salida);
}

double ejecutarRR(vector<Actividad> lista, int Q, ofstream &salida) {
    auto start = high_resolution_clock::now();
    int n = lista.size(), reloj = 0, hechos = 0;
    queue<int> cola_ready;
    vector<bool> en_cola(n, false), completado(n, false);
    auto revisar = [&]() {
        for (int i = 0; i < n; i++) {
            if (!completado[i] && !en_cola[i] && lista[i].ti <= reloj) {
                cola_ready.push(i); en_cola[i] = true;
            }
        }
    };
    revisar();
    while (hechos < n) {
        if (cola_ready.empty()) { reloj++; revisar(); continue; }
        int idx = cola_ready.front(); cola_ready.pop();
        int t_ejec = min(Q, lista[idx].t_restante);
        for(int i=0; i<t_ejec; i++) { reloj++; revisar(); }
        lista[idx].t_restante -= t_ejec;
        if (lista[idx].t_restante == 0) {
            lista[idx].tf = reloj; completado[idx] = true; hechos++;
        } else {
            revisar(); cola_ready.push(idx);
        }
    }
    return procesarYGrabar("ROUND ROBIN", lista, duration_cast<microseconds>(high_resolution_clock::now()-start).count(), salida);
}

int main() {
    vector<Actividad> base;
    cargarDatos("data/Datos.csv", base);

    // Abrimos el archivo de salida .txt
    ofstream archivoSalida("Resultados.txt");
    if (!archivoSalida) { cerr << "No se pudo crear el archivo de resultados." << endl; return 1; }

    double ptF = ejecutarFIFO(base, archivoSalida);
    double ptL = ejecutarLIFO(base, archivoSalida);
    double ptR = ejecutarRR(base, 4, archivoSalida);

    // Comparativa Final
    string finalComp = "\n************************************************************\n";
    finalComp += "   COMPARATIVA FINAL POR pT (Promedio de Eficacia)\n";
    finalComp += "************************************************************\n";
    finalComp += " - FIFO: " + to_string(ptF) + "\n - LIFO: " + to_string(ptL) + "\n - RR:   " + to_string(ptR) + "\n";
    
    double mejorVal = min({ptF, ptL, ptR});
    string ganador = (mejorVal == ptF) ? "FIFO" : (mejorVal == ptL) ? "LIFO" : "ROUND ROBIN";
    finalComp += "\n CONCLUSION: El algoritmo mas optimo es " + ganador + "\n";
    finalComp += "************************************************************\n";

    cout << finalComp;
    archivoSalida << finalComp;

    archivoSalida.close();
    cout << "Los resultados han sido guardados en 'Resultados.txt'" << endl;

    return 0;
}