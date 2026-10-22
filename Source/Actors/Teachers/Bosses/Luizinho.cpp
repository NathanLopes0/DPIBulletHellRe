//
// O chefe da INF 340 - Linguagens de Programacao.
//

#include "Luizinho.h"

#include <cmath>

#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../CaminhosArquivo.h"
#include "../../../Font.h"
#include "../../../Attacks/AttackParameters/SerpenteAttackParams.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../../Components/DrawComponents/DrawTextComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Game.h"
#include "../../Player/Player.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {

    /// O que o professor diz quando o coletor passa: descansem em paz.
    const char* const kNaVarredura = "requiescant in pace";
    const char* const kTraducaoDaMemoria = "lembra-te de que morrerás";

    const SDL_Color kReferenciaViva = {246, 206, 92, 150};    // ouro: alcancado
    const SDL_Color kReferenciaMorta = {150, 140, 130, 90};   // cinza: lixo

    /// Na frente dos projeteis, como o marcador do intercambio.
    constexpr int kOrdemDoRotulo = 95;
}

/**
 * @brief O marcador do rotulo do goto: uma moldura de ouro com "II:" dentro.
 *
 * A FONTE E DELE, e nao do professor: o texto guarda um ponteiro para ela, e o
 * ator e da cena - viva o quanto a cena viver, a fonte vive junto.
 */
class RotuloDoGoto : public Actor {
public:
    explicit RotuloDoGoto(Scene* cena) : Actor(cena) {
        mFonte.Load(Caminhos::Asset("Fonts/Carlito-Regular.ttf"));
        mCaixa = AddComponent<DrawCaixaComponent>(84, 40, SDL_Color{246, 206, 92, 230},
                                                  SDL_Color{40, 30, 20, 170}, 2, kOrdemDoRotulo);
        mTexto = AddComponent<DrawTextComponent>(" ", &mFonte, 76, 34, 26, kOrdemDoRotulo + 1);
        mTexto->SetAjustarAoTexto(true);
        mTexto->SetColor(Vector3(1.0f, 0.86f, 0.42f));
        Mostrar(false);
    }

    void Mostrar(const bool mostrar) {
        mCaixa->SetIsVisible(mostrar);
        mTexto->SetIsVisible(mostrar);
    }

    void Escrever(const std::string& texto) { mTexto->SetText(texto); }

private:
    Font mFonte;
    DrawCaixaComponent* mCaixa = nullptr;
    DrawTextComponent* mTexto = nullptr;
};

Luizinho::Luizinho(Scene* scene) : Boss(scene) {
}

void Luizinho::OnUpdate(const float deltaTime) {

    // STOP THE WORLD: com o mundo parado, o professor para junto - nem anda nem
    // dispara. A maquina de estados continua (ela e componente, e roda antes
    // daqui), entao a pausa NAO alonga a fase.
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naMemoria = fsm && fsm->GetStateName() == kFaseDaMemoria;
    if (naMemoria && mNaMemoria && Coletor::Estado(mColetor) == Coletor::Mundo::Parado) {
        if (auto* rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    } else {
        Boss::OnUpdate(deltaTime);
    }

    AtualizarMemoria(deltaTime);
    AtualizarGoto(deltaTime);
}

void Luizinho::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    if (stateName == "StateOne" || stateName == "StateTwo") {
        // FASE 1 - A AMARRACAO ESTATICA E FEITA NA HORA DO DISPARO: o leque de
        // pedra sai centrado em onde o aluno esta AGORA e nao muda mais. Quem
        // andou depois ja nao esta la.
        //
        // A fase tem tres ataques e este gancho nao sabe qual dispara, entao o
        // angulo e escrito em todos. Os outros sao as rodas da amarracao
        // dinamica, CircleSpreadAttack, que ignora o angulo central.
        //
        // FASE 2 - A CAPSULA VOA para onde o aluno esta no disparo, e e por isso
        // que ela fica no caminho do tiro dele: o tiro sobe em direcao ao
        // professor, e a capsula desce pela mesma linha.
        const Vector2 d = GetDirectionToPlayer();
        params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
    }
    else if (stateName == kFaseDaSequencia) {
        // A CABECA SAI EM DIRECAO AO ALUNO, e leva o goto do professor.
        const Vector2 d = GetDirectionToPlayer();
        params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
        if (auto* serpente = dynamic_cast<SerpenteAttackParams*>(&params)) serpente->desvio = mDesvio;
    }
    else if (stateName == kFaseDaMemoria) {
        // A MEMORIA E ALOCADA ONDE O ALUNO ESTA. O leque sai centrado nele, e a
        // velocidade e a que leva o bloco mais longe da rajada a parar EM CIMA
        // dele no instante kTempoAteOBlocoMaisLonge: quem ficou parado recebe o
        // bloco, quem andou o deixa para tras, ocupando o lugar onde estava.
        // Escrita inteira a cada disparo, como pede o gancho (Boss.h).
        const Vector2 d = GetDirectionToPlayer();
        params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
        params.projectileSpeed = GetDistanceToPlayer() / kTempoAteOBlocoMaisLonge;
    }
}

// ---------------------------------------------------------------------------
// A fase 3: o coletor de lixo
// ---------------------------------------------------------------------------

std::vector<BossProjectile*> Luizinho::Blocos() const {
    std::vector<BossProjectile*> blocos;
    auto* batalha = dynamic_cast<Battle*>(mScene);
    const ProjectileFactory* fabrica = GetProjectileFactory("Bloco");
    if (!batalha || !batalha->GetProjectileManager() || !fabrica) return blocos;

    // O BLOCO E RECONHECIDO PELA FABRICA, e nao pela cara: a cara muda quando o
    // coletor o marca. Os outros projeteis (uma letra da fase 2 que ainda
    // voa no comeco da fase) nao sao memoria.
    //
    // E O LIXO QUE JA FOI JOGADO FORA NAO E MAIS MEMORIA: marcado e rodando, ele
    // esta caindo para fora da tela. Marcado e PARADO e o lixo durante a pausa,
    // e esse ainda conta - e o grafo daquele instante.
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (!p || p->GetOriginFactory() != fabrica) continue;
        const auto estado = p->GetState();
        if (estado == ActorState::Paused) { blocos.push_back(p.get()); continue; }
        if (estado != ActorState::Active) continue;
        const auto* anim = p->GetComponent<DrawAnimatedComponent>();
        if (anim && anim->GetAnimation() == kAnimacaoDoLixo) continue;
        blocos.push_back(p.get());
    }
    return blocos;
}

void Luizinho::PararOMundo(const bool parar) const {
    auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return;

    // PARADO, O PROJETIL NAO FAZ NADA: Actor::Update so roda o que esta ativo,
    // entao o corpo, os behaviors e a animacao esperam juntos. Congelar so a
    // velocidade nao bastaria - um SlowDown agendado dispararia durante a pausa
    // e se perderia, e o bloco sairia dela na velocidade do disparo.
    //
    // E PARADO ELE NAO ACERTA NINGUEM (a colisao da batalha so olha o que esta
    // ativo): durante a varredura o aluno pode passar por entre os blocos.
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (!p) continue;
        if (parar && p->GetState() == ActorState::Active) p->SetState(ActorState::Paused);
        else if (!parar && p->GetState() == ActorState::Paused) p->SetState(ActorState::Active);
    }
}

void Luizinho::DevolverAMemoria() {
    PararOMundo(false);

    // TODO BLOCO, e nao so os de Blocos(): a ultima pausa da fase termina junto
    // com ela (4 x 4,25 s = 17 s), entao o lixo daquela pausa sai marcado e
    // ainda nao varrido - parado e com a cara de lixo, que Blocos() deixa de
    // fora. Medido, ele ficava 20 s no campo da fase seguinte ate o tempo
    // maximo de vida o encerrar.
    auto* batalha = dynamic_cast<Battle*>(mScene);
    const ProjectileFactory* fabrica = GetProjectileFactory("Bloco");
    if (batalha && batalha->GetProjectileManager() && fabrica) {
        for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
            if (p && p->GetOriginFactory() == fabrica && p->GetState() == ActorState::Active) p->Kill();
        }
    }
    if (mReferencias) mReferencias->Limpar();
    mColetor.t = 0.0f;
    mNaMemoria = false;
}

void Luizinho::AnunciarAMemoria() {
    // O PROFESSOR DIZ O LATIM E TRADUZ, alternando; na varredura ele diz outra
    // coisa. O placar le o anuncio da fase a cada quadro e so redesenha quando
    // o texto muda (Battle::AtualizarPlacar).
    std::string texto;
    if (Coletor::Estado(mColetor) == Coletor::Mundo::Parado) {
        texto = kNaVarredura;
    } else {
        const int vez = static_cast<int>(mTempoNoPlacar / kTempoDeCadaLingua);
        texto = (vez % 2 == 0) ? mLatimDaMemoria : kTraducaoDaMemoria;
    }
    if (AnuncioDaFase(kFaseDaMemoria) != texto) RegistrarAnuncio(kFaseDaMemoria, texto);
}

void Luizinho::AtualizarMemoria(const float deltaTime) {

    const auto* fsm = GetComponent<FSMComponent>();
    const bool naMemoria = fsm && fsm->GetStateName() == kFaseDaMemoria && !EhClone();

    if (!naMemoria) {
        if (mNaMemoria) DevolverAMemoria();   // a fase acabou: o programa terminou
        return;
    }

    if (!mNaMemoria) {
        mNaMemoria = true;
        mColetor.t = 0.0f;
        mTempoNoPlacar = 0.0f;
        if (mLatimDaMemoria.empty()) mLatimDaMemoria = AnuncioDaFase(kFaseDaMemoria);
    }

    if (!mReferencias) {
        auto ator = std::make_unique<Actor>(mScene);
        mReferencias = ator->AddComponent<DrawLinhasComponent>(kOrdemDasReferencias);
        mScene->AddActor(std::move(ator));
    }

    // O GRAFO DESTE QUADRO, com o professor de raiz.
    const auto blocos = Blocos();
    std::vector<Vector2> onde;
    onde.reserve(blocos.size());
    for (const BossProjectile* b : blocos) onde.push_back(b->GetPosition());
    const auto grafo = Coletor::Referencias(onde, GetPosition(), kAlcanceDaReferencia);

    switch (Coletor::Avancar(mColetor, deltaTime)) {
        case Coletor::Evento::Para:
            // O MUNDO PARA, e o lixo ganha a marca: e o aviso do que vai sumir.
            PararOMundo(true);
            for (size_t i = 0; i < blocos.size(); ++i) {
                if (grafo.vivo[i]) continue;
                if (auto* anim = blocos[i]->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(kAnimacaoDoLixo);
            }
            break;

        case Coletor::Evento::Varre:
            // A VARREDURA: o grafo e o do fim da pausa, que e o do comeco - nada
            // se mexeu no meio. O que ninguem alcanca e JOGADO FORA: cai reto
            // para fora da tela, e na queda ainda acerta quem estiver embaixo.
            // A marca da pausa e o aviso de onde vai chover.
            for (size_t i = 0; i < blocos.size(); ++i) {
                if (grafo.vivo[i]) continue;
                blocos[i]->DescartarComportamentos();
                if (auto* rb = blocos[i]->GetComponent<RigidBodyComponent>()) {
                    rb->SetVelocity(Vector2(0.0f, kQuedaDoLixo));
                }
            }
            PararOMundo(false);
            break;

        case Coletor::Evento::Nenhum:
            break;
    }

    if (Coletor::Estado(mColetor) == Coletor::Mundo::Rodando) mTempoNoPlacar += deltaTime;
    AnunciarAMemoria();

    // AS LINHAS: ouro entre o que esta vivo, cinza entre o lixo. No quadro da
    // varredura o lixo ja foi jogado fora, e nao tem mais linha.
    mReferencias->Limpar();
    const bool varreu = (Coletor::Estado(mColetor) == Coletor::Mundo::Rodando);
    for (const auto& [a, b] : grafo.arestas) {
        const bool vivo = grafo.vivo[static_cast<size_t>(b)];
        if (!vivo && varreu) continue;
        const Vector2 de = (a == Coletor::kRaiz) ? GetPosition() : blocos[static_cast<size_t>(a)]->GetPosition();
        mReferencias->Acrescentar(de, blocos[static_cast<size_t>(b)]->GetPosition(),
                                  vivo ? kReferenciaViva : kReferenciaMorta);
    }
}

// ---------------------------------------------------------------------------
// A final: o goto
// ---------------------------------------------------------------------------

Vector2 Luizinho::OndeFicaORotulo() const {
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    const float largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    float x = largura / 2.0f;
    if (batalha && batalha->Alvo()) x = largura - batalha->Alvo()->GetPosition().x;
    x = std::fmax(kMargemDoRotulo, std::fmin(largura - kMargemDoRotulo, x));
    return Vector2(x, kAlturaDoRotulo);
}

void Luizinho::AtualizarGoto(const float deltaTime) {

    const auto* fsm = GetComponent<FSMComponent>();
    const bool naSequencia = fsm && fsm->GetStateName() == kFaseDaSequencia && !EhClone();

    if (!naSequencia) {
        if (mNaSequencia) {
            mNaSequencia = false;
            mAvisandoOGoto = false;
            if (mRotulo) mRotulo->Mostrar(false);
            if (!mLatimDaSequencia.empty()) RegistrarAnuncio(kFaseDaSequencia, mLatimDaSequencia);
        }
        return;
    }

    if (!mNaSequencia) {
        mNaSequencia = true;
        mTempoDoGoto = 0.0f;
        if (mLatimDaSequencia.empty()) mLatimDaSequencia = AnuncioDaFase(kFaseDaSequencia);
    }
    if (!mRotulo) {
        auto rotulo = std::make_unique<RotuloDoGoto>(mScene);
        mRotulo = rotulo.get();
        mScene->AddActor(std::move(rotulo));
    }

    mTempoDoGoto += deltaTime;
    const int proximo = mDesvio->geracao + 1;

    if (!mAvisandoOGoto && mTempoDoGoto >= kGotoACada - kGotoAviso) {
        // O AVISO: o rotulo aparece onde a cabeca vai cair, e o placar diz o
        // goto em latim. O lugar e decidido AGORA e nao muda - quem andou depois
        // do aviso ja sabe para onde a serpente vai.
        mAvisandoOGoto = true;
        mProximoRotulo = OndeFicaORotulo();
        const std::string numero = Rastro::Romano(proximo);
        mRotulo->SetPosition(mProximoRotulo);
        mRotulo->Escrever(numero + ":");
        mRotulo->Mostrar(true);
        RegistrarAnuncio(kFaseDaSequencia, "vāde ad " + numero);
    }

    if (mTempoDoGoto >= kGotoACada) {
        mTempoDoGoto -= kGotoACada;
        mAvisandoOGoto = false;
        mDesvio->rotulo = mProximoRotulo;
        mDesvio->geracao = proximo;
        mRotulo->Mostrar(false);
        RegistrarAnuncio(kFaseDaSequencia, mLatimDaSequencia);
    }
}
