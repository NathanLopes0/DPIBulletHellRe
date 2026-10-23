//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#pragma once

#include "../Boss.h"

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
 */
class Vladimir : public Boss {
public:
    explicit Vladimir(Scene* scene);
    ~Vladimir() override = default;

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] std::string AvisoDoIntercambio() const override;
    void OnIntercambio() override;

    /// O Tracking que o "a" ganha ao virar "b" na troca: a forca e a duracao do
    /// "b" da fase 1, que e o que o aluno aprendeu a esperar de um "b". O atraso
    /// e curto - a letra ja esta em voo, e o aviso veio antes.
    static constexpr float kAtrasoDaTroca = 0.15f;
    static constexpr float kForcaDaTroca = 1.4f;
    static constexpr float kDuracaoDaTroca = 2.2f;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
};
