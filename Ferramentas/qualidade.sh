#!/usr/bin/env bash
# A QUALIDADE DO CODIGO, de uma vez: os testes com AddressSanitizer e UBSan, e o clang-tidy.
#
#   bash Ferramentas/qualidade.sh            # os dois
#   bash Ferramentas/qualidade.sh testes     # so os testes com sanitizers
#   bash Ferramentas/qualidade.sh tidy       # so o clang-tidy (precisa de um build com
#                                            # compile_commands.json em build-qualidade/)
#
# Os testes compilam em build-qualidade/ com -DDPI_SANITIZE=ON (CMakeLists.txt) e rodam inteiros: um
# uso de memoria liberada, um estouro ou um comportamento indefinido param a suite com a pilha.
# O clang-tidy usa as checagens de .clang-tidy (bugs de verdade e o tamanho das funcoes) e escreve o
# relatorio em build-qualidade/clang-tidy.txt, com a contagem por checagem no fim.
#
# Precisa de: cmake, ninja, clang-tidy (e run-clang-tidy), e o SDL2 com os cabecalhos (o jogo entra
# no compile_commands.json, e o clang-tidy le os cabecalhos dele).
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
OBRA="$RAIZ/build-qualidade"
QUE="${1:-tudo}"

if [ "$QUE" = tudo ] || [ "$QUE" = testes ]; then
    echo "== testes com AddressSanitizer e UBSan"
    cmake -S "$RAIZ" -B "$OBRA" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DDPI_SANITIZE=ON > /dev/null
    ninja -C "$OBRA" dpi_tests
    ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=print_stacktrace=1 "$OBRA/dpi_tests"
fi

if [ "$QUE" = tudo ] || [ "$QUE" = tidy ]; then
    echo "== clang-tidy"
    [ -f "$OBRA/compile_commands.json" ] || cmake -S "$RAIZ" -B "$OBRA" -G Ninja > /dev/null
    TIDY="$(command -v run-clang-tidy-18 || command -v run-clang-tidy)"
    # So os fontes do projeto: o Json.h e o doctest.h ficam de fora (.clang-tidy).
    "$TIDY" -p "$OBRA" -quiet -j "$(nproc)" "$RAIZ/(Source|tests)/.*\.cpp" > "$OBRA/clang-tidy.txt" 2>&1 || true
    echo "avisos por checagem:"
    grep -oE "\[[a-z]+-[a-zA-Z0-9.-]+\]$" "$OBRA/clang-tidy.txt" | sort | uniq -c | sort -rn
    echo "relatorio: $OBRA/clang-tidy.txt"
fi
