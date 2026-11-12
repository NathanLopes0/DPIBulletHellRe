//
// A fase 2 do Thiago: o INSERT, o UPDATE e o DELETE, com os registros voando.
//

#pragma once

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"

/// Na fase 2 a tabela muda, e cada comando tem o seu gesto: o INSERT arremessa o registro em arco
/// ate a linha vazia, o DELETE derruba a linha (que se espatifa no chao) e o UPDATE chuta o valor
/// velho no aluno enquanto o novo voa do professor ate a casa. Nenhum varre linha: isso e o SELECT.
namespace Dml {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kGatilhoDoRisco = "riscar";
    constexpr const char* kGatilhoDoCaco = "caco";

    /// A cara do rascunho que risca a linha do DELETE (o gatilho e o do rascunho da transacao).
    constexpr const char* kCaraDoRisco = "Riscado";

    /// O INSERT: os valores aparecem no professor e, depois de kEscrevendo, voam em arco.
    constexpr float kEscrevendo = 0.35f;
    constexpr float kDuracaoDoArco = 0.95f;
    constexpr float kAlturaDoArco = 140.0f;

    /// O DELETE: a linha riscada por kAvisoDoDelete, e a queda. Os cacos de cada valor que bate no
    /// chao sao o leque do ataque "caco" (thiago/fases.json), e caem com a mesma gravidade.
    constexpr float kAvisoDoDelete = 0.9f;
    constexpr float kGravidade = 900.0f;

    /// O UPDATE: a casa marcada por kAvisoDoUpdate, o chute no aluno e a escrita do valor novo.
    constexpr float kAvisoDoUpdate = 0.9f;
    constexpr float kVelocidadeDoChute = 360.0f;
    constexpr float kVelocidadeDaEscrita = 620.0f;

    /// Onde o valor esta na fracao t (0 a 1) da viagem de 'de' a 'para': a reta entre os dois,
    /// erguida por uma parabola que sobe 'altura' no meio do caminho.
    Vector2 NoArco(const Vector2& de, const Vector2& para, float altura, float t);

    /// Quanto sobe o que sai do chao para cima a 'velocidade', com a 'gravidade': os cacos.
    float Subida(float velocidade, float gravidade);

    /// O que o Dificil muda: avisos mais curtos, arco e queda mais rapidos e um chute mais forte.
    /// Os cacos a mais estao no bloco "dificil" do ataque.
    struct Degrau {
        float avisoDoDelete = kAvisoDoDelete;
        float avisoDoUpdate = kAvisoDoUpdate;
        float duracaoDoArco = kDuracaoDoArco;
        float gravidade = kGravidade;
        float velocidadeDoChute = kVelocidadeDoChute;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
