# Race Condition com Mutex/Semáforo: PSI Microcontroladores 2, Aula 06

## Integrantes

- Yara Rodrigues (GitHub: yjmoreno)
- Bárbara Santos (GitHub: dssbarbara) 

## Cenários escolhidos

1. Saque em conta bancária (Bárbara)
2. Estacionamento com duas cancelas (Yara)


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

---

## Cenário 2: Estacionamento com duas cancelas (Yara)

Programa em C++ com `std::thread` em que uma cancela de entrada e uma cancela de
saída atualizam o mesmo contador de carros (`carros_dentro`).

Uso: `./estacionamento <carros_iniciais> <carros_que_entram> <carros_que_saem>`

### Descrição da race condition

Os erros no caso 1 e no caso 3 mostram que a cancela de saída está gravando por
último, mas só está calculando os carros que tinham menos os que saem; não soma os
carros que entram com os que tinham antes de subtrair os que saíram. Dessa forma,
gera resultados errados. No caso 1, quando o resultado deu +1000, a cancela de
entrada gravou por último, e os carros que sumiram foram os da cancela de saída.
No caso 2, quando os carros que saem são zerados, não há
ação dessa cancela de saída a ser computada, então os resultados dão certo porque
não há disputa entre as cancelas.

Nos casos 1 e 3 é como se quem grava por último vencesse a corrida de condição e não
esperasse o resultado da primeira, causando erro no resultado real.

### Casos de teste

| Caso | Pré-condição | Etapas de teste | Pós-condição esperada |
|---|---|---|---|
| 1 | Estacionamento começa vazio | Entram 1000 carros e saem 1000 carros (`./estacionamento 0 1000 1000`, 10 execuções) | O resultado correto seria 0 |
| 2 | O estacionamento começa com 100 carros | Entram 6 carros e não sai nenhum carro (`./estacionamento 100 6 0`, 10 execuções) | O estacionamento tem 106 carros |
| 3 | O estacionamento começa com 200 carros | Recebe 8 novos carros e saem 10 carros (`./estacionamento 200 8 10`, 10 execuções) | O resultado esperado seria 198 |

### Resultados no código original

| Caso | Esperado | Obtido | Resultado |
|---|---|---|---|
| 1 | 0 | Saídas de -1000, 1000, 126, 620; nunca chegou a zero | Falha |
| 2 | 106 | Deu 106 carros, como o esperado | Passou |
| 3 | 198 | As saídas no terminal mostram 190 carros | Falha |

Evidências: `evidencias/yara_original_0_1000_1000.txt`, `evidencias/yara_original_0_1000_1000.png`,
`evidencias/yara_original_100_6_0.txt`, `evidencias/yara_original_200_8_10.txt`

### Código corrigido

Eu corrigi o código usando mutex porque ele tira a competição entre as cancelas,
conseguindo criar uma área restrita para uma cancela atuar por vez; assim, essa área
restrita ficou sendo a leitura e gravação do saldo atualizado do estacionamento.

Para usar o mutex, incluí a biblioteca no código, coloquei um lock antes de copiar
quantos carros tinha e um unlock depois de gravar a quantidade atualizada, evitando
erro no saldo final, porque cada cancela pega o valor realmente atualizado, contando
o que tinha e o que entrou.

**Por que mutex e não semáforo:** o semáforo poderia deixar uma cancela trabalhando por
vez, mas, diferente do mutex, quem "usa a chave" não precisa ser quem devolve.

### Resultados após a correção

| Caso | Esperado | Obtido | Resultado |
|---|---|---|---|
| 1 | 0 | 0 em 10/10 execuções | Passou |
| 2 | 106 | 106 em 10/10 execuções | Passou |
| 3 | 198 | 198 em 10/10 execuções | Passou |

**Estabilidade:** os testes mostraram que, com isso, os resultados saem sempre como o esperado.

Evidências: `evidencias/yara_corrigido_0_1000_1000.txt`, `evidencias/yara_corrigido_100_6_0.txt`,
`evidencias/yara_corrigido_200_8_10.txt`, `evidencias/yara_corrigido_testes.png`

---

## Avaliações cruzadas

### Yara avalia o código da Bárbara (Cenário 1)

Executei o código original e o corrigido da Barbara com os 3 casos de teste planejados
por ela (10 execuções cada).

| Caso | Comando | Esperado | Original | Corrigido |
|---|---|---|---|---|
| 1 | `./conta 100 80 2` | 20 | -60 em 10/10 | 20 em 10/10 |
| 2 | `./conta 100 30 5` | 10 | -50 em 10/10 | 10 em 10/10 |
| 3 | `./conta 100 50 2` | 0 | 0 em 10/10 | 0 em 10/10 |

**O que estava errado antes:** no primeiro código, a thread 2 tinha acesso a iniciar a
execução quando a thread 1 ainda estava funcionando; assim, não tinha acesso ao saldo
atualizado que ficaria após a execução da thread 1. O ideal seria que só conseguisse
executar depois que uma terminasse, protegendo o saldo.

**O que mudou com a correção:** no código corrigido, a Barbara também usou mutex para
proteger o saldo e permitir que apenas uma thread tivesse acesso ao saldo por vez e,
consequentemente, a fazer operações como saque. Com a correção, as threads alteram o
saldo conforme a disponibilidade, até que o saldo seja menor que a tentativa de
alteração, como no caso 2, que permite saques de 30 reais até que o saldo seja menor
que 30.

**Estabilidade:** os testes do código corrigido mostram que o mutex para proteger o
saldo resolveu o problema.

Evidências: `evidencias/yara_avalia_barbara_original.png`, `evidencias/yara_avalia_barbara_corrigido.png`,
`evidencias/yara_avalia_barbara_*.txt`

### Bárbara avalia o código da Yara (Cenário 2)

(a fazer pela Bárbara: executar `bash rodar_testes_yara.sh`, que roda os 3 casos do
Cenário 2 no código original e no corrigido, e registrar o que estava errado antes,
o que mudou com a correção e se o comportamento agora é estável)
