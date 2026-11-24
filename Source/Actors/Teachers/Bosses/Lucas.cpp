//
// O chefe da INF 321 - Programacao Web.
//

#include "Lucas.h"

#include <algorithm>
#include <cmath>

#include "PainelDeCodigo.h"
#include "BossesProjectiles/BossProjectile.h"
#include "../../Player/Player.h"
#include "../../Player/PlayerProjectile.h"
#include "../../../Attacks/AttackParameters/AttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Motor/AudioSystem.h"
#include "../../../Motor/Game.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../../Scenes/Scene.h"

namespace {
    // A <div>: o azul do CSS na fase 1, o amarelo do JavaScript na fase 2.
    constexpr SDL_Color kBordaCss{120, 180, 255, 230};
    constexpr SDL_Color kFundoCss{70, 140, 240, 70};
    constexpr SDL_Color kBordaJs{250, 214, 90, 230};
    constexpr SDL_Color kFundoJs{240, 200, 60, 60};
    constexpr SDL_Color kTintaCss{190, 220, 255, 255};
    constexpr SDL_Color kTintaJs{255, 236, 160, 255};
    constexpr SDL_Color kMudou{255, 255, 255, 255};
    /// A distancia entre as pecas que machucam da <div>.
    constexpr float kPassoDasPecas = 30.0f;
    /// A regra que acerta a <div>: o raio dela para a conta (Dom::Toca).
    constexpr float kRaioDaRegra = 10.0f;
    constexpr float kMostrarAMudanca = 1.4f;

    // A sala do Elixir.
    constexpr SDL_Color kCaminho{255, 160, 70, 170};
    /// Acendendo, o professor fica logo abaixo da tocha: o corpo dele a protege do tiro do aluno, que
    /// so a apaga depois que ele sai - voltar para apaga-la e uma escolha. No bau, ele chega por cima.
    constexpr float kAbaixoDaTocha = 44.0f;
    constexpr float kAcimaDoBau = 40.0f;
    constexpr SDL_Color kCoracao{255, 90, 110, 255};
    // A fadinha e a mira amarela.
    constexpr SDL_Color kMira{255, 222, 60, 235};
    constexpr SDL_Color kMiraEscura{170, 120, 20, 235};
    constexpr float kRaioDaMiraNaGosma = 30.0f;
    constexpr float kRaioDaMiraNoProfessor = 62.0f;
    constexpr size_t kRastroDaFada = 14;
    // O Phoenix.
    constexpr SDL_Color kSocket{180, 140, 255, 220};
    constexpr SDL_Color kBroadcast{255, 170, 80, 200};

    constexpr float kYDoProfessor = 95.0f;
    /// A moldura do campo fica onde o aluno chega (Player::PrenderNoCampo), um pouco para dentro
    /// da tela: a <div>, as gosmas e as etiquetas ficam dentro dela.
    constexpr float kDentroDaMoldura = 30.0f;
    /// O traco do professor ate a regra que ele acabou de lancar.
    constexpr float kTracoDoLance = 0.35f;
    constexpr float kAbaixoDoAnuncio = 64.0f;
    constexpr float kTagsNoJs = 1.6f;
    constexpr SDL_Color kLance{200, 210, 255, 120};

    /// Quanto o texto de uma etiqueta ocupa, com a folga da caixa.
    int LarguraDaEtiqueta(const std::string& texto) {
        return std::clamp(static_cast<int>(static_cast<float>(texto.size()) * 8.2f) + 18, 40, 420);
    }
    constexpr int kAlturaDaEtiqueta = 26;
    /// Um pouco maior que a das outras etiquetas de codigo: a regra se le enquanto voa.
    constexpr int kLetraDaEtiqueta = 24;
}

Lucas::Lucas(Scene* scene) : Boss(scene) {}

void Lucas::SortearALuta() {
    mLutaSorteada = true;
    mSorteioDoDom = SorteioDaFase(Dom::kFaseDoCss);
    mSorteioDaMasmorra = SorteioDaFase(Masmorra::kFase);
    mSorteioDaFenix = SorteioDaFase(Fenix::kFase);
    mDegrauDoDom = Dom::DegrauDo(NivelDeDificuldade());
    mDegrauDaMasmorra = Masmorra::DegrauDo(NivelDeDificuldade());
    mDegrauDaFenix = Fenix::DegrauDo(NivelDeDificuldade());
}

// ---- O quadro ----

void Lucas::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    // Antes de qualquer disparo deste quadro: o que morreu no quadro passado e esquecido.
    Sanear();
    Boss::OnUpdate(deltaTime);
    if (EhClone()) return;
    mRelogio += deltaTime;

    const auto* fsm = GetComponent<FSMComponent>();
    const std::string fase = fsm ? fsm->GetStateName() : std::string();
    const float t = fsm ? fsm->GetStateTime() : 0.0f;
    if (fase != mFase) {
        // document.querySelector("div"): a fase 2 pega a MESMA <div> da fase 1, com tudo o que o
        // CSS fez nela. Ela nunca some nem e destruida no meio da luta.
        Desmontar(mFase == Dom::kFaseDoCss && fase == Dom::kFaseDoJs);
        mFase = fase;
    }
    if (!mLinhas && mScene) {
        auto ator = std::make_unique<Actor>(mScene);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();
    EsconderEtiquetas();

    const Battle* b = Batalha();
    if (!b || !b->EmCombate()) return;
    if (fase == Dom::kFaseDoCss) AtualizarDom(t, deltaTime, false);
    else if (fase == Dom::kFaseDoJs) AtualizarDom(t, deltaTime, true);
    else if (fase == Masmorra::kFase) AtualizarMasmorra(t, deltaTime);
    else if (fase == Fenix::kFase) AtualizarFenix(t, deltaTime);
}

void Lucas::AoTerminarABatalha() {
    Desmontar();
}

void Lucas::Desmontar(const bool manterADiv) {
    if (manterADiv) {
        // A fase 2 recomeca os lances (e a demonstracao), mas com a mesma <div> e as regras no ar.
        mLancesNaFase = 0;
        mProximoLance = mRelogio + 1.0f;
    } else {
        for (DivViva& d : mDivs) MatarTodos(d.pecas);
        mDivs.clear();
        for (RegraViva& r : mRegras) Matar(r.p);
        mRegras.clear();
        mDomComecou = false;
    }

    MatarTodos(mGosmas);
    mRumoDasGosmas.clear();
    Matar(mBauP);
    Matar(mChaveP);
    Matar(mElixirP);
    MatarTodos(mTochasP);
    mTochas.clear();
    mFadas.clear();
    mRastros.clear();
    mAcesas.clear();
    mAcendendo = -1;
    mRodadaNaSala = false;
    mDesfecho = Desfecho::EmAberto;
    mComAChave = -1;
    mInicioDaFuria = -1.0f;
    RestaurarAnuncio(Masmorra::kFase);

    Matar(mFenixP);
    Matar(mCinzasP);
    mEstado = EstadoDaFenix::Nenhuma;
    mVersao = 0;
    mBroadcastEm = -1.0f;
    RestaurarAnuncio(Fenix::kFase);

    mAlvo.reset();
    if (mLinhas) mLinhas->Limpar();
    EsconderEtiquetas();
}

void Lucas::Varrer(const float t, const float amplitude, const float periodo) {
    const SDL_FRect c = Campo();
    mAlvo = Vector2(c.x + c.w / 2.0f + c.w * amplitude * Math::Sin(Math::TwoPi * t / periodo), c.y + kYDoProfessor);
}

float Lucas::FatorDoGanho() const {
    return mRelogio < mElixirAte ? mDegrauDaMasmorra.fatorDoGanho : 1.0f;
}

void Lucas::OnDerrubou(BossProjectile* qual) {
    if (!qual) return;
    for (const BossProjectile* g : mGosmas) {
        if (g == qual) {
            ++mGosmasDerrubadas;
            mUltimaGosma = qual->GetPosition();
        }
    }
    if (qual == mFenixP) {
        mFenixDerrubada = true;
        mOndeCaiu = qual->GetPosition();
    }
    if (qual == mCinzasP) mCinzasApagadas = true;
}

// ---- O que as fases dividem ----

std::vector<BossProjectile*> Lucas::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
                                               const std::optional<float> centro, const std::optional<float> abertura,
                                               const std::optional<float> velocidade) {
    mDisparandoEm = true;
    mOndeDisparar = onde;
    mQuantos = quantos;
    mCentroEm = centro;
    mAberturaEm = abertura;
    mVelocidadeEm = velocidade;
    mCriados.clear();
    Disparar(gatilho);
    mDisparandoEm = false;
    mQuantos = 0;
    mCentroEm.reset();
    mAberturaEm.reset();
    mVelocidadeEm.reset();
    return mCriados;
}

BossProjectile* Lucas::Um(const char* gatilho, const Vector2 onde, const std::string& cara,
                          const std::optional<Vector2> velocidade) {
    std::optional<float> centro, rapidez;
    if (velocidade) {
        centro = Math::ToDegrees(Math::Atan2(velocidade->y, velocidade->x));
        rapidez = velocidade->Length();
    }
    const auto criados = DispararEm(gatilho, onde, 1, centro, 0.0f, rapidez.value_or(0.0f));
    if (criados.empty()) return nullptr;
    BossProjectile* p = criados.front();
    for (size_t i = 1; i < criados.size(); ++i) criados[i]->Kill();
    p->SetPosition(onde);
    Cara(p, cara);
    Andar(p, velocidade.value_or(Vector2::Zero));
    return p;
}

BossProjectile* Lucas::Peca(const char* gatilho, const Vector2 onde, const std::string& cara) {
    BossProjectile* p = Um(gatilho, onde, cara);
    if (p) p->DefinirMargemDeSaida(2.0f, 1.0f);
    return p;
}

void Lucas::Leque(const char* gatilho, const Vector2 de, const int quantos, const float abertura, const float velocidade) {
    const float centro = AnguloAteOAluno(de);
    const auto criados = DispararEm(gatilho, de, std::max(1, quantos), centro, abertura, velocidade);
    // O mesmo leque do WaveAttack, mas ja (ele so poe a velocidade no primeiro quadro).
    const size_t n = criados.size();
    const float inicio = centro - abertura / 2.0f;
    const float passo = n > 1 ? abertura / static_cast<float>(n - 1) : 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const float a = Math::ToRadians(inicio + passo * static_cast<float>(i));
        Andar(criados[i], Vector2(Math::Cos(a), Math::Sin(a)) * velocidade);
    }
    SomDoDisparo();
}

float Lucas::AnguloAteOAluno(const Vector2 de) const {
    const Vector2 d = PosicaoDoAluno() - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

bool Lucas::Vivo(const BossProjectile* p) {
    return p && p->GetState() == ActorState::Active;
}

void Lucas::Matar(BossProjectile*& p) {
    if (Vivo(p)) p->Kill();
    p = nullptr;
}

void Lucas::MatarTodos(std::vector<BossProjectile*>& ps) {
    for (BossProjectile*& p : ps) Matar(p);
    ps.clear();
}

void Lucas::Cara(BossProjectile* p, const std::string& cara) {
    if (!p || cara.empty()) return;
    if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) anim->SetAnimation(cara);
}

void Lucas::Andar(BossProjectile* p, const Vector2 velocidade) {
    if (!p) return;
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(velocidade);
}

void Lucas::Prender(BossProjectile* p, const Vector2 onde) {
    if (!p) return;
    p->SetPosition(onde);
    Andar(p, Vector2::Zero);
    p->RenovarTempoDeVida();
}

Dom::Campo Lucas::CampoPuro() const {
    const SDL_FRect c = Campo();
    return Dom::Campo{c.x + kDentroDaMoldura, c.y + 8.0f, c.w - 2.0f * kDentroDaMoldura, c.h - 16.0f};
}

void Lucas::Sanear() {
    // A regra que morreu (derrubada, no aluno, fora do campo ou na onda do Ponto Extra) sai da lista.
    // A que acerta o aluno o prende sozinha (DefinirAcorrentamento, no lance): o CSS pesa, o
    // JavaScript trava. A que tocou a <div> o Lucas ja matou e esqueceu.
    for (RegraViva& r : mRegras) if (!Vivo(r.p)) r.p = nullptr;
    mRegras.erase(std::remove_if(mRegras.begin(), mRegras.end(), [](const RegraViva& r) { return !r.p; }), mRegras.end());
    // A peca da <div> que morreu sem o Lucas mata-la acertou um aluno (ou foi a onda do Ponto Extra):
    // a <div> recua, em vez de ficar colada nele.
    for (DivViva& d : mDivs) {
        bool acertou = false;
        for (BossProjectile*& p : d.pecas) {
            if (p && !Vivo(p)) {
                acertou = true;
                p = nullptr;
            }
        }
        if (acertou) Dom::Recuar(d.div, PosicaoDoAluno(), mRelogio);
    }
    // A gosma e a fenix tambem morrem na onda do Ponto Extra, sem passar por OnDerrubou: contam como
    // derrubadas, e no lugar onde estavam (a posicao do projetil morto vale ate ele voltar do pool).
    for (BossProjectile*& g : mGosmas) {
        if (g && !Vivo(g)) {
            mUltimaGosma = g->GetPosition();
            g = nullptr;
        }
    }
    // A tocha acesa derrubada (o tiro do aluno a apagou): a sala poe a apagada no lugar.
    for (BossProjectile*& p : mTochasP) if (!Vivo(p)) p = nullptr;
    if (!Vivo(mBauP)) mBauP = nullptr;
    if (!Vivo(mChaveP)) mChaveP = nullptr;
    if (!Vivo(mElixirP)) mElixirP = nullptr;
    if (mFenixP && !Vivo(mFenixP)) {
        mOndeCaiu = mFenixP->GetPosition();
        mFenixDerrubada = true;
    }
    if (!Vivo(mFenixP)) mFenixP = nullptr;
    if (!Vivo(mCinzasP)) mCinzasP = nullptr;
}

void Lucas::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
    if (!mDisparandoEm) return;
    params.firePosition = mOndeDisparar;
    if (mQuantos > 0) params.numProjectiles = mQuantos;
    if (mCentroEm) params.centralAngle = *mCentroEm;
    if (mAberturaEm) params.angle = *mAberturaEm;
    if (mVelocidadeEm) params.projectileSpeed = *mVelocidadeEm;
}

void Lucas::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles, const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

// ---- O desenho ----

void Lucas::Linha(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Acrescentar(de, ate, cor);
}

void Lucas::Retangulo(const Vector2 centro, const float largura, const float altura, const SDL_Color cor) {
    const Vector2 a = centro - Vector2(largura / 2.0f, altura / 2.0f), b = centro + Vector2(largura / 2.0f, altura / 2.0f);
    Linha(a, Vector2(b.x, a.y), cor);
    Linha(Vector2(b.x, a.y), b, cor);
    Linha(b, Vector2(a.x, b.y), cor);
    Linha(Vector2(a.x, b.y), a, cor);
}

void Lucas::Preencher(const Vector2 centro, const float largura, const float altura, const SDL_Color cor) {
    if (mLinhas) mLinhas->Preencher(centro - Vector2(largura / 2.0f, altura / 2.0f), centro + Vector2(largura / 2.0f, altura / 2.0f), cor);
}

void Lucas::Circulo(const Vector2 centro, const float raio, const SDL_Color cor, const int lados) {
    for (int k = 0; k < lados; ++k) {
        const float a0 = Math::TwoPi * static_cast<float>(k) / static_cast<float>(lados);
        const float a1 = Math::TwoPi * static_cast<float>(k + 1) / static_cast<float>(lados);
        Linha(centro + Vector2(Math::Cos(a0), Math::Sin(a0)) * raio, centro + Vector2(Math::Cos(a1), Math::Sin(a1)) * raio, cor);
    }
}

void Lucas::Disco(const Vector2 centro, const float raio, const SDL_Color cor) {
    for (float dy = -raio; dy <= raio; dy += 1.0f) {
        const float dx = std::sqrt(std::max(0.0f, raio * raio - dy * dy));
        Linha(centro + Vector2(-dx, dy), centro + Vector2(dx, dy), cor);
    }
}

void Lucas::Tracejada(const Vector2 de, const Vector2 ate, const SDL_Color cor, const float traco) {
    const Vector2 d = ate - de;
    const float comprimento = d.Length();
    if (comprimento < 1.0f) return;
    const Vector2 u = d * (1.0f / comprimento);
    for (float s = 0.0f; s < comprimento; s += traco * 2.0f) Linha(de + u * s, de + u * std::min(comprimento, s + traco), cor);
}

void Lucas::Coracao(const Vector2 centro, const float tamanho, const SDL_Color cor) {
    Disco(centro + Vector2(-tamanho * 0.5f, 0.0f), tamanho * 0.55f, cor);
    Disco(centro + Vector2(tamanho * 0.5f, 0.0f), tamanho * 0.55f, cor);
    for (float s = 0.0f; s <= tamanho * 1.1f; s += 1.0f) {
        const float meia = tamanho * 1.05f * (1.0f - s / (tamanho * 1.1f));
        Linha(centro + Vector2(-meia, s), centro + Vector2(meia, s), cor);
    }
}

void Lucas::Etiqueta(const std::string& texto, Vector2 onde, const SDL_Color tinta) {
    if (!mScene) return;
    while (mEtiquetas.size() <= mEtiquetasUsadas) {
        auto nova = std::make_unique<EtiquetaDeCodigo>(mScene, " ", 250, kAlturaDaEtiqueta, kOrdemDasEtiquetas, kLetraDaEtiqueta);
        nova->Mostrar(false);
        mEtiquetas.push_back(nova.get());
        mScene->AddActor(std::move(nova));
    }
    EtiquetaDeCodigo* e = mEtiquetas[mEtiquetasUsadas++];
    const int largura = LarguraDaEtiqueta(texto);
    e->Redimensionar(largura, kAlturaDaEtiqueta);
    // Inteira dentro do campo, mesmo a da <div> encostada na borda.
    const Dom::Campo c = CampoPuro();
    const float meia = static_cast<float>(largura) / 2.0f, meiaAltura = kAlturaDaEtiqueta / 2.0f;
    onde = Vector2(std::clamp(onde.x, c.x + meia, std::max(c.x + meia, c.x + c.w - meia)),
                   std::clamp(onde.y, c.y + meiaAltura, std::max(c.y + meiaAltura, c.y + c.h - meiaAltura)));
    e->Pintar(SDL_Color{tinta.r, tinta.g, tinta.b, 150}, SDL_Color{12, 16, 28, 215}, tinta);
    e->Escrever(texto);
    e->SetPosition(onde);
    e->Mostrar(true);
}

void Lucas::EsconderEtiquetas() {
    for (EtiquetaDeCodigo* e : mEtiquetas) if (e) e->Mostrar(false);
    mEtiquetasUsadas = 0;
}

// ---- Fases 1 e 2: a <div> ----

void Lucas::PosicionarPecas(DivViva& d, const float agora) {
    const std::vector<Vector2> onde = Dom::Pecas(d.div, agora, kPassoDasPecas);
    // Sobram pecas: as de mais saem. Faltam: nascem.
    while (d.pecas.size() > onde.size()) {
        Matar(d.pecas.back());
        d.pecas.pop_back();
    }
    d.pecas.resize(onde.size(), nullptr);
    for (size_t i = 0; i < onde.size(); ++i) {
        if (!d.pecas[i]) d.pecas[i] = Peca("peca", onde[i], "Vazia");
        Prender(d.pecas[i], onde[i]);
    }
}

void Lucas::DesenharDiv(const DivViva& d, const float agora, const bool js) {
    const auto caixas = Dom::Caixas(d.div, agora);
    if (caixas.empty()) return;
    const SDL_Color borda = js ? kBordaJs : kBordaCss, fundo = js ? kFundoJs : kFundoCss;
    // O aviso do transbordo: a borda que vai transbordar acende logo antes.
    const bool vaiTransbordar = d.div.proximoTransbordo - agora < Dom::kAvisoDoTransbordo;
    for (const Dom::Caixa& k : caixas) {
        if (k.redonda) {
            const float r = std::min(k.largura, k.altura) / 2.0f;
            Disco(k.centro, r, fundo);
            Circulo(k.centro, r, vaiTransbordar ? kMudou : borda, 28);
            Circulo(k.centro, r - 1.0f, borda, 28);
            if (vaiTransbordar) Circulo(k.centro, r + 2.0f, kMudou, 28);
        } else {
            Preencher(k.centro, k.largura, k.altura, fundo);
            Retangulo(k.centro, k.largura, k.altura, borda);
            Retangulo(k.centro, k.largura - 2.0f, k.altura - 2.0f, borda);
            if (vaiTransbordar) {
                const float y = k.centro.y + k.altura / 2.0f;
                for (float dy = 0.0f; dy < 3.0f; dy += 1.0f) {
                    Linha(Vector2(k.centro.x - k.largura / 2.0f, y + dy), Vector2(k.centro.x + k.largura / 2.0f, y + dy), kMudou);
                }
            }
        }
    }
    // A tag em cima, e o comportamento (o que o JavaScript ja ligou) embaixo.
    const float meiaAltura = d.div.altura * Dom::Escala(d.div, agora) / 2.0f;
    // A regra que acabou de muda-la aparece um instante no lugar da tag.
    const bool mudou = agora < d.ultimaAte && !d.ultima.empty();
    Etiqueta(mudou ? d.ultima : "<div>", d.div.centro - Vector2(0.0f, meiaAltura + 18.0f),
             mudou ? kMudou : (js ? kTintaJs : kTintaCss));
    std::string comportamento;
    const auto juntar = [&](const char* s) { comportamento += (comportamento.empty() ? "" : " ") + std::string(s); };
    if (d.div.intervalo > 0.0f) juntar("setInterval");
    if (d.div.entreLeques > 0.0f) juntar("forEach");
    if (d.div.revida) juntar("onclick");
    if (d.div.segue) juntar("mousemove");
    if (d.div.sobre) juntar("onmouseover");
    if (d.div.teleporteACada > 0.0f) juntar("random");
    if (d.div.bombaEm >= agora) juntar("setTimeout");
    if (!comportamento.empty()) Etiqueta(comportamento, d.div.centro + Vector2(0.0f, meiaAltura + 18.0f), kTintaJs);
    // A bomba do setTimeout pisca mais rapido conforme o pavio acaba.
    if (d.div.bombaEm >= agora) {
        const float falta = d.div.bombaEm - agora;
        if (std::fmod(agora, std::max(0.08f, falta / 6.0f)) < 0.05f) {
            Circulo(d.div.centro, caixas.front().altura * 0.4f + 6.0f, SDL_Color{255, 90, 60, 255}, 24);
        }
    }
}

void Lucas::AtualizarDom(const float t, const float deltaTime, const bool js) {
    Battle* b = Batalha();
    if (!b) return;
    // O relogio da <div> e o do chefe, que nao zera entre as fases: a mesma <div> passa da fase 1
    // para a 2, com os relogios dela.
    const float agora = mRelogio;
    // A <div> fica abaixo do anuncio do professor.
    Dom::Campo c = CampoPuro();
    c.y += kAbaixoDoAnuncio;
    c.h -= kAbaixoDoAnuncio;
    const Dom::Degrau& g = mDegrauDoDom;
    Varrer(t, 0.32f, 9.0f);

    // A <div> nova, pequena e previsivel, no comeco da fase 1.
    if (!mDomComecou) {
        mDomComecou = true;
        mDivs.push_back(DivViva{Dom::Nova(c, g.rapidezDaDiv, g.entreTransbordos, agora, mSorteioDoDom), {}, "", -1.0f});
        mProximoLance = agora + g.calma;
        mProximaTag = agora + g.calma + 1.0f;
        mLancesNaFase = 0;
    }

    const Vector2 aluno = PosicaoDoAluno();
    const ProjectileManager* tiros = b->GerenteDeProjeteis();
    std::vector<Dom::Div> clones;
    for (DivViva& d : mDivs) {
        Dom::Andar(d.div, deltaTime, agora, aluno, c, mSorteioDoDom);
        const Vector2 centro = d.div.centro;
        const Dom::Tiros disparos = Dom::Atirar(d.div, agora, aluno);
        // O overflow, sempre: o conteudo transborda da <div>.
        if (disparos.transbordo) {
            for (const Dom::Gota& gota : Dom::Gotas(d.div, agora)) {
                Um("transbordo", gota.onde + gota.rumo * 6.0f, "", gota.rumo * g.velocidadeDasGotas);
            }
        }
        // O comportamento: o setTimeout que estoura, o setInterval, o forEach e o onmouseover.
        if (disparos.explosao) {
            DispararEm("explosao", centro, g.tirosDaExplosao);
            SomDoDisparo();
        }
        if (disparos.anel) DispararEm("setinterval", centro, g.tirosDoAnel);
        if (disparos.leque) Leque("foreach", centro, g.tirosDoLeque, 36.0f, 240.0f);
        if (disparos.queda) {
            const Vector2 base = centro + Vector2(0.0f, d.div.altura * Dom::Escala(d.div, agora) / 2.0f);
            for (int k = 0; k < 3; ++k) Um("mouseover", base + Vector2(0.0f, -14.0f * static_cast<float>(k)), "", Vector2(0.0f, g.velocidadeDaQueda));
        }
        // O onclick: o tiro do aluno que passa pela <div> volta como leque.
        if (d.div.revida && tiros) {
            for (const auto& tiro : const_cast<ProjectileManager*>(tiros)->GetPlayerProjectiles()) {
                if (!tiro || tiro->GetState() != ActorState::Active) continue;
                if (!Dom::Toca(d.div, tiro->GetPosition(), 6.0f, agora) || !Dom::Revidar(d.div, agora)) continue;
                Leque("revide", centro, g.tirosDoRevide, 30.0f, 230.0f);
                break;
            }
        }
        // As regras que acertaram esta <div>: mudam ela, e somem.
        for (RegraViva& r : mRegras) {
            if (!Vivo(r.p) || !Dom::Toca(d.div, r.p->GetPosition(), kRaioDaRegra, agora)) continue;
            if (Dom::Aplicar(d.div, r.efeito, agora) && mDivs.size() + clones.size() < static_cast<size_t>(Dom::kMaisDivs)) {
                // cloneNode: a copia sai para o outro lado.
                Dom::Div copia = d.div;
                copia.velocidade = Vector2(-d.div.velocidade.x, d.div.velocidade.y);
                // Do lado de onde ela vem, a uma largura dela: as duas nao nascem uma em cima da outra.
                const float lado = d.div.largura * Dom::Escala(d.div, agora) + 30.0f;
                copia.centro = d.div.centro + Vector2(d.div.velocidade.x < 0.0f ? lado : -lado, 0.0f);
                clones.push_back(copia);
            }
            d.ultima = Dom::Texto(r.efeito);
            d.ultimaAte = agora + kMostrarAMudanca;
            b->Faiscar(r.p->GetPosition(), Hud::Faisca::Dissolvido);
            Matar(r.p);
        }
        PosicionarPecas(d, agora);
        DesenharDiv(d, agora, js);
    }
    for (const Dom::Div& copia : clones) mDivs.push_back(DivViva{copia, {}, "", -1.0f});
    mRegras.erase(std::remove_if(mRegras.begin(), mRegras.end(), [](const RegraViva& r) { return !r.p; }), mRegras.end());

    // O lance: o professor joga as regras no campo, e elas entram pelas laterais, rumo a um ponto
    // dele - as vezes a propria <div>. Pelas laterais, e nao de dentro dele: o aluno que so atira
    // no professor nao as derruba sem querer, e escolhe o que derrubar. Os primeiros lances de cada
    // fase vao todos na <div>, e na fase 2 todos de JavaScript: a mecanica se mostra.
    if (agora >= mProximoLance) {
        mProximoLance = agora + g.entreLances;
        const bool demonstracao = mLancesNaFase < Dom::kLancesDeDemonstracao;
        ++mLancesNaFase;
        for (int k = 0; k < g.regrasPorLance; ++k) {
            const bool ehJs = js && (demonstracao || !mSorteioDoDom.Chance(g.chanceDeCssNoJs));
            const bool daEsquerda = mSorteioDoDom.Chance(0.5f);
            const Vector2 de(daEsquerda ? c.x + 4.0f : c.x + c.w - 4.0f, c.y + c.h * mSorteioDoDom.Real(0.12f, 0.6f));
            // A mirada na <div> sai para onde ela vai estar (sem contar os quiques), e faz a curva
            // atras dela no caminho; a outra vai reta para um ponto da metade de baixo.
            int qual = -1;
            if (!mDivs.empty() && (demonstracao || mSorteioDoDom.Chance(Dom::kChanceDeMirarADiv))) {
                qual = mSorteioDoDom.Inteiro(0, static_cast<int>(mDivs.size()) - 1);
            }
            Vector2 para;
            if (qual >= 0) {
                const Dom::Div& alvo = mDivs[static_cast<size_t>(qual)].div;
                const float chegada = (alvo.centro - de).Length() / std::max(1.0f, g.velocidadeDasRegras);
                para = alvo.centro + alvo.velocidade * chegada;
                para = Vector2(std::clamp(para.x, c.x, c.x + c.w), std::clamp(para.y, c.y, c.y + c.h));
            } else {
                para = Dom::Destino(c, nullptr, mSorteioDoDom);
            }
            // A regra que muda alguma coisa na <div> que ela persegue (ou na primeira).
            const Dom::Div* quem = qual >= 0 ? &mDivs[static_cast<size_t>(qual)].div : (mDivs.empty() ? nullptr : &mDivs.front().div);
            const Dom::Efeito e = Dom::Sortear(ehJs, mSorteioDoDom, quem);
            Vector2 rumo = para - de;
            if (rumo.Length() < 1.0f) rumo = Vector2(0.0f, 1.0f);
            rumo = rumo * (g.velocidadeDasRegras / rumo.Length());
            if (BossProjectile* p = Um("regra", de, ehJs ? "Js" : "Css", rumo)) {
                // No aluno, alem da nota, a regra o prende um instante.
                if (Dom::EhJs(e)) p->DefinirAcorrentamento(Dom::kTravaDoJs, Dom::kFatorDoJs, 1.0f);
                else p->DefinirAcorrentamento(Dom::kPesoDoCss, Dom::kFatorDoCss, 1.0f);
                mRegras.push_back(RegraViva{p, e, agora, qual});
            }
        }
        SomDoDisparo();
    }
    // O leque do proprio professor - mais espacado na fase 2, em que a <div> ja atira por conta dela.
    if (agora >= mProximaTag) {
        mProximaTag = agora + g.entreTags * (js ? kTagsNoJs : 1.0f);
        Leque("tag", GetPosition() + Vector2(0.0f, 40.0f), g.tagsPorLeque, 40.0f, 230.0f);
    }
    // A curva atras da <div>, e a etiqueta de cada regra, em cima dela.
    for (const RegraViva& r : mRegras) {
        if (!Vivo(r.p)) continue;
        if (r.div >= 0 && static_cast<size_t>(r.div) < mDivs.size() && agora - r.desde < Dom::kCurvaDaRegra) {
            if (auto* rb = r.p->GetComponent<RigidBodyComponent>()) {
                rb->SetVelocity(Dom::Curvar(rb->GetVelocity(), r.p->GetPosition(), mDivs[static_cast<size_t>(r.div)].div.centro,
                                            Dom::kGiroDaRegra, deltaTime));
            }
        }
        if (agora - r.desde < kTracoDoLance) Tracejada(GetPosition(), r.p->GetPosition(), kLance, 6.0f);
        Etiqueta(Dom::Texto(r.efeito), r.p->GetPosition() + Vector2(0.0f, -24.0f), Dom::EhJs(r.efeito) ? kTintaJs : kTintaCss);
    }
}

// ---- Fase 3: a sala do Elixir ----

void Lucas::NovoCiclo(const float t) {
    MatarTodos(mGosmas);
    mRumoDasGosmas.clear();
    Matar(mBauP);
    Matar(mChaveP);
    Matar(mElixirP);
    MatarTodos(mTochasP);
    const Masmorra::Degrau& g = mDegrauDaMasmorra;
    const Dom::Campo c = CampoPuro();
    mBau = Masmorra::LugarDoBau(mSorteioDaMasmorra, c, PosicaoDoAluno());
    mBauP = Peca("item", mBau, "Bau");
    if (mBauP) mBauP->DefinirEnfeite(true);
    for (const Vector2& p : Masmorra::LugaresDasGosmas(mSorteioDaMasmorra, g.gosmas, c, mBau)) {
        mGosmas.push_back(Peca("gosma", p, "Andando"));
        const float a = mSorteioDaMasmorra.Real(0.0f, Math::TwoPi);
        mRumoDasGosmas.push_back(Vector2(Math::Cos(a), Math::Sin(a)) * g.rapidezDasGosmas);
    }
    // As tochas, apagadas, na ordem em que o professor vai acende-las.
    mTochas = Masmorra::LugaresDasTochas(mSorteioDaMasmorra, Masmorra::kTochas, c, mBau, GetPosition());
    mAcesas.assign(mTochas.size(), false);
    mTochasP.assign(mTochas.size(), nullptr);
    mAcendendo = -1;
    mRodadaNaSala = true;
    mGosmasDerrubadas = 0;
    mChaveCaiu = false;
    mComAChave = -1;
    mDesfecho = Desfecho::EmAberto;
    mInicioDaFuria = -1.0f;
    mProximoLeque = t + 1.2f;
    // \xc3\x89 e o E agudo.
    TrocarAnuncio(Masmorra::kFase, "\xc3\x89 perigoso ir sozinho!");
}

void Lucas::AtualizarMasmorra(const float t, const float deltaTime) {
    Battle* b = Batalha();
    if (!b) return;
    const Masmorra::Degrau& g = mDegrauDaMasmorra;
    const Dom::Campo c = CampoPuro();
    // A rodada seguinte comeca um pouco depois do desfecho da anterior.
    if (!mRodadaNaSala || (mDesfecho != Desfecho::EmAberto && t >= mProximaRodada)) NovoCiclo(t);
    const std::vector<Player*>& alunos = b->Jogadores();

    // As gosmas andam pela faixa do meio da sala, quicando.
    for (size_t i = 0; i < mGosmas.size() && i < mRumoDasGosmas.size(); ++i) {
        BossProjectile* gosma = mGosmas[i];
        if (!gosma) continue;
        Vector2 p = gosma->GetPosition() + mRumoDasGosmas[i] * deltaTime;
        Vector2& v = mRumoDasGosmas[i];
        if (p.x < c.x + 30.0f || p.x > c.x + c.w - 30.0f) v.x = -v.x;
        if (p.y < c.y + c.h * 0.2f || p.y > c.y + c.h * 0.72f) v.y = -v.y;
        p = Vector2(std::clamp(p.x, c.x + 30.0f, c.x + c.w - 30.0f), std::clamp(p.y, c.y + c.h * 0.2f, c.y + c.h * 0.72f));
        Prender(gosma, p);
    }
    // Derrotadas todas, a chave pequena cai onde caiu a ultima.
    const bool semGosmas = std::none_of(mGosmas.begin(), mGosmas.end(), [](const BossProjectile* p) { return p != nullptr; });
    if (semGosmas && !mChaveCaiu && mDesfecho == Desfecho::EmAberto) {
        mChaveCaiu = true;
        mChaveP = Peca("item", mUltimaGosma, "Chave");
        if (mChaveP) mChaveP->DefinirEnfeite(true);
        b->Faiscar(mUltimaGosma, Hud::Faisca::Coleta);
        TrocarAnuncio(Masmorra::kFase, "{:ok, :chave}");
    }
    // A chave: quem encosta pega, e ela vai em cima da cabeca dele.
    if (mChaveP && mComAChave < 0) {
        Prender(mChaveP, mChaveP->GetPosition());
        for (size_t i = 0; i < alunos.size(); ++i) {
            if (alunos[i] && Masmorra::Alcanca(alunos[i]->GetPosition(), mChaveP->GetPosition())) {
                mComAChave = static_cast<int>(i);
                b->Faiscar(mChaveP->GetPosition(), Hud::Faisca::Coleta);
                break;
            }
        }
    }
    if (mChaveP && mComAChave >= 0 && static_cast<size_t>(mComAChave) < alunos.size() && alunos[static_cast<size_t>(mComAChave)]) {
        Prender(mChaveP, alunos[static_cast<size_t>(mComAChave)]->GetPosition() + Vector2(0.0f, -Masmorra::kAcimaDaCabeca));
    }
    if (mBauP) Prender(mBauP, mBau);
    // As tochas: a apagada e enfeite; a acesa, o tiro do aluno apaga - e ela volta apagada.
    for (size_t i = 0; i < mTochas.size() && i < mTochasP.size(); ++i) {
        if (!mTochasP[i]) {
            if (mAcesas[i]) b->Faiscar(mTochas[i], Hud::Faisca::Dissolvido);
            mAcesas[i] = false;
            mTochasP[i] = Peca("tocha", mTochas[i], "Apagada");
            if (mTochasP[i]) mTochasP[i]->DefinirEnfeite(true);
        }
        Prender(mTochasP[i], mTochas[i]);
        if (mAcesas[i]) Circulo(mTochas[i] + Vector2(0.0f, -10.0f), 16.0f + 2.0f * std::sin(mRelogio * 9.0f), kCaminho, 16);
    }

    if (mDesfecho == Desfecho::EmAberto) {
        // O bau aberto por quem tem a chave: o Elixir erguido acima da cabeca, e o bonus.
        if (mComAChave >= 0 && static_cast<size_t>(mComAChave) < alunos.size()) {
            Player* a = alunos[static_cast<size_t>(mComAChave)];
            if (a && Masmorra::Alcanca(a->GetPosition(), mBau)) {
                mDesfecho = Desfecho::DoAluno;
                mProximaRodada = t + Masmorra::kPausaDepoisDoAluno;
                mQuemBebeu = mComAChave;
                Cara(mBauP, "Aberto");
                Matar(mChaveP);
                mElixirP = Peca("item", a->GetPosition() + Vector2(0.0f, -Masmorra::kAcimaDaCabeca), "Elixir");
                if (mElixirP) mElixirP->DefinirEnfeite(true);
                mErguidoAte = t + Masmorra::kErguido;
                mElixirAte = mRelogio + g.bonus;
                a->Proteger(g.bonus);
                a->Impulsionar(g.bonus, g.impulso);
                b->Faiscar(a->GetPosition(), Hud::Faisca::Coleta);
                if (mScene && mScene->GetGame() && mScene->GetGame()->GetAudio()) {
                    mScene->GetGame()->GetAudio()->PlaySound("powerup.wav", false);
                }
                TrocarAnuncio(Masmorra::kFase, "{:ok, :elixir}");
            }
        }
        // O professor: cada tocha apagada, na ordem, e so entao o bau. O tracejado mostra para onde.
        const int tocha = mAcendendo >= 0 ? mAcendendo : Masmorra::ProximaTocha(mAcesas);
        const Vector2 lugar = tocha >= 0 ? mTochas[static_cast<size_t>(tocha)] : mBau;
        const Vector2 destino = lugar + Vector2(0.0f, tocha >= 0 ? kAbaixoDaTocha : -kAcimaDoBau);
        mAlvo = Masmorra::Passo(GetPosition(), destino, g.rapidezDoProfessor, deltaTime);
        Tracejada(GetPosition(), lugar, kCaminho);
        const bool chegou = (GetPosition() - destino).Length() <= Masmorra::kChegou + 6.0f;
        if (tocha >= 0 && chegou && mAcendendo < 0) {
            mAcendendo = tocha;
            mAcendeEm = t + g.acender;
        }
        if (mAcendendo >= 0) {
            // O fogo que pega, em volta da tocha.
            const float u = std::clamp(1.0f - (mAcendeEm - t) / std::max(0.05f, g.acender), 0.0f, 1.0f);
            Circulo(mTochas[static_cast<size_t>(mAcendendo)], 8.0f + 18.0f * u, kCaminho, 18);
            if (t >= mAcendeEm) {
                const size_t i = static_cast<size_t>(mAcendendo);
                mAcesas[i] = true;
                if (mTochasP[i]) {
                    Cara(mTochasP[i], "Acesa");
                    mTochasP[i]->DefinirEnfeite(false);
                }
                b->Faiscar(mTochas[i], Hud::Faisca::Coleta);
                mAcendendo = -1;
            }
        }
        if (mDesfecho == Desfecho::EmAberto && tocha < 0 && chegou) {
            // Chegou antes, com as tochas acesas: bebe o Elixir.
            mDesfecho = Desfecho::DoProfessor;
            mProximaRodada = t + g.furia + Masmorra::kPausaDepoisDaFuria;
            Cara(mBauP, "Aberto");
            mInicioDaFuria = t;
            mProximaRajada = t + 0.5f;
            mProximoAnel = t + 0.5f;
            mElixirP = Peca("item", GetPosition() + Vector2(0.0f, -60.0f), "Elixir");
            if (mElixirP) mElixirP->DefinirEnfeite(true);
            mErguidoAte = t + 0.5f;
            TrocarAnuncio(Masmorra::kFase, "{:error, :furia}");
        }
        // O leque com que ele atrapalha a cacada.
        if (mDesfecho == Desfecho::EmAberto && t >= mProximoLeque) {
            mProximoLeque = t + g.entreLeques;
            Leque("leque", GetPosition() + Vector2(0.0f, 40.0f), g.tirosDoLeque, 30.0f, 230.0f);
        }
    }

    // O Elixir erguido: em cima de quem abriu o bau (ou do professor), por um instante.
    if (mElixirP) {
        if (t >= mErguidoAte) {
            Matar(mElixirP);
        } else if (mDesfecho == Desfecho::DoAluno && mQuemBebeu >= 0 && static_cast<size_t>(mQuemBebeu) < alunos.size() &&
                   alunos[static_cast<size_t>(mQuemBebeu)]) {
            Prender(mElixirP, alunos[static_cast<size_t>(mQuemBebeu)]->GetPosition() + Vector2(0.0f, -Masmorra::kAcimaDaCabeca));
        } else {
            Prender(mElixirP, GetPosition() + Vector2(0.0f, -60.0f));
        }
    }
    // O bonus do aluno: os coracoes em volta dele.
    if (mRelogio < mElixirAte && mQuemBebeu >= 0 && static_cast<size_t>(mQuemBebeu) < alunos.size() && alunos[static_cast<size_t>(mQuemBebeu)]) {
        const Vector2 centro = alunos[static_cast<size_t>(mQuemBebeu)]->GetPosition();
        for (int k = 0; k < 3; ++k) {
            const float a = mRelogio * 2.5f + Math::TwoPi * static_cast<float>(k) / 3.0f;
            Coracao(centro + Vector2(Math::Cos(a), Math::Sin(a)) * 34.0f, 4.0f, kCoracao);
        }
    }
    // Depois do desfecho, o professor volta para o alto; na furia, de la ele atira, balancando de um
    // lado para o outro - a espiral nao sai sempre do mesmo ponto, e ele nao e um alvo parado.
    if (mDesfecho != Desfecho::EmAberto) {
        const float balanco = mDesfecho == Desfecho::DoProfessor ? 0.22f * Math::Sin(Math::TwoPi * (t - mInicioDaFuria) / 3.0f) : 0.0f;
        mAlvo = Vector2(c.x + c.w * (0.5f + balanco), c.y + kYDoProfessor);
    }
    AtualizarFadas(deltaTime);
    if (mDesfecho == Desfecho::DoProfessor && t - mInicioDaFuria < g.furia) {
        const float tf = t - mInicioDaFuria;
        const Vector2 de = GetPosition() + Vector2(0.0f, 30.0f);
        if (t >= mProximaRajada) {
            mProximaRajada = t + g.entreRajadas;
            for (const float rumo : Masmorra::RumosDaEspiral(g.bracos, tf, g.giro)) {
                const float a = Math::ToRadians(rumo);
                Um("furia", de, "Pipe", Vector2(Math::Cos(a), Math::Sin(a)) * g.velocidadeDaFuria);
            }
        }
        if (t >= mProximoAnel) {
            mProximoAnel = t + g.entreAneis;
            DispararEm("anel_furia", de, g.tirosDoAnel);
            SomDoDisparo();
        }
    }
}

void Lucas::AtualizarFadas(const float deltaTime) {
    const Battle* b = Batalha();
    if (!b) return;
    const std::vector<Player*>& alunos = b->Jogadores();
    // Os inimigos: as gosmas vivas, e o professor. A fada de cada aluno vai ao mais perto dele.
    std::vector<Vector2> alvos;
    std::vector<float> raios;
    for (const BossProjectile* g : mGosmas) {
        if (!Vivo(g)) continue;
        alvos.push_back(g->GetPosition());
        raios.push_back(kRaioDaMiraNaGosma);
    }
    alvos.push_back(GetPosition());
    raios.push_back(kRaioDaMiraNoProfessor);

    mFadas.resize(alunos.size(), Vector2::Zero);
    mRastros.resize(alunos.size());
    for (size_t i = 0; i < alunos.size(); ++i) {
        if (!alunos[i]) continue;
        const Vector2 aluno = alunos[i]->GetPosition();
        // A fada nasce no aluno (e volta a ele se ficou para tras de uma fase anterior).
        if (mRastros[i].empty()) mFadas[i] = aluno + Vector2(0.0f, -20.0f);
        const int qual = Masmorra::MaisPerto(aluno, alvos);
        if (qual < 0) continue;
        const Vector2 alvo = alvos[static_cast<size_t>(qual)];
        const float tempo = mRelogio + static_cast<float>(i) * 1.7f;
        mFadas[i] = Masmorra::Passo(mFadas[i], Masmorra::PousoDaFada(alvo, tempo), Masmorra::kRapidezDaFada, deltaTime);
        // O rastro: os ultimos lugares por onde ela passou.
        auto& rastro = mRastros[i];
        rastro.insert(rastro.begin(), mFadas[i]);
        if (rastro.size() > kRastroDaFada) rastro.pop_back();
        for (size_t k = 1; k < rastro.size(); k += 2) {
            const float u = 1.0f - static_cast<float>(k) / static_cast<float>(kRastroDaFada);
            Disco(rastro[k], 1.0f + 1.5f * u, SDL_Color{190, 235, 255, static_cast<Uint8>(150.0f * u)});
        }
        // A fada: o brilho, o corpo de luz e as asas batendo.
        const Vector2 f = mFadas[i];
        Circulo(f, 10.0f + 1.5f * std::sin(tempo * 8.0f), SDL_Color{170, 225, 255, 90}, 16);
        Disco(f, 6.0f, SDL_Color{150, 215, 255, 120});
        Disco(f, 3.5f, SDL_Color{245, 252, 255, 255});
        const float bate = 0.5f + 0.5f * std::sin(tempo * 32.0f);
        for (const float lado : {-1.0f, 1.0f}) {
            Disco(f + Vector2(lado * (6.0f + 2.0f * bate), -3.0f - 2.0f * bate), 2.5f + bate, SDL_Color{220, 245, 255, 170});
        }
        // A mira amarela no alvo, quando a fada ja chegou perto.
        if ((f - Masmorra::PousoDaFada(alvo, tempo)).Length() < 40.0f) {
            for (const Masmorra::Seta& s : Masmorra::Mira(alvo, raios[static_cast<size_t>(qual)], mRelogio)) {
                for (float k = 0.0f; k <= 1.0f; k += 0.25f) {
                    // A seta cheia: linhas da ponta ate a base, e o contorno.
                    Linha(s.ponta, s.esquerda + (s.direita - s.esquerda) * k, kMira);
                }
                Linha(s.esquerda, s.direita, kMiraEscura);
            }
        }
    }
}

// ---- A final: o Phoenix ----

void Lucas::FazerRenascer(const float t, const bool comAnel) {
    const Dom::Campo c = CampoPuro();
    const Vector2 onde = comAnel ? mOndeCaiu : Vector2(c.x + c.w / 2.0f, c.y + c.h * 0.3f);
    Matar(mCinzasP);
    mFenixP = Peca("fenix", onde, "Voando");
    mFenixDerrubada = false;
    mEstado = EstadoDaFenix::Voando;
    mVoo = 0.0f;
    mMeioX = onde.x;
    mProximaPena = t + 0.8f;
    if (comAnel) {
        ++mVersao;
        DispararEm("renascer", onde, mDegrauDaFenix.tirosDoRenascimento, std::nullopt, std::nullopt,
                   mDegrauDaFenix.velocidadeDoRenascimento);
        SomDoDisparo();
        TrocarAnuncio(Fenix::kFase, "Phoenix 1." + std::to_string(7 + mVersao) + " - renasceu");
    } else {
        TrocarAnuncio(Fenix::kFase, "mix phx.server");
    }
}

void Lucas::AtualizarFenix(const float t, const float deltaTime) {
    Battle* b = Batalha();
    if (!b) return;
    const Fenix::Degrau& g = mDegrauDaFenix;
    const Dom::Campo c = CampoPuro();
    Varrer(t, 0.18f, 11.0f);

    if (mEstado == EstadoDaFenix::Nenhuma) {
        mProximoBroadcast = t + g.entreBroadcasts;
        mProximaTagDaFinal = t + 2.0f;
        FazerRenascer(t, false);
    }
    const float rapidez = std::pow(std::max(1.0f, g.maisRapidaPorVersao), static_cast<float>(mVersao));

    // Os sockets do PubSub, sempre no campo.
    const auto sockets = Fenix::Sockets(c);
    for (const Vector2& s : sockets) {
        Circulo(s, 12.0f, kSocket, 16);
        Circulo(s, 6.0f + 3.0f * std::sin(mRelogio * 4.0f), kSocket, 12);
    }

    if (mEstado == EstadoDaFenix::Voando) {
        if (mFenixDerrubada || !mFenixP) {
            // Derrubada: vira cinzas, e o aluno ganha.
            if (mFenixP) mOndeCaiu = mFenixP->GetPosition();
            Matar(mFenixP);
            mCinzasP = Peca("cinzas", mOndeCaiu, "Brasas");
            mCinzasApagadas = false;
            mRenasceEm = t + Fenix::kRenascer;
            mEstado = EstadoDaFenix::Cinzas;
            b->Conquistar(Fenix::kPremioPorDerrubar);
            b->Faiscar(mOndeCaiu, Hud::Faisca::Coleta);
            TrocarAnuncio(Fenix::kFase, "** (EXIT) :fenix");
        } else {
            // O voo em oito, com o meio do oito atras do aluno (o LiveView).
            mVoo += deltaTime;
            mMeioX = Fenix::SeguirX(mMeioX, PosicaoDoAluno().x, deltaTime);
            const Vector2 alvo = Fenix::NoVoo(mVoo, mMeioX, c, g.periodoDoOito, mVersao, g.maisRapidaPorVersao);
            // Logo depois de renascer, ela sai das cinzas ate o oito, sem saltar.
            const float u = std::clamp(mVoo / 0.8f, 0.0f, 1.0f);
            Prender(mFenixP, mFenixP->GetPosition() + (alvo - mFenixP->GetPosition()) * (u < 1.0f ? 0.15f : 1.0f));
            if (t >= mProximaPena) {
                mProximaPena = t + g.entrePenas / rapidez;
                const Vector2 de = mFenixP->GetPosition() + Vector2(0.0f, 20.0f);
                const float centro = AnguloAteOAluno(de);
                const float velocidade = g.velocidadeDasPenas * rapidez;
                const auto penas = DispararEm("pena", de, g.penas, centro, 50.0f, velocidade);
                const size_t n = penas.size();
                for (size_t i = 0; i < n; ++i) {
                    const float a = Math::ToRadians(centro - 25.0f + (n > 1 ? 50.0f * static_cast<float>(i) / static_cast<float>(n - 1) : 25.0f));
                    Andar(penas[i], Vector2(Math::Cos(a), Math::Sin(a)) * velocidade);
                }
            }
            // O broadcast: a linha da fenix a cada socket, e depois o anel de todos ao mesmo tempo.
            if (mBroadcastEm < 0.0f && t >= mProximoBroadcast) {
                mBroadcastEm = t + Fenix::kAvisoDoBroadcast;
                mProximoBroadcast = t + g.entreBroadcasts;
            }
            if (mBroadcastEm >= 0.0f) {
                for (const Vector2& s : sockets) Tracejada(mFenixP->GetPosition(), s, kBroadcast, 8.0f);
                if (t >= mBroadcastEm) {
                    mBroadcastEm = -1.0f;
                    for (const Vector2& s : sockets) DispararEm("socket", s, g.tirosDoSocket, std::nullopt, std::nullopt, g.velocidadeDoSocket);
                    SomDoDisparo();
                }
            }
        }
    } else if (mEstado == EstadoDaFenix::Cinzas) {
        mBroadcastEm = -1.0f;
        if (mCinzasApagadas || !mCinzasP) {
            // Apagadas a tempo: ela demora a voltar, e o aluno ganha de novo.
            Matar(mCinzasP);
            mEstado = EstadoDaFenix::Apagada;
            mRenasceEm = t + Fenix::kApagada;
            b->Conquistar(Fenix::kPremioPorApagar);
            TrocarAnuncio(Fenix::kFase, "{:error, :sem_cinzas}");
        } else {
            Prender(mCinzasP, mOndeCaiu);
            // As brasas sobem enquanto ela se refaz.
            const float falta = std::max(0.0f, mRenasceEm - t);
            Circulo(mOndeCaiu, 14.0f + 26.0f * (1.0f - falta / Fenix::kRenascer), kBroadcast, 20);
            if (t >= mRenasceEm) FazerRenascer(t, true);
        }
    } else if (mEstado == EstadoDaFenix::Apagada) {
        if (t >= mRenasceEm) FazerRenascer(t, false);
    }

    // O professor, no alto, tambem atira.
    if (t >= mProximaTagDaFinal) {
        mProximaTagDaFinal = t + (NivelDeDificuldade() == Dificuldade::Nivel::Dificil ? 1.8f : 2.6f);
        Leque("tag", GetPosition() + Vector2(0.0f, 40.0f), 3, 30.0f, 230.0f);
    }
}
