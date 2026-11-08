//
// A barra da nota: o que ela mostra entre um quadro e outro.
//

#pragma once

#include <array>

/// A nota pula; a barra, nao. Um acerto tira pontos de uma vez, e uma barra que pula junto nao
/// conta o que aconteceu - so que o numero mudou.
namespace BarraDaNota {

    struct Estado {
        float mostrada = 0.0f;
        float rastro = 0.0f;
        float rastroParado = 0.0f;   ///< quanto tempo o rastro ainda fica antes de encolher
        float brilho = 0.0f;         ///< 0 a 1
        float tremor = 0.0f;         ///< 0 a 1
        float nota = 0.0f;           ///< a ultima nota recebida
    };

    /// Quanto da distancia ate a nota o preenchimento anda por segundo.
    constexpr float kPerseguicao = 14.0f;
    /// O rastro fica parado isto antes de encolher...
    constexpr float kRastroParado = 0.35f;
    /// ...e encolhe a isto, em pontos de nota por segundo.
    constexpr float kRastroEncolhe = 45.0f;
    /// O brilho e o tremor se apagam a isto por segundo.
    constexpr float kApagaBrilho = 3.0f;
    constexpr float kApagaTremor = 4.0f;
    /// Uma queda menor que isto nao sacode (o ganho por segundo do chefe que
    /// oscila nao deve fazer a barra tremer).
    constexpr float kQuedaQueSacode = 0.5f;

    /// A barra parada em 'nota': sem rastro, sem brilho.
    Estado Comecar(float nota);

    /// Um quadro de 'dt' segundos com a nota em 'nota'.
    void Avancar(Estado& e, float nota, float dt);

    /// As marcas da barra: 40, a fase final (o exame); 60, a aprovacao.
    constexpr float kMarcaDoExame = 40.0f;
    constexpr float kMarcaDaAprovacao = 60.0f;
    /// E as finas, a cada 10.
    constexpr std::array<float, 9> kMarcasFinas = {10, 20, 30, 40, 50, 60, 70, 80, 90};

    /// Onde, em pixels a partir do comeco da barra, cai a nota 'nota' numa barra de 'largura'.
    float X(float nota, int largura);
}
