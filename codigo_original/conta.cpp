#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
using namespace std;

int saldo;

void sacar(int id, int valor) {
    if (saldo >= valor) {                                    // 1) verifica
        this_thread::sleep_for(chrono::milliseconds(100));   // simula processamento
        saldo -= valor;                                      // 2) age
        cout << "Thread " << id << " sacou " << valor
             << " | saldo: " << saldo << endl;
    } else {
        cout << "Thread " << id << " saque negado | saldo: " << saldo << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Uso: ./conta <saldo_inicial> <valor_saque> <num_threads>\n";
        return 1;
    }
    saldo = stoi(argv[1]);
    int valor = stoi(argv[2]);
    int n = stoi(argv[3]);

    vector<thread> threads;
    for (int i = 1; i <= n; i++)
        threads.emplace_back(sacar, i, valor);
    for (auto& t : threads) t.join();

    cout << "SALDO FINAL: " << saldo << endl;
    return 0;
}