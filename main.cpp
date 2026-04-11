#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <queue>

using namespace std;

// Estructura 
struct Proceso {
    string id;
    int ti;            // Tiempo de llegada
    int t;             // Tiempo de ejecución
    int tf = 0;        // Tiempo final
    int t_restante;    // Para Round Robin
    double T = 0;      // Tiempo de retorno
    double E = 0;      // Tiempo de espera
    double I = 0;      // Índice de rendimiento
};

class GestorProcesos {
private:
    vector<Proceso> listaBase;

    void calcularMetricas(Proceso &p) {
        p.T = (double)p.tf - p.ti;
        p.E = p.T - p.t; // Formula: E = T - t
        p.I = (p.T > 0) ? (double)p.t / p.T : 0;
    }

public:
    void cargarDatos(string ruta) {
        ifstream f(ruta);
        string l;
        bool cabecera = true;
        while (getline(f, l)) {
            if (l.empty()) continue;
            stringstream ss(l);
            string id, s_ti, s_t;
            if (getline(ss, id, ',') && getline(ss, s_ti, ',') && getline(ss, s_t)) {
                if (cabecera && (s_ti.find_first_not_of("0123456789") != string::npos)) {
                    cabecera = false; continue;
                }
                listaBase.push_back({id, stoi(s_ti), stoi(s_t), 0, stoi(s_t)});
            }
            cabecera = false;
        }
    }

    // 
    double imprimirTabla(string metodo, const vector<Proceso> &resultados) {
        double acumT = 0, acumE = 0, acumI = 0;
        int n = resultados.size();

        cout << "\n>>> ESTRATEGIA: " << metodo << " <<<\n";
        cout << "ID\tti\tt\ttf\tT\tE\tI\n" << string(50, '-') << endl;

        for (const auto &p : resultados) {
            acumT += p.T; acumE += p.E; acumI += p.I;
            cout << p.id << "\t" << p.ti << "\t" << p.t << "\t" << p.tf << "\t" 
                 << (int)p.T << "\t" << (int)p.E << "\t" 
                 << fixed << setprecision(4) << p.I << endl;
        }

        double promedioI = (acumI / n) * 100; // Convertimos a porcentaje para la comparativa
        cout << string(50, '-') << endl;
        cout << "PROMEDIOS: pT=" << fixed << setprecision(2) << acumT/n 
             << " | pE=" << acumE/n << " | pI=" << promedioI << "%\n";
        
        return promedioI; 
    }

    // Algoritmo fifo y lifo
    vector<Proceso> modoEstatico(bool usarLifo) {
        vector<Proceso> copia = listaBase;
        vector<bool> completado(copia.size(), false);
        int cronometro = 0, finalizados = 0;

        while (finalizados < copia.size()) {
            int seleccionado = -1;
            for (int i = 0; i < (int)copia.size(); i++) {
                int idx = usarLifo ? (copia.size() - 1 - i) : i;
                if (!completado[idx] && copia[idx].ti <= cronometro) {
                    seleccionado = idx; 
                    break;
                }
            }

            if (seleccionado == -1) { 
                cronometro++; 
                continue; 
            }

            cronometro += copia[seleccionado].t;
            copia[seleccionado].tf = cronometro;
            completado[seleccionado] = true;
            calcularMetricas(copia[seleccionado]);
            finalizados++;
        }
        return copia; 
    }

    // Algoritmo Round Robin 
    vector<Proceso> modoCircular(int quantum) {
        vector<Proceso> copia = listaBase;
        int n = copia.size();
        queue<int> colaEspera;
        vector<bool> enCola(n, false), terminado(n, false);
        int cronometro = 0, finalizados = 0;

        auto chequearEntradas = [&]() {
            for (int i = 0; i < n; i++) {
                if (!terminado[i] && !enCola[i] && copia[i].ti <= cronometro) {
                    colaEspera.push(i); enCola[i] = true;
                }
            }
        };

        while (finalizados < n) {
            chequearEntradas();
            if (colaEspera.empty()) { cronometro++; continue; }

            int idx = colaEspera.front(); colaEspera.pop();
            int rafagaActual = min(quantum, copia[idx].t_restante);
            
            for(int i=0; i<rafagaActual; i++) { 
                cronometro++; 
                chequearEntradas(); 
            }
            copia[idx].t_restante -= rafagaActual;

            if (copia[idx].t_restante == 0) {
                copia[idx].tf = cronometro;
                calcularMetricas(copia[idx]);
                terminado[idx] = true;
                finalizados++;
            } else {
                chequearEntradas();
                colaEspera.push(idx);
            }
        }
        return copia;
    }
};

int main() {
    GestorProcesos sistema;
    sistema.cargarDatos("data/Datos.csv");

    int qUsuario;
    cout << "Ingrese el valor del Quantum para Round Robin: ";
    cin >> qUsuario;

    double pIFIFO = sistema.imprimirTabla("FIFO", sistema.modoEstatico(false));
    double pILIFO = sistema.imprimirTabla("LIFO", sistema.modoEstatico(true));
    double pIRR   = sistema.imprimirTabla("ROUND ROBIN (Q=" + to_string(qUsuario) + ")", sistema.modoCircular(qUsuario));

    double mejorPI = max({pIFIFO, pILIFO, pIRR});
    string mejorOp = (mejorPI == pIFIFO) ? "FIFO" : (mejorPI == pILIFO) ? "LIFO" : "ROUND ROBIN";

    cout << "\n=============================================\n";
    cout << " El mejor algoritmo es " << mejorOp << endl;
    cout << " con un rendimiento promedio de " << fixed << setprecision(2) << mejorPI << "%.\n";
    cout << "=============================================\n";

    return 0;
}