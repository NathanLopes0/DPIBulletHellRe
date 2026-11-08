//
// O escudo do Hugo e o objetivo de cada etapa do TCC.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Golpe.h"
#include "../Motor/Math.h"
#include "../Regras/Variacao.h"

/// O TCC ainda nao esta pronto, e enquanto nao estiver o Hugo e blindado: o tiro que o acerta so
/// carrega o poder do orientador, e nao paga nota.
namespace Objetivo {

    enum class Etapa { Nenhuma, Proposta, Desenvolvimento, Defesa, Correcoes };

    /// "StateOne" e a proposta, e assim por diante.
    Etapa DaFase(const std::string& fase);

    constexpr const char* kPostoDoObjetivo = "objetivo";
    constexpr const char* kGatilhoDoAlvo = "alvo";     ///< assinatura, resposta, correcao
    constexpr const char* kGatilhoDoBug = "bug";

    /// A faixa do campo onde os alvos ficam: abaixo do cronograma, acima do aluno.
    constexpr float kYDeCima = 380.0f;
    constexpr float kYDeBaixo = 540.0f;
    constexpr float kMargem = 120.0f;

    /// Um alvo do objetivo: a animacao, onde ele fica, e se derruba-lo e certo.
    struct Alvo {
        std::string animacao;
        Vector2 lugar;
        bool certo = true;
    };

    /// Os alvos de um objetivo novo da 'etapa', numa janela de 'largura'. Na defesa, 'certa' (0 a
    /// 2) e a resposta certa.
    std::vector<Alvo> Gerar(Etapa etapa, Variacao::Sorteio& s, float largura, Dificuldade::Nivel n, int certa = 0);

    Vector2 Balanco(Etapa etapa, const Vector2& lugar, int k, float t);

    /// A proxima parada de um bug: um ponto qualquer da faixa.
    Vector2 Paragem(Variacao::Sorteio& s, float largura);
    constexpr float kRapidezDoBug = 130.0f;

    // ---- A defesa ----

    /// Uma pergunta da banca, com as tres respostas na ordem do placar.
    struct Questao {
        std::string pergunta;
        std::array<std::string, 3> respostas;
        int certa = 0;
    };

    /// A pergunta do professor 'm', com as respostas embaralhadas.
    Questao Perguntar(Golpe::Materia m, Variacao::Sorteio& s);

    // ---- A janela ----

    /// O que o objetivo paga, em nota bruta (Battle::Conquistar): cada alvo derrubado, e o escudo
    /// aberto.
    float PorAlvo(Dificuldade::Nivel n);
    float PorJanela(Dificuldade::Nivel n);

    /// Aberto, a monografia se desfaz: as paginas escorrem retas embaixo do Hugo, uma a cada tanto.
    float EntrePaginasSoltas(Dificuldade::Nivel n);

    /// Quantos segundos o Hugo fica aberto depois de um objetivo.
    float Janela(Etapa etapa, Dificuldade::Nivel n);
    constexpr float kRecomeco = 0.8f;    ///< da janela fechada ao objetivo seguinte

    /// O que o placar diz do objetivo, faltando 'faltam' alvos.
    std::string Falta(Etapa etapa, int faltam);

    // ---- As outras maneiras de abrir ----

    enum class Tipo { Derrubar, Reuniao, Referencias, Arguicao, Entrega };

    /// A maneira de andar da 'etapa'.
    Tipo DeAndar(Etapa etapa);

    /// A maneira do objetivo numero 'vez' da etapa: alternam, a partir de 'primeira' (0 derruba
    /// primeiro, 1 anda primeiro).
    Tipo DaVez(Etapa etapa, int vez, int primeira);

    /// O chao do aluno, onde ficam os objetivos de andar: abaixo da faixa dos alvos, mas acima do
    /// pe dele - ninguem vai buscar nada no rodape.
    constexpr float kChaoDeCima = 500.0f;
    constexpr float kChaoDeBaixo = 640.0f;

    // ---- A reuniao ----
    constexpr float kRaioDaReuniao = 80.0f;
    /// Quantos segundos dentro do anel a reuniao pede.
    float TempoDeReuniao(Dificuldade::Nivel n);
    /// Onde a reuniao e marcada: no chao, longe de onde o aluno esta ('xDoAluno') - quem ja estava
    /// la nao ganharia nada de graca.
    Vector2 LugarDaReuniao(Variacao::Sorteio& s, float largura, float xDoAluno);

    // ---- As referencias ----
    constexpr float kRaioDaColeta = 42.0f;
    /// Os livros, espalhados pelo chao.
    std::vector<Vector2> Referencias(Variacao::Sorteio& s, float largura, Dificuldade::Nivel n);

    // ---- A arguicao ----
    constexpr float kFolgaDaRaspada = 34.0f;
    /// Quantas raspadas a arguicao pede.
    int Raspadas(Dificuldade::Nivel n);
    /// Se o tiro a 'distancia' do aluno (de centro a centro) o raspou, sendo 'raios' a soma dos
    /// dois raios.
    bool Raspou(float distancia, float raios);

    // ---- A entrega ----
    constexpr float kRaioDaSecretaria = 70.0f;
    struct Entrega {
        Vector2 versao;       ///< onde a versao final aparece
        Vector2 secretaria;   ///< onde ela e entregue: um canto do chao
    };
    /// A versao final aparece de um lado, e a secretaria fica no canto do outro: a entrega
    /// atravessa o campo.
    Entrega Entregar(Variacao::Sorteio& s, float largura);

    /// Quantas placas o escudo tem para cada objetivo - uma por coisa a fazer: os alvos certos, os
    /// livros, as raspadas.
    int Placas(Tipo tipo, Dificuldade::Nivel n, int alvosCertos);

    /// Quantas placas continuam de pe com o objetivo 'progresso' (0 a 1) feito. Nenhuma so com ele
    /// inteiro.
    int PlacasDePe(int placas, float progresso);

    /// O que cada passo do objetivo paga, em nota bruta: um alvo, um livro, uma raspada (meio alvo:
    /// sao mais), pegar ou entregar a versao.
    float PorPasso(Tipo tipo, Dificuldade::Nivel n);

    /// O que o placar diz de um objetivo de andar ('faltam' livros ou raspadas; na entrega, 1 antes
    /// de pegar e 0 com ela na mao).
    std::string Pede(Tipo tipo, int faltam);
}
