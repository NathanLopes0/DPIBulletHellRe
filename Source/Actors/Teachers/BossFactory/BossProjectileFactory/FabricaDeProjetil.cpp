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

    // Antes, cada fabrica fazia dynamic_cast para o chefe CONCRETO dela
    // (dynamic_cast<Salles*>, <Julio*>...). Aqui o teste e para Boss, que e o que
    // o construtor do projetil realmente exige.
    //
    // Isso afrouxa uma verificacao: a fabrica da capivara passaria a aceitar o
    // Julio como dono. Na pratica aquele teste nunca podia falhar - a fabrica e
    // registrada DENTRO de um chefe, e Acquire e Prewarm sempre recebem esse
    // mesmo chefe -, entao o que se perde e um alarme que nunca soou, e o que se
    // ganha e nao precisar de uma classe por chefe.
    auto* bossOwner = dynamic_cast<Boss*>(owner);
    if (!bossOwner) {
        SDL_Log("ERRO FATAL: a fabrica do projetil \"%s\" recebeu um dono que nao e um Boss.",
                mNome.c_str());
        return nullptr;
    }

    auto projectile = std::make_unique<ProjetilDeChefe>(scene, bossOwner);

    // Quando este projetil morre por sair da tela. Antes da migracao, os baloes do
    // Andre tinham uma SUBCLASSE INTEIRA so para mudar estes dois numeros.
    projectile->DefinirMargemDeSaida(mDescricao.margemEmSprites, mDescricao.margemDivisorDeTela);

    // Se o sprite gira com a direcao do movimento. So para arte que tem frente -
    // as listas do Salles, cuja seta precisa apontar para o proximo no.
    projectile->DefinirRotacaoPelaVelocidade(mDescricao.rotacionarComAVelocidade);

    // A escala vem ANTES do colisor: CircleColliderComponent multiplica o raio
    // por GetScale(), entao escalar depois mudaria a hitbox sem mudar o numero
    // que passamos adiante.
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

    // SEMPRE uma animacao valida definida aqui. O leitor garante que
    // animacaoInicial nao e vazia e que nomeia uma das animacoes acima, entao
    // esta linha nao pode falhar silenciosamente - que era exatamente o bug da
    // fabrica dos baloes, onde a cor so era escolhida la em Andre::ExecuteAttack
    // e quem nascia pelo Prewarm nunca passava por ali.
    drawComp->SetAnimation(mDescricao.animacaoInicial);
    drawComp->SetIsVisible(true);

    const float medida = (mDescricao.colisorDimensao == "altura")
                             ? static_cast<float>(drawComp->GetSpriteHeight())
                             : static_cast<float>(drawComp->GetSpriteWidth());

    auto collider = projectile->AddComponent<CircleColliderComponent>(
        medida / mDescricao.colisorDivisor);

    // Antes, SO a fabrica da capivara marcava a tag; as outras cinco deixavam
    // ColliderTag::None. Nao havia efeito nenhum - o unico GetColliderTag() do
    // projeto esta em PlayerProjectile e compara com ColliderTag::Boss, nunca com
    // BossProjectile -, mas deixar certo custa uma linha e evita que alguem
    // escreva uma regra de colisao por tag e descubra do jeito ruim que cinco em
    // seis projeteis estavam sem.
    collider->SetTag(ColliderTag::BossProjectile);

    projectile->SetState(ActorState::Active);

    return projectile;
}

std::unique_ptr<Projectile> FabricaDeProjetil::Acquire(Scene* scene, Actor* owner) {

    // A receita de criacao e a mesma createProjectile - so e chamada quando o
    // pool esta vazio (primeira vez, ou todos os objetos deste tipo em uso
    // simultaneo na tela).
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

    // Posicao base igual a de um projetil recem-construido: createProjectile
    // deixa o objeto na posicao do dono, mas quem vem do pool nao passa por ela e
    // carregaria a posicao onde morreu. A strategy continua livre para
    // sobrescrever logo em seguida.
    //
    // Respeita posicionarNoDono pelo mesmo motivo que createProjectile: as duas
    // entradas no mundo tem de deixar o objeto no mesmo lugar, senao "novo" e
    // "reciclado" se comportam diferente.
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

    // As instancias ficam TODAS vivas ao mesmo tempo durante a criacao, e so
    // depois voltam para o pool. Fazer Acquire+Release em sequencia reciclaria o
    // mesmo objeto count vezes e o pool terminaria com um.
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
