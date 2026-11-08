//
// Quem acertou o aluno ha pouco: o mesmo causador nao acerta duas vezes seguidas.
//

#pragma once

#include <vector>

#include "Dificuldade.h"

/**
 * CAMADA PURA. Coberto por test_dificuldade.cpp.
 *
 * NO DIFICIL A INVENCIBILIDADE E CURTA (Dificuldade::Invencibilidade): o tiro
 * que vem atras do primeiro tambem acerta. Mas curta demais para o MESMO
 * causador - a parede de lapis do Thiago, o corpo do professor que desceu
 * em cima do aluno, o projetil grande que ainda o atravessa - fazia um so
 * encontro custar tres ou quatro acertos, um a cada 0,4 s. Isso nao cobra
 * desviar: so pune quem ficou preso.
 *
 * ENTAO A MEMORIA: cada causador que acertou fica lembrado por
 * kMesmoCausador segundos - a invencibilidade do Normal -, e nesse tempo nao
 * acerta de novo. Causadores DIFERENTES acertam no ritmo do modo.
 *
 * O CAUSADOR E UM ENDERECO, e o pool recicla enderecos: um projetil que morreu
 * e voltou como outro tiro teria a memoria do anterior. Por isso a memoria
 * guarda tambem a IDADE dele no acerto; um tiro mais novo do que isso e outro
 * tiro. Para quem nao envelhece (o corpo do chefe), a idade e 0.
 */
namespace Acerto {

    constexpr float kMesmoCausador = Dificuldade::kInvencibilidadeNormal;

    class Memoria {
    public:
        /// Os segundos passam; o que ficou velho e esquecido.
        void Avancar(float deltaTime);

        /// 'quem', com 'idade' segundos de vida, acabou de acertar.
        void Lembrar(const void* quem, float idade);

        /// Se 'quem' (com 'idade' agora) ja acertou ha menos de kMesmoCausador.
        [[nodiscard]] bool Lembra(const void* quem, float idade) const;

        [[nodiscard]] size_t Quantos() const { return mItens.size(); }

    private:
        struct Item {
            const void* quem = nullptr;
            float idade = 0.0f;   ///< a idade dele no acerto
            float ha = 0.0f;      ///< ha quanto tempo foi
        };
        std::vector<Item> mItens;
    };
}
