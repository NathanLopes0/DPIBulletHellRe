//
// A fase tutorial (INF 191): o ritmo das etapas e o que cada uma desenha, sem texto.
//

#pragma once

#include <array>
#include <cstddef>

#include "../Motor/Math.h"

/// A primeira prova de quem acabou de criar o perfil, com um professor generico. Nao ha uma palavra
/// na tela: cada etapa ensina uma coisa com desenho - o manche ao lado do aluno enquanto chove
/// (desviar), o Botao 1 e a linha ate o professor (atirar), a nota que sobe a cada acerto e desce a
/// cada tiro levado, o Botao 4 segurado nos aneis apertados (o foco) e o Ponto Extra: a seta ate ele
/// no campo, e com ele na mao o Botao 2, que limpa a chuva cheia.
namespace Tutorial {

    enum class Etapa { Chegada, Desviar, Atirar, SobeEDesce, Foco, PontoExtra, Fim };
    constexpr std::size_t kEtapas = 7;

    /// Segundos de cada etapa, na ordem de Etapa - os mesmos de Assets/Attacks/generico/fases.json.
    constexpr std::array<float, kEtapas> kDuracoes{3.0f, 13.0f, 12.0f, 14.0f, 13.0f, 14.0f, 9.0f};

    /// O Ponto Extra que o professor solta, esta altura acima do aluno: perto, mas e preciso ir ate ele.
    constexpr float kPontoExtraAcima = 150.0f;

    /// A etapa 't' segundos depois de a prova comecar, e quanto dela ja passou.
    Etapa EtapaEm(float t);
    float NaEtapa(float t);
    /// Quando cada etapa comeca.
    float ComecoDa(Etapa e);

    // ---- Quem ja sabe passa mais rapido ----

    /// Como o aluno esta indo na etapa em curso: o relogio dela, os acertos no professor, os tiros
    /// levados, ha quanto tempo ele nao apanha e se ja gastou o Ponto Extra.
    struct Desempenho {
        float naEtapa = 0.0f;
        int acertosDados = 0;
        int acertosSofridos = 0;
        float semApanhar = 0.0f;
        bool gastouPontoExtra = false;
    };
    /// Se a etapa pode acabar agora, antes do tempo: o aluno ja mostrou o que ela ensina. Desviar e
    /// o foco, kSemApanharParaPassar segundos sem levar tiro; atirar, kAcertosParaPassar acertos; a
    /// nota que sobe e desce, os dois; o Ponto Extra, gasto. Nunca antes de kMinimoNaEtapa, para o
    /// desenho dela ser visto. A chegada e o fim tem o tempo deles.
    constexpr float kMinimoNaEtapa = 4.0f;
    constexpr float kSemApanharParaPassar = 6.0f;
    constexpr int kAcertosParaPassar = 8;
    bool PodeAvancar(Etapa e, const Desempenho& d);

    /// Os tiros de cada etapa saem do proprio professor (o gatilho de fases.json), no ritmo dela -
    /// e nao do relogio da fase, que nao anda mais rapido quando a etapa acaba antes. Sem tiro: nulo.
    const char* GatilhoDe(Etapa e);
    float CadenciaDe(Etapa e);
    /// O primeiro tiro de cada etapa espera isto: o desenho dela aparece antes.
    constexpr float kPrimeiroTiro = 0.6f;

    /// O professor encerra a prova kAplausos segundos depois de o Fim comecar: o tempo da festa.
    constexpr float kAplausos = 3.0f;
    bool Acabou(float t);

    /// Ninguem reprova no tutorial: a nota nunca fica abaixo disto.
    constexpr float kNotaMinima = 15.0f;

    /// A nota mudou de um quadro para o outro: subiu (o acerto no professor) ou desceu (o tiro
    /// levado). Uma folga pequena ignora o arredondamento.
    enum class Sinal { Nada, Subiu, Desceu };
    Sinal SinalDaNota(float antes, float agora);

    /// O "+" ou o "-" voa do lugar do acontecido ate a ponta da barra da nota em kVooDoSinal segundos.
    constexpr float kVooDoSinal = 0.7f;
    /// Onde ele esta, 'idade' segundos depois de sair: devagar no fim, como quem pousa.
    Vector2 NoVoo(Vector2 de, Vector2 ate, float idade);

    /// O manche desenhado ao lado do aluno aponta para cada lado, a vez de cada um: esquerda, direita,
    /// cima e baixo, kPorLado segundos cada.
    constexpr float kPorLado = 0.5f;
    Vector2 LadoDoManche(float t);

    /// O botao desenhado e apertado e solto, num ritmo de quem atira: aceso metade de cada kToque.
    constexpr float kToque = 0.5f;
    bool BotaoApertado(float t);
}
