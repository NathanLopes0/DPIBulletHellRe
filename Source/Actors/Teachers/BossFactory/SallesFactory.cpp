
#include "SallesFactory.h"
#include "../../Teachers/Bosses/Salles.h"
#include "../../../Attacks/FasesDeAtaqueArquivo.h"
#include "../../../Attacks/ProjeteisDeChefeArquivo.h"

#include "../../../CaminhosArquivo.h"

// Este arquivo so monta os componentes do chefe. As fases dele estao em
// Assets/Attacks/fases.json sob "salles", e os tipos de projetil em
// Assets/Attacks/projeteis.json, no conjunto de mesmo nome.


SallesFactory::SallesFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> SallesFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Salles>(scene);
}

void SallesFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(Caminhos::Asset("Teachers/DPIBHSalles.png"),
                                                                                Caminhos::Asset("Teachers/DPIBHSalles.json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");


    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // Os projeteis vem de Assets/Attacks/projeteis.json, conjunto "salles".
    // Acrescentar um tipo novo a este chefe e uma entrada naquele arquivo - nao
    // ha mais uma classe de fabrica por projetil.
    if (!RegistrarProjeteisDeArquivo(boss, "salles")) {
        SDL_Log("ERRO CRITICO: o chefe \"salles\" ficou sem projetil. As linhas PROJETEIS: "
                "acima dizem o motivo.");
    }
}


void SallesFactory::ConfigureAttacksAndFSM(Boss* boss) {


    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) { SDL_Log("ERRO CRÍTICO: Boss não tem FSMComponent!"); return; }

    // As fases deste chefe vivem em Assets/Attacks/fases.json, no conjunto
    // "salles", e as regras dos projeteis em Assets/Attacks/regras.json.
    //
    // ATENCAO: parte do balanceamento NAO esta la. Salles::CustomizeAttackParams
    // sobrescreve o anguloCentral nas QUATRO fases a cada disparo - nas tres
    // primeiras sorteando entre mirar no jogador e um angulo aleatorio, e na
    // final sempre sorteando. Escrever anguloCentral no arquivo nao tem efeito.
    //
    // NAO HA MAIS CONFIGURACAO DE RESERVA EM C++. Se o arquivo faltar, estiver
    // malformado ou tiver uma transicao quebrada, ConfigurarFasesDeArquivo
    // devolve false depois de explicar o motivo no log, e este chefe fica SEM
    // ATAQUE NENHUM. Quem impede isso de chegar ao jogo e
    // tests/test_arquivos_de_dados.cpp, que le os Assets de verdade e quebra a
    // suite ao primeiro erro de digitacao.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "salles")) {
        SDL_Log("ERRO CRITICO: o chefe \"salles\" nao pode ser montado a partir de "
                "Assets/Attacks/fases.json e ficara sem ataque. As linhas FASES: acima "
                "dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
