//
// O escudo do Hugo e o objetivo de cada etapa do TCC.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Orientador/Golpe.h"
#include "../../Core/Math.h"
#include "../../Gameplay/Variacao.h"

/**
 * CAMADA PURA. Coberto por test_objetivo.cpp.
 *
 * O TCC AINDA NAO ESTA PRONTO, e enquanto nao estiver o Hugo e blindado: o
 * tiro que o acerta so carrega o poder do orientador, e nao paga nota. Em cada
 * etapa o aluno cumpre um OBJETIVO - derrubar os alvos que o Hugo poe em campo
 * - e abre uma JANELA de kJanela segundos em que o Hugo toma dano. Acabada a
 * janela, o escudo volta e vem outro objetivo.
 *
 *   a proposta        as ASSINATURAS (do orientador, da coordenacao, da banca)
 *   o desenvolvimento os BUGS dos experimentos, que andam e aguentam dois tiros
 *   a defesa          a RESPOSTA CERTA a pergunta da banca: o placar escreve a
 *                     pergunta, e tres baloes - A, B e C, cada um com a
 *                     resposta escrita embaixo - voam no campo. A errada
 *                     custa: quem perguntou ataca.
 *   as correcoes      as CORRECOES em vermelho na versao final
 *
 * Os alvos sao projeteis do Hugo, DERRUBAVEIS, no posto "objetivo" - os poderes
 * do orientador nao os apagam (Orientador::TirosSoltos), e o Hugo os move.
 *
 * DERRUBAR E UMA DAS DUAS MANEIRAS de cada etapa; a outra e de ANDAR (Tipo,
 * mais abaixo), e as duas se alternam.
 *
 * O ESCUDO SE LE SEM TEXTO: e feito de PLACAS (Placas, PlacasDePe), uma por
 * coisa a fazer, que caem com o progresso - o Hugo as desenha, com um fio ate
 * cada coisa que ainda o segura.
 */
namespace Objetivo {

    enum class Etapa { Nenhuma, Proposta, Desenvolvimento, Defesa, Correcoes };

    /// "StateOne" e a proposta, e assim por diante.
    Etapa DaFase(const std::string& fase);

    constexpr const char* kPostoDoObjetivo = "objetivo";
    constexpr const char* kGatilhoDoAlvo = "alvo";     ///< assinatura, resposta, correcao
    constexpr const char* kGatilhoDoBug = "bug";       ///< o bug, que aguenta dois tiros

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

    /// @brief Os alvos de um objetivo novo da 'etapa', numa janela de
    /// 'largura'. Na defesa, 'certa' (0 a 2) e a resposta certa.
    std::vector<Alvo> Gerar(Etapa etapa, Variacao::Sorteio& s, float largura, Dificuldade::Nivel n, int certa = 0);

    /// @brief Para onde o alvo 'k' vai agora, 't' segundos depois de nascer em
    /// 'lugar': as assinaturas e as respostas balancam, as correcoes boiam. O
    /// bug anda por conta propria (Paragem).
    Vector2 Balanco(Etapa etapa, const Vector2& lugar, int k, float t);

    /// @brief A proxima parada de um bug: um ponto qualquer da faixa.
    Vector2 Paragem(Variacao::Sorteio& s, float largura);
    constexpr float kRapidezDoBug = 130.0f;

    // ---- A DEFESA ------------------------------------------------------------------

    /// Uma pergunta da banca, com as tres respostas na ordem do placar.
    struct Questao {
        std::string pergunta;
        std::array<std::string, 3> respostas;
        int certa = 0;
    };

    /// @brief A pergunta do professor 'm', com as respostas embaralhadas.
    Questao Perguntar(Golpe::Materia m, Variacao::Sorteio& s);

    // ---- A JANELA ------------------------------------------------------------------

    /// O QUE O OBJETIVO PAGA, em nota bruta (Battle::Conquistar): cada alvo
    /// derrubado, e o escudo aberto. Sem isso, blindado, o Hugo so tirava nota
    /// - e o aluno que tomava uns acertos antes da primeira janela zerava.
    /// O mesmo nos dois modos: o Dificil ja cobra pelo acerto sofrido.
    float PorAlvo(Dificuldade::Nivel n);
    float PorJanela(Dificuldade::Nivel n);

    /// ABERTO, A MONOGRAFIA SE DESFAZ: as paginas escorrem retas embaixo do
    /// Hugo, uma a cada tanto. A janela e a hora de acertar, e ficar parado
    /// embaixo dele, atirando, custa.
    float EntrePaginasSoltas(Dificuldade::Nivel n);

    /// @brief Quantos segundos o Hugo fica aberto depois de um objetivo.
    float Janela(Etapa etapa, Dificuldade::Nivel n);
    constexpr float kRecomeco = 0.8f;    ///< da janela fechada ao objetivo seguinte

    /// @brief O que o placar diz do objetivo, faltando 'faltam' alvos.
    std::string Falta(Etapa etapa, int faltam);

    // ---- AS OUTRAS MANEIRAS DE ABRIR -----------------------------------------------
    //
    // DERRUBAR NAO E O UNICO JEITO. Cada etapa tem duas maneiras de abrir o
    // escudo, e os objetivos dela se alternam - a primeira e sorteada pela
    // luta. A outra maneira e de ANDAR, e nao de atirar: o aluno vai ate algum
    // lugar, no meio dos tiros, e o que ele pega e enfeite (o tiro atravessa).
    //
    //   a proposta        a REUNIAO DE ORIENTACAO: o orientador marca um
    //                     lugar no chao, voa ate la, e o aluno fica com ele o
    //                     tempo da reuniao - o anel enche enquanto ele esta
    //                     dentro, e nao esvazia quando ele sai
    //   o desenvolvimento as REFERENCIAS: livros espalhados embaixo, e o aluno
    //                     passa por cima de cada um
    //   a defesa          a ARGUICAO: sustentar as perguntas - cada tiro em que
    //                     o aluno RASPA, sem ser acertado, conta uma
    //   as correcoes      a ENTREGA: pegar a versao final e levar ate a
    //                     secretaria; acertado no caminho, ele a deixa cair ali

    enum class Tipo { Derrubar, Reuniao, Referencias, Arguicao, Entrega };

    /// @brief A maneira de andar da 'etapa'.
    Tipo DeAndar(Etapa etapa);

    /// @brief A maneira do objetivo numero 'vez' da etapa: alternam, a partir
    /// de 'primeira' (0 derruba primeiro, 1 anda primeiro).
    Tipo DaVez(Etapa etapa, int vez, int primeira);

    /// O CHAO DO ALUNO, onde ficam os objetivos de andar: abaixo da faixa dos
    /// alvos, mas acima do pe dele - ninguem vai buscar nada no rodape.
    constexpr float kChaoDeCima = 500.0f;
    constexpr float kChaoDeBaixo = 640.0f;

    // -- a reuniao
    constexpr float kRaioDaReuniao = 80.0f;
    /// @brief Quantos segundos dentro do anel a reuniao pede.
    float TempoDeReuniao(Dificuldade::Nivel n);
    /// @brief Onde a reuniao e marcada: no chao, LONGE de onde o aluno esta
    /// ('xDoAluno') - quem ja estava la nao ganharia nada de graca.
    Vector2 LugarDaReuniao(Variacao::Sorteio& s, float largura, float xDoAluno);

    // -- as referencias
    constexpr float kRaioDaColeta = 42.0f;
    /// @brief Os livros, espalhados pelo chao.
    std::vector<Vector2> Referencias(Variacao::Sorteio& s, float largura, Dificuldade::Nivel n);

    // -- a arguicao
    /// QUANTO E RASPAR: passar a menos disto da borda de um tiro sem encostar.
    constexpr float kFolgaDaRaspada = 34.0f;
    /// @brief Quantas raspadas a arguicao pede.
    int Raspadas(Dificuldade::Nivel n);
    /// @brief Se o tiro a 'distancia' do aluno (de centro a centro) o raspou,
    /// sendo 'raios' a soma dos dois raios.
    bool Raspou(float distancia, float raios);

    // -- a entrega
    constexpr float kRaioDaSecretaria = 70.0f;
    struct Entrega {
        Vector2 versao;       ///< onde a versao final aparece
        Vector2 secretaria;   ///< onde ela e entregue: um canto do chao
    };
    /// @brief A versao final aparece de um lado, e a secretaria fica no canto
    /// do OUTRO: a entrega atravessa o campo.
    Entrega Entregar(Variacao::Sorteio& s, float largura);

    /// @brief Quantas PLACAS o escudo tem para cada objetivo - uma por coisa a
    /// fazer: os alvos certos, os livros, as raspadas; a reuniao tem cinco
    /// (um quinto do tempo cada) e a entrega duas (pegar e entregar).
    int Placas(Tipo tipo, Dificuldade::Nivel n, int alvosCertos);

    /// @brief Quantas placas continuam de pe com o objetivo 'progresso' (0 a 1)
    /// feito. Nenhuma so com ele inteiro.
    int PlacasDePe(int placas, float progresso);

    /// @brief O que cada passo do objetivo paga, em nota bruta: um alvo, um
    /// livro, uma raspada (meio alvo: sao mais), pegar ou entregar a versao. A
    /// reuniao paga dois de uma vez, no fim.
    float PorPasso(Tipo tipo, Dificuldade::Nivel n);

    /// @brief O que o placar diz de um objetivo de andar ('faltam' livros ou
    /// raspadas; na entrega, 1 antes de pegar e 0 com ela na mao).
    std::string Pede(Tipo tipo, int faltam);
}
