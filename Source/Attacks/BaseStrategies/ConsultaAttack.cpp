//
// A varredura de uma linha ou coluna da tabela.
//

#include "ConsultaAttack.h"

#include <SDL_log.h>

#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../AttackParameters/ConsultaAttackParams.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Regras/Tabela.h"

ConsultaAttack::ConsultaAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

std::vector<std::unique_ptr<Projectile>> ConsultaAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const auto* consulta = dynamic_cast<const ConsultaAttackParams*>(&params);
    if (!consulta) {
        SDL_Log("ConsultaAttack: recebeu AttackParams comum, sem o bloco \"consulta\". "
                "Nada foi disparado.");
        return projectiles;
    }

    if (!Tabela::Valida(consulta->forma)) {
        SDL_Log("ConsultaAttack: tabela de %dx%d nao tem onde varrer.",
                consulta->forma.linhas, consulta->forma.colunas);
        return projectiles;
    }

    const auto battle = dynamic_cast<Battle*>(mOwner->GetScene());
    if (!battle) {
        SDL_Log("ConsultaAttack: sem cena Battle. Ataque cancelado.");
        return projectiles;
    }

    // A tabela e o campo de jogo, e nao a janela.
    const SDL_FRect campo = battle->GetPlayfieldBounds();

    const bool porLinha = (consulta->eixo == ConsultaAttackParams::Eixo::Linha);

    // Onde a faixa fica, e quao grossa ela e.
    const int quantas   = porLinha ? consulta->forma.linhas : consulta->forma.colunas;
    const float origem  = porLinha ? campo.y : campo.x;
    const float extensao= porLinha ? campo.h : campo.w;

    const int indice = (consulta->indice < 0) ? 0
                     : (consulta->indice >= quantas) ? quantas - 1
                     : consulta->indice;

    const float inicioDaFaixa = Tabela::Inicio(origem, extensao, quantas, indice);
    const float espessura     = Tabela::Espessura(extensao, quantas);

    // A varredura corre no eixo perpendicular a faixa.
    const float comprimento = porLinha ? campo.w : campo.h;
    const float partida     = porLinha ? campo.x : campo.y;

    // Dentro da tela, e nao fora dela.
    const float margem = 20.0f;
    const float ondeNasce = consulta->invertido ? (partida + comprimento - margem)
                                                : (partida + margem);

    const float sentido = consulta->invertido ? -1.0f : 1.0f;
    const Vector2 direcao = porLinha ? Vector2(sentido, 0.0f) : Vector2(0.0f, sentido);

    const int quantos = (params.numProjectiles > 0) ? params.numProjectiles : 1;
    projectiles.reserve(static_cast<size_t>(quantos));

    // Os projeteis se espalham pela espessura da faixa, nao pelo comprimento dela.
    const float passo = (quantos > 1) ? espessura / static_cast<float>(quantos)
                                      : espessura / 2.0f;

    for (int i = 0; i < quantos; ++i) {

        auto projectile = Acquire();
        if (!projectile) continue;

        // Meio do i-esimo quinhao da espessura.
        const float atravessado = inicioDaFaixa + passo * (static_cast<float>(i) + 0.5f);

        projectile->SetPosition(porLinha ? Vector2(ondeNasce, atravessado)
                                         : Vector2(atravessado, ondeNasce));

        // Nasce parado e visivel - este e o telegrafo.
        if (const auto rb = projectile->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(Vector2::Zero);
        }

        projectile->insertModifier<ActivateBehavior>(consulta->aviso,
                                                     direcao * params.projectileSpeed);

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
