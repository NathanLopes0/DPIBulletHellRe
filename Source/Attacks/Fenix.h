//
// A final do Lucas (INF 321): o Phoenix, o framework do Elixir - e a fenix.
//

#pragma once

#include <array>

#include "Dom.h"
#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"

/// A repescagem e o Phoenix. A fenix voa um oito deitado sobre o campo, e o meio do oito anda atras
/// do aluno com atraso, como o LiveView, que mantem a pagina em dia com o servidor. Ela solta penas de
/// fogo, e de tempos em tempos faz um broadcast: o PubSub manda a mensagem a quatro sockets nos cantos
/// do campo, e cada um responde com um anel ao mesmo tempo. O tiro do aluno a derruba (ela aguenta
/// muitos); derrubada, vira um monte de cinzas, e em kRenascer segundos renasce delas, mais forte, com
/// um anel de fogo - a menos que o aluno apague as cinzas a tempo, e ai ela demora a voltar.
namespace Fenix {

    constexpr const char* kFase = "StateFinal";

    using Campo = Dom::Campo;

    struct Degrau {
        float periodoDoOito = 6.0f;      ///< uma volta do oito
        float entrePenas = 1.1f;
        int penas = 4;
        float velocidadeDasPenas = 200.0f;
        float entreBroadcasts = 4.0f;
        int tirosDoSocket = 8;
        float velocidadeDoSocket = 150.0f;
        int tirosDoRenascimento = 16;
        float velocidadeDoRenascimento = 190.0f;
        /// Cada renascimento a deixa mais rapida assim (as penas e o oito).
        float maisRapidaPorVersao = 1.15f;
    };
    Degrau DegrauDo(Dificuldade::Nivel nivel);

    /// As cinzas: quanto esperam para a fenix renascer, e quanto ela demora se o aluno as apaga.
    constexpr float kRenascer = 3.5f;
    constexpr float kApagada = 4.0f;
    /// O broadcast: a linha do PubSub aos sockets aparece isto antes de eles atirarem.
    constexpr float kAvisoDoBroadcast = 0.6f;
    /// O que vale para o aluno: derrubar a fenix, e apagar as cinzas (Battle::Conquistar).
    constexpr float kPremioPorDerrubar = 3.0f;
    constexpr float kPremioPorApagar = 2.0f;

    /// O meio do oito anda atras do x do aluno, devagar (o atraso do LiveView).
    constexpr float kRapidezDoLiveView = 90.0f;
    float SeguirX(float atual, float aluno, float dt);

    /// Onde a fenix esta 't' segundos depois de comecar o voo, com o meio do oito em 'meioX'. A cada
    /// versao (renascimento) ela voa mais rapido.
    Vector2 NoVoo(float t, float meioX, const Campo& c, float periodo, int versao, float maisRapidaPorVersao);

    /// Os quatro sockets do PubSub: dois nas laterais e dois no meio do campo - longe o bastante do
    /// pe dele para o anel ter tempo de abrir.
    std::array<Vector2, 4> Sockets(const Campo& c);
}
