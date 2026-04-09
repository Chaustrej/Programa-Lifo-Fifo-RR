#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct Actividad {
    string id;
    int ti, t, tf, T;
    long long E; 
    double I;
};

// Carga de datos desde data/Datos.csv 
void cargarDatos(string ruta, vector<string> &ids, vector<int> &ti, vector<int> &t) {
    ifstream archivo(ruta);
    if (!archivo) {
        cerr << "Error: No se encuentra el archivo en " << ruta << endl;
        exit(1);
    }
    string linea;
    bool primeraLinea = true;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        stringstream ss(linea);
        string id_val, ti_val, t_val;
        if (getline(ss, id_val, ',') && getline(ss, ti_val, ',') && getline(ss, t_val)) {
            if (primeraLinea && (ti_val.find_first_not_of("0123456789") != string::npos)) {
                primeraLinea = false;
                continue;
            }
            ids.push_back(id_val);
            ti.push_back(stoi(ti_val));
            t.push_back(stoi(t_val));
        }
        primeraLinea = false;
    }
}

// Procesa métricas y retorna pE para la comparativa 
double obtenerPE(string nombre, const vector<string> &ids, const vector<int> &ti, const vector<int> &t, const vector<int> &tf, double micro) {
    int n = ids.size();
    double sumT = 0, sumI = 0;
    long long sumE = 0;

    cout << "\n" << string(70, '=') << "\n";
    cout << " ESTRATEGIA: " << nombre << "\n";
    cout << string(70, '=') << "\n";
    cout << left << setw(6) << "Proc" << " | " << setw(4) << "ti" << " | " << setw(4) << "t" 
         << " | " << setw(4) << "tf" << " | " << setw(4) << "T" << " | " << setw(14) << "E" << " | I" << endl;
    cout << string(70, '-') << "\n";

    for (int i = 0; i < n; i++) {
        int T_val = tf[i] - ti[i];                
        long long E_val = (long long)T_val * t[i]; 
        double I_val = (T_val != 0) ? (double)t[i] / T_val : 0;

        cout << left << setw(6) << ids[i] << " | " << setw(4) << ti[i] << " | " << setw(4) << t[i] 
             << " | " << setw(4) << tf[i] << " | " << setw(4) << T_val << " | " << setw(14) << E_val 
             << " | " << fixed << setprecision(4) << I_val << endl;
        
        sumT += T_val; sumE += E_val; sumI += I_val;
    }

    double pE = (double)sumE / n;
    cout << string(70, '-') << "\n";
    cout << " PROMEDIOS: pT = " << sumT/n << " | pE = " << pE << " | pI = " << sumI/n << "\n";
    cout << " TIEMPO DE CALCULO: " << micro << " microsegundos\n";
    return pE; 
}

// Algoritmo FIFO
vector<int> ejecutarFIFO(const vector<int> &ti, const vector<int> &t) {
    int n = ti.size();
    vector<int> tf(n, 0);
    vector<bool> done(n, false);
    int t_clock = 0, completados = 0;
    while (completados < n) {
        bool encontrado = false;
        for (int i = 0; i < n; i++) {
            if (!done[i] && ti[i] <= t_clock) {
                t_clock += t[i];
                tf[i] = t_clock;
                done[i] = true;
                completados++;
                encontrado = true;
                break; 
            }
        }
        if (!encontrado) t_clock++; 
    }
    return tf;
}
// Algoritmo LIFO

vector<int> ejecutarLIFO(const vector<int> &ti, const vector<int> &t) {
    int n = ti.size();
    vector<int> tf(n, 0);
    vector<bool> done(n, false);
    int t_clock = 0, completados = 0;
    while (completados < n) {
        bool encontrado = false;
        for (int i = n - 1; i >= 0; i--) { 
            if (!done[i] && ti[i] <= t_clock) {
                t_clock += t[i];
                tf[i] = t_clock;
                done[i] = true;
                completados++;
                encontrado = true;
                break;
            }
        }
        if (!encontrado) t_clock++;
    }
    return tf;
}
// Algoritmo Round robin con quantum q = 4

vector<int> ejecutarRR(const vector<int> &ti, const vector<int> &t, int q) {
    int n = ti.size();
    vector<int> tf(n, 0), rem = t;
    vector<bool> done(n, false);
    int t_clock = 0, completados = 0;
    while (completados < n) {
        bool encontrado_ciclo = false;
        for (int i = 0; i < n; i++) {
            if (!done[i] && ti[i] <= t_clock) {
                int run_time = (rem[i] < q) ? rem[i] : q; 
                t_clock += run_time;
                rem[i] -= run_time;
                encontrado_ciclo = true;
                if (rem[i] == 0) {
                    done[i] = true;
                    tf[i] = t_clock;
                    completados++;
                }
            }
        }
        if (!encontrado_ciclo) t_clock++;
    }
    return tf;
}