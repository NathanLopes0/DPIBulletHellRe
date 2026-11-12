//
// Júlio — professor de Inteligência Artificial (INF 420)
//

#pragma once

#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Geracao.h"
#include "../../../Attacks/ReAct.h"
#include "../../../Attacks/Temperatura.h"

/// Boss da matéria INF 420. Tema: um modelo de linguagem, a área do professor.
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
    /// Gancho 1 — ajusta os parâmetros antes do disparo.
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

    /// Gancho 2 — roda depois de a rajada ser criada: na final, poe mais um "e" no placar a cada
    /// disparo.
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

    /// Na fase 3, o guardrail que comeu o tiro diz que nao pode ajudar...
    void OnProjetilBarrado() override;

    /// ...e o que caiu e um jailbreak.
    void OnProjetilDerrubado() override;

private:
    // ---- A luta sorteada ----

    void SortearALuta();

    [[nodiscard]] int EtapaDosTokens() const;

    bool mLutaSorteada = false;
    Temperatura::Temperaturas mTemperaturas = Temperatura::kDeSempre;
    ReAct::Degrau mDegrauDosAgentes;
    Variacao::Sorteio mSorteioDosAgentes{0u};

    static constexpr const char* kFaseDosTokens = "StateOne";

    // ---- A geracao da fase 1 (Geracao.h) ----

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

    /// O disparo de um token, na ponta: CustomizeAttackParams le daqui.
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
        class BossProjectile* p = nullptr;
        float idade = 0.0f;
        Vector2 lugar;
    };
    /// Na ordem do manager, e nao num mapa por ponteiro.
    std::vector<Agente> mAgentes;
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
