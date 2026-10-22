//
// O chefe da INF 250 - Organizacao de Computadores.
//

#pragma once

#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Logica.h"

/**
 * @class Ricardo
 * @brief O professor de Organizacao de Computadores (INF 250). Ele tambem da
 * Sistemas Logicos e Arquitetura, e pesquisa hardware.
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/ricardo/fases.json e as regras no regras.json ao lado. O que
 * mora aqui e o que dados nao sabem fazer.
 *
 * A FASE 1 E SISTEMAS LOGICOS (Logica.h): palavras de 16 bits descem como
 * paredes - o UM machuca, o ZERO se atravessa - e cruzam uma fileira de portas
 * logicas, uma por coluna, que transformam cada bit. Aqui moram as portas (a
 * cara de cada uma, conforme a etapa), a travessia, e o sorteio da palavra.
 */
class Ricardo : public Boss {
public:
    explicit Ricardo(Scene* scene);
    ~Ricardo() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

private:
    // ---- AS PORTAS DA FASE 1 ------------------------------------------------
    //
    // Logo que a fase comeca o Ricardo forma a fileira de portas (o gatilho
    // "portas"), e dali em diante, a cada quadro: poe em cada porta a cara da
    // operacao da etapa, e transforma todo bit que chega na altura delas. SO O
    // CORPO ORIGINAL - ele nao se divide, mas um clone nao teria portas suas.

    void AtualizarPortas();

    /// Os projeteis do chefe que estao no posto nomeado.
    [[nodiscard]] std::vector<BossProjectile*> NoPosto(const char* posto) const;

    /// A PORTA ESCRITA NO PLACAR no instante 't' da fase 1 - o placar e a
    /// porta (Logica::LerPorta). Se o placar daquele instante nao for porta, a
    /// ultima que valeu.
    [[nodiscard]] Logica::Porta PortaEm(float t) const;

    /// O que as portas fazem agora.
    Logica::Porta mPorta;
    bool mPortasFormadas = false;

    /// Formando as portas: o CustomizeAttackParams nao sorteia palavra para a
    /// fileira delas.
    bool mDisparandoPortas = false;

    /// Quanto depois do comeco da fase as portas se formam: antes da primeira
    /// palavra, que sai a 1 s.
    static constexpr float kFormaAsPortas = 0.2f;
};
