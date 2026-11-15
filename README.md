# Race Condition com Mutex/Semáforo: PSI Microcontroladores 2, Aula 06

---

## Cenário 1: Saque em conta bancária

Programa em C++ com `std::thread` em que várias threads tentam sacar de
uma mesma conta. Cada thread verifica se há saldo suficiente e, só depois,
subtrai o valor.


### Descrição da race condition

A variável global `saldo` é lida e alterada por várias threads sem nenhuma
proteção.

Se duas threads executam a verificação antes de qualquer uma subtrair, ambas
enxergam o saldo antigo, ambas passam no `if` e ambas sacam. O saldo fica
negativo. Um `sleep_for` de 100 ms foi colocado entre a verificação e a
subtração para ampliar essa janela e tornar o bug fácil de reproduzir.

Outro problema verificado foi na saída do `cout` que aparece embaralhada
(ex.: `Thread Thread 1 sacou 80 | saldo: 2-60 sacou 80`), pois várias threads
escrevem no terminal ao mesmo tempo.

### Casos de teste

| Caso | Pré-condição | Etapas de teste | Pós-condição esperada |
|---|---|---|---|
| 1 | Programa compilado; saldo inicial = 100 | `./conta 100 80 2` | 1 saque aprovado e 1 negado; saldo final = 20 |
| 2 | Programa compilado; saldo inicial = 100 | `./conta 100 30 5` | 3 saques aprovados e 2 negados; saldo final = 10 |
| 3 (controle) | Programa compilado; saldo inicial = 100 | `./conta 100 50 2` | 2 saques aprovados; saldo final = 0 |

Regra geral, o saldo final nunca pode ser negativo.

### Resultados no código original

Cada caso foi executado várias vezes, pois o comportamento é irregular, característica de um race condition.

| Caso | Esperado | Obtido | Resultado |
|---|---|---|---|
| 1 | 20 | -60 em quase todas as execuções; 20 em algumas | Falha |
| 2 | 10 | -20 e -50, variando entre execuções | Falha |
| 3 | 0 | 0 |  Passou |


### Observações

- **Não determinismo:** o mesmo comando produziu resultados diferentes
  entre execuções (ex.: caso 2 com saldo -50 e -20).
- **Erro:** ocorre quando duas ou mais threads executam o `if`
  antes de qualquer uma executar `saldo -= valor`.

### Código corrigido

**Mudanças:**
- Adicionado `#include <mutex>` e um `mutex mtx` global.
- Adicionado `lock_guard<mutex> lock(mtx);` no início de `sacar()`, de modo que
  a verificação do saldo, a subtração e a impressão formem uma única seção
  crítica.

**Por que usei mutex** O problema é proteger um único recurso
compartilhado (`saldo`) com acesso exclusivo. O mutex expressa isso
diretamente e tem quem trava é quem destrava, o que evita liberações
indevidas. Um semáforo é mais indicado para limitar o número de threads
simultâneas em um recurso ou sincronizar eventos, o que não é o caso.


**Resultados após a correção** (20 execuções por caso):

| Caso | Esperado | Obtido | Resultado |
|---|---|---|---|
| 1 | 20 | 20 em 20/20 execuções |  Passou |
| 2 | 10 (3 aprovados, 2 negados) | 10 em 20/20 execuções; contagem 3 e 2 | Passou |
| 3 | 0 | 0 em 20/20 execuções | Passou |

**Estabilidade:** o saldo final foi idêntico em todas as execuções, e a saída
do terminal não aparece mais embaralhada. 

