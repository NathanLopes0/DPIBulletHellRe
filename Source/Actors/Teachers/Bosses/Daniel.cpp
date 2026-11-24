//
// O chefe da INF 425 - Ciencia de Dados, a primeira optativa.
//

#include "Daniel.h"

#include <algorithm>
#include <cmath>

#include "PainelDeCodigo.h"
#include "BossesProjectiles/BossProjectile.h"
#include "../../Player/Player.h"
#include "../../Player/PlayerProjectile.h"
#include "../../../Attacks/AttackParameters/AttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../../Scenes/Scene.h"

namespace {
    /// As cores dos tres tipos: fogo, agua e planta. A forma tambem diz o tipo (a chama, a gota, o
    /// broto, nos monstrinhos; a fagulha, a gota e a folha, nos efeitos) - nunca so a cor.
    constexpr SDL_Color kCor[Tipos::kTipos] = {{255, 130, 70, 230}, {90, 170, 255, 230}, {110, 210, 110, 230}};
    constexpr SDL_Color kNeutro{200, 205, 220, 200};
    constexpr SDL_Color kEixo{150, 170, 200, 120};
    constexpr SDL_Color kGrade{110, 125, 160, 40};
    constexpr SDL_Color kMemoria{235, 240, 255, 210};
    constexpr SDL_Color kCurva{150, 225, 255, 200};
    /// Na validacao, os dados novos (verdes, como quem acertou) e o erro do modelo ate eles.
    constexpr SDL_Color kDadoNovo{120, 235, 140, 240};
    constexpr SDL_Color kErroDoModelo{200, 205, 220, 150};
    constexpr int kDadosNovos = 5;
    constexpr SDL_Color kTim{190, 195, 210, 220};

    SDL_Color CorDo(const int tipo) {
        return tipo >= 0 && tipo < Tipos::kTipos ? kCor[tipo] : kNeutro;
    }
    SDL_Color Alfa(const SDL_Color c, const float a) {
        return SDL_Color{c.r, c.g, c.b, static_cast<Uint8>(std::clamp(static_cast<float>(c.a) * a, 0.0f, 255.0f))};
    }

    constexpr float kRaioDaAmostra = 15.0f;
    constexpr float kRaioDoOutlier = 15.0f;
    constexpr float kRaioDoNucleo = 20.0f;
    constexpr float kYDoProfessor = 62.0f;
    constexpr float kYDasFontes = 118.0f;
    constexpr float kYDoProfessorNoModelo = 150.0f;
    /// O leque do professor, sem tipo: fechado o bastante para quem fica parado embaixo dele levar.
    constexpr float kAberturaDoLeque = 26.0f;
    constexpr float kVelocidadeDoLeque = 230.0f;
    /// Os gatilhos (fases.json).
    constexpr const char* kGatilhoDaAmostra = "amostra";
    constexpr const char* kGatilhoDaFonte = "fonte";
    constexpr const char* kGatilhoDoLeque = "estouro";
    constexpr const char* kGatilhoDoNucleo = "nucleo";
    constexpr const char* kGatilhoDaPrevisao = "previsao";
    constexpr const char* kGatilhoDoDecorado = "decorado";
}

Daniel::Daniel(Scene* scene) : Boss(scene) {}

void Daniel::SortearALuta() {
    mLutaSorteada = true;
    mSorteio = SorteioDaFase(Ecossistema::kFase);
    mSorteioDosEfeitos = SorteioDaFase(Sobreajuste::kFase);
    const Dificuldade::Nivel n = NivelDeDificuldade();
    mDegrauDaColeta = Ecossistema::DegrauDo(n);
    mDegrauDoAgrupamento = Agrupamento::DegrauDo(n);
    mDegrauDoSobreajuste = Sobreajuste::DegrauDo(n);
    mDegrauDaConfusao = Confusao::DegrauDo(n);
    // A ordem dos monstrinhos na escolha: o do meio (o mais perto de quem sobe) muda de luta em luta.
    for (int k = Tipos::kTipos - 1; k > 0; --k) {
        std::swap(mLugarDoTipo[static_cast<size_t>(k)], mLugarDoTipo[static_cast<size_t>(mSorteio.Inteiro(0, k))]);
    }
}

// ---- O quadro ----

void Daniel::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    // Antes de qualquer disparo deste quadro: o que morreu no quadro passado e esquecido (o pool o
    // reaproveita, e o ponteiro passaria a ser de outro projetil).
    for (BossProjectile*& s : mSelos) if (!Vivo(s)) s = nullptr;
    for (BossProjectile*& e : mEscolhas) if (!Vivo(e)) e = nullptr;
    for (BossProjectile*& l : mLegenda) if (!Vivo(l)) l = nullptr;
    for (BossProjectile*& f : mFontes) if (!Vivo(f)) f = nullptr;
    for (BossProjectile*& c : mCentroidesP) if (!Vivo(c)) c = nullptr;
    mAmostras.erase(std::remove_if(mAmostras.begin(), mAmostras.end(), [](const Amostra& a) { return !Vivo(a.p); }),
                    mAmostras.end());
    mPontos.erase(std::remove_if(mPontos.begin(), mPontos.end(), [](const Ponto& p) { return !Vivo(p.p); }), mPontos.end());
    mOutliers.erase(std::remove_if(mOutliers.begin(), mOutliers.end(), [](const Outlier& o) { return !Vivo(o.p); }),
                    mOutliers.end());
    for (Nucleo& n : mNucleos) if (!Vivo(n.p)) n.p = nullptr;
    for (auto it = mJaPassou.begin(); it != mJaPassou.end();) it = Vivo(*it) ? std::next(it) : mJaPassou.erase(it);

    Boss::OnUpdate(deltaTime);
    if (EhClone()) return;
    mRelogio += deltaTime;

    const auto* fsm = GetComponent<FSMComponent>();
    const std::string fase = fsm ? fsm->GetStateName() : std::string();
    const float t = fsm ? fsm->GetStateTime() : 0.0f;
    if (fase != mFase) {
        Desmontar(mFase == Ecossistema::kFase && fase == Agrupamento::kFase);
        mFase = fase;
    }
    if (mScene && !mLinhas) {
        auto ator = std::make_unique<Actor>(mScene);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
        mEfeitos = ator->AddComponent<DrawLinhasComponent>(kOrdemDosEfeitos);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();
    if (mEfeitos) mEfeitos->Limpar();

    const Battle* b = Batalha();
    if (!b || !b->EmCombate()) return;

    // O tipo: escolhido no comeco da fase 1; quem chega numa outra fase sem ter escolhido (o arnes
    // que pula fases) fica com um sorteado.
    if (!mEscolheu && fase == Ecossistema::kFase) Escolher(t, deltaTime);
    if (!mEscolheu && !fase.empty() && fase != Ecossistema::kFase) {
        mTipos.resize(b->Jogadores().size(), -1);
        for (int& tp : mTipos) if (tp < 0) tp = mSorteio.Inteiro(0, Tipos::kTipos - 1);
        mEscolheu = true;
    }

    if (fase == Ecossistema::kFase) AtualizarColeta(t, deltaTime);
    else if (fase == Agrupamento::kFase) AtualizarAgrupamento(t, deltaTime);
    else if (fase == Sobreajuste::kFase) AtualizarSobreajuste(t, deltaTime);
    else if (fase == Confusao::kFase) AtualizarConfusao(t, deltaTime);
    if (mEscolheu) {
        AtualizarTipos();
        DesenharTriangulo();
        RastroDosTiros();
    }
    AndarOsEfeitos(deltaTime);
}

void Daniel::AoTerminarABatalha() {
    Desmontar();
    for (BossProjectile*& s : mSelos) Matar(s);
    for (BossProjectile*& l : mLegenda) Matar(l);
    mParticulas.clear();
}

void Daniel::Desmontar(const bool manterOsPontos) {
    for (BossProjectile*& e : mEscolhas) Matar(e);
    for (Amostra& a : mAmostras) Matar(a.p);
    mAmostras.clear();
    if (!manterOsPontos) {
        for (Ponto& p : mPontos) Matar(p.p);
        mPontos.clear();
    }
    for (BossProjectile*& f : mFontes) Matar(f);
    for (BossProjectile*& c : mCentroidesP) Matar(c);
    mCentroidesP.clear();
    mCentros.clear();
    mGrupos.clear();
    mTiposDosGrupos.clear();
    mAgrupou = false;
    mDeslizeDesde = -1.0f;
    for (Outlier& o : mOutliers) Matar(o.p);
    mOutliers.clear();
    mOutliersFeitos = 0;
    for (Nucleo& n : mNucleos) Matar(n.p);
    mModeloComecou = false;
    mMemorias.clear();
    mPedacos.clear();
    mRiscou = false;
    mDecoradoEm = -1.0f;
    mAmostrasFeitas = 0;
    mProximaAmostra = 0.0f;
    mProximoLeque = 0.0f;
    for (EtiquetaDeCodigo* e : mEtiquetas) if (e) e->Mostrar(false);
    if (mVeredito) mVeredito->Mostrar(false);
    for (Classificado& cl : mClassificados) Matar(cl.p);
    mClassificados.clear();
    mMatriz = Confusao::Matriz{};
    mProximaClassificada = 1.0f;
    mFantasmas.clear();
    mRajadasDaPrevisao = 0;
    mUltimoErrou = -10.0f;
    mRotulosUsados = 0;
    EsconderRotulosSobrando();
    mAlvo.reset();
    if (mLinhas) mLinhas->Limpar();
    if (mEfeitos) mEfeitos->Limpar();
}

void Daniel::Varrer(const float t, const float amplitude, const float periodo) {
    const SDL_FRect c = Campo();
    mAlvo = Vector2(c.x + c.w * (0.5f + amplitude * Math::Sin(Math::TwoPi * t / periodo)), c.y + kYDoProfessor);
}

// ---- O que as fases dividem ----

std::vector<BossProjectile*> Daniel::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
                                                const std::optional<float> centro,
                                                const std::optional<float> abertura,
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

BossProjectile* Daniel::Um(const char* gatilho, const Vector2 onde, const std::string& cara,
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

BossProjectile* Daniel::Monstrinho(const char* gatilho, const Vector2 onde, const int tipo,
                                   const std::optional<Vector2> velocidade) {
    BossProjectile* p = Um(gatilho, onde, Tipos::kCaras[static_cast<size_t>(std::clamp(tipo, 0, Tipos::kTipos - 1))], velocidade);
    if (p) p->DefinirClasse(tipo);
    return p;
}

void Daniel::Leque(const Vector2 de, const int quantos, const float abertura, const float velocidade) {
    const float centro = AnguloAteOAluno(de);
    const auto criados = DispararEm(kGatilhoDoLeque, de, std::max(1, quantos), centro, abertura, velocidade);
    const size_t n = criados.size();
    const float inicio = centro - abertura / 2.0f;
    const float passo = n > 1 ? abertura / static_cast<float>(n - 1) : 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const float a = Math::ToRadians(inicio + passo * static_cast<float>(i));
        Andar(criados[i], Vector2(Math::Cos(a), Math::Sin(a)) * velocidade);
    }
    SomDoDisparo();
}

void Daniel::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
    if (!mDisparandoEm) return;
    params.firePosition = mOndeDisparar;
    if (mQuantos > 0) params.numProjectiles = mQuantos;
    if (mCentroEm) params.centralAngle = *mCentroEm;
    if (mAberturaEm) params.angle = *mAberturaEm;
    if (mVelocidadeEm) params.projectileSpeed = *mVelocidadeEm;
}

void Daniel::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                  const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

float Daniel::AnguloAteOAluno(const Vector2 de) const {
    const Vector2 d = PosicaoDoAluno() - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

bool Daniel::Vivo(const BossProjectile* p) {
    return p && p->GetState() == ActorState::Active;
}

void Daniel::Matar(BossProjectile*& p) {
    if (Vivo(p)) p->Kill();
    p = nullptr;
}

void Daniel::Cara(BossProjectile* p, const std::string& cara) {
    if (!p || cara.empty()) return;
    if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) anim->SetAnimation(cara);
}

void Daniel::Parar(BossProjectile* p) {
    Andar(p, Vector2::Zero);
}

void Daniel::Andar(BossProjectile* p, const Vector2 velocidade) {
    if (!p) return;
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(velocidade);
}

void Daniel::Linha(const Vector2 de, const Vector2 ate, const SDL_Color cor, const bool emCima) {
    DrawLinhasComponent* onde = emCima ? mEfeitos : mLinhas;
    if (onde) onde->Acrescentar(de, ate, cor);
}

void Daniel::Tracejada(const Vector2 de, const Vector2 ate, const SDL_Color cor, const float traco) {
    const Vector2 d = ate - de;
    const float comprimento = d.Length();
    if (comprimento < 1.0f) return;
    const Vector2 u = d * (1.0f / comprimento);
    for (float s = 0.0f; s < comprimento; s += 2.0f * traco) Linha(de + u * s, de + u * std::min(comprimento, s + traco), cor);
}

void Daniel::Circulo(const Vector2 centro, const float raio, const SDL_Color cor, const int lados, const bool emCima) {
    for (int k = 0; k < lados; ++k) {
        const float a0 = Math::TwoPi * static_cast<float>(k) / static_cast<float>(lados);
        const float a1 = Math::TwoPi * static_cast<float>(k + 1) / static_cast<float>(lados);
        Linha(centro + Vector2(Math::Cos(a0), Math::Sin(a0)) * raio, centro + Vector2(Math::Cos(a1), Math::Sin(a1)) * raio, cor, emCima);
    }
}

void Daniel::Disco(const Vector2 centro, const float raio, const SDL_Color cor, const bool emCima) {
    for (float dy = -raio; dy <= raio; dy += 1.0f) {
        const float dx = std::sqrt(std::max(0.0f, raio * raio - dy * dy));
        Linha(centro + Vector2(-dx, dy), centro + Vector2(dx, dy), cor, emCima);
    }
}

// ---- O grafico ----

SDL_FRect Daniel::Grafico() const {
    const SDL_FRect c = Campo();
    return SDL_FRect{c.x + 70.0f, c.y + 160.0f, c.w - 140.0f, c.h - 200.0f};
}

Vector2 Daniel::NoCampo(const Vector2 n) const {
    const SDL_FRect g = Grafico();
    return Vector2(g.x + g.w * n.x, g.y + g.h * (1.0f - n.y));
}

void Daniel::DesenharGrafico() {
    const SDL_FRect g = Grafico();
    const Vector2 origem(g.x, g.y + g.h);
    // A grade, os eixos e as setas: x e o ataque, y a defesa.
    for (int k = 1; k < 5; ++k) {
        const float f = static_cast<float>(k) / 5.0f;
        Linha(Vector2(g.x + g.w * f, g.y), Vector2(g.x + g.w * f, g.y + g.h), kGrade);
        Linha(Vector2(g.x, g.y + g.h * f), Vector2(g.x + g.w, g.y + g.h * f), kGrade);
    }
    Linha(origem, Vector2(g.x + g.w + 10.0f, origem.y), kEixo);
    Linha(origem, Vector2(g.x, g.y - 10.0f), kEixo);
    Linha(Vector2(g.x + g.w + 10.0f, origem.y), Vector2(g.x + g.w + 2.0f, origem.y - 5.0f), kEixo);
    Linha(Vector2(g.x + g.w + 10.0f, origem.y), Vector2(g.x + g.w + 2.0f, origem.y + 5.0f), kEixo);
    Linha(Vector2(g.x, g.y - 10.0f), Vector2(g.x - 5.0f, g.y - 2.0f), kEixo);
    Linha(Vector2(g.x, g.y - 10.0f), Vector2(g.x + 5.0f, g.y - 2.0f), kEixo);
    // O nome de cada eixo.
    if (mScene && mEtiquetas.empty()) {
        for (const char* texto : {"ataque", "defesa"}) {
            auto nova = std::make_unique<EtiquetaDeCodigo>(mScene, texto, 80, 22, kOrdemDasLinhas + 1, 20);
            nova->Pintar(SDL_Color{150, 170, 200, 0}, SDL_Color{12, 16, 28, 0}, SDL_Color{160, 180, 215, 255});
            mEtiquetas.push_back(nova.get());
            mScene->AddActor(std::move(nova));
        }
    }
    if (mEtiquetas.size() >= 2) {
        mEtiquetas[0]->SetPosition(Vector2(g.x + g.w - 30.0f, origem.y + 14.0f));
        mEtiquetas[1]->SetPosition(Vector2(g.x + 34.0f, g.y - 12.0f));
        for (EtiquetaDeCodigo* e : mEtiquetas) e->Mostrar(true);
    }
    // A regiao de cada especie, de leve: onde ela mora.
    for (int tipo = 0; tipo < Tipos::kTipos; ++tipo) {
        const Vector2 c = NoCampo(Ecossistema::CentroDaEspecie(tipo));
        for (int k = 0; k < 24; k += 2) {
            const float a0 = Math::TwoPi * static_cast<float>(k) / 24.0f, a1 = Math::TwoPi * static_cast<float>(k + 1) / 24.0f;
            const Vector2 r(g.w * 0.13f, g.h * 0.13f);
            Linha(c + Vector2(Math::Cos(a0) * r.x, Math::Sin(a0) * r.y), c + Vector2(Math::Cos(a1) * r.x, Math::Sin(a1) * r.y),
                  Alfa(kCor[tipo], 0.35f));
        }
    }
}

void Daniel::DesenharTriangulo() {
    // No canto de cima, a direita: os tres tipos, e uma seta de quem vence para quem perde. O do
    // aluno tem um anel.
    const SDL_FRect c = Campo();
    const Vector2 centro(c.x + c.w - 70.0f, c.y + 58.0f);
    std::array<Vector2, Tipos::kTipos> onde{};
    for (int k = 0; k < Tipos::kTipos; ++k) {
        const float a = Math::ToRadians(-90.0f + 120.0f * static_cast<float>(k));
        onde[static_cast<size_t>(k)] = centro + Vector2(Math::Cos(a), Math::Sin(a)) * 30.0f;
        BossProjectile*& l = mLegenda[static_cast<size_t>(k)];
        if (!l) {
            l = Monstrinho(kGatilhoDaAmostra, onde[static_cast<size_t>(k)], k, std::nullopt);
            if (l) {
                l->DefinirEnfeite(true);
                l->DefinirClasse(-1);
                l->SetScale(1.2f);
            }
        }
        if (l) {
            l->SetPosition(onde[static_cast<size_t>(k)]);
            l->RenovarTempoDeVida();
        }
    }
    for (int k = 0; k < Tipos::kTipos; ++k) {
        const Vector2 de = onde[static_cast<size_t>(k)], para = onde[static_cast<size_t>(Tipos::Vencido(k))];
        Vector2 d = para - de;
        const float l = d.Length();
        if (l < 1.0f) continue;
        d = d * (1.0f / l);
        const Vector2 a = de + d * 13.0f, b = para - d * 13.0f;
        const SDL_Color cor = Alfa(kCor[k], 0.9f);
        Linha(a, b, cor, true);
        const Vector2 lado(-d.y, d.x);
        Linha(b, b - d * 6.0f + lado * 4.0f, cor, true);
        Linha(b, b - d * 6.0f - lado * 4.0f, cor, true);
    }
    for (const int tipo : mTipos) {
        if (tipo >= 0 && tipo < Tipos::kTipos) Circulo(onde[static_cast<size_t>(tipo)], 12.0f, SDL_Color{255, 255, 255, 230}, 16, true);
    }
}

// ---- O tipo do aluno ----

std::vector<Vector2> Daniel::LugaresDaEscolha() const {
    // O lugar de cada tipo, na ordem dos tipos: os lugares sao os de Tipos, e a ordem foi sorteada.
    const SDL_FRect c = Campo();
    const std::vector<Vector2> lugares = Tipos::LugaresDaEscolha(c.x, c.y, c.w, c.h);
    std::vector<Vector2> v;
    for (int k = 0; k < Tipos::kTipos; ++k) v.push_back(lugares[static_cast<size_t>(mLugarDoTipo[static_cast<size_t>(k)])]);
    return v;
}

void Daniel::Escolher(const float t, const float deltaTime) {
    const Battle* b = Batalha();
    if (!b) return;
    const auto& jogadores = b->Jogadores();
    if (mTipos.size() < jogadores.size()) mTipos.resize(jogadores.size(), -1);
    if (mEmCima.size() < jogadores.size()) {
        mEmCima.resize(jogadores.size(), 0.0f);
        mEmCimaDe.resize(jogadores.size(), -1);
    }
    const std::vector<Vector2> lugares = LugaresDaEscolha();
    for (int k = 0; k < Tipos::kTipos; ++k) {
        BossProjectile*& e = mEscolhas[static_cast<size_t>(k)];
        if (!e) {
            e = Monstrinho(kGatilhoDaAmostra, lugares[static_cast<size_t>(k)], k, std::nullopt);
            if (e) {
                e->DefinirEnfeite(true);
                e->SetScale(2.6f);
            }
        }
        if (e) e->SetPosition(lugares[static_cast<size_t>(k)]);
        // O anel do tipo, pulsando, e o elemento escapando dele: chama que sobe, gota, folha.
        const Vector2 l = lugares[static_cast<size_t>(k)];
        Circulo(l, 34.0f + 3.0f * Math::Sin(mRelogio * 7.0f), kCor[k], 20);
        if (mSorteioDosEfeitos.Chance(deltaTime * 6.0f)) Efeito(k, l + Vector2(0.0f, -10.0f), 1, 40.0f);
    }
    bool todos = true;
    for (size_t i = 0; i < jogadores.size(); ++i) {
        if (!jogadores[i] || mTipos[i] >= 0) continue;
        const int k = Tipos::MaisPerto(lugares, jogadores[i]->GetPosition());
        const bool emCima = k >= 0 && (lugares[static_cast<size_t>(k)] - jogadores[i]->GetPosition()).Length() < Tipos::kToqueDaEscolha;
        // Ficar em cima enche um anel; sair dele, ou passar para outro, recomeca.
        if (!emCima) {
            mEmCima[i] = 0.0f;
            mEmCimaDe[i] = -1;
        } else {
            mEmCima[i] = mEmCimaDe[i] == k ? mEmCima[i] + deltaTime : deltaTime;
            mEmCimaDe[i] = k;
            const float cheio = std::min(1.0f, mEmCima[i] / Tipos::kParaEscolher);
            const int lados = std::max(1, static_cast<int>(24.0f * cheio));
            const Vector2 c = lugares[static_cast<size_t>(k)];
            for (int s = 0; s < lados; ++s) {
                const float a0 = -Math::Pi / 2.0f + Math::TwoPi * static_cast<float>(s) / 24.0f;
                const float a1 = -Math::Pi / 2.0f + Math::TwoPi * static_cast<float>(s + 1) / 24.0f;
                for (const float r : {40.0f, 42.0f, 44.0f}) {
                    Linha(c + Vector2(Math::Cos(a0), Math::Sin(a0)) * r, c + Vector2(Math::Cos(a1), Math::Sin(a1)) * r,
                          SDL_Color{255, 255, 255, 235}, true);
                }
            }
        }
        if (emCima && mEmCima[i] >= Tipos::kParaEscolher) {
            mTipos[i] = k;
            // Eu escolho voce: a explosao do elemento, grande.
            Efeito(k, lugares[static_cast<size_t>(k)], 46, 230.0f);
            if (Battle* bb = Batalha()) bb->Faiscar(lugares[static_cast<size_t>(k)], Hud::Faisca::Coleta);
        }
        if (mTipos[i] < 0) todos = false;
    }
    if (todos || t >= Tipos::kTempoDaEscolha) {
        for (size_t i = 0; i < jogadores.size(); ++i) {
            if (jogadores[i] && mTipos[i] < 0) {
                mTipos[i] = Tipos::MaisPerto(lugares, jogadores[i]->GetPosition());
                Efeito(mTipos[i], jogadores[i]->GetPosition(), 30, 180.0f);
            }
        }
        for (BossProjectile*& e : mEscolhas) Matar(e);
        mEscolheu = true;
    }
}

int Daniel::TipoDoTiro(const PlayerProjectile* tiro) const {
    const Battle* b = Batalha();
    if (!b || !tiro) return -1;
    const auto& jogadores = b->Jogadores();
    for (size_t i = 0; i < jogadores.size() && i < mTipos.size(); ++i) {
        if (jogadores[i] && tiro->Dono() == jogadores[i]) return mTipos[i];
    }
    return -1;
}

void Daniel::AtualizarTipos() {
    const Battle* b = Batalha();
    if (!b) return;
    const auto& jogadores = b->Jogadores();
    if (mTipos.size() < jogadores.size()) mTipos.resize(jogadores.size(), 0);
    if (mSelos.size() != jogadores.size()) {
        for (BossProjectile*& s : mSelos) Matar(s);
        mSelos.assign(jogadores.size(), nullptr);
    }
    ProjectileManager* gerente = const_cast<Battle*>(b)->GerenteDeProjeteis();
    for (size_t i = 0; i < jogadores.size(); ++i) {
        Player* aluno = jogadores[i];
        if (!aluno) continue;
        const int tipo = mTipos[i];
        aluno->DefinirClasse(tipo);
        if (tipo < 0) continue;
        const Vector2 pa = aluno->GetPosition();
        const Vector2 ondeDoSelo = pa + Vector2(22.0f, -26.0f);
        if (!mSelos[i]) {
            mSelos[i] = Um(kGatilhoDoSelo, ondeDoSelo, Tipos::kCaras[static_cast<size_t>(tipo)]);
            if (mSelos[i]) {
                mSelos[i]->DefinirEnfeite(true);
                mSelos[i]->SetScale(1.6f);
            }
        }
        if (mSelos[i]) {
            mSelos[i]->SetPosition(ondeDoSelo);
            mSelos[i]->RenovarTempoDeVida();
        }
        Circulo(pa, 19.0f, kCor[tipo], 16);
        if (!gerente) continue;
        const auto* colisor = aluno->GetComponent<CircleColliderComponent>();
        const float raio = colisor ? colisor->GetRadius() : 6.0f;
        for (const auto& tiro : gerente->GetBossProjectiles()) {
            if (tiro->GetState() != ActorState::Active || tiro->Enfeite() || tiro->Classe() < 0) continue;
            const auto* outro = tiro->GetComponent<CircleColliderComponent>();
            const float r = outro ? outro->GetRadius() : 6.0f;
            if ((tiro->GetPosition() - pa).Length() > raio + r + 10.0f) continue;
            const Tipos::Efeito e = Tipos::Contra(tipo, tiro->Classe());
            if (e == Tipos::Efeito::Passa) {
                // O do seu tipo atravessa voce: um efeitinho, uma vez por tiro.
                if (mJaPassou.insert(tiro.get()).second) Efeito(tipo, tiro->GetPosition(), 6, 70.0f);
                continue;
            }
            if (e != Tipos::Efeito::Absorve) continue;
            // O do tipo que ele vence e super efetivo: some nele, vale nota, e o elemento dele explode.
            Efeito(tipo, tiro->GetPosition(), 12, 150.0f);
            Efeito(tiro->Classe(), tiro->GetPosition(), 5, 90.0f);
            if (Battle* bb = Batalha()) bb->Faiscar(tiro->GetPosition(), Hud::Faisca::Coleta);
            tiro->Kill();
            mPremioDoTipo += Tipos::kPremioDoSuperEfetivo;
        }
    }
    if (mPremioDoTipo >= 0.6f) {
        if (Battle* bb = Batalha()) bb->Conquistar(mPremioDoTipo);
        mPremioDoTipo = 0.0f;
    }
}

// ---- Os efeitinhos ----

void Daniel::Efeito(const int tipo, const Vector2 onde, const int quantas, const float forca) {
    if (tipo < 0 || tipo >= Tipos::kTipos) return;
    if (mParticulas.size() >= Tipos::kMaisParticulas) return;
    for (const Tipos::Particula& p : Tipos::Explodir(tipo, onde, quantas, forca, mSorteioDosEfeitos)) {
        if (mParticulas.size() >= Tipos::kMaisParticulas) break;
        mParticulas.push_back(p);
    }
}

void Daniel::AndarOsEfeitos(const float deltaTime) {
    for (Tipos::Particula& p : mParticulas) Tipos::Andar(p, deltaTime);
    mParticulas.erase(std::remove_if(mParticulas.begin(), mParticulas.end(),
                                     [](const Tipos::Particula& p) { return !Tipos::Viva(p); }),
                      mParticulas.end());
    for (const Tipos::Particula& p : mParticulas) {
        const float u = std::clamp(p.idade / std::max(0.01f, p.vida), 0.0f, 1.0f);
        const float a = 1.0f - u;
        const float r = p.tamanho * (1.0f - 0.5f * u);
        switch (p.tipo) {
            case Tipos::kFogo: {
                // A fagulha: amarela, esfriando para o vermelho, com a chama para cima.
                const SDL_Color cor{255, static_cast<Uint8>(230.0f - 150.0f * u), static_cast<Uint8>(90.0f - 70.0f * u),
                                    static_cast<Uint8>(230.0f * a)};
                Disco(p.onde, r * 0.8f, cor, true);
                Linha(p.onde, p.onde + Vector2(0.0f, -r * 2.2f), cor, true);
                break;
            }
            case Tipos::kAgua:
                // A gota: azul, com o brilho.
                Disco(p.onde, r * 0.8f, SDL_Color{110, 185, 255, static_cast<Uint8>(230.0f * a)}, true);
                Linha(p.onde + Vector2(-r * 0.3f, -r * 0.3f), p.onde + Vector2(-r * 0.3f, -r * 0.3f), SDL_Color{230, 245, 255, static_cast<Uint8>(230.0f * a)}, true);
                break;
            default: {
                // A folha: tres tracos ao longo do giro, o do meio mais comprido.
                const Vector2 d(Math::Cos(p.giro), Math::Sin(p.giro)), lado(-d.y, d.x);
                const SDL_Color cor{120, 220, 110, static_cast<Uint8>(230.0f * a)};
                Linha(p.onde - d * r * 1.4f, p.onde + d * r * 1.4f, cor, true);
                Linha(p.onde - d * r * 0.8f + lado, p.onde + d * r * 0.8f + lado, cor, true);
                Linha(p.onde - d * r * 0.8f - lado, p.onde + d * r * 0.8f - lado, cor, true);
                break;
            }
        }
    }
}

void Daniel::RastroDosTiros() {
    const Battle* b = Batalha();
    if (!b) return;
    ProjectileManager* gerente = const_cast<Battle*>(b)->GerenteDeProjeteis();
    if (!gerente) return;
    // O tiro do aluno deixa um rastro pequeno do elemento dele: a fagulha, a gota, a folha.
    for (const auto& tiro : gerente->GetPlayerProjectiles()) {
        if (!tiro || tiro->GetState() != ActorState::Active) continue;
        const int tipo = TipoDoTiro(tiro.get());
        if (tipo < 0 || !mSorteioDosEfeitos.Chance(0.3f)) continue;
        Efeito(tipo, tiro->GetPosition() + Vector2(0.0f, 6.0f), 1, 25.0f);
    }
}

float Daniel::Golpear(const Vector2 onde, const float raio, const int tipo, const bool fechado) {
    const Battle* b = Batalha();
    if (!b) return 0.0f;
    ProjectileManager* gerente = const_cast<Battle*>(b)->GerenteDeProjeteis();
    if (!gerente) return 0.0f;
    float dano = 0.0f;
    for (const auto& tiro : gerente->GetPlayerProjectiles()) {
        if (!tiro || tiro->GetState() != ActorState::Active) continue;
        if ((tiro->GetPosition() - onde).Length() > raio) continue;
        const int doTiro = TipoDoTiro(tiro.get());
        const Vector2 p = tiro->GetPosition();
        tiro->Kill();
        if (fechado) {
            // Fechado: o tiro bate e faz "tim".
            Circulo(onde, raio + 3.0f, kTim, 16, true);
            if (Battle* bb = Batalha()) bb->Faiscar(p, Hud::Faisca::Raspao);
            continue;
        }
        const Tipos::Golpe golpe = Tipos::GolpeContra(doTiro, tipo);
        dano += Tipos::FatorDoDano(doTiro, tipo);
        if (golpe == Tipos::Golpe::Vantagem) {
            // Super efetivo: o elemento de quem atirou explode no alvo, com um anel branco.
            Efeito(doTiro, p, 12, 170.0f);
            Circulo(onde, raio + 6.0f, SDL_Color{255, 255, 255, 240}, 18, true);
            if (Battle* bb = Batalha()) bb->Faiscar(p, Hud::Faisca::Derrubada);
        } else if (golpe == Tipos::Golpe::Resistencia) {
            // Resistiu: o arco cinza do escudo, e quase nada do elemento.
            Circulo(onde, raio + 2.0f, kTim, 10, true);
            Efeito(doTiro, p, 2, 40.0f);
            if (Battle* bb = Batalha()) bb->Faiscar(p, Hud::Faisca::Raspao);
        } else {
            Efeito(doTiro, p, 5, 100.0f);
            if (Battle* bb = Batalha()) bb->Faiscar(p, Hud::Faisca::Acerto);
        }
    }
    return dano;
}

// ---- Fase 1: a coleta ----

void Daniel::Pousar(Amostra& a) {
    // Pousou: vira um ponto do grafico, sem machucar - o conjunto de dados cresce.
    if (!a.p) return;
    a.p->SetPosition(a.destino);
    Parar(a.p);
    a.p->DefinirEnfeite(true);
    a.p->SetScale(1.1f);
    mPontos.push_back(Ponto{a.p, a.tipo, a.destino});
    a.p = nullptr;
    while (static_cast<int>(mPontos.size()) > Ecossistema::kMaisPontos) {
        Matar(mPontos.front().p);
        mPontos.erase(mPontos.begin());
    }
}

void Daniel::AtualizarColeta(const float t, const float deltaTime) {
    const SDL_FRect c = Campo();
    const Ecossistema::Degrau& g = mDegrauDaColeta;
    Varrer(t, 0.3f, 10.0f);
    DesenharGrafico();

    // As fontes, no alto: o sensor, a planilha e a API.
    {
        for (size_t k = 0; k < mFontes.size(); ++k) {
            const Vector2 onde(c.x + c.w * Ecossistema::XDaFonte(static_cast<int>(k)), c.y + kYDasFontes);
            if (!mFontes[k]) {
                mFontes[k] = Um(kGatilhoDaFonte, onde, Ecossistema::kFontes[k]);
                if (mFontes[k]) mFontes[k]->DefinirEnfeite(true);
            }
            if (mFontes[k]) {
                mFontes[k]->SetPosition(onde);
                mFontes[k]->RenovarTempoDeVida();
            }
        }
    }
    for (const Ponto& p : mPontos) if (p.p) p.p->RenovarTempoDeVida();

    // As amostras: cada onda de um tipo, na ordem que ensina o triangulo.
    const Ecossistema::Onda onda = Ecossistema::OndaNoTempo(t);
    const float entre = g.entreAmostras;
    if (onda != Ecossistema::Onda::Nenhuma && mEscolheu && t >= mProximaAmostra) {
        mProximaAmostra = t + entre;
        const std::vector<Vector2> alunos = [&]() {
            std::vector<Vector2> v;
            if (const Battle* b = Batalha()) for (const Player* p : b->Jogadores()) if (p) v.push_back(p->GetPosition());
            return v;
        }();
        for (int k = 0; k < g.porVez; ++k) {
            const int tipo = Ecossistema::TipoDaOnda(onda, mTipos.empty() ? 0 : mTipos[0], mSorteio);
            const int fonte = mSorteio.Inteiro(0, static_cast<int>(mFontes.size()) - 1);
            const Vector2 de(c.x + c.w * Ecossistema::XDaFonte(fonte), c.y + kYDasFontes);
            ++mAmostrasFeitas;
            // Umas sao "medidas" no aluno: passam por onde ele estava e pousam um pouco depois.
            Vector2 para = NoCampo(Ecossistema::LugarNoGrafico(tipo, mSorteio));
            if (!alunos.empty() && mSorteio.Chance(Ecossistema::kChanceDeMedirOAluno)) {
                const Vector2 aluno = alunos[static_cast<size_t>(mSorteio.Inteiro(0, static_cast<int>(alunos.size()) - 1))];
                const SDL_FRect gr = Grafico();
                const Vector2 alem = de + (aluno - de) * 1.3f;
                para = Vector2(std::clamp(alem.x, gr.x, gr.x + gr.w), std::clamp(alem.y, gr.y, c.y + c.h - 12.0f));
            }
            Vector2 rumo = para - de;
            if (rumo.Length() < 1.0f) rumo = Vector2(0.0f, 1.0f);
            rumo = rumo * (g.velocidade / rumo.Length());
            if (BossProjectile* p = Monstrinho(kGatilhoDaAmostra, de, tipo, rumo)) {
                p->SetScale(1.7f);
                mAmostras.push_back(Amostra{p, tipo, Ecossistema::kVidaDaAmostra, para});
            }
        }
    }
    // Cada amostra: o tiro do aluno a acerta (o triangulo vale), e a que chega pousa.
    for (Amostra& a : mAmostras) {
        if (!a.p) continue;
        a.vida -= Golpear(a.p->GetPosition(), kRaioDaAmostra, a.tipo);
        if (a.vida <= 0.0f) {
            Efeito(a.tipo, a.p->GetPosition(), 10, 120.0f);
            Matar(a.p);
            continue;
        }
        a.p->RenovarTempoDeVida();
        const auto* rb = a.p->GetComponent<RigidBodyComponent>();
        const float passo = rb ? rb->GetVelocity().Length() * deltaTime : 0.0f;
        if ((a.p->GetPosition() - a.destino).Length() <= std::max(4.0f, passo)) Pousar(a);
    }
    mAmostras.erase(std::remove_if(mAmostras.begin(), mAmostras.end(), [](const Amostra& a) { return !a.p; }), mAmostras.end());

    // O leque do proprio professor, sem tipo.
    if (mEscolheu && t >= Ecossistema::kFimDaEscolha && t >= mProximoLeque) {
        mProximoLeque = t + g.entreLeques;
        Leque(GetPosition() + Vector2(0.0f, 40.0f), g.tirosDoLeque, kAberturaDoLeque, kVelocidadeDoLeque);
    }
}

// ---- Fase 2: as k-medias ----

void Daniel::AtualizarAgrupamento(const float t, const float deltaTime) {
    (void)deltaTime;
    const SDL_FRect c = Campo();
    const Agrupamento::Degrau& g = mDegrauDoAgrupamento;
    Varrer(t, 0.3f, 12.0f);
    DesenharGrafico();

    // Quem chega aqui sem a coleta (ou com pouco) ganha um conjunto de dados.
    if (!mAgrupou) {
        mAgrupou = true;
        while (mPontos.size() < 30) {
            const int tipo = mSorteio.Inteiro(0, Tipos::kTipos - 1);
            const Vector2 onde = NoCampo(Ecossistema::LugarNoGrafico(tipo, mSorteio));
            if (BossProjectile* p = Monstrinho(kGatilhoDaAmostra, onde, tipo, std::nullopt)) {
                p->DefinirEnfeite(true);
                p->SetScale(1.1f);
                mPontos.push_back(Ponto{p, tipo, onde});
                Efeito(tipo, onde, 3, 50.0f);
            } else {
                break;
            }
        }
        std::vector<Vector2> pontos;
        for (const Ponto& p : mPontos) pontos.push_back(p.onde);
        mCentros = Agrupamento::Sementes(pontos, Agrupamento::kGrupos, mSorteio);
        mDeOnde = mCentros;
        mParaOnde = mCentros;
        mCentroidesP.assign(mCentros.size(), nullptr);
        mProximaIteracao = 1.2f;
        mAtribuiuEm = -10.0f;
        mProximoOutlier = Agrupamento::kOutliersDesde;
        mProximoLeque = 2.0f;
    }
    for (const Ponto& p : mPontos) if (p.p) p.p->RenovarTempoDeVida();

    // Os outliers: chegam a partir de kOutliersDesde, pousam longe de tudo e puxam a media.
    const SDL_FRect gr = Grafico();
    if (t >= mProximoOutlier && mOutliersFeitos < g.maisOutliers) {
        mProximoOutlier = t + g.entreOutliers;
        ++mOutliersFeitos;
        const Vector2 de(c.x + c.w * Ecossistema::XDaFonte(mSorteio.Inteiro(0, 2)), c.y + kYDasFontes);
        const Vector2 para = Agrupamento::LugarDoOutlier(mCentros, Vector2(gr.x, gr.y), Vector2(gr.x + gr.w, gr.y + gr.h), mSorteio);
        Vector2 rumo = para - de;
        rumo = rumo * (170.0f / std::max(1.0f, rumo.Length()));
        if (BossProjectile* p = Um(Agrupamento::kGatilhoDoOutlier, de, "Outlier", rumo)) {
            p->SetScale(1.8f);
            mOutliers.push_back(Outlier{p, Agrupamento::kVidaDoOutlier, para, false});
        }
    }
    std::vector<Vector2> pesados;
    for (Outlier& o : mOutliers) {
        if (!o.p) continue;
        o.vida -= Golpear(o.p->GetPosition(), kRaioDoOutlier, -1);
        if (o.vida <= 0.0f) {
            // Derrubou o dado que nao combinava: o grupo se conserta.
            if (Battle* b = Batalha()) {
                b->Faiscar(o.p->GetPosition(), Hud::Faisca::Coleta);
                b->Conquistar(Agrupamento::kPremioDoOutlier);
            }
            Matar(o.p);
            continue;
        }
        o.p->RenovarTempoDeVida();
        if (!o.pousou && (o.p->GetPosition() - o.destino).Length() < 6.0f) {
            o.pousou = true;
            o.p->SetPosition(o.destino);
            Parar(o.p);
        }
        if (o.pousou) {
            pesados.push_back(o.destino);
            // O puxao: uma linha vermelha do outlier ao centroide que ele arrasta.
            const int alvo = Agrupamento::MaisPerto(mCentros, o.destino);
            if (alvo >= 0) Tracejada(o.destino, mCentros[static_cast<size_t>(alvo)], SDL_Color{255, 90, 80, 170}, 6.0f);
        }
    }
    mOutliers.erase(std::remove_if(mOutliers.begin(), mOutliers.end(), [](const Outlier& o) { return !o.p; }), mOutliers.end());

    // A iteracao: a atribuicao, o deslize ate a media, e o anel de cada grupo, no tipo dele.
    std::vector<Vector2> pontos;
    std::vector<int> tipos;
    for (const Ponto& p : mPontos) {
        pontos.push_back(p.onde);
        tipos.push_back(p.tipo);
    }
    const bool instavel = t >= Agrupamento::kInstavelDesde;
    if (t >= mProximaIteracao && mDeslizeDesde < 0.0f) {
        mProximaIteracao = t + g.entreIteracoes * (instavel ? g.fatorInstavel : 1.0f);
        mGrupos = Agrupamento::Atribuir(pontos, mCentros);
        mAtribuiuEm = t;
        mDeOnde = mCentros;
        mParaOnde = Agrupamento::Atualizar(pontos, mGrupos, mCentros, pesados, Agrupamento::kPesoDoOutlier);
        mDeslizeDesde = t;
    }
    if (mDeslizeDesde >= 0.0f) {
        const float u = std::clamp((t - mDeslizeDesde) / std::max(0.05f, g.deslize), 0.0f, 1.0f);
        const float suave = u * u * (3.0f - 2.0f * u);
        for (size_t k = 0; k < mCentros.size(); ++k) mCentros[k] = mDeOnde[k] + (mParaOnde[k] - mDeOnde[k]) * suave;
        if (u >= 1.0f) {
            mDeslizeDesde = -1.0f;
            mGrupos = Agrupamento::Atribuir(pontos, mCentros);
            mTiposDosGrupos.assign(mCentros.size(), -1);
            for (size_t k = 0; k < mCentros.size(); ++k) {
                const int tipo = Agrupamento::TipoDoGrupo(tipos, mGrupos, static_cast<int>(k));
                mTiposDosGrupos[k] = tipo;
                // O grupo ataca com o tipo dele: um anel de monstrinhos.
                const auto anel = DispararEm(Agrupamento::kGatilhoDoAnel, mCentros[k], g.tirosDoAnel, std::nullopt,
                                             std::nullopt, g.velocidadeDoAnel);
                for (BossProjectile* p : anel) {
                    if (tipo >= 0) {
                        Cara(p, Tipos::kCaras[static_cast<size_t>(tipo)]);
                        p->DefinirClasse(tipo);
                    } else {
                        Cara(p, "Outlier");
                        p->DefinirClasse(-1);
                    }
                }
                Efeito(tipo, mCentros[k], 10, 140.0f);
            }
            SomDoDisparo();
        }
    }
    if (mTiposDosGrupos.size() != mCentros.size()) {
        mTiposDosGrupos.assign(mCentros.size(), -1);
        if (!mGrupos.empty()) {
            for (size_t k = 0; k < mCentros.size(); ++k) mTiposDosGrupos[k] = Agrupamento::TipoDoGrupo(tipos, mGrupos, static_cast<int>(k));
        }
    }
    // O desenho: a linha de cada ponto ao centroide dele (forte logo depois da atribuicao), e o centroide.
    const float brilho = std::clamp(1.0f - (t - mAtribuiuEm) / 0.9f, 0.25f, 1.0f);
    for (size_t k = 0; k < pontos.size() && k < mGrupos.size(); ++k) {
        const int gr2 = mGrupos[k];
        if (gr2 < 0 || gr2 >= static_cast<int>(mCentros.size())) continue;
        Linha(pontos[k], mCentros[static_cast<size_t>(gr2)], Alfa(CorDo(mTiposDosGrupos[static_cast<size_t>(gr2)]), 0.55f * brilho));
    }
    const float tremor = instavel ? 4.0f : 0.0f;
    for (size_t k = 0; k < mCentros.size(); ++k) {
        const Vector2 onde = mCentros[k] + Vector2(tremor * Math::Sin(mRelogio * 31.0f + static_cast<float>(k)),
                                                    tremor * Math::Cos(mRelogio * 27.0f + static_cast<float>(k)));
        const int tipo = mTiposDosGrupos[k];
        if (!mCentroidesP[k]) {
            mCentroidesP[k] = Um(Agrupamento::kGatilhoDoCentroide, onde, "C4");
            if (mCentroidesP[k]) mCentroidesP[k]->DefinirEnfeite(true);
        }
        if (mCentroidesP[k]) {
            Cara(mCentroidesP[k], tipo >= 0 ? "C" + std::to_string(tipo) : "C4");
            mCentroidesP[k]->SetPosition(onde);
            mCentroidesP[k]->RenovarTempoDeVida();
        }
        Circulo(onde, 22.0f, Alfa(CorDo(tipo), 0.9f), 20);
    }

    if (t >= mProximoLeque) {
        mProximoLeque = t + g.entreLeques;
        Leque(GetPosition() + Vector2(0.0f, 40.0f), g.tirosDoLeque, kAberturaDoLeque, kVelocidadeDoLeque);
    }
}

// ---- Fase 3: o overfitting ----

void Daniel::Rotulo(const std::string& texto, const Vector2 onde, const SDL_Color tinta, const int largura) {
    if (!mScene) return;
    if (mRotulos.size() <= mRotulosUsados) {
        auto novo = std::make_unique<EtiquetaDeCodigo>(mScene, " ", 96, 22, kOrdemDasLinhas + 2, 18);
        mRotulos.push_back(novo.get());
        mScene->AddActor(std::move(novo));
    }
    EtiquetaDeCodigo* e = mRotulos[mRotulosUsados++];
    e->Redimensionar(largura, 22);
    e->Pintar(SDL_Color{tinta.r, tinta.g, tinta.b, 150}, SDL_Color{10, 14, 26, 200}, tinta);
    e->Escrever(texto);
    e->SetPosition(onde);
    e->Mostrar(true);
}

void Daniel::EsconderRotulosSobrando() {
    for (size_t i = mRotulosUsados; i < mRotulos.size(); ++i) if (mRotulos[i]) mRotulos[i]->Mostrar(false);
}

void Daniel::AtualizarSobreajuste(const float t, const float deltaTime) {
    const Battle* b = Batalha();
    if (!b) return;
    mRotulosUsados = 0;
    const Sobreajuste::Degrau& g = mDegrauDoSobreajuste;
    const auto& jogadores = b->Jogadores();
    // Mais baixo que nas outras fases: os nucleos giram em volta dele inteiros, longe do alto.
    Varrer(t, 0.22f, 14.0f);
    if (mAlvo) mAlvo->y = Campo().y + kYDoProfessorNoModelo;
    const Vector2 professor = GetPosition();

    if (!mModeloComecou) {
        mModeloComecou = true;
        mMemorias.assign(jogadores.size(), Sobreajuste::Memoria{});
        for (int k = 0; k < Sobreajuste::kNucleos; ++k) mNucleos[static_cast<size_t>(k)] = Nucleo{nullptr, g.vidaDoNucleo, false};
        mProximoTiro = Sobreajuste::kTreino;
        mProximoDecorado = Sobreajuste::kTreino + 1.5f;
        mDecoradoEm = -1.0f;
        mAlunoDaVez = 0;
        mProximoLeque = Sobreajuste::kTreino + 1.0f;
    }
    if (mMemorias.size() < jogadores.size()) mMemorias.resize(jogadores.size());

    // Os nucleos: um de cada tipo, girando em volta do professor. O aberto apanha; o fechado, "tim".
    for (int k = 0; k < Sobreajuste::kNucleos; ++k) {
        Nucleo& n = mNucleos[static_cast<size_t>(k)];
        if (n.quebrado) continue;
        const Vector2 onde = Sobreajuste::OndeDoNucleo(k, professor, mRelogio);
        const bool aberto = Sobreajuste::Aberto(k, t);
        if (!n.p) {
            n.p = Um(kGatilhoDoNucleo, onde, std::string(Tipos::kCaras[static_cast<size_t>(k)]) + "Fechado");
            if (n.p) n.p->DefinirEnfeite(true);
        }
        if (!n.p) continue;
        n.p->SetPosition(onde);
        n.p->RenovarTempoDeVida();
        Cara(n.p, std::string(Tipos::kCaras[static_cast<size_t>(k)]) + (aberto ? "Aberto" : "Fechado"));
        if (aberto) {
            Circulo(onde, kRaioDoNucleo + 5.0f + 2.0f * Math::Sin(mRelogio * 9.0f), kCor[k], 20, true);
            // O alvo amarelo piscando: e este que apanha.
            const float r = kRaioDoNucleo + 18.0f + 3.0f * Math::Sin(mRelogio * 7.0f);
            const SDL_Color alvo{255, 222, 60, static_cast<Uint8>(170.0f + 80.0f * (0.5f + 0.5f * Math::Sin(mRelogio * 10.0f)))};
            Circulo(onde, r, alvo, 24, true);
            for (const Vector2 d : {Vector2(1.0f, 0.0f), Vector2(-1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(0.0f, -1.0f)}) {
                Linha(onde + d * (r - 6.0f), onde + d * (r + 8.0f), alvo, true);
            }
            // A vida que falta, num arco em volta.
            const float u = std::clamp(n.vida / std::max(1.0f, g.vidaDoNucleo), 0.0f, 1.0f);
            const int lados = std::max(1, static_cast<int>(24.0f * u));
            for (int s = 0; s < lados; ++s) {
                const float a0 = -Math::Pi / 2.0f + Math::TwoPi * static_cast<float>(s) / 24.0f;
                const float a1 = -Math::Pi / 2.0f + Math::TwoPi * static_cast<float>(s + 1) / 24.0f;
                Linha(onde + Vector2(Math::Cos(a0), Math::Sin(a0)) * (kRaioDoNucleo + 10.0f),
                      onde + Vector2(Math::Cos(a1), Math::Sin(a1)) * (kRaioDoNucleo + 10.0f), SDL_Color{255, 255, 255, 200}, true);
            }
        }
        n.vida -= Golpear(onde, kRaioDoNucleo, k, !aberto);
        if (n.vida <= 0.0f) {
            // Quebrou: o elemento explode, e vale muita nota.
            n.quebrado = true;
            Efeito(k, onde, 50, 240.0f);
            if (Battle* bb = Batalha()) {
                bb->Faiscar(onde, Hud::Faisca::Coleta);
                bb->Conquistar(g.premioDoNucleo);
            }
            Matar(n.p);
        }
    }

    // O modelo grava os passos (os dados de treino) e desenha a curva que passa por todos.
    const bool validando = t >= Sobreajuste::kValidacao;
    for (size_t i = 0; i < jogadores.size() && i < mMemorias.size(); ++i) {
        if (!jogadores[i]) continue;
        if (!validando) Sobreajuste::Registrar(mMemorias[i], jogadores[i]->GetPosition(), t);
        if (mRiscou) continue;
        const auto& pontos = mMemorias[i].pontos;
        for (const Vector2& p : pontos) Disco(p, 2.0f, kMemoria);
        const float tremor = validando ? 3.0f * Math::Sin(mRelogio * 40.0f) : 0.0f;
        const auto curva = Sobreajuste::Curva(pontos, 6, 12.0f + (validando ? 10.0f : 0.0f));
        // Na validacao ela treme, na cor dela: quem falhou foi o modelo, e nao o aluno.
        const SDL_Color cor = kCurva;
        for (size_t k = 1; k < curva.size(); ++k) {
            Linha(curva[k - 1] + Vector2(0.0f, tremor), curva[k] + Vector2(0.0f, tremor), cor);
            Linha(curva[k - 1] + Vector2(0.0f, tremor + 1.0f), curva[k] + Vector2(0.0f, tremor + 1.0f), Alfa(cor, 0.5f));
        }
    }

    if (!validando) {
        // A previsao: os tres nucleos atiram juntos para onde o aluno estava indo - um tiro de cada
        // tipo, e o do tipo que vence o seu e o que importa desviar.
        if (t >= mProximoTiro && !mMemorias.empty()) {
            mProximoTiro = t + g.entreTiros;
            const size_t i = static_cast<size_t>(mAlunoDaVez) % mMemorias.size();
            mAlunoDaVez = (mAlunoDaVez + 1) % static_cast<int>(mMemorias.size());
            if (!mMemorias[i].pontos.empty()) {
                bool marcou = false;
                for (int k = 0; k < Sobreajuste::kNucleos; ++k) {
                    const Nucleo& n = mNucleos[static_cast<size_t>(k)];
                    if (n.quebrado || !n.p) continue;
                    const Vector2 de = n.p->GetPosition();
                    // Mira onde o aluno vai estar quando o tiro chegar (o tempo de viagem ate ele).
                    const float viagem = (mMemorias[i].pontos.back() - de).Length() / std::max(1.0f, g.velocidade);
                    const Vector2 para = Sobreajuste::Previsao(mMemorias[i], viagem * g.adiante);
                    Vector2 rumo = para - de;
                    rumo = rumo * (g.velocidade / std::max(1.0f, rumo.Length()));
                    if (BossProjectile* p = Monstrinho(kGatilhoDaPrevisao, de, k, rumo)) p->SetScale(1.4f);
                    // O fantasma: onde o modelo acha que o aluno vai estar quando a rajada chegar.
                    if (!marcou) {
                        marcou = true;
                        mFantasmas.push_back(Fantasma{para, i, t, t + (para - de).Length() / std::max(1.0f, g.velocidade)});
                        ++mRajadasDaPrevisao;
                    }
                }
            }
        }
        // E o leque do proprio professor, sem tipo.
        if (t >= mProximoLeque) {
            mProximoLeque = t + g.entreLeques;
            Leque(professor + Vector2(0.0f, 40.0f), g.tirosDoLeque, kAberturaDoLeque, kVelocidadeDoLeque);
        }
        // O que ele decorou: a mira aparece onde o aluno mais passou, e os nucleos atiram la.
        if (t >= mProximoDecorado && mDecoradoEm < 0.0f && !mMemorias.empty()) {
            mProximoDecorado = t + g.entreDecorados;
            const size_t i = static_cast<size_t>(mSorteio.Inteiro(0, static_cast<int>(mMemorias.size()) - 1));
            const auto decorados = Sobreajuste::Decorados(mMemorias[i], 70.0f, 1);
            if (!decorados.empty()) {
                mDecorado = decorados.front();
                mDecoradoEm = t + g.avisoDoDecorado;
            }
        }
        if (mDecoradoEm >= 0.0f) {
            const float falta = std::max(0.0f, mDecoradoEm - t);
            const float r = 14.0f + 30.0f * falta / std::max(0.05f, g.avisoDoDecorado);
            Circulo(mDecorado, r, SDL_Color{255, 255, 255, 200}, 20, true);
            Rotulo("decorado", mDecorado + Vector2(0.0f, -r - 22.0f), SDL_Color{235, 240, 255, 255});
            Linha(mDecorado + Vector2(-r - 6.0f, 0.0f), mDecorado + Vector2(-r + 6.0f, 0.0f), SDL_Color{255, 255, 255, 220}, true);
            Linha(mDecorado + Vector2(r - 6.0f, 0.0f), mDecorado + Vector2(r + 6.0f, 0.0f), SDL_Color{255, 255, 255, 220}, true);
            Linha(mDecorado + Vector2(0.0f, -r - 6.0f), mDecorado + Vector2(0.0f, -r + 6.0f), SDL_Color{255, 255, 255, 220}, true);
            Linha(mDecorado + Vector2(0.0f, r - 6.0f), mDecorado + Vector2(0.0f, r + 6.0f), SDL_Color{255, 255, 255, 220}, true);
            if (t >= mDecoradoEm) {
                mDecoradoEm = -1.0f;
                for (int k = 0; k < Sobreajuste::kNucleos; ++k) {
                    const Nucleo& n = mNucleos[static_cast<size_t>(k)];
                    if (n.quebrado || !n.p) continue;
                    const Vector2 de = n.p->GetPosition();
                    const float centro = Math::ToDegrees(Math::Atan2(mDecorado.y - de.y, mDecorado.x - de.x));
                    for (int s = 0; s < g.tirosNoDecorado; ++s) {
                        const float a = Math::ToRadians(centro + (static_cast<float>(s) - (g.tirosNoDecorado - 1) * 0.5f) * 6.0f);
                        Monstrinho(kGatilhoDoDecorado, de, k, Vector2(Math::Cos(a), Math::Sin(a)) * g.velocidade * 0.9f);
                    }
                }
                SomDoDisparo();
            }
        }
    } else {
        // A validacao: chegam dados novos - o aluno de verdade, que mudou de caminho -, e o modelo,
        // que so decorou o treino, erra todos: o tracejado e o erro dele ate cada um. Quem falha e o
        // modelo, e o aluno ganhou: a curva se desfaz na cor dela, e um brilho dourado sai dele.
        const float desde = t - Sobreajuste::kValidacao;
        Vector2 de(1e9f, 1e9f), ate(-1e9f, -1e9f);
        for (const auto& m : mMemorias) {
            for (const Vector2& p : m.pontos) {
                de = Vector2(std::min(de.x, p.x), std::min(de.y, p.y));
                ate = Vector2(std::max(ate.x, p.x), std::max(ate.y, p.y));
            }
        }
        const bool temCurva = de.x <= ate.x;
        if (!mRiscou && temCurva) {
            const float u = std::clamp(desde / Sobreajuste::kRiscando, 0.0f, 1.0f);
            for (const auto& m : mMemorias) {
                const auto curva = Sobreajuste::Curva(m.pontos, 6, 22.0f);
                if (curva.size() < 3) continue;
                // Os dados novos aparecem um a um, cada um de um lado da curva, longe dela.
                for (int k = 0; k < kDadosNovos; ++k) {
                    if (u < static_cast<float>(k) / static_cast<float>(kDadosNovos)) break;
                    const size_t i = 1 + (curva.size() - 2) * static_cast<size_t>(k) / static_cast<size_t>(kDadosNovos);
                    Vector2 tangente = curva[i + 1] - curva[i - 1];
                    const float comprimento = std::max(1.0f, tangente.Length());
                    tangente = tangente * (1.0f / comprimento);
                    const Vector2 normal(-tangente.y, tangente.x);
                    const float lado = (k % 2 == 0 ? 1.0f : -1.0f) * (42.0f + 14.0f * static_cast<float>(k % 3));
                    const Vector2 novo = curva[i] + normal * lado;
                    Tracejada(curva[i], novo, kErroDoModelo, 5.0f);
                    Disco(novo, 5.0f, kDadoNovo, true);
                    Circulo(novo, 9.0f, kDadoNovo, 16, true);
                }
            }
            if (u >= 1.0f) {
                // Nao generalizou: a curva vira pedacos, que caem.
                mRiscou = true;
                for (const auto& m : mMemorias) {
                    const auto curva = Sobreajuste::Curva(m.pontos, 6, 22.0f);
                    for (size_t k = 1; k < curva.size(); k += 2) mPedacos.emplace_back(curva[k - 1], curva[k]);
                }
                if (Battle* bb = Batalha()) {
                    for (const Player* aluno : jogadores) if (aluno) bb->Faiscar(aluno->GetPosition(), Hud::Faisca::Coleta);
                }
            }
        }
        if (mRiscou) {
            const float caiu = std::max(0.0f, desde - Sobreajuste::kRiscando);
            if (caiu < Sobreajuste::kCaindo) {
                // Os pedacos se espalham - um pulinho para cima e para os lados - e caem devagar,
                // apagando: a curva fica la embaixo, e eles nao podem sumir da tela de uma vez.
                const float a = 1.0f - caiu / Sobreajuste::kCaindo;
                for (size_t k = 0; k < mPedacos.size(); ++k) {
                    const float lado = (k % 2 == 0 ? 1.0f : -1.0f) * (25.0f + 20.0f * static_cast<float>(k % 3)) * caiu;
                    const float y = -70.0f * caiu + 0.5f * 160.0f * caiu * caiu;
                    const Vector2 d(lado, y);
                    const Vector2 meio = (mPedacos[k].first + mPedacos[k].second) * 0.5f;
                    const Vector2 giro = (mPedacos[k].second - meio) * std::max(0.2f, 1.0f - caiu);
                    Linha(meio - giro + d, meio + giro + d, Alfa(kCurva, a));
                }
            }
        }
        // O veredito, acima da curva, enquanto ela se desfaz: o overfitting e do modelo.
        if (mScene && !mVeredito) {
            auto nova = std::make_unique<EtiquetaDeCodigo>(mScene, " ", 340, 26, kOrdemDasLinhas + 2, 20);
            nova->Pintar(SDL_Color{150, 225, 255, 200}, SDL_Color{10, 18, 32, 215}, SDL_Color{180, 235, 255, 255});
            nova->Escrever("overfitting: o modelo decorou, n\xc3\xa3o generalizou");
            mVeredito = nova.get();
            mScene->AddActor(std::move(nova));
        }
        if (mVeredito) {
            const bool mostrar = temCurva && desde < Sobreajuste::kRiscando + Sobreajuste::kCaindo + 0.8f;
            if (mostrar) {
                const SDL_FRect c = Campo();
                const float x = std::clamp((de.x + ate.x) * 0.5f, c.x + 180.0f, c.x + c.w - 180.0f);
                const float y = std::clamp(de.y - 60.0f, c.y + 200.0f, c.y + c.h - 40.0f);
                mVeredito->SetPosition(Vector2(x, y));
            }
            mVeredito->Mostrar(mostrar);
        }
        // Sem o modelo, o professor atira ele mesmo, sem tipo.
        if (t >= mProximoLeque) {
            mProximoLeque = t + g.entreLeques;
            Leque(professor + Vector2(0.0f, 40.0f), g.tirosDoLeque, kAberturaDoLeque, kVelocidadeDoLeque);
        }
    }
    // Os fantasmas da previsao: o modelo mostra onde acha que o aluno vai estar, e quando a rajada
    // chega e ele nao esta la, "errou!" - mudar o caminho e o jeito de ganhar dele.
    for (size_t q = 0; q < mFantasmas.size(); ++q) {
        Fantasma& f = mFantasmas[q];
        const Player* aluno = f.aluno < jogadores.size() ? jogadores[f.aluno] : nullptr;
        // So o da rajada mais nova aparece; os outros so sao julgados.
        const bool maisNovo = q + 1 == mFantasmas.size();
        if (t < f.chega) {
            if (!maisNovo) continue;
            const float pulso = 0.5f + 0.5f * Math::Sin(mRelogio * 12.0f);
            Circulo(f.onde, 18.0f, SDL_Color{235, 240, 255, static_cast<Uint8>(120.0f + 80.0f * pulso)}, 20, true);
            Disco(f.onde, 6.0f, SDL_Color{235, 240, 255, 110}, true);
            // O tracejado dos nucleos ate ele, so no instante do tiro.
            if (t - f.saiu < 0.3f) {
                for (const Nucleo& n : mNucleos) {
                    if (!n.quebrado && n.p) Tracejada(n.p->GetPosition(), f.onde, SDL_Color{235, 240, 255, 80}, 6.0f);
                }
            }
            if (mRajadasDaPrevisao <= kRajadasComNome) {
                Rotulo("previs\xc3\xa3o", f.onde + Vector2(0.0f, -36.0f), SDL_Color{235, 240, 255, 255});
            }
        } else {
            if (!f.julgado) {
                f.julgado = true;
                f.errou = aluno && (aluno->GetPosition() - f.onde).Length() > 48.0f && t - mUltimoErrou >= kEntreErrous;
                if (f.errou) mUltimoErrou = t;
            }
            if (f.errou) {
                const float u = std::clamp((t - f.chega) / 0.9f, 0.0f, 1.0f);
                Circulo(f.onde, 18.0f + 26.0f * u, Alfa(kDadoNovo, 1.0f - u), 20, true);
                Rotulo("errou!", f.onde + Vector2(0.0f, -36.0f - 14.0f * u), kDadoNovo);
            }
        }
    }
    mFantasmas.erase(std::remove_if(mFantasmas.begin(), mFantasmas.end(),
                                    [t](const Fantasma& f) { return t > f.chega + 0.9f; }),
                     mFantasmas.end());
    EsconderRotulosSobrando();
    (void)deltaTime;
}

// ---- Final: a matriz de confusao ----

Confusao::Grade Daniel::GradeDaMatriz() const {
    const SDL_FRect c = Campo();
    return Confusao::Grade{c.x + c.w * 0.24f, c.y + 190.0f, c.w * 0.52f, c.h * 0.4f};
}

void Daniel::AtualizarConfusao(const float t, const float deltaTime) {
    (void)deltaTime;
    const Battle* b = Batalha();
    if (!b) return;
    mRotulosUsados = 0;
    const Confusao::Degrau& g = mDegrauDaConfusao;
    const Confusao::Grade grade = GradeDaMatriz();
    const auto& jogadores = b->Jogadores();
    Varrer(t, 0.3f, 10.0f);
    const float lw = grade.w / static_cast<float>(Confusao::kTipos);
    const float lh = grade.h / static_cast<float>(Confusao::kTipos);

    // Os erros de cada casa, agora: a casa acende enquanto tem um.
    std::array<std::array<int, Confusao::kTipos>, Confusao::kTipos> errosNaCasa{};
    int errosNoCampo = 0;
    for (const Classificado& cl : mClassificados) {
        if (cl.p && cl.pousou && cl.real != cl.previsto && cl.corrigidoEm < 0.0f) {
            ++errosNaCasa[static_cast<size_t>(cl.real)][static_cast<size_t>(cl.previsto)];
            ++errosNoCampo;
        }
    }

    // A grade: a diagonal (os acertos) em verde, a casa com erro em vermelho, piscando.
    for (int r = 0; r < Confusao::kTipos; ++r) {
        for (int pr = 0; pr < Confusao::kTipos; ++pr) {
            const float x0 = grade.x + lw * static_cast<float>(pr), y0 = grade.y + lh * static_cast<float>(r);
            const bool diagonal = r == pr;
            const bool comErro = errosNaCasa[static_cast<size_t>(r)][static_cast<size_t>(pr)] > 0;
            SDL_Color cor = diagonal ? SDL_Color{110, 220, 130, 150} : SDL_Color{150, 165, 195, 90};
            if (comErro) cor = SDL_Color{255, 80, 70, static_cast<Uint8>(150.0f + 90.0f * (0.5f + 0.5f * Math::Sin(mRelogio * 10.0f)))};
            const float m = 4.0f;
            const Vector2 a(x0 + m, y0 + m), bb(x0 + lw - m, y0 + m), cc(x0 + lw - m, y0 + lh - m), d(x0 + m, y0 + lh - m);
            Linha(a, bb, cor);
            Linha(bb, cc, cor);
            Linha(cc, d, cor);
            Linha(d, a, cor);
            if (diagonal || comErro) {
                // Uma segunda moldura por dentro: a casa que importa se ve de longe.
                const float n = 8.0f;
                Linha(a + Vector2(n, n), bb + Vector2(-n, n), Alfa(cor, 0.5f));
                Linha(d + Vector2(n, -n), cc + Vector2(-n, -n), Alfa(cor, 0.5f));
            }
            // Quantos o modelo pos nesta casa.
            const int quantos = mMatriz[static_cast<size_t>(r)][static_cast<size_t>(pr)];
            Rotulo(std::to_string(quantos), Vector2(x0 + lw - 24.0f, y0 + 18.0f),
                   diagonal ? SDL_Color{150, 240, 165, 255} : comErro ? SDL_Color{255, 140, 130, 255} : SDL_Color{190, 200, 220, 255}, 40);
        }
    }
    // Os nomes: as linhas sao o tipo de verdade, as colunas o que o modelo previu.
    static const char* const kNomes[Confusao::kTipos] = {"fogo", "\xc3\xa1gua", "planta"};
    for (int k = 0; k < Confusao::kTipos; ++k) {
        Rotulo(kNomes[k], Vector2(grade.x - 52.0f, grade.y + lh * (static_cast<float>(k) + 0.5f)), kCor[k], 84);
        Rotulo(kNomes[k], Vector2(grade.x + lw * (static_cast<float>(k) + 0.5f), grade.y - 16.0f), kCor[k], 84);
    }
    Rotulo("real", Vector2(grade.x - 52.0f, grade.y - 16.0f), SDL_Color{210, 215, 230, 255}, 84);
    Rotulo("previsto \xe2\x86\x92", Vector2(grade.x + grade.w * 0.5f, grade.y - 44.0f), SDL_Color{210, 215, 230, 255}, 120);
    Rotulo("acur\xc3\xa1" "cia: " + std::to_string(Confusao::Porcento(mMatriz)) + "%",
           Vector2(grade.x + grade.w + 92.0f, grade.y + 14.0f), SDL_Color{150, 240, 165, 255}, 150);

    // O modelo classifica: cada monstrinho voa do professor para a casa que ele escolheu.
    const float entre = t < Confusao::kFimDaClassificacao ? g.entreAmostras : g.naSegundaEtapa;
    if (mEscolheu && t >= mProximaClassificada) {
        mProximaClassificada = t + entre;
        const int real = mSorteio.Inteiro(0, Confusao::kTipos - 1);
        // Com a tela cheia de erros, ele acerta: nao ha mais lugar para errar.
        const int previsto = errosNoCampo >= g.errosNoCampo ? real : Confusao::Previsto(mSorteio, real, g.acuracia);
        const Vector2 de = GetPosition() + Vector2(0.0f, 40.0f);
        Vector2 rumo = Confusao::CentroDaCelula(grade, real, previsto) - de;
        rumo = rumo * (g.velocidade / std::max(1.0f, rumo.Length()));
        if (BossProjectile* p = Monstrinho(Confusao::kGatilhoDaAmostra, de, real, rumo)) {
            p->SetScale(1.5f);
            mClassificados.push_back(Classificado{p, real, previsto, false, g.vidaDoErro, 0.0f, 0.0f, -1.0f, de});
        }
    }

    // Os lugares de cada casa, na ordem: os quietos e os erros dividem a casa.
    std::array<std::array<int, Confusao::kTipos>, Confusao::kTipos> ocupados{};
    for (Classificado& cl : mClassificados) {
        if (!cl.p) continue;
        cl.p->RenovarTempoDeVida();
        const Vector2 centro = Confusao::CentroDaCelula(grade, cl.real, cl.previsto);
        if (!cl.pousou) {
            // Voando: chega na casa e pousa. Na diagonal, quieto; fora dela, um erro.
            const auto* rb = cl.p->GetComponent<RigidBodyComponent>();
            const float passo = rb ? rb->GetVelocity().Length() * deltaTime : 0.0f;
            if ((cl.p->GetPosition() - centro).Length() <= std::max(6.0f, passo)) {
                cl.pousou = true;
                Parar(cl.p);
                cl.p->DefinirEnfeite(true);
                ++mMatriz[static_cast<size_t>(cl.real)][static_cast<size_t>(cl.previsto)];
                if (cl.real != cl.previsto) {
                    cl.vaza = t + g.paciencia;
                    cl.proximoTiro = t + 0.7f;
                }
            }
            continue;
        }
        // A casa em que ele mora agora (o corrigido, ja na diagonal).
        const int coluna = cl.corrigidoEm >= 0.0f ? cl.real : cl.previsto;
        int& vaga = ocupados[static_cast<size_t>(cl.real)][static_cast<size_t>(coluna)];
        const Vector2 lugar = Confusao::LugarNaCelula(grade, cl.real, coluna, vaga);
        ++vaga;
        if (cl.real == cl.previsto || cl.corrigidoEm >= 0.0f) {
            // Quieto - ou deslizando para a diagonal, corrigido.
            if (cl.corrigidoEm >= 0.0f) {
                const float u = std::clamp((t - cl.corrigidoEm) / 0.45f, 0.0f, 1.0f);
                cl.p->SetPosition(cl.de + (lugar - cl.de) * (u * (2.0f - u)));
                if (u < 1.0f) Tracejada(cl.de, lugar, SDL_Color{150, 240, 165, 170}, 6.0f);
            } else {
                cl.p->SetPosition(lugar);
            }
            continue;
        }
        // Um erro: atira no aluno, apanha do tiro dele e, sem correcao, vaza.
        cl.p->SetPosition(lugar);
        const float falta = std::clamp((cl.vaza - t) / std::max(0.1f, g.paciencia), 0.0f, 1.0f);
        Circulo(lugar, kRaioDaAmostra + 6.0f + 2.0f * Math::Sin(mRelogio * 12.0f), SDL_Color{255, 80, 70, 230}, 16, true);
        const int lados = std::max(1, static_cast<int>(20.0f * falta));
        for (int s = 0; s < lados; ++s) {
            const float a0 = -Math::Pi / 2.0f + Math::TwoPi * static_cast<float>(s) / 20.0f;
            const float a1 = -Math::Pi / 2.0f + Math::TwoPi * static_cast<float>(s + 1) / 20.0f;
            Linha(lugar + Vector2(Math::Cos(a0), Math::Sin(a0)) * (kRaioDaAmostra + 11.0f),
                  lugar + Vector2(Math::Cos(a1), Math::Sin(a1)) * (kRaioDaAmostra + 11.0f), SDL_Color{255, 255, 255, 190}, true);
        }
        cl.vida -= Golpear(lugar, kRaioDaAmostra + 4.0f, cl.real);
        if (cl.vida <= 0.0f) {
            // Corrigido: sai da casa errada e desliza para a diagonal - e vale nota.
            --mMatriz[static_cast<size_t>(cl.real)][static_cast<size_t>(cl.previsto)];
            ++mMatriz[static_cast<size_t>(cl.real)][static_cast<size_t>(cl.real)];
            cl.corrigidoEm = t;
            cl.de = lugar;
            Efeito(cl.real, lugar, 14, 150.0f);
            if (Battle* bb = Batalha()) {
                bb->Faiscar(lugar, Hud::Faisca::Coleta);
                bb->Conquistar(g.premioDaCorrecao);
            }
            continue;
        }
        if (t >= cl.proximoTiro) {
            cl.proximoTiro = t + g.entreTirosDoErro;
            // No aluno mais perto, no tipo de verdade dele.
            const Player* alvo = nullptr;
            for (const Player* a : jogadores) {
                if (a && (!alvo || (a->GetPosition() - lugar).Length() < (alvo->GetPosition() - lugar).Length())) alvo = a;
            }
            if (alvo) {
                Vector2 rumo = alvo->GetPosition() - lugar;
                rumo = rumo * (g.velocidadeDoErro / std::max(1.0f, rumo.Length()));
                Monstrinho(Confusao::kGatilhoDoErro, lugar, cl.real, rumo);
            }
        }
        if (t >= cl.vaza) {
            // Vazou para a producao: o erro estoura num anel do tipo dele, e fica na matriz.
            const auto anel = DispararEm(Confusao::kGatilhoDoVazamento, lugar, g.tirosDoVazamento, std::nullopt,
                                         std::nullopt, g.velocidadeDoVazamento);
            for (BossProjectile* p : anel) {
                Cara(p, Tipos::kCaras[static_cast<size_t>(cl.real)]);
                p->DefinirClasse(cl.real);
            }
            Efeito(cl.real, lugar, 16, 180.0f);
            SomDoDisparo();
            Matar(cl.p);
        }
    }
    // Os quietos demais de uma casa saem, os mais velhos primeiro (a contagem fica).
    std::array<std::array<int, Confusao::kTipos>, Confusao::kTipos> quietos{};
    for (auto it = mClassificados.rbegin(); it != mClassificados.rend(); ++it) {
        if (!it->p || !it->pousou) continue;
        const bool quieto = it->real == it->previsto || it->corrigidoEm >= 0.0f;
        if (!quieto) continue;
        int& n = quietos[static_cast<size_t>(it->real)][static_cast<size_t>(it->real)];
        if (++n > Confusao::kPorCelula - 1) Matar(it->p);
    }
    mClassificados.erase(std::remove_if(mClassificados.begin(), mClassificados.end(),
                                        [](const Classificado& cl) { return !cl.p; }),
                         mClassificados.end());

    // E o leque do proprio professor, sem tipo.
    if (mEscolheu && t >= 1.0f && t >= mProximoLeque) {
        mProximoLeque = t + g.entreLeques;
        Leque(GetPosition() + Vector2(0.0f, 40.0f), g.tirosDoLeque, kAberturaDoLeque, kVelocidadeDoLeque);
    }
    EsconderRotulosSobrando();
}
