#!/bin/bash
# Compila o codigo do Cenario 2 (estacionamento, Yara), original e corrigido,
# e roda os 3 casos de teste (10 execucoes cada), salvando os logs em evidencias/.
# Funciona no macOS (clang) e no Linux (g++).

BASE="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$BASE/evidencias"

for versao in original corrigido; do
  cd "$BASE/codigo_$versao/yara_estacionamento" || exit 1
  c++ -std=c++17 -pthread estacionamento.cpp -o estacionamento || exit 1

  for caso in "0 1000 1000" "100 6 0" "200 8 10"; do
    nome="${caso// /_}"
    echo "=== Yara $versao: ./estacionamento $caso ==="
    for i in {1..10}; do ./estacionamento $caso | tail -1; done | tee "$BASE/evidencias/barbara_avalia_yara_${versao}_${nome}.txt"
  done
done
