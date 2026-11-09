//
// O orientador do TCC, em campo.
//

#include "Orientador.h"
#include "PoderEmCampo.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "../../Motor/CaminhosArquivo.h"
#include "../../Regras/Escolta.h"
#include "../../Motor/Math.h"
#include "../../Entrada/Painel.h"
#include "../Player/Player.h"
#include "../Player/PlayerProjectile.h"
#include "../Teachers/Boss.h"
#include "../Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../Components/DrawComponents/DrawSpriteComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Scenes/Battle/ProjectileManager.h"

namespace {
    /// O contorno dos aliados: o tiro do orientador tem a cara da materia, e a
    /// mesma cara, do lado do Hugo, nao tem contorno azul.
    constexpr SDL_Color kAliado{90, 220, 255, 230};
    constexpr float kEscalaDoTiro = 0.8f;

    /// O anel do poder em volta dele: a carga, e dourado quando esta pronto.
    constexpr float kRaioDoAnel = 34.0f;
    constexpr int kSegmentos = 32;
    constexpr SDL_Color kCarregando{120, 190, 255, 170};
    constexpr SDL_Color kVazio{80, 90, 120, 90};
    constexpr SDL_Color kPronto{250, 210, 80, 240};
    constexpr SDL_Color kOnda{120, 230, 255, 200};
    constexpr int kOrdemDoAnel = 95;
}

Orientador::Orientador(Scene* scene, std::string professor, const std::string& retrato)
    : Actor(scene),
      mProfessor(std::move(professor)),
      mRetrato(retrato),
      mMateria(Golpe::DoProfessor(mProfessor))
{
    SetScale(kEscala);
    AddComponent<DrawSpriteComponent>(retrato, kOrdemDeDesenho);
    mPoder = Poder::Da(mMateria);
    mEmCampo = PoderEmCampo::Do(mPoder.tipo);

    // O anel do poder e desenhado por um ator a parte, em coordenadas de tela.
    auto anel = std::make_unique<Actor>(scene);
    mLinhas = anel->AddComponent<DrawLinhasComponent>(kOrdemDoAnel);
    scene->AddActor(std::move(anel));
}

// Aqui, e nao no cabecalho: e onde PoderEmCampo esta completo.
Orientador::~Orientador() = default;

Battle* Orientador::Batalha() {
    return dynamic_cast<Battle*>(GetScene());
}

Vector2 Orientador::PostoAgora() {
    Battle* batalha = Batalha();
    if (!batalha) return GetPosition();
    // Na reuniao ele espera o aluno no lugar marcado, um pouco acima do centro.
    if (mReuniao) return *mReuniao + Vector2(0.0f, -24.0f);

    std::vector<Vector2> alunos;
    for (const Player* p : batalha->Jogadores()) {
        if (p) alunos.push_back(p->GetPosition());
    }

    const SDL_FRect campo = batalha->GetPlayfieldBounds();
    const float esquerda = campo.x;
    const float direita = campo.x + campo.w;
    if (alunos.size() == 1) mLado = Escolta::Lado(alunos.front().x, mLado, esquerda, direita);
    return Escolta::Posto(alunos, mLado, esquerda, direita);
}

Vector2 Orientador::PosicaoDoAluno() {
    if (Battle* batalha = Batalha(); batalha && batalha->Alvo()) return batalha->Alvo()->GetPosition();
    return GetPosition();
}

void Orientador::Pousar() {
    mPouso = PostoAgora();
    SetPosition(mPouso);
}

void Orientador::OnUpdate(const float deltaTime) {

    Battle* batalha = Batalha();
    if (!batalha) return;

    mTempo += deltaTime;
    mPouso = Escolta::Seguir(mPouso, PostoAgora(), deltaTime);
    const float balanco = kBalanco * std::sin(mTempo * kBalancoPorSegundo * 2.0f * Math::Pi);
    SetPosition(mPouso + Vector2(0.0f, balanco));

    Guiar(*batalha, deltaTime);
    AtualizarPoder(*batalha, deltaTime);
    DesenharAnel();
    if (!batalha->EmCombate()) return;
    Golpear(*batalha, deltaTime);
}

void Orientador::OnProcessInput(const Uint8* keyState) {
    // O botao 3 de qualquer aluno: o orientador e da dupla.
    bool apertado = false;
    if (const Battle* batalha = dynamic_cast<Battle*>(GetScene())) {
        for (size_t i = 0; i < batalha->Jogadores().size(); ++i) {
            const auto quem = i == 0 ? Painel::Jogador::Um : Painel::Jogador::Dois;
            apertado = apertado || Painel::Apertado(keyState, quem, Painel::Botao::Tres);
        }
    }
    if (apertado && !mBotaoAntes) SoltarPoder();
    mBotaoAntes = apertado;
}

// O golpe.

void Orientador::Golpear(Battle& batalha, const float deltaTime) {

    const Boss* chefe = batalha.Chefe();
    if (!chefe) return;
    if (!mSorteado) {
        mSorteado = true;
        mSorteio = Variacao::SorteioDa(batalha.SementeDaBatalha(), "orientador");
    }
    if (mEspera > 0.0f) {
        mEspera -= deltaTime;
        return;
    }
    const Vector2 alvo = chefe->GetPosition();

    // O observer: um tiro a cada kNotificacoes do aluno - da media, em dupla.
    if (mMateria == Golpe::Materia::MariaLucia) {
        int doAluno = 0, alunos = 0;
        for (const Player* p : batalha.Jogadores()) {
            if (!p) continue;
            doAluno += p->Disparos();
            ++alunos;
        }
        const int devidos = Golpe::Notificacoes(doAluno / std::max(1, alunos), mNotificacoes);
        for (int i = 0; i < devidos; ++i) {
            Disparar(batalha, Golpe::Tiro{}, GetPosition(), Golpe::Rumo(GetPosition(), alvo, 0.0f));
        }
        mNotificacoes += devidos;
        return;
    }

    // A rodada: quando acaba, vem a proxima, e cada tiro sai na hora dele.
    mNaRodada += deltaTime;
    if (mRodada < 0 || mNaRodada >= mDaVez.duracao) {
        if (mRodada >= 0) mNaRodada -= mDaVez.duracao;
        mDaVez = Golpe::DaVez(mMateria, ++mRodada, mSorteio);
        mProximoTiro = 0;
    }
    while (mProximoTiro < mDaVez.tiros.size() && mNaRodada >= mDaVez.tiros[mProximoTiro].quando) {
        const Golpe::Tiro& tiro = mDaVez.tiros[mProximoTiro++];
        const Golpe::Saida saida = Golpe::Sair(tiro, GetPosition(), alvo);
        Disparar(batalha, tiro, saida.de, saida.direcao);
    }
}

PlayerProjectile* Orientador::Disparar(Battle& batalha, const Golpe::Tiro& tiro, const Vector2& de,
                                      const Vector2& direcao, const std::string& folha, const int quadro,
                                      const float escala) {

    ProjectileManager* tiros = batalha.GerenteDeProjeteis();
    if (!tiros) return nullptr;

    // A cara: a da materia dele, ou a do poder; sem materia, o tiro de sempre.
    const int daMateria = Poder::QuadroDaMateria(mMateria);
    std::unique_ptr<PlayerProjectile> p;
    if (quadro >= 0) {
        p = std::make_unique<PlayerProjectile>(GetScene(), this, folha, quadro, escala, kAliado);
    } else if (daMateria >= 0) {
        p = std::make_unique<PlayerProjectile>(GetScene(), this, Poder::kFolhaDasMaterias, daMateria,
                                               kEscalaDoTiro, kAliado);
    } else {
        p = std::make_unique<PlayerProjectile>(GetScene(), this);
    }
    p->ActivateProjectile();   // nasce onde o orientador esta
    p->SetPosition(de);
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) {
        rb->SetVelocity(tiro.espera > 0.0f ? Vector2::Zero : direcao * tiro.velocidade);
    }
    if (tiro.Guiado()) mGuiados.push_back({p->Id(), Golpe::Comecar(tiro, direcao), folha, quadro, escala});
    PlayerProjectile* bruto = p.get();
    tiros->AddPlayerProjectile(std::move(p));
    ++mDisparos;
    return bruto;
}

void Orientador::Guiar(Battle& batalha, const float deltaTime) {

    if (mGuiados.empty()) return;
    ProjectileManager* tiros = batalha.GerenteDeProjeteis();
    const Boss* chefe = batalha.Chefe();
    if (!tiros || !chefe) {
        mGuiados.clear();
        return;
    }
    const Vector2 alvo = chefe->GetPosition();

    // O manager apaga o tiro que morre.
    std::unordered_map<std::uint64_t, PlayerProjectile*> meus;
    for (const auto& p : tiros->GetPlayerProjectiles()) {
        if (p && p->mOwner == this && p->GetState() == ActorState::Active && !p->Murchando()) meus[p->Id()] = p.get();
    }

    struct Filho {
        Golpe::Tiro tiro;
        Vector2 de;
        Vector2 direcao;
        std::string folha;
        int quadro;
        float escala;
    };
    std::vector<Filho> filhos;

    for (auto it = mGuiados.begin(); it != mGuiados.end();) {
        const auto achado = meus.find(it->id);
        auto* rb = achado == meus.end() ? nullptr : achado->second->GetComponent<RigidBodyComponent>();
        if (!rb) {
            it = mGuiados.erase(it);
            continue;
        }
        PlayerProjectile* p = achado->second;
        // A promessa que volta nao e mais guiada: quem a leva e ela mesma.
        if (p->Voltando()) {
            it = mGuiados.erase(it);
            continue;
        }
        const Golpe::Passo passo = Golpe::Avancar(it->voo, p->GetPosition(), rb->GetVelocity(),
                                                  GetPosition(), alvo, deltaTime);
        p->SetPosition(passo.posicao);
        rb->SetVelocity(passo.velocidade);

        // A gramatica: s -> s s.
        if (passo.divide) {
            for (const Golpe::Filho& f : Golpe::Filhos(it->voo, passo.posicao, passo.velocidade, alvo)) {
                filhos.push_back({f.tiro, passo.posicao, f.direcao, it->folha, it->quadro, it->escala});
            }
            p->DeactivateProjectile();
            it = mGuiados.erase(it);
            continue;
        }
        ++it;
    }

    for (const Filho& f : filhos) Disparar(batalha, f.tiro, f.de, f.direcao, f.folha, f.quadro, f.escala);
}

// O poder.

void Orientador::Carregar() {
    if (mPoder.tipo == Poder::Tipo::Nenhum) return;
    mCarga = std::min(Poder::kCarga, mCarga + 1);
    if (PoderPronto() && !mAvisouPronto) {
        mAvisouPronto = true;
        Anunciar("BOTÃO 3: " + mPoder.nome + "\n" + mProfessor + " está pronto");
    }
}

bool Orientador::PoderPronto() const {
    return mPoder.tipo != Poder::Tipo::Nenhum && mCarga >= Poder::kCarga && mTempo >= mPoderAte;
}

void Orientador::Anunciar(const std::string& texto) {
    if (Battle* batalha = Batalha(); batalha && batalha->Chefe()) {
        Boss* chefe = batalha->Chefe();
        chefe->RegistrarAnuncio(chefe->FaseAtual(), texto);
    }
}

Actor* Orientador::Figura(const std::string& folha, const std::vector<int>& quadros, const float escala) {
    auto figura = std::make_unique<Actor>(GetScene());
    figura->SetScale(escala);
    auto* anim = figura->AddComponent<DrawAnimatedComponent>(Caminhos::Asset(folha + ".png"),
                                                             Caminhos::Asset(folha + ".json"), kOrdemDeDesenho + 1);
    anim->AddAnimation("Anda", quadros);
    anim->SetAnimation("Anda");
    figura->SetPosition(GetPosition());
    return GetScene()->AddActor(std::move(figura));
}

std::vector<BossProjectile*> Orientador::TirosSoltos(Battle& batalha) const {
    std::vector<BossProjectile*> soltos;
    if (const ProjectileManager* tiros = batalha.GerenteDeProjeteis()) {
        for (const auto& p : const_cast<ProjectileManager*>(tiros)->GetBossProjectiles()) {
            if (!p || p->GetState() != ActorState::Active) continue;
            if (!p->Posto().empty() || p->Enfeite() || p->Abrigo()) continue;
            soltos.push_back(p.get());
        }
    }
    return soltos;
}

bool Orientador::SoltarPoder() {
    Battle* batalha = Batalha();
    if (!batalha || !batalha->EmCombate() || !PoderPronto()) return false;
    Boss* chefe = batalha->Chefe();
    if (!chefe) return false;

    mCarga = 0;
    mAvisouPronto = false;
    ++mPoderes;
    mPoderAte = mTempo + mPoder.duracao;
    Anunciar(mPoder.nome + "\n" + mPoder.efeito);

    if (mEmCampo) mEmCampo->Soltar(*this, *batalha, PosicaoDoAluno(), chefe->GetPosition());
    return true;
}

void Orientador::AtualizarPoder(Battle& batalha, const float deltaTime) {
    if (mAnelDoPoder >= 0.0f) {
        mAnelDoPoder += deltaTime * 900.0f;
        if (mAnelDoPoder > mPoder.raio) mAnelDoPoder = -1.0f;
    }
    if (mEmCampo) mEmCampo->Atualizar(*this, batalha, deltaTime, mTempo < mPoderAte);
}

void Orientador::DesenharAnel() {
    if (!mLinhas) return;
    mLinhas->Limpar();
    if (mPoder.tipo == Poder::Tipo::Nenhum) return;
    const Vector2 c = GetPosition();
    const float fracao = Poder::Fracao(mCarga);
    const bool pronto = PoderPronto();
    // Pronto, o anel pulsa dourado; carregando, enche no sentido do relogio.
    SDL_Color dourado = kPronto;
    dourado.a = static_cast<Uint8>(150.0f + 100.0f * std::fabs(std::sin(mTempo * 5.0f)));
    for (int k = 0; k < kSegmentos; ++k) {
        const float u0 = static_cast<float>(k) / kSegmentos, u1 = static_cast<float>(k + 1) / kSegmentos;
        const float a0 = -Math::Pi / 2.0f + 2.0f * Math::Pi * u0, a1 = -Math::Pi / 2.0f + 2.0f * Math::Pi * u1;
        const SDL_Color cor = pronto ? dourado : (u1 <= fracao ? kCarregando : kVazio);
        mLinhas->Acrescentar(c + Vector2(Math::Cos(a0), Math::Sin(a0)) * kRaioDoAnel,
                             c + Vector2(Math::Cos(a1), Math::Sin(a1)) * kRaioDoAnel, cor);
    }
    // A onda do free() e do automato, abrindo do aluno.
    if (mAnelDoPoder > 0.0f) {
        const Vector2 aluno = PosicaoDoAluno();
        for (int k = 0; k < kSegmentos; ++k) {
            const float a0 = 2.0f * Math::Pi * static_cast<float>(k) / kSegmentos;
            const float a1 = 2.0f * Math::Pi * static_cast<float>(k + 1) / kSegmentos;
            mLinhas->Acrescentar(aluno + Vector2(Math::Cos(a0), Math::Sin(a0)) * mAnelDoPoder,
                                 aluno + Vector2(Math::Cos(a1), Math::Sin(a1)) * mAnelDoPoder, kOnda);
        }
    }
}
