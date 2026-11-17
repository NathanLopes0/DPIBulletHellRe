//
// O chefe da INF 216 - Projeto e Implementacao de Jogos Digitais, a optativa secreta.
//

#include "Ferreira.h"

#include <algorithm>
#include <cmath>

#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "../../Player/PlayerProjectile.h"
#include "BossesProjectiles/BossProjectile.h"
#include "PainelDeCodigo.h"

namespace {

    float Mistura(const float a, const float b, const float u) {
        return a + (b - a) * u;
    }

    std::string Inteiro(const float x) {
        return std::to_string(static_cast<int>(std::lround(x)));
    }

    /// Se pisca aceso em 't', a cada 'periodo' segundos.
    bool Aceso(const float t, const float periodo) {
        return static_cast<int>(std::floor(t / periodo)) % 2 == 0;
    }
}

Ferreira::Ferreira(Scene* scene) : Boss(scene) {
}

void Ferreira::SortearALuta() {
    mLutaSorteada = true;
    mSorteio = SorteioDaFase(HudHostil::kFase);
    const Dificuldade::Nivel n = NivelDeDificuldade();
    mDegrauDoHud = HudHostil::DegrauDo(n);
    mDegrauDasRegras = RegrasTrocadas::DegrauDo(n);
    mDegrauDoMotor = MotorQuebrado::DegrauDo(n);
    mDegrauDoLancamento = Lancamento::DegrauDo(n);
}

// ---- O que as fases dividem ----

std::vector<BossProjectile*> Ferreira::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
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

BossProjectile* Ferreira::Um(const char* gatilho, const Vector2 onde, const std::string& cara,
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

void Ferreira::DarVelocidade(const std::vector<BossProjectile*>& ps, const float centro, const float abertura,
                             const float velocidade) {
    // O mesmo leque do WaveAttack, mas ja: ele so poe a velocidade no primeiro quadro do tiro, e quem
    // le o tiro no quadro em que ele nasce (o lag, a travada) precisa dela agora.
    const size_t n = ps.size();
    const float inicio = centro - abertura / 2.0f;
    const float passo = n > 1 ? abertura / static_cast<float>(n - 1) : 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const float a = Math::ToRadians(inicio + passo * static_cast<float>(i));
        Andar(ps[i], Vector2(Math::Cos(a), Math::Sin(a)) * velocidade);
    }
}

BossProjectile* Ferreira::Peca(const char* gatilho, const Vector2 onde, const std::string& cara) {
    BossProjectile* p = Um(gatilho, onde, cara);
    if (p) p->DefinirMargemDeSaida(2.0f, 1.0f);
    return p;
}

std::vector<BossProjectile*> Ferreira::Anel(const char* gatilho, const int quantos, const float velocidade,
                                            const std::string& cara) {
    const int n = std::max(1, quantos);
    const float centro = mSorteio.Real(0.0f, 360.0f), abertura = 360.0f - 360.0f / static_cast<float>(n);
    auto criados = DispararEm(gatilho, GetPosition() + Vector2(0.0f, 30.0f), n, centro, abertura, velocidade);
    DarVelocidade(criados, centro, abertura, velocidade);
    for (BossProjectile* p : criados) Cara(p, cara);
    SomDoDisparo();
    return criados;
}

std::vector<BossProjectile*> Ferreira::Leque(const char* gatilho, const int quantos, const float abertura,
                                             const float velocidade, const std::string& cara) {
    const Vector2 de = GetPosition() + Vector2(0.0f, 40.0f);
    const float centro = AnguloAteOAluno(de);
    auto criados = DispararEm(gatilho, de, std::max(1, quantos), centro, abertura, velocidade);
    DarVelocidade(criados, centro, abertura, velocidade);
    for (BossProjectile* p : criados) Cara(p, cara);
    SomDoDisparo();
    return criados;
}

void Ferreira::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
    if (!mDisparandoEm) return;
    params.firePosition = mOndeDisparar;
    if (mQuantos > 0) params.numProjectiles = mQuantos;
    if (mCentroEm) params.centralAngle = *mCentroEm;
    if (mAberturaEm) params.angle = *mAberturaEm;
    if (mVelocidadeEm) params.projectileSpeed = *mVelocidadeEm;
}

void Ferreira::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                    const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

float Ferreira::AnguloAteOAluno(const Vector2 de) const {
    const Vector2 d = PosicaoDoAluno() - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

bool Ferreira::Vivo(const BossProjectile* p) {
    return p && p->GetState() == ActorState::Active;
}

void Ferreira::Matar(BossProjectile*& p) {
    if (Vivo(p)) p->Kill();
    p = nullptr;
}

void Ferreira::MatarTodos(std::vector<BossProjectile*>& ps) {
    for (BossProjectile*& p : ps) Matar(p);
    ps.clear();
}

void Ferreira::Cara(BossProjectile* p, const std::string& cara) {
    if (!p || cara.empty()) return;
    if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) anim->SetAnimation(cara);
}

void Ferreira::Parar(BossProjectile* p) {
    Andar(p, Vector2::Zero);
}

void Ferreira::Andar(BossProjectile* p, const Vector2 velocidade) {
    if (!p) return;
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(velocidade);
}

Vector2 Ferreira::VelocidadeDe(const BossProjectile* p) {
    const auto* rb = p ? p->GetComponent<RigidBodyComponent>() : nullptr;
    return rb ? rb->GetVelocity() : Vector2::Zero;
}

std::vector<Player*> Ferreira::Jogadores() const {
    std::vector<Player*> v;
    if (const Battle* b = Batalha()) {
        for (Player* p : b->Jogadores()) {
            if (p) v.push_back(p);
        }
    }
    return v;
}

float Ferreira::RaioDe(const Player* aluno) {
    const auto* colisor = aluno ? aluno->GetComponent<CircleColliderComponent>() : nullptr;
    return colisor ? colisor->GetRadius() : 6.0f;
}

bool Ferreira::NoCampo(const Vector2 p, const float folga) const {
    const SDL_FRect c = Campo();
    return p.x >= c.x - folga && p.x <= c.x + c.w + folga && p.y >= c.y - folga && p.y <= c.y + c.h + folga;
}

float Ferreira::Nota() const {
    return Batalha() ? Batalha()->NotaAtual() : 50.0f;
}

void Ferreira::Sanear(std::vector<BossProjectile*>& ps, const bool encolher) {
    for (BossProjectile*& p : ps) {
        if (!Vivo(p)) p = nullptr;
    }
    if (encolher) ps.erase(std::remove(ps.begin(), ps.end(), nullptr), ps.end());
}

void Ferreira::Sanear() {
    // As listas soltas encolhem; as que tem lugar marcado (a barra, a frota, os botoes) so esquecem.
    Sanear(mTiros, true);
    Sanear(mBarra, false);
    for (Janela& j : mJanelas) {
        Sanear(j.pecas, true);
        if (!Vivo(j.fechar)) j.fechar = nullptr;
    }
    Sanear(mEspinhos, true);
    mArcos.erase(std::remove_if(mArcos.begin(), mArcos.end(), [](const Arco& a) { return !Vivo(a.p); }), mArcos.end());
    mQueDaoAVolta.erase(std::remove_if(mQueDaoAVolta.begin(), mQueDaoAVolta.end(),
                                       [](const QueDaAVolta& q) { return !Vivo(q.p); }), mQueDaoAVolta.end());
    mAosSaltos.erase(std::remove_if(mAosSaltos.begin(), mAosSaltos.end(), [](const AosSaltos& s) { return !Vivo(s.p); }),
                     mAosSaltos.end());
    for (Descolado& d : mDescolados) {
        if (!Vivo(d.desenho)) d.desenho = nullptr;
        if (!Vivo(d.colisor)) d.colisor = nullptr;
    }
    Sanear(mDisquetes, true);
    mTravados.erase(std::remove_if(mTravados.begin(), mTravados.end(), [](const Travado& t) { return !Vivo(t.p); }),
                    mTravados.end());
    for (BossProjectile*& b : mBotoes) {
        if (!Vivo(b)) b = nullptr;
    }
    Sanear(mFrota, false);
    for (LinhaDeCredito& l : mCreditos) Sanear(l.blocos, false);
}

bool Ferreira::TrocouDeEtapa(const int etapa) {
    if (etapa == mEtapaAnterior) return false;
    DesmontarEtapa();
    mEtapaAnterior = etapa;
    return true;
}

void Ferreira::LimparClasses() {
    for (Player* a : Jogadores()) a->DefinirClasse(-1);
}

void Ferreira::DesmontarEtapa() {
    // Os tiros soltos seguem o caminho deles; o que a etapa montou sai com ela.
    mTiros.clear();
    MatarTodos(mBarra);
    while (!mJanelas.empty()) FecharJanela(mJanelas.size() - 1, false);
    mProximaJanela = 0.0f;
    mJanelasFeitas = 0;
    mCobrando = false;
    mPremioDasMoedas = 0.0f;
    LimparClasses();
    MatarTodos(mEspinhos);
    mArcos.clear();
    mProximoArremesso = 0.0f;
    mVoltaram.clear();
    mQueDaoAVolta.clear();
    for (AosSaltos& s : mAosSaltos) {
        // Fora do lag, o tiro volta a andar sozinho.
        if (Vivo(s.p)) Andar(s.p, s.v);
    }
    mAosSaltos.clear();
    mQuadroDoLag = -1;
    for (Descolado& d : mDescolados) {
        Matar(d.colisor);
        Matar(d.desenho);
    }
    mDescolados.clear();
    mSalvo = Salvo{};
    MatarTodos(mDisquetes);
    mTeAnterior = -1.0f;
    for (Travado& t : mTravados) {
        if (Vivo(t.p)) Andar(t.p, t.v);
    }
    mTravados.clear();
    mTravou = {};
    mTravadaAberta = -1;
    mEscolhaPendente = Lancamento::Escolha::Nenhuma;
    for (BossProjectile*& b : mBotoes) Matar(b);
    MatarTodos(mFrota);
    mFrotaMontada = false;
    mFrotaLimpa = false;
    mBugsDerrubados = 0;
    mProximaQueda = 0.0f;
    for (LinhaDeCredito& l : mCreditos) MatarTodos(l.blocos);
    mCreditos.clear();
    mLinhasFeitas = 0;
    mProximaLinha = 0.0f;
    Esconder(mEtiquetasDasJanelas);
    Esconder(mEtiquetasDoDialogo);
    Esconder(mEtiquetasDosCreditos);
    mProximoAnel = 0.0f;
    mProximoLeque = 0.0f;
}

void Ferreira::Desmontar() {
    DesmontarEtapa();
    if (mLinhas) mLinhas->Limpar();
    mAlvo.reset();
    mAvisoAte = -1.0f;
    mEtapaAnterior = -1;
    mClarao = 0.0f;
}

void Ferreira::AoTerminarABatalha() {
    Desmontar();
}

// ---- O desenho e o texto ----

void Ferreira::Linha(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Acrescentar(de, ate, cor);
}

void Ferreira::Tracejada(const Vector2 de, const Vector2 ate, const SDL_Color cor, const float traco) {
    const Vector2 d = ate - de;
    const float comprimento = d.Length();
    if (comprimento < 1.0f) return;
    const Vector2 u = d * (1.0f / comprimento);
    for (float s = 0.0f; s < comprimento; s += 2.0f * traco) {
        Linha(de + u * s, de + u * std::min(comprimento, s + traco), cor);
    }
}

void Ferreira::Circulo(const Vector2 centro, const float raio, const SDL_Color cor, const int lados) {
    for (int k = 0; k < lados; ++k) {
        const float a0 = Math::TwoPi * static_cast<float>(k) / static_cast<float>(lados);
        const float a1 = Math::TwoPi * static_cast<float>(k + 1) / static_cast<float>(lados);
        Linha(centro + Vector2(Math::Cos(a0), Math::Sin(a0)) * raio, centro + Vector2(Math::Cos(a1), Math::Sin(a1)) * raio, cor);
    }
}

void Ferreira::Retangulo(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    const Vector2 cantos[4] = {de, Vector2(ate.x, de.y), ate, Vector2(de.x, ate.y)};
    for (int k = 0; k < 4; ++k) Linha(cantos[k], cantos[(k + 1) % 4], cor);
}

void Ferreira::Preencher(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Preencher(de, ate, cor);
}

void Ferreira::Seta(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    const Vector2 d = ate - de;
    const float comprimento = d.Length();
    if (comprimento < 1.0f) return;
    const Vector2 u = d * (1.0f / comprimento), lado(-u.y, u.x);
    Linha(de, ate, cor);
    Linha(ate, ate - u * 8.0f + lado * 5.0f, cor);
    Linha(ate, ate - u * 8.0f - lado * 5.0f, cor);
}

EtiquetaDeCodigo* Ferreira::Etiqueta(std::vector<EtiquetaDeCodigo*>& grupo, const size_t k, const int largura, const int altura) {
    if (!mScene) return nullptr;
    while (grupo.size() <= k) {
        auto nova = std::make_unique<EtiquetaDeCodigo>(mScene, " ", largura, altura, kOrdemDasEtiquetas);
        nova->Pintar(SDL_Color{235, 240, 255, 120}, SDL_Color{10, 12, 24, 215}, SDL_Color{240, 244, 255, 255});
        nova->Mostrar(false);
        grupo.push_back(nova.get());
        mScene->AddActor(std::move(nova));
    }
    return grupo[k];
}

void Ferreira::Escrever(EtiquetaDeCodigo* e, const std::string& texto, const Vector2 onde) {
    if (!e) return;
    e->Escrever(texto);
    e->SetPosition(onde);
    e->Mostrar(true);
}

void Ferreira::Esconder(std::vector<EtiquetaDeCodigo*>& grupo) {
    for (EtiquetaDeCodigo* e : grupo) {
        if (e) e->Mostrar(false);
    }
}

void Ferreira::PlacarDaEtapa(const std::string& fase, const size_t etapa, const std::string& texto) {
    auto etapas = EtapasDa(fase);
    if (etapa >= etapas.size() || etapas[etapa].second == texto) return;
    etapas[etapa].second = texto;
    RegistrarEtapas(fase, etapas);
}

void Ferreira::Avisar(const std::string& texto, const float duracao) {
    mAviso = texto;
    mAvisoAte = mRelogio + duracao;
}

void Ferreira::Placar(const std::string& fase, const size_t etapa, const std::string& texto) {
    const std::string& vale = mRelogio < mAvisoAte ? mAviso : texto;
    if (!vale.empty()) PlacarDaEtapa(fase, etapa, vale);
}

std::optional<Vector2> Ferreira::AlvoDoMovimento() const {
    return mAlvo;
}

void Ferreira::Varrer(const float t, const float amplitude, const float periodo) {
    const SDL_FRect c = Campo();
    mAlvo = Vector2(c.x + c.w * (0.5f + amplitude * Math::Sin(Math::TwoPi * t / periodo)), c.y + 62.0f);
}

void Ferreira::OnUpdate(const float deltaTime) {
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
        Desmontar();
        mFase = fase;
    }
    if (!mLinhas && mScene) {
        auto ator = std::make_unique<Actor>(mScene);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();

    if (fase == HudHostil::kFase) AtualizarHud(t, deltaTime);
    else if (fase == RegrasTrocadas::kFase) AtualizarRegras(t, deltaTime);
    else if (fase == MotorQuebrado::kFase) AtualizarMotor(t, deltaTime);
    else if (fase == Lancamento::kFase) AtualizarLancamento(t, deltaTime);

    // O clarao de um instante, por cima do campo.
    if (mClarao > 0.0f) {
        const SDL_FRect c = Campo();
        SDL_Color cor = mCorDoClarao;
        cor.a = static_cast<Uint8>(150.0f * std::clamp(mClarao / 0.25f, 0.0f, 1.0f));
        Preencher(Vector2(c.x, c.y), Vector2(c.x + c.w, c.y + c.h), cor);
        mClarao -= deltaTime;
    }
}

// ---- Fase 1: a interface ----

void Ferreira::AtualizarHud(const float t, const float deltaTime) {
    using namespace HudHostil;
    (void)deltaTime;
    const Degrau& g = mDegrauDoHud;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    TrocouDeEtapa(etapa);
    Varrer(t, 0.3f, 9.0f);

    if (etapa >= 2) {
        AtualizarJanelas(te);
        return;
    }
    AtualizarBarra(te, etapa == 1);
    if (te >= kAvisoDaBarra && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        Leque(kGatilhoDoLeque, g.tirosDoLeque, 26.0f, g.velocidade, "Tiro");
    }
    const std::string cheia = Inteiro(std::clamp(Nota(), 0.0f, 100.0f)) + "% cheia: passe pelo VAZIO";
    Placar(kFase, static_cast<size_t>(etapa), (etapa == 0 ? "A barra \xc3\xa9 a sua nota, " : "Agora ela vem de lado, ") + cheia);
}

void Ferreira::AtualizarBarra(const float te, const bool emPe) {
    using namespace HudHostil;
    const SDL_FRect c = Campo();
    const float nota = Nota();
    const Varredura v = NaVarredura(te, mDegrauDoHud.idaDaBarra);
    const float meio = kSegmento / 2.0f;

    // Deitada, ela tem a largura do campo e sobe do rodape (de cima da barra de verdade) ate sumir
    // no alto; de pe, tem a altura dele e vai de fora da esquerda ate fora da direita. Nas pontas
    // ela descansa fora do campo: ninguem e pego pela virada.
    const float comprimento = emPe ? c.h : c.w;
    const int n = std::max(1, static_cast<int>(comprimento / kSegmento));
    const float sobra = (comprimento - static_cast<float>(n) * kSegmento) / 2.0f;
    if (static_cast<int>(mBarra.size()) != n) {
        MatarTodos(mBarra);
        mBarra.assign(static_cast<size_t>(n), nullptr);
    }
    const float deitadaEm = Mistura(c.y + c.h + 27.0f, c.y - meio, v.onde);
    const float emPeEm = Mistura(c.x - meio, c.x + c.w + meio, v.onde);
    const bool aviso = te < kAvisoDaBarra;
    const std::string cara = CaraDaBarra(nota);
    SDL_Color cor = cara == "Vermelha" ? SDL_Color{235, 70, 70, 150} : (cara == "Amarela" ? SDL_Color{240, 200, 60, 150} : SDL_Color{110, 220, 130, 150});

    const int cheios = Cheios(nota, n);
    for (int k = 0; k < n; ++k) {
        // Deitada, o segmento 0 e o da esquerda (como no rodape); de pe, o de baixo.
        const float ao = sobra + (static_cast<float>(k) + 0.5f) * kSegmento;
        const Vector2 onde = emPe ? Vector2(emPeEm, c.y + c.h - ao) : Vector2(c.x + ao, deitadaEm);
        BossProjectile*& p = mBarra[static_cast<size_t>(k)];
        const bool cheio = Cheio(k, n, nota, v.espelhada);
        if (cheio && !aviso) {
            if (!p) p = Peca(kGatilhoDaBarra, onde, cara);
            if (p) {
                p->SetPosition(onde);
                Parar(p);
                Cara(p, cara);
            }
            continue;
        }
        Matar(p);
        // No aviso, o pedaco cheio pisca onde a barra esta (deitada, em cima da do rodape).
        if (cheio && !emPe && Aceso(te, 0.14f)) Preencher(onde - Vector2(meio, meio - 3.0f), onde + Vector2(meio, meio - 3.0f), cor);
    }

    // O guia: a faixa por onde se passa, do tamanho do pedaco vazio, e a linha onde o cheio acaba.
    const float vazio = static_cast<float>(n - cheios) * kSegmento;
    const float cheioAte = static_cast<float>(cheios) * kSegmento;
    if (emPe) {
        // De pe, cheia de baixo para cima: o vazio e em cima (espelhada, embaixo).
        const float y0 = v.espelhada ? c.y + c.h - sobra - vazio : c.y + sobra;
        const float y1 = y0 + vazio;
        if (cheios < n) Preencher(Vector2(c.x, y0), Vector2(c.x + c.w, y1), kSeguro);
        const float corte = v.espelhada ? y0 : y1;
        Tracejada(Vector2(c.x, corte), Vector2(c.x + c.w, corte), kGuia, 10.0f);
        if (cheios < n) {
            // A seta do lado de onde a barra vem.
            const bool daEsquerda = v.passada % 2 == 0;
            const float x = daEsquerda ? c.x + 16.0f : c.x + c.w - 16.0f;
            Seta(Vector2(x, (y0 + y1) / 2.0f), Vector2(x + (daEsquerda ? 34.0f : -34.0f), (y0 + y1) / 2.0f), kGuia);
        }
        if (!v.parada || aviso) Retangulo(Vector2(emPeEm - meio + 3.0f, c.y + sobra), Vector2(emPeEm + meio - 3.0f, c.y + c.h - sobra), kVazio);
    } else {
        const float x0 = v.espelhada ? c.x + sobra : c.x + sobra + cheioAte;
        const float x1 = x0 + vazio;
        if (cheios < n) Preencher(Vector2(x0, c.y), Vector2(x1, c.y + c.h), kSeguro);
        const float corte = v.espelhada ? x1 : x0;
        Tracejada(Vector2(corte, c.y), Vector2(corte, c.y + c.h), kGuia, 10.0f);
        if (!v.parada || aviso) Retangulo(Vector2(c.x + sobra, deitadaEm - meio + 3.0f), Vector2(c.x + c.w - sobra, deitadaEm + meio - 3.0f), kVazio);
    }
}

void Ferreira::AtualizarJanelas(const float te) {
    using namespace HudHostil;
    const Degrau& g = mDegrauDoHud;
    const SDL_FRect c = Campo();

    // Uma janela nova, em volta de um aluno (na dupla, um de cada vez). Com todas abertas, a mais
    // velha fecha sozinha - e estoura.
    if (te >= 0.6f && te >= mProximaJanela) {
        mProximaJanela = te + g.entreJanelas;
        if (static_cast<int>(mJanelas.size()) >= kJanelasAbertas) FecharJanela(0, true);
        const std::vector<Player*> alunos = Jogadores();
        Vector2 aluno = PosicaoDoAluno();
        if (!alunos.empty()) aluno = alunos[static_cast<size_t>(mJanelasFeitas) % alunos.size()]->GetPosition();
        Janela j;
        j.centro = CentroDaJanela(aluno, c.x, c.y + 170.0f, c.x + c.w, c.y + c.h);
        for (int tentativa = 0; tentativa < 6; ++tentativa) {
            bool emCima = false;
            for (const Janela& outra : mJanelas) emCima = emCima || (outra.centro - j.centro).Length() < 70.0f;
            if (!emCima) break;
            const float lado = (tentativa % 2 == 0 ? 1.0f : -1.0f) * kLarguraDaJanela * 0.6f * static_cast<float>(tentativa / 2 + 1);
            j.centro = CentroDaJanela(aluno + Vector2(lado, -30.0f * static_cast<float>(tentativa / 2 + 1)), c.x, c.y + 170.0f,
                                      c.x + c.w, c.y + c.h);
        }
        j.abre = te + kAvisoDaJanela;
        j.dica = mJanelasFeitas % static_cast<int>(kDicas.size());
        for (size_t livre = 0; livre <= mJanelas.size(); ++livre) {
            bool usada = false;
            for (const Janela& outra : mJanelas) usada = usada || outra.etiqueta == livre;
            if (!usada) {
                j.etiqueta = livre;
                break;
            }
        }
        ++mJanelasFeitas;
        mJanelas.push_back(j);
    }

    const Vector2 meia(kLarguraDaJanela / 2.0f, kAlturaDaJanela / 2.0f);
    for (size_t k = 0; k < mJanelas.size();) {
        Janela& j = mJanelas[k];
        if (!j.aberta) {
            // O aviso: o contorno de onde ela vai abrir.
            if (Aceso(te, 0.1f)) {
                const Vector2 a = j.centro - meia, b = j.centro + meia;
                Tracejada(a, Vector2(b.x, a.y), kPerigo);
                Tracejada(Vector2(b.x, a.y), b, kPerigo);
                Tracejada(b, Vector2(a.x, b.y), kPerigo);
                Tracejada(Vector2(a.x, b.y), a, kPerigo);
            }
            if (te >= j.abre) {
                j.aberta = true;
                for (const Vector2& lugar : Moldura(j.centro)) {
                    const bool titulo = lugar.y < j.centro.y - meia.y + 1.0f;
                    if (BossProjectile* peca = Peca(kGatilhoDaJanela, lugar, titulo ? "Titulo" : "Borda")) j.pecas.push_back(peca);
                }
                j.fechar = Peca(kGatilhoDeFechar, BotaoDeFechar(j.centro), "X");
                Escrever(Etiqueta(mEtiquetasDasJanelas, j.etiqueta, 204, 24), kDicas[static_cast<size_t>(j.dica)],
                         j.centro + Vector2(0.0f, -meia.y + 28.0f));
                SomDoDisparo();
            }
            ++k;
            continue;
        }
        // Aberta e sem o X (ele morreu por outro caminho): fecha.
        if (!j.fechar) {
            FecharJanela(k, false);
            continue;
        }
        Preencher(j.centro - meia, j.centro + meia, kJanelaPorDentro);
        ++k;
    }

    if (te >= 1.0f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques * 1.15f;
        Leque(kGatilhoDoLeque, g.tirosDoLeque, 26.0f, g.velocidade, "Tiro");
    }
    Placar(kFase, 2, "Janelas abertas: " + std::to_string(mJanelas.size()) + ". Atire no X para fechar");
}

void Ferreira::FecharJanela(const size_t k, const bool comEstouro) {
    if (k >= mJanelas.size()) return;
    Janela j = mJanelas[k];
    mJanelas.erase(mJanelas.begin() + static_cast<std::ptrdiff_t>(k));
    MatarTodos(j.pecas);
    Matar(j.fechar);
    if (j.etiqueta < mEtiquetasDasJanelas.size() && mEtiquetasDasJanelas[j.etiqueta]) mEtiquetasDasJanelas[j.etiqueta]->Mostrar(false);
    if (comEstouro && j.aberta) {
        const float centro = mSorteio.Real(0.0f, 45.0f);
        const auto cacos = DispararEm(HudHostil::kGatilhoDoLeque, j.centro, 8, centro, 315.0f, 170.0f);
        DarVelocidade(cacos, centro, 315.0f, 170.0f);
        for (BossProjectile* p : cacos) Cara(p, "Tiro");
        SomDoDisparo();
    }
}

// ---- Fase 2: as regras ----

void Ferreira::AtualizarRegras(const float t, const float deltaTime) {
    using namespace RegrasTrocadas;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    TrocouDeEtapa(etapa);
    Varrer(t, 0.32f, 8.0f);
    if (etapa == 0) AtualizarInversao(te);
    else if (etapa == 1) AtualizarGravidade(te, deltaTime);
    else AtualizarFogoAmigo(te);
}

void Ferreira::AtualizarInversao(const float te) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const Regra r = RegraEm(te);
    const bool moeda = TiroEMoeda(r);
    const bool pisca = MoedaPisca(r, te);
    mCobrando = AcertoCobra(r);
    Battle* b = Batalha();
    const std::vector<Player*> alunos = Jogadores();

    // Invertido, o tiro e moeda: tem a classe do aluno (passa por ele) e paga a quem encosta.
    for (Player* a : alunos) a->DefinirClasse(moeda ? kClasseDaMoeda : -1);
    for (BossProjectile*& p : mTiros) {
        if (!p) continue;
        p->DefinirClasse(moeda ? kClasseDaMoeda : -1);
        Cara(p, moeda && !pisca ? "Moeda" : "Tiro");
        if (!moeda) continue;
        for (const Player* a : alunos) {
            if ((p->GetPosition() - a->GetPosition()).Length() > RaioDe(a) + kToqueDaMoeda + 8.0f) continue;
            if (b) b->Faiscar(p->GetPosition(), Hud::Faisca::Coleta);
            mPremioDasMoedas += kPremioDaMoeda;
            Matar(p);
            break;
        }
    }
    if (mPremioDasMoedas >= 0.9f) {
        if (b) b->Conquistar(mPremioDasMoedas);
        mPremioDasMoedas = 0.0f;
    }

    // O professor atira igual nas duas regras: o que muda e o que o tiro vale.
    if (te >= 0.6f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneis;
        for (BossProjectile* p : Anel(kGatilhoDoAnel, g.tirosDoAnel, g.velocidade, moeda ? "Moeda" : "Tiro")) {
            p->DefinirClasse(moeda ? kClasseDaMoeda : -1);
            mTiros.push_back(p);
        }
    }
    if (te >= 1.0f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, g.tirosDoLeque, 40.0f, g.velocidade + 30.0f, moeda ? "Moeda" : "Tiro")) {
            p->DefinirClasse(moeda ? kClasseDaMoeda : -1);
            mTiros.push_back(p);
        }
    }

    // Sem texto: o professor fica vermelho, com um sinal de menos (acertar custa), e o aluno ganha
    // um anel dourado com um mais (pegar paga). No aviso, os dois piscam.
    if (mCobrando || (r == Regra::Aviso && Aceso(te, 0.15f))) {
        const Vector2 eu = GetPosition();
        Circulo(eu, 76.0f + 3.0f * Math::Sin(mRelogio * 9.0f), kPerigo, 24);
        Preencher(eu + Vector2(-24.0f, 80.0f), eu + Vector2(24.0f, 88.0f), kPerigo);
        for (const Player* a : alunos) {
            const Vector2 onde = a->GetPosition();
            Circulo(onde, 26.0f + 2.0f * Math::Sin(mRelogio * 9.0f), kOuro, 16);
            Preencher(onde + Vector2(-9.0f, -44.0f), onde + Vector2(9.0f, -40.0f), kOuro);
            Preencher(onde + Vector2(-2.0f, -51.0f), onde + Vector2(2.0f, -33.0f), kOuro);
        }
    }
    switch (r) {
        case Regra::Aviso: Placar(kFase, 0, "regras.inverter() em 3, 2, 1..."); break;
        case Regra::Invertida: Placar(kFase, 0, "INVERTIDO: pegue as moedas e N\xc3\x83O atire"); break;
        case Regra::Voltando: Placar(kFase, 0, "regras.restaurar(): as moedas viram TIROS!"); break;
        case Regra::Normal: Placar(kFase, 0, "Regras de sempre: atire e desvie"); break;
    }
}

void Ferreira::AtualizarGravidade(const float te, const float deltaTime) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const SDL_FRect c = Campo();
    const bool ligada = te >= kAvisoDaGravidade;

    // O aluno cai; o chao dele e ate onde ele consegue descer (Player::PrenderNoCampo).
    float chao = c.y + c.h - 48.0f;
    for (Player* a : Jogadores()) {
        if (const auto* desenho = a->GetComponent<DrawAnimatedComponent>()) chao = c.y + c.h - static_cast<float>(desenho->GetSpriteHeight());
        if (ligada) a->SetPosition(a->GetPosition() + Vector2(0.0f, kQueda * deltaTime));
    }

    // Os espinhos no chao: o colisor do aluno e um ponto no meio do desenho, entao eles ficam a
    // poucos pixels de onde esse ponto para quando o aluno encosta embaixo - quem pousa, espeta.
    const float espinhosEm = chao + 5.0f;
    if (ligada && mEspinhos.empty()) {
        for (float x = c.x + 10.0f; x <= c.x + c.w - 10.0f; x += 20.0f) {
            if (BossProjectile* p = Peca(kGatilhoDoEspinho, Vector2(x, espinhosEm), "Espinho")) mEspinhos.push_back(p);
        }
        SomDoDisparo();
    }
    if (!ligada && Aceso(te, 0.15f)) Tracejada(Vector2(c.x, espinhosEm), Vector2(c.x + c.w, espinhosEm), kPerigo, 10.0f);

    // Os arremessos: em arco, para cair em volta de onde o aluno esta.
    if (ligada && te >= mProximoArremesso) {
        mProximoArremesso = te + g.entreArremessos;
        const Vector2 de = GetPosition() + Vector2(0.0f, 30.0f);
        const Vector2 aluno = PosicaoDoAluno();
        for (int k = 0; k < g.arremessos; ++k) {
            const float lado = (static_cast<float>(k) - static_cast<float>(g.arremessos - 1) / 2.0f) * 120.0f;
            const Vector2 alvo(std::clamp(aluno.x + lado + mSorteio.Real(-30.0f, 30.0f), c.x + 20.0f, c.x + c.w - 20.0f), aluno.y);
            const Vector2 v = Lancar(de, alvo, kVooDoArremesso, kGravidade);
            if (BossProjectile* p = Um(kGatilhoDoArremesso, de, "Tiro", v)) mArcos.push_back({p, v});
        }
        SomDoDisparo();
    }
    for (Arco& a : mArcos) {
        a.v.y += kGravidade * deltaTime;
        Andar(a.p, a.v);
    }

    // As setas que descem: a gravidade, a vista.
    for (int k = 0; k < 6; ++k) {
        const float x = c.x + c.w * (static_cast<float>(k) + 0.5f) / 6.0f;
        const float y = c.y + 140.0f + std::fmod(mRelogio * 170.0f + static_cast<float>(k) * 97.0f, c.h - 230.0f);
        Seta(Vector2(x, y), Vector2(x, y + 26.0f), ligada ? kGuia : kVazio);
    }
    Placar(kFase, 1, ligada ? "gravidade = 9.8: segure para CIMA, o ch\xc3\xa3o espeta" : "fisica.gravidade = 9.8 em 3, 2, 1...");
}

void Ferreira::AtualizarFogoAmigo(const float te) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const SDL_FRect c = Campo();
    const bool ligado = te >= kAvisoDoFogoAmigo;
    const Battle* b = Batalha();
    ProjectileManager* gerente = b ? b->GerenteDeProjeteis() : nullptr;

    // O tiro do aluno que sai por cima (o que errou o professor) volta por baixo, uma vez.
    if (ligado && gerente) {
        for (const auto& p : gerente->GetPlayerProjectiles()) {
            if (!p || p->GetState() != ActorState::Active) continue;
            const Vector2 onde = p->GetPosition();
            if (!SaiuPorCima(onde.y, c.y) || !mVoltaram.insert(p->Id()).second) continue;
            Um(kGatilhoDoRetorno, OndeVolta(onde.x, c.y + c.h), "Seu", Vector2(0.0f, -kVelocidadeDoRetorno));
        }
    }

    // Os leques do professor tambem dao a volta: o que sai por baixo reaparece em cima, uma vez.
    if (ligado && te >= mProximoLeque) {
        mProximoLeque = te + g.entreTiros;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, 3, 30.0f, g.velocidade, "Tiro")) {
            p->DefinirMargemDeSaida(2.0f, 1.0f);
            mQueDaoAVolta.push_back({p, false});
        }
    }
    for (QueDaAVolta& q : mQueDaoAVolta) {
        const Vector2 onde = q.p->GetPosition();
        if (!q.voltou && SaiuPorBaixo(onde.y, c.y + c.h)) {
            q.voltou = true;
            q.p->SetPosition(Vector2(onde.x, c.y - 8.0f));
        } else if (!NoCampo(onde, 40.0f)) {
            Matar(q.p);
        }
    }
    mQueDaoAVolta.erase(std::remove_if(mQueDaoAVolta.begin(), mQueDaoAVolta.end(), [](const QueDaAVolta& q) { return !q.p; }),
                        mQueDaoAVolta.end());

    // As duas bordas ligadas: o que sai por uma entra pela outra.
    const SDL_Color cor = ligado ? kPerigo : kVazio;
    Tracejada(Vector2(c.x, c.y + 4.0f), Vector2(c.x + c.w, c.y + 4.0f), cor, 10.0f);
    Tracejada(Vector2(c.x, c.y + c.h - 4.0f), Vector2(c.x + c.w, c.y + c.h - 4.0f), cor, 10.0f);
    for (int k = 0; k < 8; ++k) {
        const float x = c.x + c.w * (static_cast<float>(k) + 0.5f) / 8.0f;
        Seta(Vector2(x, c.y + 28.0f), Vector2(x, c.y + 8.0f), cor);
        Seta(Vector2(x, c.y + c.h - 6.0f), Vector2(x, c.y + c.h - 26.0f), cor);
    }
    Placar(kFase, 2, ligado ? "fogoAmigo = true: tiro que ERRA volta por baixo" : "fogoAmigo = true em 3, 2, 1...");
}

// ---- Fase 3: o motor ----

void Ferreira::AtualizarMotor(const float t, const float deltaTime) {
    using namespace MotorQuebrado;
    (void)deltaTime;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    TrocouDeEtapa(etapa);
    Varrer(t, 0.3f, 8.5f);
    if (etapa == 0) AtualizarLag(te);
    else if (etapa == 1) AtualizarHitbox(te);
    else AtualizarSaveLoad(te);
}

void Ferreira::AtualizarLag(const float te) {
    using namespace MotorQuebrado;
    const Degrau& g = mDegrauDoMotor;
    const SDL_FRect c = Campo();

    // O tiro novo entra na lista e para: quem o move, de quadro em quadro, e o lag.
    const auto entrar = [this](const std::vector<BossProjectile*>& novos) {
        for (BossProjectile* p : novos) {
            p->DefinirMargemDeSaida(2.0f, 1.0f);
            mAosSaltos.push_back({p, p->GetPosition(), VelocidadeDe(p)});
            Parar(p);
        }
    };
    if (te >= 0.8f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneis;
        entrar(Anel(kGatilhoDoAnel, g.tirosDoAnel, g.velocidade, "Tiro"));
    }
    if (te >= 1.4f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        entrar(Leque(kGatilhoDoTiro, g.tirosDoLeque, 28.0f, g.velocidade + 110.0f, "Tiro"));
    }

    const int quadro = QuadroEm(te);
    const bool saltou = quadro != mQuadroDoLag;
    mQuadroDoLag = quadro;
    for (AosSaltos& s : mAosSaltos) {
        if (saltou) s.onde = NoQuadro(s.onde, s.v, 1);
        if (!NoCampo(s.onde, 50.0f)) {
            Matar(s.p);
            continue;
        }
        s.p->SetPosition(s.onde);
        Parar(s.p);
        // O proximo quadro: o circulo, e o risco ate ele.
        const Vector2 proximo = NoQuadro(s.onde, s.v, 1);
        Circulo(proximo, 9.0f, kFantasma, 8);
        Linha(s.onde, proximo, kFantasmaFraco);
    }
    mAosSaltos.erase(std::remove_if(mAosSaltos.begin(), mAosSaltos.end(), [](const AosSaltos& s) { return !s.p; }),
                     mAosSaltos.end());

    // O medidor do quadro, no alto: enche ate o proximo salto.
    const float u = std::fmod(te, kQuadroDoLag) / kQuadroDoLag;
    const Vector2 a(c.x + c.w * 0.4f, c.y + 134.0f), z(c.x + c.w * 0.6f, c.y + 142.0f);
    Retangulo(a, z, kFantasma);
    Preencher(a, Vector2(Mistura(a.x, z.x, u), z.y), kFantasma);
    Placar(kFase, 0, "fps = 2.5: o c\xc3\xadrculo \xc3\xa9 o PR\xc3\x93XIMO quadro");
}

void Ferreira::AtualizarHitbox(const float te) {
    using namespace MotorQuebrado;
    const Degrau& g = mDegrauDoMotor;

    // O desenho vira enfeite, e a colisao vai para um tiro sem cara, que o professor conduz.
    const auto entrar = [this](const std::vector<BossProjectile*>& novos) {
        for (BossProjectile* desenho : novos) {
            desenho->DefinirEnfeite(true);
            BossProjectile* colisor = Peca(kGatilhoDoColisor, desenho->GetPosition(), "Vazio");
            mDescolados.push_back({desenho, colisor});
        }
    };
    if (te >= 0.8f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneis;
        entrar(Anel(kGatilhoDoSprite, g.tirosDoAnel, g.velocidade, "Tiro"));
    }
    if (te >= 1.4f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        entrar(Leque(kGatilhoDoSprite, g.tirosDoLeque, 28.0f, g.velocidade + 60.0f, "Tiro"));
    }

    const Vector2 fora = Descolamento(te);
    for (Descolado& d : mDescolados) {
        if (!d.desenho) {
            Matar(d.colisor);
            continue;
        }
        if (!d.colisor) continue;
        const Vector2 desenho = d.desenho->GetPosition();
        const Vector2 onde = desenho + fora;
        d.colisor->SetPosition(onde);
        Parar(d.colisor);
        Circulo(onde, 9.0f, kDebug, 10);
        Linha(desenho, onde, kDebugFraco);
    }
    mDescolados.erase(std::remove_if(mDescolados.begin(), mDescolados.end(),
                                     [](const Descolado& d) { return !d.desenho && !d.colisor; }), mDescolados.end());

    // O modo debug mostra todo colisor: o do aluno e o do professor tambem.
    for (const Player* a : Jogadores()) Circulo(a->GetPosition(), RaioDe(a) + 1.0f, kDebug, 10);
    Circulo(GetPosition(), 56.0f, kDebugFraco, 20);
    Placar(kFase, 1, "debug: a hitbox DESCOLOU. Desvie do c\xc3\xadrculo verde");
}

void Ferreira::AtualizarSaveLoad(const float te) {
    using namespace MotorQuebrado;
    const Degrau& g = mDegrauDoMotor;
    const SDL_FRect c = Campo();
    Battle* b = Batalha();

    if (te >= 0.5f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneis;
        for (BossProjectile* p : Anel(kGatilhoDoAnel, g.tirosDoAnel, g.velocidade, "Tiro")) mTiros.push_back(p);
    }
    if (te >= 0.9f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, g.tirosDoLeque, 28.0f, g.velocidade + 70.0f, "Tiro")) mTiros.push_back(p);
    }

    const Evento evento = EventoEntre(mTeAnterior, te);
    mTeAnterior = te;
    const std::vector<Player*> alunos = Jogadores();
    if (evento == Evento::Salvar) {
        // O save: onde cada aluno esta, cada tiro em campo e a nota.
        mSalvo = Salvo{};
        mSalvo.tem = true;
        mSalvo.nota = Nota();
        for (const Player* a : alunos) mSalvo.alunos.push_back(a->GetPosition());
        for (const BossProjectile* p : mTiros) {
            if (p) mSalvo.tiros.push_back({p->GetPosition(), VelocidadeDe(p)});
        }
        MatarTodos(mDisquetes);
        for (const Vector2& onde : mSalvo.alunos) {
            if (BossProjectile* d = Peca(kGatilhoDoDisquete, onde + Vector2(0.0f, -46.0f), "Disquete")) {
                d->DefinirEnfeite(true);
                mDisquetes.push_back(d);
            }
        }
        mClarao = 0.2f;
        mCorDoClarao = kSave;
        Avisar("quicksave(): salvo com nota " + Inteiro(mSalvo.nota), 1.5f);
        SomDoDisparo();
    } else if (evento == Evento::Carregar && mSalvo.tem) {
        // O load: tudo volta ao que era - o aluno, os tiros e a nota.
        for (size_t i = 0; i < alunos.size() && i < mSalvo.alunos.size(); ++i) {
            alunos[i]->SetPosition(mSalvo.alunos[i]);
            alunos[i]->Proteger(0.4f);
        }
        if (b) b->DefinirNota(mSalvo.nota);
        MatarTodos(mTiros);
        for (const Salvo::Tiro& s : mSalvo.tiros) {
            if (BossProjectile* p = Um(kGatilhoDoTiro, s.onde, "Tiro", s.v)) mTiros.push_back(p);
        }
        MatarTodos(mDisquetes);
        mSalvo.tem = false;
        mClarao = 0.25f;
        mCorDoClarao = SDL_Color{255, 255, 255, 255};
        Avisar("quickload(): tudo voltou, a nota tamb\xc3\xa9m", 1.6f);
        SomDoDisparo();
    }

    if (mSalvo.tem) {
        // O ponto do save, e a barra que enche ate a hora do load.
        for (size_t i = 0; i < mSalvo.alunos.size(); ++i) {
            const Vector2 onde = mSalvo.alunos[i];
            Circulo(onde, 22.0f, kSave, 14);
            Tracejada(onde + Vector2(-16.0f, 0.0f), onde + Vector2(16.0f, 0.0f), kSave, 4.0f);
            Tracejada(onde + Vector2(0.0f, -16.0f), onde + Vector2(0.0f, 16.0f), kSave, 4.0f);
            if (i < alunos.size()) Tracejada(alunos[i]->GetPosition(), onde, SDL_Color{110, 170, 255, 70}, 6.0f);
        }
        const float u = RumoAoLoad(te);
        if (u >= 0.0f) {
            const Vector2 a(c.x + c.w * 0.3f, c.y + 134.0f), z(c.x + c.w * 0.7f, c.y + 144.0f);
            Retangulo(a, z, kSave);
            Preencher(a, Vector2(Mistura(a.x, z.x, u), z.y), kSave);
        }
        Placar(kFase, 2, "Salvo com nota " + Inteiro(mSalvo.nota) + ": o load desfaz TUDO desde o save");
    } else {
        Placar(kFase, 2, "quicksave() a caminho: o jogo vai voltar no tempo");
    }
}

// ---- A repescagem: o lancamento ----

void Ferreira::AtualizarLancamento(const float t, const float deltaTime) {
    using namespace Lancamento;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    TrocouDeEtapa(etapa);
    Varrer(t, 0.3f, 8.0f);
    if (etapa == 0) AtualizarCrash(te);
    else if (etapa == 1) AtualizarFrota(te);
    else AtualizarCreditos(te, deltaTime);
}

void Ferreira::Travar(const int k) {
    using namespace Lancamento;
    mTravou[static_cast<size_t>(k)] = true;
    mTravadaAberta = k;
    mEscolhaPendente = Escolha::Nenhuma;
    mTravados.clear();
    for (BossProjectile* p : mTiros) {
        if (!p) continue;
        mTravados.push_back({p, VelocidadeDe(p)});
        Parar(p);
    }
    // Os dois botoes, embaixo da mensagem: ESPERAR troca de lado de uma travada para a outra.
    const SDL_FRect c = Campo();
    const Vector2 meio(c.x + c.w / 2.0f, c.y + c.h * 0.42f);
    const Vector2 esquerda = meio + Vector2(-110.0f, 44.0f), direita = meio + Vector2(110.0f, 44.0f);
    const bool esperarAEsquerda = LadoDoEsperar(k) == 0;
    mBotoes[0] = Peca(kGatilhoDoBotao, esperarAEsquerda ? esquerda : direita, "Esperar");
    mBotoes[1] = Peca(kGatilhoDoBotao, esperarAEsquerda ? direita : esquerda, "Fechar");
    Escrever(Etiqueta(mEtiquetasDoDialogo, 0, 440, 26), "DPIBulletHell n\xc3\xa3o est\xc3\xa1 respondendo", meio + Vector2(0.0f, -28.0f));
    mClarao = 0.18f;
    mCorDoClarao = SDL_Color{255, 255, 255, 255};
    SomDoDisparo();
}

void Ferreira::Destravar(const Lancamento::Escolha escolha) {
    using namespace Lancamento;
    if (mTravadaAberta < 0) return;
    mTravadaAberta = -1;
    for (BossProjectile*& botao : mBotoes) Matar(botao);
    Esconder(mEtiquetasDoDialogo);
    Battle* b = Batalha();
    if (escolha == Escolha::Fechar) {
        // Fechar o programa: cada tiro congelado estilhaca em dois.
        for (const Travado& t : mTravados) {
            if (!Vivo(t.p)) continue;
            const float centro = mSorteio.Real(0.0f, 360.0f);
            const auto cacos = DispararEm(kGatilhoDoEstilhaco, t.p->GetPosition(), 2, centro, 180.0f, 210.0f);
            DarVelocidade(cacos, centro, 180.0f, 210.0f);
            for (BossProjectile* f : cacos) Cara(f, "Fragmento");
            t.p->Kill();
        }
        Avisar("Fechou o programa: os tiros estilha\xc3\xa7" "aram!", 1.8f);
        mClarao = 0.25f;
        mCorDoClarao = kSave;
    } else {
        const float fator = FatorDaVolta(escolha);
        for (const Travado& t : mTravados) {
            if (Vivo(t.p)) Andar(t.p, t.v * fator);
        }
        if (escolha == Escolha::Esperar) {
            if (b) b->Conquistar(kPremioPorEsperar);
            Avisar("Esperou: o jogo voltou de onde parou", 1.6f);
        } else {
            Avisar("Sem resposta: recuperando os quadros perdidos!", 1.8f);
        }
    }
    mTravados.clear();
    SomDoDisparo();
}

void Ferreira::AtualizarCrash(const float te) {
    using namespace Lancamento;
    const Degrau& g = mDegrauDoLancamento;
    const SDL_FRect c = Campo();

    const int k = TravadaEm(te);
    if (k >= 0 && !mTravou[static_cast<size_t>(k)]) Travar(k);
    if (mTravadaAberta >= 0 && mEscolhaPendente != Escolha::Nenhuma) Destravar(mEscolhaPendente);
    if (mTravadaAberta >= 0 && k != mTravadaAberta) Destravar(Escolha::Nenhuma);
    mEscolhaPendente = Escolha::Nenhuma;

    if (mTravadaAberta >= 0) {
        // Travado: nada anda, e o dialogo espera no meio do campo.
        for (const Travado& t : mTravados) Parar(t.p);
        const Vector2 meio(c.x + c.w / 2.0f, c.y + c.h * 0.42f);
        const Vector2 meia(250.0f, 86.0f);
        Preencher(meio - meia, meio + meia, SDL_Color{16, 18, 34, 235});
        Preencher(meio - meia, Vector2(meio.x + meia.x, meio.y - meia.y + 20.0f), SDL_Color{40, 70, 190, 235});
        Retangulo(meio - meia, meio + meia, kGuia);
        // O tempo que resta para escolher.
        const float resta = std::clamp(1.0f - (te - kTravadas[static_cast<size_t>(mTravadaAberta)]) / kEspera, 0.0f, 1.0f);
        Preencher(Vector2(meio.x - 200.0f, meio.y - 2.0f), Vector2(meio.x - 200.0f + 400.0f * resta, meio.y + 4.0f), kPerigo);
        Placar(kFase, 0, "O jogo travou: atire em ESPERAR (ou em FECHAR...)");
        return;
    }

    // Solto: aneis cheios, que e o que vai congelar.
    if (te >= 0.4f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneis;
        for (BossProjectile* p : Anel(kGatilhoDoAnel, g.tirosDoAnel, g.velocidade, "Tiro")) mTiros.push_back(p);
    }
    if (te >= 0.9f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreAneis * 1.7f;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, 3, 26.0f, g.velocidade + 45.0f, "Tiro")) mTiros.push_back(p);
    }
    Placar(kFase, 0, "Build de lan\xc3\xa7" "amento: nada pode dar errado");
}

void Ferreira::AtualizarFrota(const float te) {
    using namespace Lancamento;
    const Degrau& g = mDegrauDoLancamento;
    const SDL_FRect c = Campo();
    Battle* b = Batalha();
    const Vector2 topo(c.x + c.w / 2.0f, c.y + 170.0f);
    const float folga = (c.w - static_cast<float>(kColunasDaFrota - 1) * kEspacoX) / 2.0f - 40.0f;
    const auto indice = [](const int fileira, const int coluna) { return static_cast<size_t>(fileira * kColunasDaFrota + coluna); };

    if (!mFrotaMontada) {
        // O aviso: os lugares da frota, piscando.
        if (Aceso(te, 0.12f)) {
            for (int f = 0; f < kFileiras; ++f) {
                for (int col = 0; col < kColunasDaFrota; ++col) Circulo(LugarNaFrota(f, col, topo, Marcha{}), 12.0f, kDebugFraco, 8);
            }
        }
        if (te >= kChegadaDaFrota) {
            mFrotaMontada = true;
            mFrota.assign(static_cast<size_t>(kFileiras * kColunasDaFrota), nullptr);
            for (int f = 0; f < kFileiras; ++f) {
                for (int col = 0; col < kColunasDaFrota; ++col) {
                    mFrota[indice(f, col)] = Peca(kGatilhoDoBug, LugarNaFrota(f, col, topo, Marcha{}), "Bug");
                }
            }
            SomDoDisparo();
        }
        Placar(kFase, 1, "genero = nave: os bugs do lan\xc3\xa7" "amento chegaram");
        return;
    }

    // A marcha: de um lado ao outro, e um degrau para baixo a cada batida na borda.
    const Marcha m = MarchaEm(static_cast<int>((te - kChegadaDaFrota) / g.entrePassos), folga);
    int vivos = 0;
    std::vector<size_t> deBaixo;
    for (int col = 0; col < kColunasDaFrota; ++col) {
        bool achou = false;
        for (int f = kFileiras - 1; f >= 0; --f) {
            BossProjectile* p = mFrota[indice(f, col)];
            if (!p) continue;
            ++vivos;
            p->SetPosition(LugarNaFrota(f, col, topo, m));
            Parar(p);
            if (!achou) deBaixo.push_back(indice(f, col));
            achou = true;
        }
    }
    // O bug de baixo de uma coluna solta um tiro.
    if (!deBaixo.empty() && te >= mProximaQueda) {
        mProximaQueda = te + g.entreQuedas;
        const size_t qual = deBaixo[static_cast<size_t>(mSorteio.Inteiro(0, static_cast<int>(deBaixo.size()) - 1))];
        Um(kGatilhoDaQueda, mFrota[qual]->GetPosition() + Vector2(0.0f, 18.0f), "Tiro", Vector2(0.0f, g.velocidadeDaQueda));
    }
    if (vivos == 0 && !mFrotaLimpa) {
        mFrotaLimpa = true;
        if (b) b->Conquistar(kPremioDaFrotaLimpa);
        Avisar("0 bugs conhecidos: pode lan\xc3\xa7" "ar!", 2.0f);
    }
    // Sem frota, o professor volta a atirar.
    if (mFrotaLimpa && te >= mProximoLeque) {
        mProximoLeque = te + 1.1f;
        Leque(kGatilhoDoTiro, 3, 26.0f, g.velocidade + 30.0f, "Tiro");
    }
    Placar(kFase, 1, "genero = nave: faltam " + std::to_string(vivos) + " bugs. Derrube todos!");
}

void Ferreira::AtualizarCreditos(const float te, const float deltaTime) {
    using namespace Lancamento;
    const Degrau& g = mDegrauDoLancamento;
    const SDL_FRect c = Campo();
    const int blocos = std::max(1, static_cast<int>(c.w / kBloco));
    const float sobra = (c.w - static_cast<float>(blocos) * kBloco) / 2.0f;
    const auto xDoBloco = [&](const int bloco) { return c.x + sobra + (static_cast<float>(bloco) + 0.5f) * kBloco; };

    // Uma linha nova sobe do rodape: uma parede de blocos com um vao, e o texto no meio.
    if (te >= 0.8f && te >= mProximaLinha && mLinhasFeitas < static_cast<int>(kCreditos.size())) {
        mProximaLinha = te + g.entreLinhas;
        LinhaDeCredito l;
        l.y = c.y + c.h + 26.0f;
        l.vao = VaoDaLinha(blocos, mSorteio);
        l.texto = mLinhasFeitas++;
        l.etiqueta = static_cast<size_t>(l.texto) % kEtiquetasDosCreditos;
        for (int bloco = 0; bloco < blocos; ++bloco) {
            if (NoVao(bloco, l.vao)) continue;
            if (BossProjectile* p = Peca(kGatilhoDoCredito, Vector2(xDoBloco(bloco), l.y), "Bloco")) {
                l.blocos.push_back(p);
                l.lugares.push_back(bloco);
            }
        }
        Escrever(Etiqueta(mEtiquetasDosCreditos, l.etiqueta, 388, 26), kCreditos[static_cast<size_t>(l.texto)],
                 Vector2(c.x + c.w / 2.0f, l.y));
        mCreditos.push_back(std::move(l));
    }

    for (LinhaDeCredito& l : mCreditos) {
        l.y -= g.subida * deltaTime;
        for (size_t i = 0; i < l.blocos.size(); ++i) {
            if (!l.blocos[i]) continue;
            l.blocos[i]->SetPosition(Vector2(xDoBloco(l.lugares[i]), l.y));
            Parar(l.blocos[i]);
        }
        if (l.etiqueta < mEtiquetasDosCreditos.size() && mEtiquetasDosCreditos[l.etiqueta]) {
            mEtiquetasDosCreditos[l.etiqueta]->SetPosition(Vector2(c.x + c.w / 2.0f, l.y));
            mEtiquetasDosCreditos[l.etiqueta]->Mostrar(l.y > c.y + 20.0f);
        }
        // O vao, marcado.
        const float x0 = c.x + sobra + static_cast<float>(l.vao) * kBloco;
        Preencher(Vector2(x0, l.y - 12.0f), Vector2(x0 + static_cast<float>(kVaoDosCreditos) * kBloco, l.y + 12.0f), kSeguro);
    }
    // A linha que passou do alto sai.
    while (!mCreditos.empty() && mCreditos.front().y < c.y - 20.0f) {
        const size_t etiqueta = mCreditos.front().etiqueta;
        if (etiqueta < mEtiquetasDosCreditos.size() && mEtiquetasDosCreditos[etiqueta]) mEtiquetasDosCreditos[etiqueta]->Mostrar(false);
        MatarTodos(mCreditos.front().blocos);
        mCreditos.erase(mCreditos.begin());
    }

    if (te >= 2.0f && te >= mProximoLeque) {
        mProximoLeque = te + 1.5f;
        Leque(kGatilhoDoTiro, 1, 0.0f, g.velocidade, "Tiro");
    }
    Placar(kFase, 2, "Cr\xc3\xa9" "ditos: passe pelos v\xc3\xa3os. Obrigado por jogar!");
}

// ---- O que o tiro do aluno faz ----

void Ferreira::OnDerrubou(BossProjectile* qual) {
    if (!qual) return;
    Battle* b = Batalha();
    // O X de uma janela: ela fecha inteira.
    for (size_t k = 0; k < mJanelas.size(); ++k) {
        if (mJanelas[k].fechar != qual) continue;
        mJanelas[k].fechar = nullptr;
        FecharJanela(k, false);
        if (b) b->Conquistar(HudHostil::kPremioPorFechar);
        return;
    }
    // Um botao do dialogo: a escolha vale no quadro seguinte (AtualizarCrash).
    if (mTravadaAberta >= 0) {
        if (qual == mBotoes[0]) {
            mBotoes[0] = nullptr;
            mEscolhaPendente = Lancamento::Escolha::Esperar;
            return;
        }
        if (qual == mBotoes[1]) {
            mBotoes[1] = nullptr;
            mEscolhaPendente = Lancamento::Escolha::Fechar;
            return;
        }
    }
    // Um bug da frota.
    for (BossProjectile*& bug : mFrota) {
        if (bug != qual) continue;
        bug = nullptr;
        ++mBugsDerrubados;
        if (b) b->Conquistar(Lancamento::kPremioPorBug);
        return;
    }
}

void Ferreira::AoSerPerguntado() {
    // Nas regras invertidas, o acerto no professor cobra.
    if (!mCobrando) return;
    if (Battle* b = Batalha()) {
        b->Cobrar(RegrasTrocadas::kMultaPorAcerto);
        b->Faiscar(GetPosition() + Vector2(0.0f, 70.0f), Hud::Faisca::Dano);
    }
}
