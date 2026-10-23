//
// Andre - professor de Programacao 1 (INF 110)
//

#pragma once

#include <optional>
#include <string>
#include <vector>

#include <SDL_rect.h>

#include "../Boss.h"
#include "../../../Attacks/Condicional.h"

class BossProjectile;
class DrawLinhasComponent;

/**
 * @class Andre
 * @brief O chefe da primeira materia do curso: o placar e o codigo que ele esta
 *        rodando, e o campo e a execucao dele.
 *
 * Os BALOES DO BOCA ficam: no BOCA, o sistema em que a turma entrega os
 * exercicios, cada problema resolvido vale um balao da cor dele.
 *
 * A FASE 1 E O DESVIO CONDICIONAL (Condicional.h): o fluxograma desenhado no
 * campo - o losango embaixo do professor e um ramo por faixa, com o
 * cabecalho no alto -, e os baloes numerados que seguem pelo ramo que a
 * condicao escolhe e estouram em confete na faixa dele.
 */
class Andre : public Boss {

public:
    explicit Andre(Scene* scene);
    ~Andre() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

    // Antes o Andre sobrescrevia ExecuteAttack inteiro so para escolher a cor
    // dos baloes. Agora sobrescreve apenas o gancho, e herda automaticamente
    // qualquer correcao feita no fluxo da base.
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    [[nodiscard]] SDL_FRect Campo() const;

    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram.
    /// 'quantos' > 0 troca o numero de projeteis do ataque.
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, int quantos = 0);

    bool mDisparandoEm = false;
    Vector2 mOndeDisparar;
    int mQuantos = 0;
    std::vector<BossProjectile*> mCriados;

    std::optional<Vector2> mAlvo;

    // ---- A FASE 1: O DESVIO CONDICIONAL -------------------------------------

    void AtualizarCondicional(float deltaTime);
    void NovoComando(float agora);
    void SoltarBalao();
    void AtualizarBaloes(float deltaTime, float agora);

    /// O balao estourou no ramo: o confete cai pela faixa dele, de cima.
    void Estourar(const Condicional::Comando& c, int ramo, int cor, float agora);

    /// O losango embaixo do professor, onde a condicao e avaliada.
    [[nodiscard]] Vector2 Decisao() const;

    /// O caminho do losango ate o cabecalho do ramo, em angulo reto, como
    /// num fluxograma: de lado ate a faixa, e para cima.
    [[nodiscard]] std::vector<Vector2> Caminho(const Condicional::Comando& c, int ramo) const;
    [[nodiscard]] Vector2 CabecalhoDoRamo(const Condicional::Comando& c, int ramo) const;

    void DesenharFluxograma(float agora);
    void DesmontarCondicional();

    bool mNoSe = false;
    std::string mAnuncioDoSe;
    int mVez = 0;
    float mProximoComando = 0.0f;
    Condicional::Comando mComando;
    int mBaloesDoComando = 0;
    float mProximoBalao = 0.0f;

    std::vector<BossProjectile*> mRotulos;     ///< um cabecalho por ramo
    std::vector<float> mAcesoAte;              ///< ate quando cada um fica aceso

    enum class Etapa { Descendo, Decidindo, Voando, Saindo };
    struct Balao {
        BossProjectile* p = nullptr;
        Condicional::Comando c;
        int n = 0;
        int cor = 0;
        Etapa etapa = Etapa::Descendo;
        float ate = 0.0f;
        std::vector<int> ramos;           ///< os que executam, em ordem
        std::vector<Vector2> caminho;     ///< o que falta andar ate o cabecalho
    };
    std::vector<Balao> mBaloes;

    struct Queda { Condicional::Comando c; int ramo = 0; int cor = 0; float quando = 0.0f; };
    std::vector<Queda> mQuedas;               ///< os cases de baixo, no switch

    DrawLinhasComponent* mFluxograma = nullptr;
    float mDecisaoAte = 0.0f;
    int mDecisaoRamo = -1;

    static constexpr int kOrdemDoFluxograma = 80;
};
