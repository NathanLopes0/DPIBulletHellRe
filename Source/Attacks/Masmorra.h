//
// A fase 3 do Lucas (INF 321): o Elixir, numa sala de masmorra.
//

#pragma once

#include <vector>

#include "Dom.h"
#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// O professor gosta de jogos de aventura, e a prova vira uma sala de masmorra. A cada ciclo
/// (kCiclo), aparece um bau trancado e algumas gosmas andando pela sala; derrotadas todas, cai a
/// chave pequena onde caiu a ultima. Com a chave, o aluno abre o bau e ergue o Elixir acima da cabeca:
/// por um tempo ele fica invencivel, mais rapido, e cada acerto no professor vale bem mais. Mas o
/// professor tambem quer o Elixir: desde o comeco do ciclo ele anda ate o bau, pelas beiradas da
/// sala, e chega em 'chegada' segundos. Se chegar antes, bebe - e a sala vira uma chuva de tiros, que
/// no Normal da para atravessar e no Dificil quase nao.
namespace Masmorra {

    constexpr const char* kFase = "StateThree";
    constexpr float kCiclo = 10.0f;

    using Campo = Dom::Campo;

    struct Degrau {
        int gosmas = 3;
        float rapidezDasGosmas = 70.0f;
        /// Quando o professor chega ao bau, em segundos desde o comeco do ciclo.
        float chegada = 6.2f;
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
        float entreLeques = 1.2f;
        int tirosDoLeque = 5;
    };
    Degrau DegrauDo(Dificuldade::Nivel nivel);

    /// O bau: na metade de baixo da sala e longe do aluno - ninguem comeca do lado dele.
    constexpr float kLongeDoAluno = 300.0f;
    Vector2 LugarDoBau(Variacao::Sorteio& s, const Campo& c, Vector2 aluno);
    /// As gosmas: espalhadas na faixa do meio, longe do bau.
    std::vector<Vector2> LugaresDasGosmas(Variacao::Sorteio& s, int n, const Campo& c, Vector2 bau);

    /// Onde o professor esta 'tc' segundos depois de o ciclo comecar, saindo de 'de': rente ao alto
    /// ate a coluna do bau, e entao descendo ate ele - chegando em 'chegada'. Ele nao corta caminho, e
    /// quem vai reto ate o bau sem a chave so ganha tempo se ja tiver a chave.
    Vector2 CaminhoDoProfessor(Vector2 de, Vector2 bau, float tc, float chegada, const Campo& c);

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
