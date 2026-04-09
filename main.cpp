#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <queue>
#include <stack>

using namespace std;
using namespace std::chrono;

struct Actividad {
    string id;
    int ti, t, tf, T, t_restante;
    long long E; 
    double I;
};

// --- CARGA DE DATOS ---
void cargarDatos(string ruta, vector<Actividad> &base) {
    ifstream archivo(ruta);
    if (!archivo) { cerr << "Error: No existe data/Datos.csv" << endl; exit(1); }
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
}

// --- MÉTRICAS ---
double procesarMétricas(string nombre, vector<Actividad> &res, double micro) {
    int n = res.size();
    double sumT = 0, sumI = 0; long long sumE = 0;
    cout << "\n================= " << nombre << " (TEORIA REAL) =================\n";
    cout << left << setw(6) << "Proc" << "| ti | t | tf | T |" << setw(14) << " E" << " | I" << endl;
    cout << string(70, '-') << endl;
    
    for (auto &a : res) {
        a.T = a.tf - a.ti;
        a.E = (long long)a.T * a.t;
        a.I = (double)a.t / a.T;
        cout << left << setw(6) << a.id << "| " << setw(3) << a.ti << "| " << setw(2) << a.t << "| " << setw(3) << a.tf << "| " << setw(3) << a.T << "| " << setw(13) << a.E << "| " << fixed << setprecision(4) << a.I << endl;
        sumT += a.T; sumE += a.E; sumI += a.I;
    }
    double pE = (double)sumE / n;
    cout << "----------------------------------------------------------\n";
    cout << "PROMEDIOS: pT=" << fixed << setprecision(2) << sumT/n << " | pE=" << pE << " | pI=" << sumI/n << endl;
    cout << "TIEMPO DE CALCULO: " << micro << " us" << endl;
    return pE;
}

// --- FIFO REAL (First-In, First-Out) ---
double fifoReal(vector<Actividad> lista) {
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
    return procesarMétricas("FIFO", resultado, duration_cast<microseconds>(high_resolution_clock::now()-start).count());
}

// --- LIFO REAL (Last-In, First-Out) ---
double lifoReal(vector<Actividad> lista) {
    auto start = high_resolution_clock::now();
    int n = lista.size(), reloj = 0, hechos = 0;
    vector<bool> done(n, false);
    vector<Actividad> resultado;

    while (hechos < n) {
        int idx = -1;
        for (int i = 0; i < n; i++) {
            if (!done[i] && lista[i].ti <= reloj) {
                // Elige al que llegó de ÚLTIMO entre los disponibles
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
    return procesarMétricas("LIFO", resultado, duration_cast<microseconds>(high_resolution_clock::now()-start).count());
}

// --- ROUND ROBIN REAL (Quantum con Cola de Listos) ---
double rrReal(vector<Actividad> lista, int Q) {
    auto start = high_resolution_clock::now();
    int n = lista.size(), reloj = 0, hechos = 0;
    queue<int> cola_ready;
    vector<bool> en_cola(n, false), completado(n, false);

    auto revisar_llegadas = [&]() {
        for (int i = 0; i < n; i++) {
            if (!completado[i] && !en_cola[i] && lista[i].ti <= reloj) {
                cola_ready.push(i);
                en_cola[i] = true;
            }
        }
    };

    revisar_llegadas();
    while (hechos < n) {
        if (cola_ready.empty()) { reloj++; revisar_llegadas(); continue; }

        int idx = cola_ready.front();
        cola_ready.pop();

        int t_ejec = min(Q, lista[idx].t_restante);
        for(int i=0; i<t_ejec; i++) { 
            reloj++; 
            revisar_llegadas(); 
        }
        lista[idx].t_restante -= t_ejec;

        if (lista[idx].t_restante == 0) {
            lista[idx].tf = reloj;
            completado[idx] = true;
            hechos++;
        } else {
            revisar_llegadas();
            cola_ready.push(idx); // Se va al final de la cola real
        }
    }
    return procesarMétricas("ROUND ROBIN", lista, duration_cast<microseconds>(high_resolution_clock::now()-start).count());
}

int main() {
    vector<Actividad> base;
    cargarDatos("data/Datos.csv", base);
    
    double pE_f = fifoReal(base);
    double pE_l = lifoReal(base);
    double pE_r = rrReal(base, 4);

    cout << "\n" << string(60, '*') << endl;
    cout << "   COMPARATIVA FINAL POR PROMEDIO DE EFICACIA (pE)" << endl;
    cout << string(60, '*') << endl;
    cout << " - FIFO: " << pE_f << "\n - LIFO: " << pE_l << "\n - RR:   " << pE_r << endl;

    double mejor = min({pE_f, pE_l, pE_r});
    string ganador = (mejor == pE_f) ? "FIFO" : (mejor == pE_l) ? "LIFO" : "ROUND ROBIN";

    cout << "\n CONCLUSION: El algoritmo mas optimo es " << ganador << " (menor pE)" << endl;
    cout << string(60, '*') << endl;

    return 0;
}