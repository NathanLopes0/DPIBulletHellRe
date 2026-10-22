//
// A geracao de tokens: um por vez, sorteados com temperatura.
//

#include "TokenAttack.h"

#include <SDL_log.h>

#include "../Behaviors.h"
#include "../Temperatura.h"
#include "../AttackParameters/TokenAttackParams.h"
#include "../../Random.h"
#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/RigidBodyComponent.h"

TokenAttack::TokenAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

std::vector<std::unique_ptr<Projectile>> TokenAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const auto* tokens = dynamic_cast<const TokenAttackParams*>(&params);
    if (!tokens) {
        SDL_Log("TokenAttack: os parametros nao sao TokenAttackParams - nada disparado");
        return projectiles;
    }
    if (params.numProjectiles <= 0) return projectiles;

    const auto candidatos = Temperatura::DeAngulo(tokens->candidatos, tokens->abertura);
    const auto probabilidades = Temperatura::Probabilidades(candidatos.logits, tokens->temperatura);

    const float grau = Math::Pi / 180.0f;
    projectiles.reserve(static_cast<size_t>(params.numProjectiles));
    for (int i = 0; i < params.numProjectiles; ++i) {
        auto token = Acquire();
        if (!token) continue;

        // O SORTEIO DO PROXIMO TOKEN: o angulo e um dos candidatos, com a
        // probabilidade que a temperatura deu a ele.
        float desvio = 0.0f;
        if (!candidatos.desvios.empty()) {
            const int k = Temperatura::Amostrar(probabilidades, Random::GetFloatRange(0.0f, 1.0f));
            desvio = candidatos.desvios[static_cast<size_t>(k)];
        }
        const float a = (params.centralAngle + desvio) * grau;

        // O TOKEN i SAI DEPOIS DOS i ANTERIORES: escondido e parado ate
        // acordar, como na WaveAttack.
        token->SetPosition(params.firePosition);
        token->insertModifier<DeactivateBehavior>(0.0f);
        token->insertModifier<ActivateBehavior>(static_cast<float>(i) * params.creationSpeed,
                                                Vector2(Math::Cos(a), Math::Sin(a)) * params.projectileSpeed);
        if (!tokens->frase.empty()) {
            const std::string& palavra = tokens->frase[static_cast<size_t>(i) % tokens->frase.size()];
            if (auto* anim = token->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(palavra);
        }
        projectiles.push_back(std::move(token));
    }
    return projectiles;
}
