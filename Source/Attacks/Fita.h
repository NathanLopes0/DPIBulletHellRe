//
// A fita da maquina de Turing do Vladimir - a regra de quando uma casa e
// reescrita, sem nada do motor.
//

#pragma once

#include <string>
#include <vector>

/**
 * CAMADA PURA (mesma disciplina de PathAim).
 *
 * A fase 3 do Vladimir e a maquina de Turing: o chefe escreve a fita na
 * passagem - simbolos que descem e param numa fileira - e, quando volta por
 * cima, LE cada casa e a REESCREVE. O "a" parado vira "b" e sai perseguindo. E
 * a transicao que o placar anuncia: delta(q, a) = (q, b, ->).
 *
 * Quem faz isso no jogo e ReescreverBehavior (Behaviors.h), que precisa do
 * projetil, da cena e dos corpos do chefe - e nada disso entra na suite. O que
 * da para separar e a DECISAO, e e nela que mora o erro que importa: a casa
 * nasce exatamente embaixo da cabeca, entao sem a idade minima toda casa seria
 * reescrita no quadro em que foi escrita, e a fita nunca existiria.
 */
namespace Fita {

    /**
     * @brief A cabeca esta em cima da casa?
     *
     * SO O X CONTA. A cabeca le a casa que esta embaixo dela, a qualquer
     * altura: o chefe varre por cima da fita, e nunca na mesma linha que ela.
     * O raio e inclusivo - uma cabeca exatamente na borda le a casa.
     */
    bool CabecaSobreACasa(float xDaCabeca, float xDaCasa, float raio);

    /**
     * @brief Esta casa deve ser reescrita agora?
     *
     * Precisa das duas coisas: a casa ja ter a idade minima - para nao ser
     * reescrita pela cabeca que acabou de escreve-la - e ALGUMA cabeca estar em
     * cima dela. "Alguma" porque o chefe pode estar dividido em dois corpos
     * (Battle::Corpos), e qualquer um deles e uma cabeca.
     *
     * Sem cabeca nenhuma em campo, nada e reescrito.
     */
    bool DeveReescrever(float idade, float idadeMinima,
                        const std::vector<float>& xDasCabecas, float xDaCasa, float raio);

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaFita = "StateThree";
    constexpr const char* kGatilhoDaEscrita = "escrever";

    /// A CABECA ANDA NA FITA E PARA NAS PONTAS. Ela varre a kAlturaDaCabeca, de
    /// kMargem a largura - kMargem, escrevendo uma casa a cada kEscrita
    /// segundos enquanto anda; na ponta le o branco, troca de estado e fica
    /// kPausaNaPonta parada antes de voltar. A pausa e o que deixa o aluno
    /// acertar o professor: andando sem parar, o tiro do aluno (quase 1s de voo)
    /// chega depois que ele saiu - medido, 12 acertos no professor na fase
    /// inteira, contra 230 do Vladimir antigo.
    constexpr float kAlturaDaCabeca = 144.0f;
    constexpr float kMargem = 110.0f;
    constexpr float kEscrita = 0.2f;
    constexpr float kPausaNaPonta = 1.2f;

    /**
     * @brief O placar da maquina: a transicao que a cabeca esta fazendo.
     *
     * A MAQUINA TEM DOIS ESTADOS, um para cada sentido da varredura: indo para
     * a direita ela esta em q0, e voltando, em q1. Nos dois le "a", escreve "b"
     * e segue - "delta(q0, a) = (q0, b, ->)" -, e e a SETA que muda com o
     * sentido em que o professor anda. Sai em UTF-8, com o delta, o subscrito
     * e a seta de verdade.
     */
    std::string Transicao(bool paraDireita);

    /**
     * @brief O placar na ponta da fita: a cabeca le o BRANCO, troca de estado e
     * volta. Chegando a direita, "delta(q0, branco) = (q1, branco, <-)".
     * 'chegouADireita' e de onde a cabeca veio, e a seta aponta para onde ela
     * vai.
     */
    std::string Branco(bool chegouADireita);
}
