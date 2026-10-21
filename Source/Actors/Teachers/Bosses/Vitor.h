//
// O chefe da INF 310 - Programacao Concorrente e Distribuida.
//

#pragma once

#include "../Boss.h"

/**
 * @class Vitor
 * @brief O professor de Programacao Concorrente e Distribuida (INF 310).
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/vitor/fases.json e as regras de projetil no regras.json ao
 * lado. O que mora aqui e a UNICA coisa que dados nao sabem fazer - mudar a
 * direcao da fresta da secao critica entre uma rajada e a outra.
 *
 * A IDEIA DO CHEFE: concorrencia e "varias coisas ao mesmo tempo", e e por isso
 * que a fase 3 dele se divide em dois corpos. As duas primeiras fases existem
 * para serem aprendidas SEPARADAS, porque a terceira as poe juntas:
 *
 *   fase 1  TAREFAS EM PARALELO - varias faixas descendo de uma vez
 *   fase 2  EXCLUSAO MUTUA      - um anel com UMA fresta, sempre embaixo
 *   fase 3  FORK                - dois corpos, um fazendo cada uma das duas
 */
class Vitor : public Boss {
public:
    explicit Vitor(Scene* scene);
    ~Vitor() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

private:
    /**
     * Para onde a fresta do anel aponta, medido a partir da ponta da janela
     * (kFrestaMaisADireita). COMECA NO MEIO, entao o primeiro anel da fase abre
     * reto para baixo e so depois a fresta passa a escorregar para os lados.
     */
    float mDesvioDaFresta = 30.0f;

    /**
     * A JANELA E A METADE DE BAIXO, E NAO A VOLTA INTEIRA. A fresta vai de 60
     * a 120 graus (90 e reto para baixo): no maximo 30 graus inclinada, que na
     * faixa em que o jogador vive - de 330 a 550px abaixo do chefe, conforme a
     * orbita - desloca a porta ate uns 320px para o lado, e ela fica entre
     * x=171 e x=1029 numa tela de 1200. Uma fresta apontando para cima nao
     * seria porta nenhuma, so mais parede.
     */
    static constexpr float kFrestaMaisADireita = 60.0f;
    static constexpr float kJanelaDaFresta = 60.0f;

    /**
     * 37 GRAUS POR RAJADA, PRIMO COM A JANELA DE 60. A fresta zigue-zagueia
     * (90, 67, 104, 81, 118, 95...) e so repete depois de 60 rajadas, entao o
     * jogador nao decora o ciclo. E o mesmo motivo dos 137 graus que a fresta
     * girava antes, quando dava a volta inteira.
     */
    static constexpr float kPassoDaFresta = 37.0f;
};
