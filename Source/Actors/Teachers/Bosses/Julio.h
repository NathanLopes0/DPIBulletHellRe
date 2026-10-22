//
// Júlio — professor de Inteligência Artificial (INF 420)
//

#pragma once

#include <string>

#include "../Boss.h"
#include "../../../Attacks/ReAct.h"

/**
 * @class Julio
 * @brief Boss da matéria INF 420.
 *
 * Tema: um MODELO DE LINGUAGEM, a área do professor. A fase 1 é a geração de
 * tokens com temperatura (TokenAttack, Temperatura.h): o professor solta
 * frases, token por token, sorteadas em volta da mira. A fase 2 são os
 * AGENTES: robôs que ele convoca e que agem por ele no ciclo ReAct (ReAct.h) —
 * o relógio do ciclo e o placar moram aqui. A fase 3 são os GUARDRAILS: o
 * anel de robôs-escudo que barra o tiro, e o placar que recusa ("desculpe, não
 * posso ajudar com isso") e comemora o jailbreak. A final ainda é a de antes —
 * o modelo entra em loop.
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
     * @brief Gancho 2 — customiza os projéteis DEPOIS de criados.
     *
     * Pinta a rajada inteira com a animação da fase atual. Diferente do André,
     * que sorteia uma cor, aqui a cor é determinística: ela comunica ao jogador
     * em que fase da luta ele está.
     */
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

    /// Na fase 3, o guardrail que comeu o tiro diz que nao pode ajudar...
    void OnProjetilBarrado() override;

    /// ...e o que caiu e um jailbreak.
    void OnProjetilDerrubado() override;

private:
    /// O ciclo dos agentes num quadro: o relogio, o gatilho do Age e o placar.
    void AtualizarAgentes(float deltaTime);

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
    std::string mNomeDosGuardrails;
    std::string mFalaDoGuardrail;
    float mTempoDaFala = 0.0f;

    static constexpr float kTempoDaFala = 0.8f;
    static constexpr const char* kRecusa = "desculpe, não posso ajudar com isso";
    static constexpr const char* kJailbreak = "jailbreak!";
};
