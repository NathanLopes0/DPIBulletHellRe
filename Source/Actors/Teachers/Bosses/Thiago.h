//
// Thiago - professor de Banco de Dados (INF 220)
//

#pragma once

#include "../Boss.h"

/**
 * @class Thiago
 * @brief O chefe que trata o campo de jogo como uma TABELA e o jogador como uma
 *        linha dela.
 *
 * A IDEIA DA BATALHA. Os outros chefes miram no jogador; o Thiago CONSULTA. Cada
 * ataque e uma varredura de uma faixa inteira da tabela, anunciada antes de
 * disparar. O jogador nao desvia de projetil - ele sai da linha que foi
 * escolhida. A pergunta que a batalha faz nao e "voce consegue passar entre as
 * balas?", e sim "voce consegue ler a consulta a tempo?".
 *
 * O QUE CADA FASE ENSINA, na mesma logica das fases do Julio:
 *
 *   StateOne   SELECT sem WHERE - varredura completa, linha por linha, em ordem.
 *              Nao sabe onde voce esta e nem precisa: passa por todas.
 *
 *   StateTwo   WHERE - a consulta encontra VOCE. Varre a sua linha, e as vezes a
 *              sua coluna. O aviso continua existindo: da para fugir, mas e
 *              preciso decidir.
 *
 *   StateThree Transacao - varre, e DESFAZ. A faixa ja varrida volta. Quem
 *              aprendeu a entrar no espaco limpo atras da varredura descobre
 *              aqui que limpo nao e seguro.
 *
 *   StateFinal CROSS JOIN - a juncao sem condicao, o produto cartesiano. Linhas
 *              e colunas ao mesmo tempo. E o acidente classico de quem esquece o
 *              ON, e e o que trava o servidor.
 *
 * QUEM ESCOLHE A FAIXA E ESTE ARQUIVO, nao a estrategia: so o chefe sabe onde o
 * jogador esta, e a ConsultaAttack nao deve saber. Ver CustomizeAttackParams.
 */
class Thiago : public Boss {

public:
    explicit Thiago(Scene* scene);
    ~Thiago() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

private:
    /// Em que faixa o jogador esta AGORA, no eixo que a consulta pede.
    [[nodiscard]] int FaixaDoJogador(const struct ConsultaAttackParams& consulta);

    /// Contador da varredura sequencial da fase 1. Vive aqui, e nao na
    /// estrategia: a estrategia e sem memoria de proposito - duas fases podem
    /// usa-la ao mesmo tempo, e um contador dentro dela seria compartilhado.
    int mProximaFaixa = 0;
};
