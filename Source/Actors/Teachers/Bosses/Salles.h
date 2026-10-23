//
// O chefe da INF 213 - Estruturas de Dados.
//

#pragma once

#include <string>

#include "../Boss.h"
#include "../../../Attacks/Capivara.h"

/**
 * @class Salles
 * @brief O professor de Estruturas de Dados (INF 213).
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/salles/fases.json e as regras no regras.json ao lado. O que
 * mora aqui e o que dados nao sabem fazer.
 *
 * A CAPIVARA (Capivara.h). Ele poe capivara em todo exercicio, e de vez em
 * quando ela atravessa o campo no meio da fase - em qualquer fase dele. Ela e
 * um abrigo: come os projeteis dele e os tiros do aluno. Aqui mora o sorteio
 * de cada fase e o aviso no placar.
 */
class Salles : public Boss {

public:
    explicit Salles(Scene* scene);
    ~Salles() override = default;
    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams &params, const std::string &stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    /// Quantos graus a arvore da fase 3 erra o jogador DE PROPOSITO quando mira
    /// nele. Ver CustomizeAttackParams: e o que impede o corredor vazio do eixo
    /// de cair em cima de quem esta parado.
    ///
    /// 4 graus porque o jogador costuma estar a uns 450px do chefe, e meio
    /// espacamento de folha (30px) a essa distancia da pouco menos de 4 graus.
    static constexpr float kDesvioDaBusca = 4.0f;

    /// Para que lado a busca desceu da ultima vez. Alterna.
    float mLadoDaBusca = 1.0f;

    // ---- A CAPIVARA ---------------------------------------------------------
    //
    // A cada fase nova, um sorteio diz se ela vem, quando e de que lado. Na hora,
    // o Salles puxa o gatilho "capivara" e o placar avisa por um tempo.

    void AtualizarCapivara(float deltaTime);

    std::string mFaseDaCapivara;
    Capivara::Visita mVisita;
    bool mVeio = false;
    bool mDisparandoCapivara = false;

    /// O placar antes do aviso, e quanto falta para ele voltar.
    std::string mAnuncioAntesDaCapivara;
    float mAvisoDaCapivara = 0.0f;
};
