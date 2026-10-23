//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Automato.h"
#include "../../../Attacks/Gramatica.h"
#include "../../../Attacks/Parada.h"

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
 * A FASE 2 E UMA GRAMATICA LIVRE DE CONTEXTO (Gramatica.h), S -> aSb | e: ele
 * atira o S, e o S deriva em voo - vira um "a", um "b" e um S novo, que deriva
 * de novo, ate virar a palavra vazia. Aqui mora a derivacao de cada S em campo.
 *
 * A FASE 3 E A MAQUINA DE TURING (Fita.h): ele escreve a fita na passagem e a
 * reescreve na volta. Aqui mora so o placar, que diz a transicao com a seta do
 * sentido em que ele anda.
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

    // ---- A GRAMATICA DA FASE 2 ----------------------------------------------
    //
    // Todo S em campo (o posto "derivacao") tem a idade dele e quantas vezes
    // ainda deriva. O S que o professor atira pelo relogio nasce com um sorteio
    // (Gramatica::SortearDerivacoes); o que nasce de uma derivacao herda um a
    // menos. Com a idade de derivar: se ainda deriva, sai o par - um "a" e um
    // "b" - e um S novo no meio, e o velho some (os gatilhos "derivar" e
    // "variavel"); se nao, vira a palavra vazia, que nao machuca e some logo.

    void AtualizarGramatica(float deltaTime);

    struct Variavel {
        float idade = 0.0f;
        int restantes = 0;
        int cadeia = 0;   ///< de qual S do professor ele descende - o placar segue o ultimo
    };
    std::unordered_map<const BossProjectile*, Variavel> mVariaveis;

    /// A forma sentencial de cada cadeia, e qual e a do placar.
    std::unordered_map<int, std::string> mFormas;
    int mCadeiaDoPlacar = 0;
    int mProximaCadeia = 1;

    /// As palavras vazias na tela, e quanto falta para cada uma sumir.
    std::vector<std::pair<BossProjectile*, float>> mVazias;

    bool mNaGramatica = false;
    std::string mAnuncioDaGramatica;

    /// Derivando: de onde e para onde sai o que nasce, e o S que nasceu.
    bool mDerivando = false;
    Vector2 mDisparoDaDerivacao;
    float mRumoDaDerivacao = 90.0f;
    BossProjectile* mNascida = nullptr;

    // ---- A FITA DA FASE 3 ---------------------------------------------------
    //
    // A cabeca vai de ponta a ponta (o movimento "Conduzido") escrevendo uma
    // casa a cada Fita::kEscrita (o gatilho "escrever"); na ponta le o branco,
    // troca de estado e para Fita::kPausaNaPonta antes de voltar. O placar e a
    // transicao que ela esta fazendo.

    void AtualizarFita(float deltaTime);
    bool mNaFita = false;
    int mSentidoDaFita = 1;   ///< +1 direita, -1 esquerda
    float mPausaNaPonta = 0.0f;
    float mProximaEscrita = 0.0f;
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
