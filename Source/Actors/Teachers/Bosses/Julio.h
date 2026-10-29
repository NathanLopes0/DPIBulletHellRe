//
// Júlio — professor de Inteligência Artificial (INF 420)
//

#pragma once

#include <string>
#include <unordered_map>

#include "../Boss.h"
#include "../../../Attacks/Julio/Geracao.h"
#include "../../../Attacks/Julio/ReAct.h"
#include "../../../Attacks/Shared/Temperatura.h"

/**
 * @class Julio
 * @brief Boss da matéria INF 420.
 *
 * Tema: um MODELO DE LINGUAGEM, a área do professor. A fase 1 é a geração de
 * tokens com temperatura (Geracao.h): cada frase se escreve um token de cada
 * vez, cada um nascendo na ponta dela, no rumo sorteado. A fase 2 são os

 * AGENTES: robôs que ele convoca e que agem por ele no ciclo ReAct (ReAct.h) —
 * o relógio do ciclo e o placar moram aqui. A fase 3 são os GUARDRAILS: o
 * anel de robôs-escudo que barra o tiro, e o placar que recusa ("desculpe, não
 * posso ajudar com isso") e comemora o jailbreak. A final é o modelo que
 * degenerou em repetição: só gera "e", e o placar repete junto.
 *
 * Toda a customização passa pelos dois ganchos do Boss; ExecuteAttack não é
 * sobrescrito.
 */
class Julio : public Boss {
public:
    explicit Julio(Scene* scene);
    ~Julio() override = default;

    void OnUpdate(float deltaTime) override;

    /// A fase dos agentes.
    static constexpr const char* kFaseDosAgentes = "StateTwo";

    /// A fase dos guardrails, a que decide a batalha.
    static constexpr const char* kFaseDosGuardrails = "StateThree";

    /// A repescagem: o modelo em loop.
    static constexpr const char* kFaseDoLoop = "StateFinal";

protected:
    /**
     * @brief Gancho 1 — ajusta os parâmetros ANTES do disparo.
     *
     * Fase 1 espalha a mira aleatoriamente (exploração), fase 2 aponta
     * exatamente para o jogador (convergência) e fase 3 aponta para o jogador
     * com um leque largo (variância alta).
     */
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

    /**
     * @brief Gancho 2 — roda DEPOIS de a rajada ser criada: na final, poe mais
     * um "e" no placar a cada disparo.
     */
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

    /// Na fase 3, o guardrail que comeu o tiro diz que nao pode ajudar...
    void OnProjetilBarrado() override;

    /// ...e o que caiu e um jailbreak.
    void OnProjetilDerrubado() override;

private:
    // ---- A LUTA SORTEADA ----------------------------------------------------
    //
    // Antes do primeiro quadro (Variacao.h): a temperatura de cada etapa da
    // fase 1, escrita no placar dela, e o degrau do modo. Na fase 2 o Pensa de
    // cada volta do ReAct e sorteado na hora, da sequencia da fase.

    void SortearALuta();

    /// A etapa da fase 1 em que o professor esta agora (0 fria, 1 morna, 2
    /// quente), pelo relogio da fase e pelos fins registrados.
    [[nodiscard]] int EtapaDosTokens() const;

    bool mLutaSorteada = false;
    Temperatura::Temperaturas mTemperaturas = Temperatura::kDeSempre;
    ReAct::Degrau mDegrauDosAgentes;
    Variacao::Sorteio mSorteioDosAgentes{0u};

    static constexpr const char* kFaseDosTokens = "StateOne";

    // ---- A GERACAO DA FASE 1 (Geracao.h) -------------------------------------
    //
    // Cada frase e gerada um token de cada vez, e cada token nasce na PONTA da
    // frase, no rumo sorteado com a temperatura da etapa. Da ponta sai, a cada
    // token, o leque da distribuicao: um raio por candidato, mais aceso quanto
    // mais provavel - a temperatura desenhada.

    void AtualizarTokens(float deltaTime);

    struct Frase {
        Vector2 ponta;          ///< onde esta o ultimo token gerado
        float rumo = 90.0f;     ///< o rumo dele, em graus
        int feitos = 0;
        int total = Geracao::kTokensDaFrase;
        float relogio = 0.0f;   ///< ate o proximo token
    };
    std::vector<Frase> mFrases;
    float mProximaFrase = 0.0f;
    bool mNosTokens = false;
    Geracao::Degrau mDegrauDaGeracao;

    struct Raio {
        Vector2 de;
        float rumo = 0.0f;
        float peso = 0.0f;      ///< a chance, relativa a do mais provavel
        bool escolhido = false;
        float idade = 0.0f;
    };
    std::vector<Raio> mRaios;
    class DrawLinhasComponent* mDistribuicao = nullptr;

    /// O disparo de UM token, na ponta: CustomizeAttackParams le daqui.
    bool mGerando = false;
    Vector2 mOndeGerar;
    float mRumoAGerar = 90.0f;
    float mVelocidadeDoToken = 300.0f;
    std::vector<class BossProjectile*> mGerados;

    static constexpr const char* kGatilhoDoToken = "token";
    static constexpr float kPrimeiraFrase = 1.0f;
    static constexpr float kVidaDoRaio = 0.35f;
    static constexpr float kTamanhoDoRaio = 70.0f;


    /// O ciclo dos agentes num quadro: o relogio, o gatilho do Age e o placar.
    void AtualizarAgentes(float deltaTime);

    /// Os agentes em campo andam: a cada volta replanejam (ReAct::Destinos) e
    /// voam ate o lugar novo enquanto pensam; com a vida acabada, vao embora
    /// por cima.
    void MoverAgentes(float deltaTime, bool replanejar);
    void SoltarAgente(class BossProjectile* agente);

    struct Agente {
        float idade = 0.0f;
        Vector2 lugar;
    };
    std::unordered_map<class BossProjectile*, Agente> mAgentes;
    static constexpr float kSaidaDoAgente = 320.0f;


    ReAct::Ciclo mCiclo;
    bool mNosAgentes = false;


    /// O anuncio da fase 2 em fases.json, guardado na primeira vez: volta ao
    /// placar quando a fase acaba.
    std::string mNomeDosAgentes;

    static constexpr const char* kPensando = "agente: pensando...";
    static constexpr const char* kAgindo = "agente: usar_ferramenta()";
    static constexpr const char* kObservando = "agente: observando";

    /// O placar da fase 3: o nome dela, e o que o guardrail diz quando barra
    /// um tiro ou quando cai.
    void AtualizarGuardrails(float deltaTime);
    [[nodiscard]] bool NosGuardrails() const;

    bool mNosGuardrails = false;

    /// Quantas ondas de guardrails ja sairam nesta fase (Guardrail::DaVez), e
    /// quanto cada anel fica amarrado - o "duracao" da regra "guardrails".
    int mOndaDosGuardrails = 0;
    static constexpr float kDuracaoDoAnel = 5.5f;

    std::string mNomeDosGuardrails;
    std::string mFalaDoGuardrail;
    float mTempoDaFala = 0.0f;

    static constexpr float kTempoDaFala = 0.8f;

    /// O placar da final: o nome dela no comeco, e um "e" a mais por disparo.
    void AtualizarLoop();
    bool mNoLoop = false;
    int mRepeticoes = 0;
    std::string mNomeDoLoop;
    static constexpr int kRepeticoesNoPlacar = 14;
    static constexpr const char* kRecusa = "desculpe, não posso ajudar com isso";
    static constexpr const char* kJailbreak = "jailbreak!";
};
