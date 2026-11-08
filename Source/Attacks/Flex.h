//
// O flexbox do CSS - onde cada item de uma linha fica, para cada valor de
// justify-content.
//

#pragma once

#include <string>
#include <vector>

#include "../Regras/Variacao.h"

/// A fase 2 do Lucas e o CSS, e a grade do DevTools dela (Estilo.h) poe cada linha de cartoes pelo
/// justify-content da regra em vigor.
namespace Flex {

    /// Os cinco valores de justify-content que a fase usa, com os nomes do CSS.
    enum class Justificar { Inicio, Fim, Centro, EspacoEntre, EspacoEmVolta };

    /// Le o valor pelo nome do CSS: flex-start, flex-end, center, space-between ou space-around.
    /// Devolve false para qualquer outro nome.
    bool PeloNome(const std::string& nome, Justificar& saida);

    /// O nome do CSS de volta - e o que o placar escreve.
    std::string Nome(Justificar j);

    /// O centro de cada um dos 'n' itens, de largura 'item', numa linha que vai de 'inicio' a
    /// 'fim'.
    std::vector<float> Posicoes(Justificar j, int n, float item, float inicio, float fim);

    // ---- O que muda de uma luta para a outra ----

    /// Se o arranjo deixa os itens juntos (flex-start, flex-end, center).
    bool Junto(Justificar j);

    /// Para onde a proxima fileira se rearranja, saindo de 'de': do junto para um espalhado, e do
    /// espalhado para um junto, sorteado entre os do outro lado.
    Justificar Proxima(Variacao::Sorteio& s, Justificar de);

    // ---- A varredura do professor ----

    /// O professor varre o alto da tela, de margem a margem, nas fases 2 e 3.
    constexpr float kYDaVarredura = 144.0f;
    constexpr float kMargemDaVarredura = 150.0f;
    constexpr float kVarredura = 90.0f;
    constexpr float kVarreduraNoDificil = 120.0f;

    /// O x do professor 't' segundos depois de comecar a varrer de 'x0': vai e volta entre as
    /// margens, a 'velocidade', comecando para a direita.
    float XDaVarredura(float t, float x0, float velocidade, float largura);
}
