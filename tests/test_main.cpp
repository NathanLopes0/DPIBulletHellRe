// Ponto de entrada dos testes (main gerado pelo doctest).
//
// A implementacao do doctest fica ISOLADA neste arquivo, e nenhum codigo do
// jogo pode ser incluido aqui. Motivo: no Windows, a implementacao do doctest
// inclui <windows.h>, que define macros vazias com nomes comuns - entre elas
// 'near' e 'far'. O Math.h usa 'near' e 'far' como nomes de parametro
// (CreateOrtho, CreatePerspectiveFOV), e com essas macros ativas a expressao
// '(far - near)' vira '( - )' e nao compila.
//
// Com a implementacao aqui, o windows.h nunca e visto pelos arquivos que
// incluem codigo do jogo. Bonus: a implementacao do doctest, que e grande,
// passa a ser compilada uma vez so, e nao em cada arquivo de teste.
//
// Os testes ficam nos outros arquivos de tests/, que incluem so "doctest.h".
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
