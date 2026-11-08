//
// O chefe da INF 250 - Organizacao de Computadores.
//

#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Logica.h"
#include "../../../Attacks/Relogio.h"
#include "../../../Attacks/Pipeline.h"
#include "../../../Attacks/Blink.h"

/// O professor de Organizacao de Computadores (INF 250). Ele tambem da Sistemas Logicos e
/// Arquitetura, e pesquisa hardware.
class Ricardo : public Boss {
public:
    explicit Ricardo(Scene* scene);
    ~Ricardo() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- O que esta luta sorteia, e o degrau ----

    void SortearALuta();
    bool mLutaSorteada = false;
    Relogio::Plano mPlanoDoRelogio;
    Relogio::Degrau mDegrauDoRelogio;
    Pipeline::Plano mPlanoDoPipeline;
    Pipeline::Degrau mDegrauDoPipeline;
    Blink::Degrau mDegrauDoBlink;
    // ---- As portas da fase 1 ----

    void AtualizarPortas();

    /// Os projeteis do chefe que estao no posto nomeado.
    [[nodiscard]] std::vector<BossProjectile*> NoPosto(const char* posto) const;

    /// A porta escrita no placar no instante 't' da fase 1 - o placar e a porta (Logica::LerPorta).
    [[nodiscard]] Logica::Porta PortaEm(float t) const;

    Logica::Porta mPorta;
    bool mPortasFormadas = false;

    /// Formando as portas: o CustomizeAttackParams nao sorteia palavra para a
    /// fileira delas.
    bool mDisparandoPortas = false;

    /// Quanto depois do comeco da fase as portas se formam: antes da primeira
    /// palavra, que sai a 1 s.
    static constexpr float kFormaAsPortas = 0.2f;

    // ---- O clock da fase 2 ----

    void AtualizarRelogio();

    /// Todo registrador em campo passa a andar a 'velocidade', na direcao
    /// dele (zero: parado).
    void MoverRegistradores(float velocidade);

    /// A direcao de cada registrador, anotada aqui e nao no vetor velocidade.
    std::unordered_map<const BossProjectile*, Vector2> mDirecoes;

    /// O tempo da fase no quadro anterior (negativo: fora da fase do clock).
    float mAntesNoRelogio = -1.0f;
    /// Quando o passo em curso acaba (negativo: nenhum passo em curso).
    float mFimDoPasso = -1.0f;
    /// A frequencia do ultimo quadro na fase - a dos registradores que sobram.
    float mFrequencia = 1.0f;
    /// O estado do contador Johnson: o do proximo leque.
    int mContador = 1;

    // ---- O pipeline da fase 3 ----

    void AtualizarPipeline(float deltaTime);

    /// Dispara o gatilho de dentro do estagio: o ataque sai dali, e nao do
    /// professor. 'mirar': o leque sai mirado no aluno.
    void DispararDoEstagio(const std::string& gatilho, int estagio, bool mirar);

    /// Onde fica cada estagio: o meio das molduras, da esquerda para a direita.
    /// Vazio enquanto elas nao se formaram.
    [[nodiscard]] std::vector<Vector2> CentrosDosEstagios() const;

    Pipeline::Estado mPipeline;
    /// O chip de cada estagio, ou nenhum - a bolha que o aluno fez a tiro nao
    /// tem chip; a do flush tem, e anda vazia ate o fim.
    std::array<BossProjectile*, Pipeline::kEstagios> mChips{};
    bool mNoPipeline = false;
    bool mEstagiosFormados = false;
    float mTempoNoPipeline = 0.0f;
    float mProximoCiclo = 0.0f;
    std::string mNomeDaFaseDoPipeline;

    /// Disparando de um estagio: de onde, se mira, e o chip que a busca criou.
    bool mDisparandoDoEstagio = false;
    bool mMirandoDoEstagio = false;
    bool mFormandoEstagios = false;
    Vector2 mDisparoDoEstagio;
    BossProjectile* mBuscada = nullptr;

    /// Quao depressa o chip anda de um estagio ao seguinte (Ancora::Aproximar).
    static constexpr float kRapidezDoChip = 12.0f;

    // ---- O blink da final ----

    void AtualizarBlink();

    /// O tempo da fase no quadro anterior (negativo: fora do Blink).
    float mAntesNoBlink = -1.0f;

    /// Quanto abaixo do professor a placa ja esta solta: a mais funda das
    /// presas fica a 100 px (regras.json), e a Ancorar nao passa do lugar.
    static constexpr float kSoltaAbaixo = 160.0f;
};
