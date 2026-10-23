//
// O chefe da INF 340 - Linguagens de Programacao.
//

#include "Luizinho.h"

#include <algorithm>
#include <cmath>

#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../CaminhosArquivo.h"
#include "../../../Font.h"
#include "../../../Attacks/AttackParameters/SerpenteAttackParams.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../../Components/DrawComponents/DrawTextComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Game.h"
#include "../../Player/Player.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"
#include "PainelDeCodigo.h"

namespace {

    /// O que o professor diz quando o coletor passa: descansem em paz.
    const char* const kNaVarredura = "requiescant in pace";
    const char* const kTraducaoDaMemoria = "lembra-te de que morrerás";

    const SDL_Color kReferenciaViva = {246, 206, 92, 150};    // ouro: alcancado
    const SDL_Color kReferenciaMorta = {150, 140, 130, 90};   // cinza: lixo

    /// Na frente dos projeteis, como o marcador do intercambio.
    constexpr int kOrdemDoRotulo = 95;

    bool Vivo(const BossProjectile* p) { return p && p->GetState() == ActorState::Active; }

    void PorCara(BossProjectile* p, const std::string& cara) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }

    void Colidir(BossProjectile* p, const bool colide) {
        if (auto* c = p->GetComponent<CircleColliderComponent>()) c->SetEnabled(colide);
    }

    // A fase 1: as cores do texto do programa.
    constexpr SDL_Color kMolduraDoPainel{100, 120, 165, 190};
    constexpr SDL_Color kFundoDoPainel{24, 30, 48, 110};
    constexpr SDL_Color kMolduraDoBuraco{96, 222, 112, 255};      // outra variavel: aqui nao chove
    constexpr SDL_Color kFundoDoBuraco{20, 60, 36, 130};

    constexpr SDL_Color kMolduraDoCanto{120, 140, 180, 230};
    constexpr SDL_Color kFundoDoCanto{28, 32, 46, 225};
    constexpr SDL_Color kTintaDoCanto{214, 226, 250, 255};
    constexpr SDL_Color kMolduraOlhada{150, 150, 160, 230};       // a busca passou e nao achou
    constexpr SDL_Color kTintaOlhada{160, 164, 176, 255};
    constexpr SDL_Color kMolduraAchada{255, 214, 72, 255};        // a declaracao achada
    constexpr SDL_Color kFundoAchado{70, 62, 24, 235};
    constexpr SDL_Color kTintaAchada{255, 240, 190, 255};
    constexpr SDL_Color kTintaDoBuraco{200, 255, 210, 255};

    constexpr SDL_Color kLinhaDaBusca{150, 160, 190, 220};
    constexpr SDL_Color kLinhaAchada{255, 214, 72, 255};
    constexpr SDL_Color kLinhaDaChamada{90, 200, 240, 220};

    constexpr int kTotalDeBlocos = Escopo::kBlocos;
    constexpr Escopo::Bloco kBlocosDoPrograma[kTotalDeBlocos] = {
        Escopo::Bloco::Global, Escopo::Bloco::F, Escopo::Bloco::G, Escopo::Bloco::Interno, Escopo::Bloco::H};
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

std::optional<Vector2> Luizinho::AlvoDoMovimento() const {
    return mAlvo;
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

    if (!mLinhas && mScene) {
        auto ator = std::make_unique<Actor>(mScene);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();

    AtualizarEscopo();
    AtualizarMemoria(deltaTime);
    AtualizarGoto(deltaTime);
}

void Luizinho::AoTerminarABatalha() {
    // A TELA DA NOTA FICA LIMPA: os projeteis a batalha ja levou; o texto do
    // programa e as linhas sao do professor.
    for (PainelDeCodigo* p : mPaineis) if (p) p->Mostrar(false);
    for (EtiquetaDeCodigo* c : mCantos) if (c) c->Mostrar(false);
    if (mLinhas) mLinhas->Limpar();
    if (mReferencias) mReferencias->Limpar();
    if (mRotulo) mRotulo->Mostrar(false);
}

void Luizinho::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O ATAQUE DE GATILHO nasce onde o professor disse, no numero que ele
    // pediu - e so ele: os de relogio das outras fases seguem abaixo.
    if (mDisparandoEm) {
        params.firePosition = mOndeDisparar;
        if (mQuantos > 0) params.numProjectiles = mQuantos;
        return;
    }

    if (stateName == "StateTwo") {
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

void Luizinho::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                    const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

SDL_FRect Luizinho::Campo() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene)) return battle->GetPlayfieldBounds();
    return SDL_FRect{0.0f, 0.0f, Escopo::kLargura, Escopo::kAltura};
}

std::vector<BossProjectile*> Luizinho::DispararEm(const char* gatilho, const Vector2 onde, const int quantos) {
    mDisparandoEm = true;
    mOndeDisparar = onde;
    mQuantos = quantos;
    mCriados.clear();
    Disparar(gatilho);
    mDisparandoEm = false;
    mQuantos = 0;
    return mCriados;
}

void Luizinho::Anunciar(const std::string& fase, const std::string& texto) {
    if (mAnunciosDoArquivo.count(fase) == 0) mAnunciosDoArquivo[fase] = AnuncioDaFase(fase);
    if (AnuncioDaFase(fase) != texto) RegistrarAnuncio(fase, texto);
}

void Luizinho::Restaurar(const std::string& fase) {
    const auto it = mAnunciosDoArquivo.find(fase);
    if (it != mAnunciosDoArquivo.end()) RegistrarAnuncio(fase, it->second);
}

// ---------------------------------------------------------------------------
// A fase 1: o escopo
// ---------------------------------------------------------------------------

void Luizinho::MontarEscopo() {
    const SDL_FRect campo = Campo();
    for (int i = 0; i < kTotalDeBlocos; ++i) {
        const Escopo::Bloco b = kBlocosDoPrograma[i];
        const Escopo::Retangulo r = Escopo::Area(b, campo.w, campo.h);

        // O PAINEL: o global e o campo inteiro, e o cartao de f e so o canto.
        if (b != Escopo::Bloco::Global && b != Escopo::Bloco::F) {
            const int ordem = (b == Escopo::Bloco::Interno) ? kOrdemDoInterno : kOrdemDosPaineis;
            auto painel = std::make_unique<PainelDeCodigo>(mScene, static_cast<int>(r.w), static_cast<int>(r.h), ordem);
            painel->SetPosition(Vector2(campo.x + r.x + r.w / 2.0f, campo.y + r.y + r.h / 2.0f));
            painel->Pintar(kMolduraDoPainel, kFundoDoPainel);
            mPaineis[static_cast<size_t>(i)] = painel.get();
            mScene->AddActor(std::move(painel));
        }

        const std::string texto = Escopo::Declaracoes(b);
        const bool cartao = (b == Escopo::Bloco::F);
        const int largura = cartao ? static_cast<int>(r.w) : static_cast<int>(Escopo::LarguraDoCanto(texto));
        const int altura = cartao ? static_cast<int>(r.h) : static_cast<int>(Escopo::kAlturaDoCanto);
        auto canto = std::make_unique<EtiquetaDeCodigo>(mScene, texto, largura, altura, kOrdemDosCantos);
        const Escopo::Ponto c = Escopo::Canto(b, campo.w, campo.h);
        canto->SetPosition(Vector2(campo.x + c.x, campo.y + c.y));
        canto->Pintar(kMolduraDoCanto, kFundoDoCanto, kTintaDoCanto);
        mCantos[static_cast<size_t>(i)] = canto.get();
        mScene->AddActor(std::move(canto));
    }
}

void Luizinho::DesmontarEscopo() {
    SecarGotas();
    for (PainelDeCodigo*& p : mPaineis) {
        if (p) p->SetState(ActorState::Destroy);
        p = nullptr;
    }
    for (EtiquetaDeCodigo*& c : mCantos) {
        if (c) c->SetState(ActorState::Destroy);
        c = nullptr;
    }
    Restaurar(Escopo::kFaseDoEscopo);
    mNoEscopo = false;
    mComandoDoEscopo = -1;
    mChuva = Chuva::Esperando;
    mAlvo.reset();
}

void Luizinho::SecarGotas() {
    for (const GotaEmCampo& g : mGotas) if (Vivo(g.p)) g.p->Kill();
    mGotas.clear();
}

void Luizinho::ComecarComando(const int k) {
    SecarGotas();
    mComandoDoEscopo = k;
    mForma = Escopo::Proxima(k);
    mInicioDoComando = Escopo::kPrimeiro + static_cast<float>(k) * Escopo::kPorComando;
    mChuva = Chuva::Esperando;
    Anunciar(Escopo::kFaseDoEscopo, Escopo::Placar(mForma));

    const SDL_FRect campo = Campo();
    const Escopo::Ponto onde = Escopo::OndeFicaOProfessor(mForma, campo.w, campo.h);
    mAlvo = Vector2(campo.x + onde.x, campo.y + onde.y);
}

void Luizinho::ChoverAvisos() {
    // A CHUVA NASCE APAGADA, de uma vez, na regiao inteira: e o aviso de onde
    // a variavel e visivel. Sem colisor ate acender - e sem ser enfeite: o
    // Ponto Extra a leva junto, como leva qualquer ataque.
    const SDL_FRect campo = Campo();
    const auto gotas = Escopo::Gotas(mForma, campo.w, campo.h);
    if (gotas.empty()) return;
    const auto criadas = DispararEm(Escopo::kGatilhoDaGota, Vector2(campo.x + campo.w / 2.0f, campo.y),
                                    static_cast<int>(gotas.size()));
    const std::string cara = Escopo::CaraDoAviso(Escopo::Nome(mForma));
    const size_t n = std::min(criadas.size(), gotas.size());
    for (size_t i = 0; i < n; ++i) {
        BossProjectile* p = criadas[i];
        p->SetPosition(Vector2(campo.x + gotas[i].x, campo.y + gotas[i].y));
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        PorCara(p, cara);
        Colidir(p, false);
        p->DefinirPosto("aviso");
        mGotas.push_back({p, campo.y + gotas[i].fim});
    }
}

void Luizinho::AcenderGotas() {
    const std::string cara = Escopo::CaraDaGota(Escopo::Nome(mForma));
    for (const GotaEmCampo& g : mGotas) {
        if (!Vivo(g.p)) continue;
        PorCara(g.p, cara);
        Colidir(g.p, true);
        g.p->DefinirPosto("");
        if (auto* rb = g.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, Escopo::kQueda));
    }
}

void Luizinho::AtualizarEscopo() {
    // O que o campo levou (o Ponto Extra, o fim da batalha) sai da conta
    // antes de qualquer disparo deste quadro: o projetil pode voltar do pool
    // como outro.
    mGotas.erase(std::remove_if(mGotas.begin(), mGotas.end(),
                                [](const GotaEmCampo& g) { return !Vivo(g.p); }), mGotas.end());

    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Escopo::kFaseDoEscopo && !EhClone();
    if (!aqui) {
        if (mNoEscopo) DesmontarEscopo();
        return;
    }
    if (!mNoEscopo) {
        MontarEscopo();
        mNoEscopo = true;
        mComandoDoEscopo = -1;
        // O professor ja vai para o primeiro comando enquanto o placar diz o latim.
        const SDL_FRect campo = Campo();
        const Escopo::Ponto onde = Escopo::OndeFicaOProfessor(Escopo::Proxima(0), campo.w, campo.h);
        mAlvo = Vector2(campo.x + onde.x, campo.y + onde.y);
    }

    const float t = fsm->GetStateTime();
    if (t < Escopo::kPrimeiro) return;
    const int k = std::min(Escopo::kComandos - 1, static_cast<int>((t - Escopo::kPrimeiro) / Escopo::kPorComando));
    if (k != mComandoDoEscopo) ComecarComando(k);

    const float u = t - mInicioDoComando;
    constexpr float kAcende = Escopo::kBusca + Escopo::kAviso;
    constexpr float kSeca = kAcende + Escopo::kVivas;

    if (mChuva == Chuva::Esperando && u >= Escopo::kBusca) {
        ChoverAvisos();
        mChuva = Chuva::Aviso;
    }
    if (mChuva == Chuva::Aviso && u >= kAcende) {
        AcenderGotas();
        mChuva = Chuva::Viva;
    }
    if (mChuva == Chuva::Viva) {
        // A GOTA MORRE NO FIM DELA - a borda de baixo do bloco, ou o buraco
        // embaixo: la a variavel ja e outra.
        for (const GotaEmCampo& g : mGotas) {
            if (Vivo(g.p) && g.p->GetPosition().y >= g.fim) g.p->Kill();
        }
        if (u >= kSeca) {
            SecarGotas();
            mChuva = Chuva::Seca;
        }
    }

    DesenharEscopo(u);
}

void Luizinho::DesenharEscopo(const float u) {
    const SDL_FRect campo = Campo();
    auto canto = [&](const Escopo::Bloco b) {
        const Escopo::Ponto c = Escopo::Canto(b, campo.w, campo.h);
        return Vector2(campo.x + c.x, campo.y + c.y);
    };
    auto indice = [](const Escopo::Bloco b) { return static_cast<size_t>(b); };

    const bool chovendo = (mChuva == Chuva::Aviso || mChuva == Chuva::Viva);
    const bool mostrar = u < Escopo::kBusca + Escopo::kAviso + Escopo::kVivas;

    // TUDO VOLTA AO NORMAL, e o comando acende o que e dele.
    for (PainelDeCodigo* p : mPaineis) if (p) p->Pintar(kMolduraDoPainel, kFundoDoPainel);
    for (EtiquetaDeCodigo* c : mCantos) if (c) c->Pintar(kMolduraDoCanto, kFundoDoCanto, kTintaDoCanto);

    // A CHAMADA: de h ate o cartao de f, o comando inteiro.
    if (const auto quem = Escopo::QuemChamou(mForma)) {
        mLinhas->Acrescentar(canto(*quem), canto(Escopo::Bloco::F), kLinhaDaChamada);
    }
    if (!mostrar) return;

    // A BUSCA: um salto por bloco olhado, do professor ate a declaracao.
    const auto busca = Escopo::Busca(mForma);
    Vector2 de = GetPosition();
    for (size_t i = 0; i < busca.size(); ++i) {
        if (u < static_cast<float>(i) * Escopo::kPorSalto) break;
        const bool achou = (i + 1 == busca.size());
        const Vector2 ate = canto(busca[i]);
        mLinhas->Acrescentar(de, ate, achou ? kLinhaAchada : kLinhaDaBusca);
        if (u >= static_cast<float>(i + 1) * Escopo::kPorSalto) {
            if (EtiquetaDeCodigo* c = mCantos[indice(busca[i])]) {
                if (achou) c->Pintar(kMolduraAchada, kFundoAchado, kTintaAchada);
                else c->Pintar(kMolduraOlhada, kFundoDoCanto, kTintaOlhada);
            }
        }
        de = ate;
    }

    // OS BURACOS: os blocos que tem outra variavel com o nome. Aqui nao chove.
    if (chovendo) {
        for (const Escopo::Bloco b : Escopo::Ocultam(Escopo::Dono(mForma), Escopo::Nome(mForma))) {
            if (PainelDeCodigo* p = mPaineis[indice(b)]) p->Pintar(kMolduraDoBuraco, kFundoDoBuraco);
            if (EtiquetaDeCodigo* c = mCantos[indice(b)]) c->Pintar(kMolduraDoBuraco, kFundoDoCanto, kTintaDoBuraco);
        }
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
    // com ela (4 x 5 s = 20 s), entao o lixo daquela pausa sai marcado e
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
