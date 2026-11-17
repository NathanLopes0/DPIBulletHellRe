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
#include "../../../Regras/Nota.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/FaixaDaNota.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "../../Player/PlayerProjectile.h"
#include "BossesProjectiles/BossProjectile.h"
#include "PainelDeCodigo.h"

namespace {

    using Plataforma::Lado;

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

    std::string NomeDoLado(const Lado l) {
        switch (l) {
            case Lado::Baixo:    return "CH\xc3\x83O";
            case Lado::Esquerda: return "ESQUERDA";
            case Lado::Cima:     return "TETO";
            case Lado::Direita:  return "DIREITA";
        }
        return "";
    }

    /// O que o placar diz num lado: para onde a gravidade puxa, e o que fazer nele.
    std::string AnuncioDoLado(const Lado l) {
        switch (Gravidade::DesafioDo(l)) {
            case Gravidade::Desafio::Pulos:   return "gravidade = CH\xc3\x83O: PULE as caixas (segure para ir alto)";
            case Gravidade::Desafio::Lancas:  return "gravidade = ESQUERDA: lan\xc3\xa7" "a de baixo, PULE; de cima, FIQUE";
            case Gravidade::Desafio::Chuva:   return "gravidade = TETO: fuja dos blocos e PULE as fagulhas";
            case Gravidade::Desafio::Esteira: return "gravidade = DIREITA: ande CONTRA a esteira e pule os espinhos";
        }
        return "";
    }
}

Ferreira::Ferreira(Scene* scene) : Boss(scene) {
}

void Ferreira::SortearALuta() {
    mLutaSorteada = true;
    mSorteio = SorteioDaFase(HudHostil::kFase);
    const Dificuldade::Nivel n = NivelDeDificuldade();
    mDegrauDoHud = HudHostil::DegrauDo(n);
    mDegrauDaGravidade = Gravidade::DegrauDo(n);
    mDegrauDasRegras = RegrasTrocadas::DegrauDo(n);
    mDegrauDoLancamento = Lancamento::DegrauDo(n);

    // A ordem dos lados: o placar de cada etapa da fisica diz o lado que ela vai ter.
    mSorteioDaFisica = SorteioDaFase(Gravidade::kFase);
    mOrdem = Gravidade::Ordem(mSorteioDaFisica);
    auto etapas = EtapasDa(Gravidade::kFase);
    for (size_t k = 0; k < etapas.size() && k < mOrdem.size(); ++k) etapas[k].second = AnuncioDoLado(mOrdem[k]);
    if (!etapas.empty()) RegistrarEtapas(Gravidade::kFase, etapas);
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

BossProjectile* Ferreira::PecaAndando(const char* gatilho, const Vector2 onde, const std::string& cara, const Vector2 velocidade) {
    BossProjectile* p = Um(gatilho, onde, cara, velocidade);
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

JogosDigitais::Campo Ferreira::CampoPuro() const {
    const SDL_FRect c = Campo();
    return JogosDigitais::Campo{c.x, c.y, c.w, c.h};
}

FaixaDaNota* Ferreira::Faixa() const {
    const Battle* b = Batalha();
    return b ? b->Faixa() : nullptr;
}

void Ferreira::Sanear(std::vector<BossProjectile*>& ps, const bool encolher) {
    for (BossProjectile*& p : ps) {
        if (!Vivo(p)) p = nullptr;
    }
    if (encolher) ps.erase(std::remove(ps.begin(), ps.end(), nullptr), ps.end());
}

void Ferreira::Sanear() {
    // As listas soltas encolhem; as que tem lugar marcado (a barra, as pilhas, a frota, os botoes) so
    // esquecem.
    Sanear(mTiros, true);
    for (AnelVivo& a : mAneis) Sanear(a.tiros, true);
    Sanear(mBarra, false);
    for (Janela& j : mJanelas) {
        Sanear(j.pecas, true);
        if (!Vivo(j.fechar)) j.fechar = nullptr;
    }
    for (PilhaViva& p : mPilhas) Sanear(p.caixas, false);
    for (LancaViva& l : mLancas) {
        if (!Vivo(l.p)) l.p = nullptr;
    }
    for (QuedaViva& q : mQuedas) {
        if (!Vivo(q.p)) q.p = nullptr;
    }
    for (FagulhaViva& f : mFagulhas) {
        if (!Vivo(f.p)) f.p = nullptr;
    }
    for (BuracoVivo& b : mBuracos) Sanear(b.espinhos, false);
    Sanear(mMoedor, false);
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
    for (AnelVivo& a : mAneis) MatarTodos(a.tiros);
    mAneis.clear();
    mPoupado = {};
    DevolverABarra();
    while (!mJanelas.empty()) FecharJanela(mJanelas.size() - 1, false);
    mProximaJanela = 0.0f;
    mJanelasFeitas = 0;
    for (PilhaViva& p : mPilhas) MatarTodos(p.caixas);
    mPilhas.clear();
    mPilhasPorVir.clear();
    mProximaPilha = 0;
    for (LancaViva& l : mLancas) Matar(l.p);
    mLancas.clear();
    mLancasPorVir.clear();
    mProximaLanca = 0;
    for (QuedaViva& q : mQuedas) Matar(q.p);
    mQuedas.clear();
    mQuedasPorVir.clear();
    mProximaDaChuva = 0;
    for (FagulhaViva& f : mFagulhas) Matar(f.p);
    mFagulhas.clear();
    for (BuracoVivo& b : mBuracos) MatarTodos(b.espinhos);
    mBuracos.clear();
    mBuracosPorVir.clear();
    mProximoBuraco = 0;
    MatarTodos(mMoedor);
    mCobrando = false;
    mPremioDasMoedas = 0.0f;
    LimparClasses();
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
    // A gravidade e da fase inteira: so a troca de fase (ou o fim da luta) a desliga.
    SoltarAGravidade();
    mVirada = -1;
    if (mLinhas) mLinhas->Limpar();
    if (mLinhasDoChao) mLinhasDoChao->Limpar();
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
    if (mLinhas) mLinhas->Preencher(Vector2(std::min(de.x, ate.x), std::min(de.y, ate.y)),
                                    Vector2(std::max(de.x, ate.x), std::max(de.y, ate.y)), cor);
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

void Ferreira::SetaGrande(const Vector2 centro, const Vector2 d, const float tamanho, const SDL_Color cor) {
    const Vector2 lado(-d.y, d.x);
    const Vector2 de = centro - d * (tamanho / 2.0f), ate = centro + d * (tamanho / 2.0f);
    // A haste, grossa, e a ponta, cheia: linhas paralelas que se estreitam.
    for (int k = -4; k <= 4; ++k) Linha(de + lado * (1.5f * static_cast<float>(k)), ate - d * 30.0f + lado * (1.5f * static_cast<float>(k)), cor);
    for (int k = 0; k < 18; ++k) {
        const float largura = 36.0f * (1.0f - static_cast<float>(k) / 18.0f);
        const Vector2 base = ate - d * (40.0f - 2.2f * static_cast<float>(k));
        Linha(base - lado * largura, base + lado * largura, cor);
    }
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
        mLinhasDoChao = ator->AddComponent<DrawLinhasComponent>(kOrdemDoChao);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();
    if (mLinhasDoChao) mLinhasDoChao->Limpar();

    if (fase == HudHostil::kFase) AtualizarHud(t, deltaTime);
    else if (fase == Gravidade::kFase) AtualizarFisica(t, deltaTime);
    else if (fase == RegrasTrocadas::kFase) AtualizarRegras(t, deltaTime);
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

    if (etapa == kEtapaDaAbertura) {
        AtualizarAbertura(te);
        return;
    }
    if (etapa == kEtapaDasJanelas) {
        AtualizarJanelas(te);
        return;
    }
    AtualizarBarra(etapa, te);
    // Os leques so comecam com a barra no campo: o susto do rodape e so dela.
    const bool arrancada = etapa == kEtapaDaBarraDeitada && te < kTremorDaBarra + kArrancadaDaBarra;
    if (!arrancada && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        Leque(kGatilhoDoLeque, g.tirosDoLeque, 26.0f, g.velocidade, "Tiro");
    }
    const FaixaDaNota* faixa = Faixa();
    const float nota = std::clamp(faixa ? faixa->Mostrada() : Nota(), 0.0f, 100.0f);
    const std::string cheia = ::Nota::ECheia(nota) ? "100% cheia: N\xc3\x83O h\xc3\xa1 vazio!" : Inteiro(nota) + "% cheia: passe pelo VAZIO";
    if (arrancada) {
        Placar(kFase, static_cast<size_t>(etapa), "hud.nota.soltar(): a barra da sua nota saiu da HUD!");
    } else {
        Placar(kFase, static_cast<size_t>(etapa), (etapa == kEtapaDaBarraDeitada ? "A barra \xc3\xa9 a sua nota, " : "Agora ela vem de lado, ") + cheia);
    }
}

void Ferreira::AtualizarAbertura(const float te) {
    using namespace HudHostil;
    const auto aneis = AneisDaAbertura(NivelDeDificuldade());
    const SDL_FRect c = Campo();
    const Vector2 meioDoCampo(c.x + c.w / 2.0f, c.y + c.h / 2.0f);

    // Cada anel nasce em volta de onde o aluno esta agora - sem aviso: e o susto.
    while (mAneisFeitos < kAneisDaAbertura && te >= aneis[static_cast<size_t>(mAneisFeitos)].quando) {
        const AnelDaAbertura& a = aneis[static_cast<size_t>(mAneisFeitos)];
        const Vector2 centro = PosicaoDoAluno();
        // O vao fica do lado em que ha campo: do aluno para o meio dele (no comeco, para cima).
        const Vector2 livre = meioDoCampo - centro;
        const float rumo = livre.Length() > 40.0f ? Math::ToDegrees(Math::Atan2(livre.y, livre.x)) : -90.0f;
        const float vao = VaoDaAbertura(mSorteio, mVaoAnterior, rumo);
        mVaoAnterior = vao;
        AnelVivo vivo;
        vivo.some = a.quando + Fechamento(a) - 0.02f;
        for (int i = 0; i < a.tiros; ++i) {
            if (NoVao(i, a.tiros, vao, a.vao)) continue;
            const float angulo = Math::ToRadians(AnguloNoAnel(i, a.tiros));
            const Vector2 dir(Math::Cos(angulo), Math::Sin(angulo));
            if (BossProjectile* p = Um(kGatilhoDaAbertura, centro + dir * a.raio, "Tiro", dir * -a.velocidade)) {
                // O anel inteiro, ate a parte que nasce fora do campo: perto da parede nao se foge por ela.
                p->DefinirMargemDeSaida(2.0f, 1.0f);
                vivo.tiros.push_back(p);
            }
        }
        mAneis.push_back(std::move(vivo));
        ++mAneisFeitos;
        mClarao = 0.12f;
        mCorDoClarao = SDL_Color{255, 60, 70, 255};
        SomDoDisparo();
    }
    // O anel que fechou some no meio.
    for (AnelVivo& a : mAneis) {
        if (te >= a.some) MatarTodos(a.tiros);
    }
    // O golpe e para assustar, e nao para acabar a prova: quem foi pego uma vez fica protegido ate o
    // fim dele.
    const std::vector<Player*> alunos = Jogadores();
    for (size_t i = 0; i < alunos.size() && i < mPoupado.size(); ++i) {
        if (mPoupado[i] || !alunos[i]->AcabouDeApanhar()) continue;
        mPoupado[i] = true;
        alunos[i]->Proteger(kDuracoes[kEtapaDaAbertura] - te + 0.3f);
    }
    Placar(kFase, kEtapaDaAbertura, mAneisFeitos < kAneisDaAbertura || !mAneis.empty() ? "quadro 1: CORRA para o v\xc3\xa3o!"
                                                                                      : "Bem-vindo \xc3\xa0 INF 216. Isso era o tutorial.");
}

void Ferreira::DevolverABarra() {
    MatarTodos(mBarra);
    if (FaixaDaNota* faixa = Faixa(); faixa && faixa->Solta()) faixa->Devolver();
}

void Ferreira::AtualizarBarra(const int etapa, const float te) {
    using namespace HudHostil;
    FaixaDaNota* faixa = Faixa();
    if (!faixa) return;
    const PoseDaBarra pose = Pose(etapa, te, CampoPuro(), faixa->CentroDoEncaixe(), faixa->ComprimentoNoEncaixe(), mDegrauDoHud);
    if (pose.noEncaixe && etapa == kEtapaDaBarraDePe) {
        // De volta ao rodape: a HUD tem a barra de novo.
        DevolverABarra();
        return;
    }
    faixa->Pousar(pose.centro, pose.comprimento, pose.angulo, pose.espelhada);

    // O que machuca: um segmento sem cara embaixo de cada pedaco cheio do que a barra MOSTRA - a
    // nota que se ve, e nao a que ela ainda vai alcancar.
    const float mostrada = faixa->Mostrada();
    const int n = std::max(1, static_cast<int>(pose.comprimento / kSegmento));
    if (static_cast<int>(mBarra.size()) != n) {
        MatarTodos(mBarra);
        mBarra.assign(static_cast<size_t>(n), nullptr);
    }
    for (int k = 0; k < n; ++k) {
        BossProjectile*& p = mBarra[static_cast<size_t>(k)];
        if (!pose.perigo || !Cheio(k, n, mostrada, pose.espelhada)) {
            Matar(p);
            continue;
        }
        const Vector2 onde = CentroDoSegmento(pose, k, n);
        if (!p) p = Peca(kGatilhoDaBarra, onde, "Vazia");
        if (p) {
            p->SetPosition(onde);
            Parar(p);
        }
    }

    // O puxao: o professor a arranca do rodape.
    if (etapa == kEtapaDaBarraDeitada && te >= kTremorDaBarra * 0.4f && te < kTremorDaBarra + kArrancadaDaBarra) {
        Tracejada(GetPosition() + Vector2(0.0f, 40.0f), pose.centro, kOuro, 9.0f);
        Circulo(pose.centro, 20.0f, kOuro, 14);
    }
    if (!pose.perigo) return;

    // O guia: a faixa por onde se passa, do tamanho do pedaco vazio, e a linha onde o cheio acaba.
    const int cheios = Cheios(mostrada, n);
    if (cheios >= n) return;
    const int primeiro = pose.espelhada ? 0 : cheios;
    const int ultimo = pose.espelhada ? n - cheios - 1 : n - 1;
    const Vector2 a = CentroDoSegmento(pose, primeiro, n) - pose.eixo * (kSegmento / 2.0f);
    const Vector2 b = CentroDoSegmento(pose, ultimo, n) + pose.eixo * (kSegmento / 2.0f);
    const Vector2 corte = pose.espelhada ? b : a;
    const SDL_FRect c = Campo();
    if (std::fabs(pose.eixo.x) > 0.5f) {
        Preencher(Vector2(a.x, c.y), Vector2(b.x, c.y + c.h), kSeguro);
        Tracejada(Vector2(corte.x, c.y), Vector2(corte.x, c.y + c.h), kGuia, 10.0f);
    } else {
        Preencher(Vector2(c.x, a.y), Vector2(c.x + c.w, b.y), kSeguro);
        Tracejada(Vector2(c.x, corte.y), Vector2(c.x + c.w, corte.y), kGuia, 10.0f);
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
    Placar(kFase, kEtapaDasJanelas, "Janelas abertas: " + std::to_string(mJanelas.size()) + ". Atire no X para fechar");
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

// ---- Fase 2: a fisica ----

void Ferreira::SoltarAGravidade() {
    for (Player* a : Jogadores()) a->DefinirChao(std::nullopt);
}

float Ferreira::ChaoDoAluno() const {
    for (const Player* a : Jogadores()) {
        if (const auto* d = a->GetComponent<DrawAnimatedComponent>()) return static_cast<float>(d->GetSpriteHeight());
    }
    return 64.0f;
}

void Ferreira::PrepararLado(const int etapa) {
    using namespace Gravidade;
    const Lado lado = mOrdem[static_cast<size_t>(etapa)];
    const Degrau& g = mDegrauDaGravidade;
    switch (DesafioDo(lado)) {
        case Desafio::Pulos:
            mPilhasPorVir = Pulos(mSorteioDaFisica, g);
            break;
        case Desafio::Lancas:
            mLancasPorVir = Lancas(mSorteioDaFisica, g);
            break;
        case Desafio::Chuva:
            mQuedasPorVir = Chuva(mSorteioDaFisica, g, Comprimento(lado, CampoPuro()));
            break;
        case Desafio::Esteira:
            mBuracosPorVir = Buracos(mSorteioDaFisica, g);
            mSentidoDaEsteira = mSorteioDaFisica.Chance(0.5f) ? 1.0f : -1.0f;
            break;
    }
}

void Ferreira::DesenharOChao(const Lado l, const float chao, const float pes, const SDL_Color cor, const bool aviso) {
    using namespace Gravidade;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    const Vector2 a = Ponto(l, c, chao, 0.0f, -pes), b = Ponto(l, c, chao, comprimento, -pes);
    if (aviso) {
        Tracejada(a, b, cor, 12.0f);
        return;
    }
    // A faixa entre os pes do aluno e a borda do campo, a linha de cima, clara, e as juntas - na
    // camada de baixo, sob o placar e os tiros.
    if (!mLinhasDoChao) return;
    const Vector2 borda = Ponto(l, c, chao, comprimento, -chao);
    mLinhasDoChao->Preencher(Vector2(std::min(a.x, borda.x), std::min(a.y, borda.y)),
                             Vector2(std::max(a.x, borda.x), std::max(a.y, borda.y)), kChaoFundo);
    mLinhasDoChao->Acrescentar(a, b, cor);
    for (float u = 0.0f; u <= comprimento; u += 56.0f) {
        mLinhasDoChao->Acrescentar(Ponto(l, c, chao, u, -pes), Ponto(l, c, chao, u, -chao), kChaoJunta);
    }
}

void Ferreira::AtualizarFisica(const float t, const float deltaTime) {
    using namespace Gravidade;
    const Degrau& g = mDegrauDaGravidade;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    if (TrocouDeEtapa(etapa)) PrepararLado(etapa);
    const Lado lado = mOrdem[static_cast<size_t>(etapa)];
    const bool virou = te >= kVirada;
    const JogosDigitais::Campo c = CampoPuro();
    const float chao = ChaoDoAluno(), pes = chao / 2.0f;

    // A gravidade: antes da virada, a do lado anterior - no primeiro, ainda se voa.
    std::optional<Lado> vale;
    if (virou) vale = lado;
    else if (etapa > 0) vale = mOrdem[static_cast<size_t>(etapa - 1)];
    for (Player* a : Jogadores()) a->DefinirChao(vale);

    // O professor fica no ceu do chao que vale; na virada, some e aparece no ceu novo.
    if (virou && mVirada != etapa) {
        mVirada = etapa;
        SetPosition(Ponto(lado, c, chao, Comprimento(lado, c) / 2.0f, Profundidade(lado, c, chao) - 60.0f));
        mClarao = 0.15f;
        mCorDoClarao = SDL_Color{200, 170, 255, 255};
        SomDoDisparo();
    }
    if (vale) {
        const float comprimento = Comprimento(*vale, c);
        const float u = comprimento / 2.0f + (comprimento / 2.0f - 150.0f) * Math::Sin(Math::TwoPi * t / 8.0f);
        mAlvo = Ponto(*vale, c, chao, u, Profundidade(*vale, c, chao) - 60.0f);
        DesenharOChao(*vale, chao, pes, kChaoLinha, false);
    } else {
        Varrer(t, 0.3f, 8.0f);
    }

    // Antes da virada: a seta no meio do campo e o chao novo, piscando.
    if (!virou) {
        if (Aceso(te, 0.12f)) {
            DesenharOChao(lado, chao, pes, kPerigo, true);
            SetaGrande(Vector2(c.x + c.w / 2.0f, c.y + c.h / 2.0f), Plataforma::Direcao(lado), 150.0f, kPerigo);
        }
        Placar(kFase, static_cast<size_t>(etapa), "fisica.gravidade = " + NomeDoLado(lado) + " em 3, 2, 1...");
        return;
    }

    switch (DesafioDo(lado)) {
        case Desafio::Pulos:   AtualizarPulos(te, deltaTime, lado, chao, pes); break;
        case Desafio::Lancas:  AtualizarLancas(te, deltaTime, lado, chao, pes); break;
        case Desafio::Chuva:   AtualizarChuva(te, deltaTime, lado, chao, pes); break;
        case Desafio::Esteira: AtualizarEsteira(te, deltaTime, lado, chao, pes); break;
    }

    // Do ceu, o professor atira de tanto em tanto: o desafio e o chao.
    if (te >= kComecaODesafio + 0.5f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        Leque(kGatilhoDoTiro, g.tirosDoLeque, 30.0f, g.velocidade, "Tiro");
    }
    Placar(kFase, static_cast<size_t>(etapa), AnuncioDoLado(lado));
}

void Ferreira::AtualizarPulos(const float te, const float dt, const Lado l, const float chao, const float pes) {
    using namespace Gravidade;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    // As pilhas que chegam: comecam logo fora da ponta de onde vem.
    while (mProximaPilha < mPilhasPorVir.size() && te >= mPilhasPorVir[mProximaPilha].quando) {
        const Pilha& p = mPilhasPorVir[mProximaPilha++];
        PilhaViva v;
        v.sentido = p.sentido;
        v.velocidade = p.velocidade;
        v.altura = p.altura;
        v.largura = p.largura;
        v.u = p.sentido > 0.0f ? -kPeca * static_cast<float>(p.largura) - 10.0f : comprimento + 10.0f;
        const Vector2 andando = Plataforma::AoLongo(l) * (p.sentido * p.velocidade);
        for (int i = 0; i < p.largura; ++i) {
            for (int j = 0; j < p.altura; ++j) {
                const Vector2 onde = Ponto(l, c, chao, v.u + (static_cast<float>(i) + 0.5f) * kPeca, AlturaDaPeca(j, pes));
                v.caixas.push_back(PecaAndando(kGatilhoDaCaixa, onde, "Caixa", andando));
            }
        }
        mPilhas.push_back(std::move(v));
    }
    // Elas andam sozinhas; aqui so se acompanha onde esta a primeira.
    for (PilhaViva& v : mPilhas) {
        v.u += v.sentido * v.velocidade * dt;
        for (const BossProjectile* caixa : v.caixas) {
            if (!caixa) continue;
            v.u = NoChao(l, c, chao, caixa->GetPosition()).x - 0.5f * kPeca;
            break;
        }
    }
    // A que saiu pela outra ponta, sai.
    mPilhas.erase(std::remove_if(mPilhas.begin(), mPilhas.end(), [&](PilhaViva& v) {
        const bool saiu = v.sentido > 0.0f ? v.u > comprimento + 40.0f
                                           : v.u + kPeca * static_cast<float>(v.largura) < -40.0f;
        if (saiu) MatarTodos(v.caixas);
        return saiu;
    }), mPilhas.end());
}

void Ferreira::AtualizarLancas(const float te, const float dt, const Lado l, const float chao, const float pes) {
    using namespace Gravidade;
    (void)pes;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    const Vector2 ao = Plataforma::AoLongo(l);

    // O aviso, na ponta de onde ela vem e na altura dela: vermelha a de baixo, dourada a de cima.
    for (size_t k = mProximaLanca; k < mLancasPorVir.size(); ++k) {
        const Lanca& x = mLancasPorVir[k];
        if (te < x.quando - kAvisoDaLanca) break;
        if (!Aceso(te, 0.08f)) continue;
        const SDL_Color cor = x.h == kLancaBaixa ? kPerigo : kOuro;
        const Vector2 p = Ponto(l, c, chao, x.sentido > 0.0f ? 16.0f : comprimento - 16.0f, x.h);
        Seta(p - ao * (x.sentido * 20.0f), p + ao * (x.sentido * 20.0f), cor);
        Circulo(p, 11.0f, cor, 10);
    }
    while (mProximaLanca < mLancasPorVir.size() && te >= mLancasPorVir[mProximaLanca].quando) {
        const Lanca& x = mLancasPorVir[mProximaLanca++];
        LancaViva v;
        v.sentido = x.sentido;
        v.velocidade = x.velocidade;
        v.h = x.h;
        v.u = x.sentido > 0.0f ? -20.0f : comprimento + 20.0f;
        v.p = PecaAndando(kGatilhoDaLanca, Ponto(l, c, chao, v.u, v.h), "Lanca", ao * (x.sentido * x.velocidade));
        if (!v.p) continue;
        // O desenho aponta para a direita; o ator gira ao contrario do SDL (DrawAnimatedComponent).
        const Vector2 d = ao * x.sentido;
        v.p->SetRotation(-Math::Atan2(d.y, d.x));
        mLancas.push_back(v);
        SomDoDisparo();
    }
    for (LancaViva& v : mLancas) {
        v.u += v.sentido * v.velocidade * dt;
        if (v.p) v.u = NoChao(l, c, chao, v.p->GetPosition()).x;
    }
    mLancas.erase(std::remove_if(mLancas.begin(), mLancas.end(), [&](LancaViva& v) {
        const bool fora = !v.p || v.u < -60.0f || v.u > comprimento + 60.0f;
        if (fora) Matar(v.p);
        return fora;
    }), mLancas.end());
}

void Ferreira::AtualizarChuva(const float te, const float dt, const Lado l, const float chao, const float pes) {
    using namespace Gravidade;
    const Degrau& g = mDegrauDaGravidade;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    const float ceu = Profundidade(l, c, chao) + 30.0f;
    const float noChao = AlturaDaPeca(0, pes);
    const float giro = -Math::ToRadians(Plataforma::Giro(l));
    Battle* b = Batalha();
    const std::vector<Player*> alunos = Jogadores();

    // A moeda tem a classe do aluno: passa por ele e paga a quem encosta.
    for (Player* a : alunos) a->DefinirClasse(kClasseDaMoedaDaChuva);
    while (mProximaDaChuva < mQuedasPorVir.size() && te >= mQuedasPorVir[mProximaDaChuva].quando) {
        const Queda& q = mQuedasPorVir[mProximaDaChuva++];
        QuedaViva v;
        v.u = q.u;
        v.h = ceu;
        v.moeda = q.moeda;
        const Vector2 caindo = Plataforma::Direcao(l) * g.queda;
        v.p = q.moeda ? PecaAndando(kGatilhoDaMoeda, Ponto(l, c, chao, v.u, v.h), "Moeda", caindo)
                      : PecaAndando(kGatilhoDoBloco, Ponto(l, c, chao, v.u, v.h), "Bloco", caindo);
        if (!v.p) continue;
        if (q.moeda) v.p->DefinirClasse(kClasseDaMoedaDaChuva);
        else v.p->SetRotation(giro);
        mQuedas.push_back(v);
    }
    for (QuedaViva& v : mQuedas) {
        if (!v.p) continue;
        v.h = NoChao(l, c, chao, v.p->GetPosition()).y;
        // A sombra no chao, embaixo de cada um.
        const SDL_Color cor = v.moeda ? kOuro : kPerigo;
        Linha(Ponto(l, c, chao, v.u - 14.0f, -pes + 3.0f), Ponto(l, c, chao, v.u + 14.0f, -pes + 3.0f), cor);
        Linha(Ponto(l, c, chao, v.u - 8.0f, -pes + 6.0f), Ponto(l, c, chao, v.u + 8.0f, -pes + 6.0f), cor);
        if (v.h <= noChao) {
            // No chao, o bloco estilhaca em duas fagulhas, uma para cada lado; a moeda que ninguem pegou some.
            if (!v.moeda) {
                for (const float s : {-1.0f, 1.0f}) {
                    FagulhaViva f;
                    f.u = v.u;
                    f.sentido = s;
                    f.p = PecaAndando(kGatilhoDaFagulha, Ponto(l, c, chao, f.u, noChao), "Fagulha",
                                      Plataforma::AoLongo(l) * (s * g.fagulha));
                    if (f.p) mFagulhas.push_back(f);
                }
                SomDoDisparo();
            }
            Matar(v.p);
            continue;
        }
        if (!v.moeda) continue;
        for (const Player* a : alunos) {
            if ((v.p->GetPosition() - a->GetPosition()).Length() > RaioDe(a) + 26.0f) continue;
            if (b) {
                b->Faiscar(v.p->GetPosition(), Hud::Faisca::Coleta);
                b->Conquistar(kPremioDaMoedaDaChuva);
            }
            Matar(v.p);
            break;
        }
    }
    mQuedas.erase(std::remove_if(mQuedas.begin(), mQuedas.end(), [](const QuedaViva& v) { return !v.p; }), mQuedas.end());
    for (FagulhaViva& f : mFagulhas) {
        f.u += f.sentido * g.fagulha * dt;
        if (f.p) f.u = NoChao(l, c, chao, f.p->GetPosition()).x;
    }
    mFagulhas.erase(std::remove_if(mFagulhas.begin(), mFagulhas.end(), [&](FagulhaViva& f) {
        const bool fora = !f.p || f.u < -40.0f || f.u > comprimento + 40.0f;
        if (fora) Matar(f.p);
        return fora;
    }), mFagulhas.end());
}

void Ferreira::AtualizarEsteira(const float te, const float dt, const Lado l, const float chao, const float pes) {
    using namespace Gravidade;
    const Degrau& g = mDegrauDaGravidade;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    const Vector2 ao = Plataforma::AoLongo(l);
    const float s = mSentidoDaEsteira;
    const float giro = -Math::ToRadians(Plataforma::Giro(l));
    const float noChao = AlturaDaPeca(0, pes);
    const float moedor = s > 0.0f ? comprimento - kRecuoDoMoedor : kRecuoDoMoedor;

    // O moedor, na ponta de chegada: uma coluna de caixas mais alta que o pulo, tremendo.
    if (mMoedor.empty()) {
        for (int j = 0; j < kPecasDoMoedor; ++j) mMoedor.push_back(Peca(kGatilhoDaCaixa, Ponto(l, c, chao, moedor, AlturaDaPeca(j, pes)), "Caixa"));
    }
    for (size_t j = 0; j < mMoedor.size(); ++j) {
        if (!mMoedor[j]) continue;
        const float tremor = 3.0f * Math::Sin(mRelogio * 40.0f + static_cast<float>(j));
        mMoedor[j]->SetPosition(Ponto(l, c, chao, moedor + tremor, AlturaDaPeca(static_cast<int>(j), pes)));
        Parar(mMoedor[j]);
    }

    // A esteira leva quem esta no chao; no ar, nao.
    for (Player* a : Jogadores()) {
        if (a->NoChao()) a->SetPosition(a->GetPosition() + ao * (s * g.esteira * dt));
    }

    // Os buracos: fileiras de espinhos que vem com o chao, da ponta de saida.
    while (mProximoBuraco < mBuracosPorVir.size() && te >= mBuracosPorVir[mProximoBuraco].quando) {
        const Buraco& x = mBuracosPorVir[mProximoBuraco++];
        BuracoVivo v;
        v.u = s > 0.0f ? -kPeca * static_cast<float>(x.espinhos) : comprimento;
        for (int k = 0; k < x.espinhos; ++k) {
            BossProjectile* p = PecaAndando(kGatilhoDoEspinho, Ponto(l, c, chao, v.u + (static_cast<float>(k) + 0.5f) * kPeca, noChao),
                                            "Espinho", ao * (s * g.esteira));
            if (p) p->SetRotation(giro);
            v.espinhos.push_back(p);
        }
        mBuracos.push_back(std::move(v));
    }
    // Os espinhos andam com a esteira; aqui so se acompanha onde esta o primeiro.
    for (BuracoVivo& v : mBuracos) {
        v.u += s * g.esteira * dt;
        for (size_t k = 0; k < v.espinhos.size(); ++k) {
            if (!v.espinhos[k]) continue;
            v.u = NoChao(l, c, chao, v.espinhos[k]->GetPosition()).x - (static_cast<float>(k) + 0.5f) * kPeca;
            break;
        }
    }
    // O buraco que chegou ao moedor e moido.
    mBuracos.erase(std::remove_if(mBuracos.begin(), mBuracos.end(), [&](BuracoVivo& v) {
        const float fim = v.u + kPeca * static_cast<float>(v.espinhos.size());
        const bool moido = s > 0.0f ? fim > moedor - kPeca / 2.0f : v.u < moedor + kPeca / 2.0f;
        if (moido) MatarTodos(v.espinhos);
        return moido;
    }), mBuracos.end());

    // As setas da esteira, andando com ela, dentro do chao.
    constexpr float kPasso = 64.0f;
    const float fase = std::fmod(mRelogio * g.esteira, kPasso);
    for (float u = -kPasso; u <= comprimento + kPasso; u += kPasso) {
        const float aqui = u + s * fase;
        if (aqui < 8.0f || aqui > comprimento - 8.0f) continue;
        const Vector2 p = Ponto(l, c, chao, aqui, -pes - (chao - pes) / 2.0f);
        if (!mLinhasDoChao) continue;
        const Vector2 ponta = p + ao * (s * 10.0f), lado(-ao.y, ao.x);
        mLinhasDoChao->Acrescentar(p - ao * (s * 10.0f), ponta, kChaoLinha);
        mLinhasDoChao->Acrescentar(ponta, ponta - ao * (s * 7.0f) + lado * 5.0f, kChaoLinha);
        mLinhasDoChao->Acrescentar(ponta, ponta - ao * (s * 7.0f) - lado * 5.0f, kChaoLinha);
    }
}

// ---- Fase 3: as regras ----

void Ferreira::AtualizarRegras(const float t, const float deltaTime) {
    using namespace RegrasTrocadas;
    (void)deltaTime;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    TrocouDeEtapa(etapa);
    Varrer(t, 0.32f, 8.0f);
    if (etapa == 0) AtualizarInversao(te);
    else if (etapa == 1) AtualizarFogoAmigo(te);
    else if (etapa == 2) AtualizarLag(te);
    else AtualizarSaveLoad(te);
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
    Placar(kFase, 1, ligado ? "fogoAmigo = true: tiro que ERRA volta por baixo" : "fogoAmigo = true em 3, 2, 1...");
}

void Ferreira::AtualizarLag(const float te) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
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
        mProximoAnel = te + g.entreAneisDoMotor;
        entrar(Anel(kGatilhoDoAnel, g.tirosDoAnelDoMotor, g.velocidadeDoMotor, "Tiro"));
    }
    if (te >= 1.4f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLequesDoMotor;
        entrar(Leque(kGatilhoDoTiro, g.tirosDoLequeDoMotor, 28.0f, g.velocidadeDoMotor + 110.0f, "Tiro"));
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
    Placar(kFase, 2, "fps = 2.5: o c\xc3\xadrculo \xc3\xa9 o PR\xc3\x93XIMO quadro");
}

void Ferreira::AtualizarSaveLoad(const float te) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const SDL_FRect c = Campo();
    Battle* b = Batalha();

    if (te >= 0.5f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneisDoMotor;
        for (BossProjectile* p : Anel(kGatilhoDoAnel, g.tirosDoAnelDoMotor, g.velocidadeDoMotor, "Tiro")) mTiros.push_back(p);
    }
    if (te >= 0.9f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLequesDoMotor;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, g.tirosDoLequeDoMotor, 28.0f, g.velocidadeDoMotor + 70.0f, "Tiro")) mTiros.push_back(p);
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
        Placar(kFase, 3, "Salvo com nota " + Inteiro(mSalvo.nota) + ": o load desfaz TUDO desde o save");
    } else {
        Placar(kFase, 3, "quicksave() a caminho: o jogo vai voltar no tempo");
    }
}

// ---- A repescagem: o lancamento ----

void Ferreira::AtualizarLancamento(const float t, const float deltaTime) {
    using namespace Lancamento;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    TrocouDeEtapa(etapa);
    Varrer(t, 0.3f, 8.0f);
    if (etapa == 0) AtualizarHitbox(te);
    else if (etapa == 1) AtualizarCrash(te);
    else if (etapa == 2) AtualizarFrota(te);
    else AtualizarCreditos(te, deltaTime);
}

void Ferreira::AtualizarHitbox(const float te) {
    using namespace Lancamento;
    const Degrau& g = mDegrauDoLancamento;

    // O desenho vira enfeite, e a colisao vai para um tiro sem cara, que o professor conduz.
    const auto entrar = [this](const std::vector<BossProjectile*>& novos) {
        for (BossProjectile* desenho : novos) {
            desenho->DefinirEnfeite(true);
            BossProjectile* colisor = Peca(kGatilhoDoColisor, desenho->GetPosition(), "Vazio");
            mDescolados.push_back({desenho, colisor});
        }
    };
    if (te >= 0.8f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneisDaHitbox;
        entrar(Anel(kGatilhoDoSprite, g.tirosDoAnelDaHitbox, g.velocidadeDaHitbox, "Tiro"));
    }
    if (te >= 1.4f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLequesDaHitbox;
        entrar(Leque(kGatilhoDoSprite, g.tirosDoLequeDaHitbox, 28.0f, g.velocidadeDaHitbox + 60.0f, "Tiro"));
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
    Placar(kFase, 0, "debug: a hitbox DESCOLOU. Desvie do c\xc3\xadrculo verde");
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
        Placar(kFase, 1, "O jogo travou: atire em ESPERAR (ou em FECHAR...)");
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
    Placar(kFase, 1, "Build de lan\xc3\xa7" "amento: nada pode dar errado");
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
        Placar(kFase, 2, "genero = nave: os bugs do lan\xc3\xa7" "amento chegaram");
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
    Placar(kFase, 2, "genero = nave: faltam " + std::to_string(vivos) + " bugs. Derrube todos!");
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
    Placar(kFase, 3, "Cr\xc3\xa9" "ditos: passe pelos v\xc3\xa3os. Obrigado por jogar!");
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
