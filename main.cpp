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

// --- ESTRUCTURA DE DATOS ---
struct Actividad {
    string id;
    int ti, t, tf, T;
    long long E; 
    double I;
};

// --- CARGA DE DATOS (data/Datos.csv) ---
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
            // Salto inteligente de encabezado
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

// --- MÉTRICAS E IMPRESIÓN ---
// Ahora retorna el pT para la comparativa final
double obtenerResultados(string nombre, const vector<string> &ids, const vector<int> &ti, const vector<int> &t, const vector<int> &tf, double micro) {
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

    double pT = sumT / n;
    cout << string(70, '-') << "\n";
    cout << " PROMEDIOS: pT = " << pT << " | pE = " << (double)sumE/n << " | pI = " << sumI/n << "\n";
    cout << " TIEMPO DE CALCULO: " << micro << " microsegundos\n";
    return pT; // Retornamos pT para comparar
}

