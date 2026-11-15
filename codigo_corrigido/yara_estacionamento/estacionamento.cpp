// Versao corrigida com mutex

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>
using namespace std;

int carros_dentro;
std::mutex m; //  as duas cancelas usam as variaveis

void atualiza_contador(int delta)
{
    m.lock();                                         // pega a chave antes de iniciar a leitura
    int copia = carros_dentro;                        // 1) lê o valor atual
    this_thread::sleep_for(chrono::microseconds(50)); // 2) tempo de processar o sensor
    carros_dentro = copia + delta;                    // 3) grava o novo valor
    m.unlock();                                        // devolve a chave depois de gravar valor atualizado
}

void cancela_entrada(int n)
{
    for (int i = 0; i < n; i++)
        atualiza_contador(+1);
    cout << "Cancela de entrada terminou: " << n << " carros entraram" << endl;
}

void cancela_saida(int n)
{
    for (int i = 0; i < n; i++)
        atualiza_contador(-1);
    cout << "Cancela de saida terminou: " << n << " carros sairam" << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << "Uso: ./estacionamento <carros_iniciais> <carros_que_entram> <carros_que_saem>\n";
        return 1;
    }
    int iniciais = stoi(argv[1]);
    int entram = stoi(argv[2]);
    int saem = stoi(argv[3]);

    carros_dentro = iniciais;

    thread entrada(cancela_entrada, entram);
    thread saida(cancela_saida, saem);
    entrada.join();
    saida.join();

    int esperado = iniciais + entram - saem;
    cout << "ESPERADO: " << esperado << " | CARROS DENTRO NO FINAL: " << carros_dentro << endl;
    return 0;
}
