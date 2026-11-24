//
// A fase 3 do Lucas (INF 321): o Elixir, numa sala de masmorra.
//

#pragma once

#include <vector>

#include "Dom.h"
#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// O professor gosta de jogos de aventura, e a prova vira uma sala de masmorra. A cada rodada
/// aparece um bau trancado, algumas gosmas andando pela sala e duas tochas apagadas; derrotadas todas
/// as gosmas, cai a chave pequena onde caiu a ultima. Com a chave, o aluno abre o bau e ergue o Elixir
/// acima da cabeca: por um tempo ele fica invencivel, mais rapido, e cada acerto no professor vale bem
/// mais. Mas o professor tambem quer o Elixir, e o bau so abre para ele com as tochas acesas: ele anda
/// ate cada uma, acende, e so entao vai ao bau. O tiro do aluno apaga a tocha acesa - e o professor
/// tem de voltar. Se ele chegar antes, bebe - e a sala vira uma chuva de tiros, que no Normal da para
/// atravessar e no Dificil quase nao. Cada rodada dura o que durar: a seguinte comeca um pouco depois
/// do desfecho.
namespace Masmorra {

    constexpr const char* kFase = "StateThree";
    /// As tochas que o professor acende antes do bau.
    constexpr int kTochas = 2;
    /// Depois do desfecho, quanto ate a rodada seguinte: a do aluno (o bonus continua correndo) e a do
    /// professor, contada do fim da furia.
    constexpr float kPausaDepoisDoAluno = 2.0f;
    constexpr float kPausaDepoisDaFuria = 1.0f;

    using Campo = Dom::Campo;

    struct Degrau {
        int gosmas = 3;
        float rapidezDasGosmas = 70.0f;
        /// O professor: a rapidez com que anda pela sala, e quanto leva acendendo cada tocha.
        float rapidezDoProfessor = 190.0f;
        float acender = 0.9f;
        /// O Elixir do aluno: quanto dura e quanto vale cada acerto no professor (Boss::FatorDoGanho).
        float bonus = 6.0f;
        float fatorDoGanho = 3.0f;
        float impulso = 1.3f;
        /// A furia do professor que bebeu: quanto dura, a espiral (bracos, ritmo, velocidade, giro) e
        /// o anel que sai junto.
        float furia = 3.5f;
        int bracos = 3;
        float entreRajadas = 0.12f;
        float velocidadeDaFuria = 170.0f;
        float giro = 70.0f;
        float entreAneis = 1.0f;
        int tirosDoAnel = 12;
        /// O leque com que ele atrapalha a cacada, enquanto anda.
        float entreLeques = 1.8f;
        int tirosDoLeque = 3;
    };
    Degrau DegrauDo(Dificuldade::Nivel nivel);

    /// O bau: na metade de baixo da sala e longe do aluno - ninguem comeca do lado dele.
    constexpr float kLongeDoAluno = 300.0f;
    Vector2 LugarDoBau(Variacao::Sorteio& s, const Campo& c, Vector2 aluno);
    /// As gosmas: espalhadas na faixa do meio, longe do bau.
    std::vector<Vector2> LugaresDasGosmas(Variacao::Sorteio& s, int n, const Campo& c, Vector2 bau);

    /// As tochas: uma de cada lado da sala, na faixa de cima (o tiro do aluno, que sobe, chega nelas),
    /// longe do bau. Na ordem em que o professor, saindo de 'professor', as acende: a mais perto
    /// primeiro.
    std::vector<Vector2> LugaresDasTochas(Variacao::Sorteio& s, int n, const Campo& c, Vector2 bau, Vector2 professor);
    /// Para onde o professor vai agora: a primeira tocha apagada, na ordem; -1 com todas acesas (o bau).
    [[nodiscard]] int ProximaTocha(const std::vector<bool>& acesas);
    /// Um passo de 'de' rumo a 'para', a 'rapidez', sem passar dele.
    Vector2 Passo(Vector2 de, Vector2 para, float rapidez, float dt);
    /// Quanto o professor leva sem ninguem apagar nada: de 'de', acendendo cada tocha, ate o bau.
    float Percurso(Vector2 de, const std::vector<Vector2>& tochas, Vector2 bau, float rapidez, float acender);
    /// Chegou (a tocha, o bau): perto assim.
    constexpr float kChegou = 8.0f;

    /// Perto o bastante para pegar (a chave, o bau).
    constexpr float kRaioDePegar = 34.0f;
    [[nodiscard]] bool Alcanca(Vector2 aluno, Vector2 coisa);

    /// A furia: o rumo (graus) de cada braco da espiral, 'tf' segundos depois de ela comecar.
    std::vector<float> RumosDaEspiral(int bracos, float tf, float giroPorSegundo);

    /// O Elixir erguido acima da cabeca: a altura em que ele fica sobre o aluno, e por quanto tempo
    /// o gesto dura (o aluno nao para; so o desenho).
    constexpr float kErguido = 1.2f;
    constexpr float kAcimaDaCabeca = 46.0f;
}
