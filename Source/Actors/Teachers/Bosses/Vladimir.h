//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Automato.h"
#include "../../../Attacks/Fita.h"
#include "../../../Attacks/Compilador.h"
#include "../../../Attacks/Parada.h"

class DrawLinhasComponent;

/**
 * @class Vladimir
 * @brief O professor de Linguagens Formais e Automatos (INF 331).
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/vladimir/fases.json e as regras de projetil no regras.json ao
 * lado. O que mora aqui e o que dados nao sabem fazer.
 *
 * A IDEIA DO CHEFE, para quem for mexer nas fases: ele e o professor de
 * LINGUAGENS nos dois sentidos - as formais da disciplina e as estrangeiras da
 * diretoria de Relacoes Internacionais. Dai vem o que o separa dos outros: os
 * outros surpreendem, e ele ANUNCIA. Cada rajada e uma palavra de uma
 * linguagem, e os projeteis sao os simbolos dela.
 *
 * O PROGRAMA DE INTERCAMBIO VALE A LUTA INTEIRA: em toda fase os alunos trocam
 * de lugar de tempos em tempos (Intercambio.h), e no mesmo quadro as letras em
 * voo trocam tambem - o homomorfismo h(a) = b, h(b) = a (Alfabeto.h). O "a" que
 * vinha reto passa a perseguir, e o "b" que perseguia segue reto. E o que mora
 * aqui: OnIntercambio.
 *
 * A FASE 1 E UM AUTOMATO FINITO (Automato.h), desenhado no campo: o diagrama
 * das transicoes e os tres estados. O professor e a cabeca do automato - le a
 * palavra do placar um simbolo por vez, atira a letra que leu e voa para o
 * estado que a tabela manda (o movimento "Conduzido", que pergunta aqui o
 * destino). Lida a palavra, aceita ou rejeita.
 *
 * A FASE 2 E UM COMPILADOR (Compilador.h) - o Vladimir tambem da Construcao de
 * Compiladores. Tres linhas de codigo por fase, "x = a + b * 2;", e cada uma
 * passa pelas tres etapas na tela: o lexico solta os tokens caindo, o
 * sintatico monta a arvore no campo, e a geracao de codigo transforma cada no
 * numa instrucao mirada no aluno. Aqui mora o relogio de cada rodada, a arvore
 * em campo e as arestas dela.
 *
 * A FASE 3 E UMA MAQUINA DE TURING DE VERDADE (Fita.h): o castor mais ocupado
 * de 4 estados, que da 107 passos e para com 13 marcas na fita. Cada passo
 * vira tiro - a marca escrita fica, a apagada persegue, a relida atira uma
 * copia. Aqui moram o relogio dos passos, as marcas em campo e a cabeca
 * desenhada.
 *
 * A REPESCAGEM E O PROBLEMA DA PARADA (Parada.h): ele roda o programa de
 * Collatz, um passo por vez, na altura do valor de x - e ninguem sabe provar
 * que ele sempre para.
 */
class Vladimir : public Boss {
public:
    explicit Vladimir(Scene* scene);
    ~Vladimir() override = default;

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] std::string AvisoDoIntercambio() const override;
    void OnIntercambio() override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;

    /// O Tracking que o "a" ganha ao virar "b" na troca: a forca e a duracao do
    /// "b" da fase 1, que e o que o aluno aprendeu a esperar de um "b". O atraso
    /// e curto - a letra ja esta em voo, e o aviso veio antes.
    static constexpr float kAtrasoDaTroca = 0.15f;
    static constexpr float kForcaDaTroca = 1.4f;
    static constexpr float kDuracaoDaTroca = 2.2f;

    /// As caras dos estados, apagados e o aceso (projeteis.json, "Estado").
    [[nodiscard]] static std::string CaraDoEstado(int q, bool aceso);

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A LUTA SORTEADA ----------------------------------------------------
    //
    // Antes do primeiro quadro (Variacao.h): a fita - o castor como ele e, ou
    // espelhado -, a ordem das sementes de Collatz e
    // o lado para onde o professor comeca a varrer na repescagem, as tres linhas
    // que o compilador compila, e o degrau do modo em cada fase. A palavra do
    // automato continua sorteada na hora, como era.

    void SortearALuta();

    bool mLutaSorteada = false;
    Compilador::Plano mPlanoDoCompilador;
    Compilador::Degrau mDegrauDoCompilador;
    Fita::Plano mPlanoDaFita;
    Fita::Degrau mDegrauDaFita;

    Parada::Plano mPlanoDaParada;
    Parada::Degrau mDegrauDaParada;
    int mProgramaDaVez = 0;

    // ---- O AUTOMATO DA FASE 1 -----------------------------------------------
    //
    // Logo que a fase comeca o Vladimir desenha o diagrama e os tres estados (os
    // gatilhos "diagrama" e "estados"). Depois, a cada passo: le um simbolo,
    // atira a letra lida do estado em que esta (os gatilhos "ler_a" e "ler_b") e
    // voa para o seguinte. Lida a palavra, aceita (um anel do estado de
    // aceitacao) ou rejeita (perseguidores), e comeca outra do q0.

    void AtualizarAutomato();

    /// Os projeteis do chefe que estao no posto nomeado.
    [[nodiscard]] std::vector<BossProjectile*> NoPosto(const char* posto) const;

    [[nodiscard]] static Vector2 CentroDoEstado(int q);

    /// Dispara 'gatilho' de 'onde', com a cara 'cara' no que nascer.
    void Formar(const char* gatilho, Vector2 onde, std::string cara);

    enum class Momento { Anuncio, Lendo, Veredito };

    Automato::Leitura mLeitura;
    Momento mMomento = Momento::Anuncio;
    bool mNoAutomato = false;
    bool mAutomatoFormado = false;
    float mProximoPasso = 0.0f;
    std::string mAnuncioDoAutomato;   ///< o da fase em fases.json, devolvido no fim

    std::optional<Vector2> mAlvo;

    /// Formando o automato: de onde sai o que nasce, e com que cara.
    bool mFormando = false;
    Vector2 mDisparoDoFormando;
    std::string mCaraDoFormando;

    /// Quanto depois do comeco da fase o automato se desenha, e quando a
    /// primeira palavra comeca: o placar mostra a linguagem ate la.
    static constexpr float kFormaOAutomato = 0.2f;
    static constexpr float kPrimeiraPalavra = 1.6f;

    /// Quanto dura cada simbolo lido, e o veredito no fim da palavra.
    static constexpr float kPasso = 1.1f;
    static constexpr float kVeredito = 1.3f;

    // ---- O COMPILADOR DA FASE 2 ---------------------------------------------
    //
    // Tres rodadas, uma linha da luta em cada (Compilador::Plano), no relogio de
    // Compilador::MarcosDa. O LEXICO solta os tokens um a um da linha, caindo
    // em coluna (os gatilhos "lexema", "ler_a" e "ler_b"); o SINTATICO para a
    // fila de tokens e monta a arvore com ela, em pos-ordem: cada ficha voa de
    // onde parou ate o lugar dela na arvore (o a e o b, que seguiram caindo
    // como letras, nascem de novo na linha - o gatilho "arvore"), e cada
    // operador reduzido solta um leque dele mesmo (o gatilho "reduzir"); a
    // GERACAO DE CODIGO percorre a arvore de novo, e cada no some virando a
    // instrucao dele, mirada no aluno (o gatilho "emitir") - o = por ultimo, no
    // anel do STORE (o gatilho "store"). O professor varre a linha no lexico e
    // depois fica em cima da raiz.


    void AtualizarCompilador(float deltaTime);
    void ComecarRodada(int rodada, float inicio);
    void DesenharArestas();

    /// Dispara 'gatilho' de 'onde', no rumo 'rumo' - ou mirado no aluno, sem
    /// rumo -, com a cara 'cara' no que nascer. Devolve o que nasceu. O
    /// compilador e a fita atiram por aqui.
    std::vector<BossProjectile*> DispararDe(const char* gatilho, Vector2 onde, std::optional<float> rumo,
                                            const std::string& cara);

    [[nodiscard]] Vector2 PosicaoDoAluno() const;

    bool mNoCompilador = false;
    std::string mAnuncioDoCompilador;
    int mRodada = -1;                    ///< -1 antes da primeira
    float mInicioDaRodada = 0.0f;        ///< no relogio da fase
    std::string mLinha;
    std::vector<Compilador::Token> mTokens;
    Compilador::Arvore mArvore;
    std::vector<int> mOrdem;             ///< a pos-ordem: a montagem e a geracao
    std::vector<Compilador::Ponto> mNaLinha;
    std::vector<Compilador::Ponto> mNaArvore;
    Compilador::Marcos mMarcos;
    Compilador::Etapa mEtapa = Compilador::Etapa::Pronto;
    int mLidos = 0;
    int mMontados = 0;
    int mEmitidos = 0;
    std::vector<BossProjectile*> mNos;   ///< o projetil de cada no em campo, por indice
    std::vector<BossProjectile*> mLexemas;   ///< a ficha de cada token caindo, por indice
    bool mFilaParada = false;            ///< os tokens ja pararam para o parser

    /// As arestas da arvore, e a cabeca da fita: um desenho so, que cada fase
    /// limpa ao sair.
    DrawLinhasComponent* mLinhas = nullptr;
    void CriarLinhas();

    /// Disparando de um lugar (DispararDe): de onde e para onde sai o que
    /// nasce, com que cara, e quem nasceu.
    bool mDisparandoDe = false;
    Vector2 mOndeDisparar;
    float mRumoDoDisparo = 90.0f;
    std::string mCaraDoDisparo;
    std::vector<BossProjectile*> mDisparados;

    /// Quao depressa um no chega ao lugar dele na arvore (Ancora::Aproximar).
    static constexpr float kRapidezDoNo = 9.0f;

    // ---- A FITA DA FASE 3 ---------------------------------------------------
    //
    // O castor mais ocupado de 4 estados (Fita.h), um passo a cada
    // Fita::kPasso: a cabeca le a casa, e o que a tabela manda vira tiro - a
    // marca nova fica na fileira (o gatilho "escrever"), a apagada sai
    // perseguindo ("apagar") e a relida atira uma copia ("reler"); parada, um
    // anel ("pare"). A cabeca e o quadro desenhado em volta da casa; o
    // professor a segue de cima, mais devagar (o movimento "Conduzido"), com
    // uma linha ate ela.

    void AtualizarFita();
    void DesenharCabeca();
    bool mNaFita = false;
    Fita::Estado mFita;
    float mProximoPassoDaFita = 0.0f;
    std::array<BossProjectile*, Fita::kCasas> mMarcas{};   ///< a marca de cada casa, se houver
    std::string mAnuncioDaFita;

    // ---- O PROBLEMA DA PARADA DA REPESCAGEM ---------------------------------
    //
    // A cada passo: atira conforme a paridade de x (os gatilhos "metade" e
    // "triplo"), da o passo e escreve o placar; no 1, o programa parou (o
    // gatilho "parou"), e depois de um tempo roda outra semente. O professor
    // vai de lado a lado, na altura do valor de x (Parada::Altura).

    void AtualizarParada(float deltaTime);
    bool mNaParada = false;
    std::uint32_t mX = 0;     ///< o x do programa; 0 = nenhum rodando
    float mProximoPassoDaParada = 0.0f;
    float mXDaParada = 0.0f;
    float mSentidoDaParada = 1.0f;
    std::string mAnuncioDaParada;

    /// De lado a lado, devagar: o tiro do aluno leva quase 1s ate ele.
    static constexpr float kVarreduraDaParada = 110.0f;
    static constexpr float kMargemDaParada = 170.0f;
};
