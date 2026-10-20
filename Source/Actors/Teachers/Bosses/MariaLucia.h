//
// A chefe da INF 221 - Engenharia de Software.
//

#pragma once

#include "../Boss.h"

/**
 * @class MariaLucia
 * @brief A professora de Engenharia de Software (INF 221).
 *
 * TODO O BALANCEAMENTO DELA VIVE EM DADOS, como o do Vladimir: as fases em
 * Assets/Attacks/marialucia/fases.json e as regras de projetil no regras.json
 * ao lado. Esta classe existe para dar um tipo a chefe e um lugar para o que
 * precisar de C++ depois - hoje, nada.
 *
 * A IDEIA DA CHEFE, para quem for mexer nas fases: as outras disciplinas do
 * jogo sao OBJETOS - uma estrutura de dados, uma linguagem, uma primitiva de
 * concorrencia. Engenharia de Software e um PROCESSO, e por isso ela e a unica
 * chefe em que o TEMPO e parte do problema.
 *
 * Onde isso aparece: na fase 3 o campo NAO se limpa. O debito para onde cai e
 * fica, e o segundo quinze da fase e a consequencia do segundo dois. Em todas
 * as outras fases do jogo, de todos os outros chefes, o campo no fim esta como
 * estava no comeco.
 */
class MariaLucia : public Boss {
public:
    explicit MariaLucia(Scene* scene);
    ~MariaLucia() override = default;

    void OnUpdate(float deltaTime) override;

    // ---- O ESCOPO QUE CRESCE (fase 1) -------------------------------------
    //
    // Cada rajada da fase 1 sai maior que a anterior ate a entrega, e aí volta
    // ao tamanho de origem. A CONTA mora em Escopo (Source/Attacks/
    // EscopoQueCresce.h), que e pura e tem teste; aqui so fica o estado - qual
    // rajada da volta e esta -, que e justamente o que fases.json nao consegue
    // guardar. Mesmo motivo da fresta giratoria do Vitor.

    /// Quantos projeteis a mais por rajada.
    static constexpr int kPassoDeProjeteis = 2;

    /// Quantos graus a mais por rajada. O LEQUE CRESCE JUNTO: com ele parado,
    /// mais projeteis so os aproximam ate a rajada virar uma barra solida - o
    /// que aconteceu com as tarefas do Vitor. Ver o teste "nem na maior rajada
    /// dois vizinhos se encostam".
    static constexpr float kPassoDeLeque = 10.0f;

    /// Quantas rajadas cabem numa volta, antes de a versao sair e o escopo
    /// recomecar. Com cooldown 1,0s e fase de 17s, dao pouco mais de quatro
    /// voltas - o jogador ve o ciclo repetir, que e o que faz virar regra em
    /// vez de parecer aleatorio.
    static constexpr int kSprintsAteAEntrega = 4;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

private:
    /// Qual rajada da volta atual. 0 = a de origem, a que fases.json descreve.
    int mSprint = 0;

    /// O QUE O ARQUIVO PEDIU, GUARDADO NO PRIMEIRO DISPARO.
    ///
    /// Precisa ser guardado porque AttackParams NAO E RECONSTRUIDO a cada tiro:
    /// Boss::ExecuteAttack passa sempre o MESMO objeto para
    /// CustomizeAttackParams, e os valores de fases.json sao escritos nele uma
    /// vez, na montagem. Uma alteracao relativa ("+= dois projeteis") portanto
    /// acumula sobre a alteracao anterior em vez de sobre o arquivo.
    ///
    /// MEDIDO, com a versao que somava: a fase 1 terminava disparando 54
    /// projeteis num leque de 285 graus, e o ciclo de quatro rajadas nunca
    /// reiniciava de verdade - cada volta so partia de um numero maior. A conta
    /// pura estava certa o tempo todo; errado estava como eu a liguei.
    ///
    /// -1 quer dizer "ainda nao disparou nenhuma vez".
    int   mProjeteisDePartida = -1;
    float mLequeDePartida     = 0.0f;
};
