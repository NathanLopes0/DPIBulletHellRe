//
// A fabrica de projetil de chefe, unica, dirigida por dados.
//

#include "FabricaDeProjetil.h"

#include <utility>
#include <vector>
#include <SDL_log.h>

#include "../../Boss.h"
#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../../Components/ColliderComponents/CircleColliderComponent.h"

FabricaDeProjetil::FabricaDeProjetil(DescricaoDeProjetil descricao, std::string nome)
    : ProjectileFactory(descricao.sprite, descricao.dados),
      mDescricao(std::move(descricao)),
      mNome(std::move(nome))
{
}

std::unique_ptr<Projectile> FabricaDeProjetil::createProjectile(Scene* scene, Actor* owner) {

    // Aqui o teste e para Boss, que e o que o construtor do projetil realmente exige.
    auto* bossOwner = dynamic_cast<Boss*>(owner);
    if (!bossOwner) {
        SDL_Log("ERRO FATAL: a fabrica do projetil \"%s\" recebeu um dono que nao e um Boss.",
                mNome.c_str());
        return nullptr;
    }

    auto projectile = std::make_unique<ProjetilDeChefe>(scene, bossOwner);

    // O tipo: o nome dele em projeteis.json.
    projectile->DefinirTipo(mNome);

    // Quando este projetil morre por sair da tela.
    projectile->DefinirMargemDeSaida(mDescricao.margemEmSprites, mDescricao.margemDivisorDeTela);

    // Se o sprite gira com a direcao do movimento.
    projectile->DefinirRotacaoPelaVelocidade(mDescricao.rotacionarComAVelocidade);

    // O efeito de acerto, se este tipo tiver um.
    projectile->DefinirAcorrentamento(mDescricao.acorrenta.segundos,
                                      mDescricao.acorrenta.fatorDeVelocidade,
                                      mDescricao.acorrenta.multiplicadorDaPerda);

    // Se o tiro do aluno o derruba, e com quantos tiros.
    projectile->DefinirDerrubavel(mDescricao.derrubavel);
    projectile->DefinirResistencia(mDescricao.resistencia);

    // Se ele abriga o aluno: come os projeteis do chefe e os tiros do aluno.
    projectile->DefinirAbrigo(mDescricao.abrigo);

    projectile->SetScale(mDescricao.escala);

    if (mDescricao.posicionarNoDono) {
        projectile->SetPosition(bossOwner->GetPosition());
    }

    projectile->AddComponent<RigidBodyComponent>();

    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>(
        mSpritePath, mDataPath, mDescricao.ordemDeDesenho);

    for (const auto& anim : mDescricao.animacoes) {
        drawComp->AddAnimation(anim.nome, anim.quadros);
    }

    // Sempre uma animacao valida definida aqui.
    drawComp->SetAnimation(mDescricao.animacaoInicial);
    drawComp->SetIsVisible(true);

    const float medida = (mDescricao.colisorDimensao == "altura")
                             ? static_cast<float>(drawComp->GetSpriteHeight())
                             : static_cast<float>(drawComp->GetSpriteWidth());

    auto collider = projectile->AddComponent<CircleColliderComponent>(
        medida / mDescricao.colisorDivisor);

    collider->SetTag(ColliderTag::BossProjectile);

    projectile->SetState(ActorState::Active);

    return projectile;
}

std::unique_ptr<Projectile> FabricaDeProjetil::Acquire(Scene* scene, Actor* owner) {

    // A receita de criacao e a mesma createProjectile.
    auto created = mPool.Acquire([this, scene, owner]() {
        return std::unique_ptr<ProjetilDeChefe>(
            dynamic_cast<ProjetilDeChefe*>(createProjectile(scene, owner).release())
        );
    });

    if (!created) {
        SDL_Log("ERRO FATAL: a fabrica do projetil \"%s\" falhou em criar ou reciclar.",
                mNome.c_str());
        return nullptr;
    }

    created->SetOwner(owner);

    // Posicao base igual a de um projetil recem-construido.
    if (owner && mDescricao.posicionarNoDono) {
        created->SetPosition(owner->GetPosition());
    }

    created->SetOriginFactory(this);

    return created;
}

void FabricaDeProjetil::Release(std::unique_ptr<Projectile> projectile) {

    if (!dynamic_cast<ProjetilDeChefe*>(projectile.get())) {
        SDL_Log("ERRO: a fabrica do projetil \"%s\" recebeu de volta um projetil de outro tipo.",
                mNome.c_str());
        return;
    }

    mPool.Release(std::unique_ptr<ProjetilDeChefe>(
        dynamic_cast<ProjetilDeChefe*>(projectile.release())
    ));
}

void FabricaDeProjetil::Prewarm(Scene* scene, Actor* owner, int count) {

    std::vector<std::unique_ptr<Projectile>> held;
    held.reserve(static_cast<size_t>(count > 0 ? count : 0));

    for (int i = 0; i < count; ++i) {
        auto p = Acquire(scene, owner);
        if (!p) {
            SDL_Log("AVISO: pre-aquecimento do projetil \"%s\" falhou na instancia %d de %d.",
                    mNome.c_str(), i, count);
            continue;
        }
        held.push_back(std::move(p));
    }

    for (auto& p : held) {
        Release(std::move(p));
    }
}
