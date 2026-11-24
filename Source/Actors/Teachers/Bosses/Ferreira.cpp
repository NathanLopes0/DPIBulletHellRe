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
    mDegrauDoInimigo = IaDoInimigo::DegrauDo(n);
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
    // Quem tira um segmento da barra e o proprio professor (Matar ja o esquece); morto e ainda
    // guardado, so a onda do Ponto Extra o levou.
    if (mQuebrada < 0.0f) {
        if (const Battle* b = Batalha()) {
            for (const BossProjectile* p : mBarra) {
                if (p && !Vivo(p) && b->DissolvidoPelaOnda(p)) mBarraAcertada = true;
            }
        }
    }
    Sanear(mBarra, false);
    Sanear(mCorpo, false);
    for (BossProjectile*& o : mOlhos) {
        if (!Vivo(o)) o = nullptr;
    }
    if (!Vivo(mPoder)) mPoder = nullptr;
    if (!Vivo(mSave)) mSave = nullptr;
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
    mVermelhos.erase(std::remove_if(mVermelhos.begin(), mVermelhos.end(), [](const Vermelho& v) { return !Vivo(v.p); }),
                     mVermelhos.end());
    for (Descolado& d : mDescolados) {
        if (!Vivo(d.desenho)) d.desenho = nullptr;
        if (!Vivo(d.colisor)) d.colisor = nullptr;
    }
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
    if (FaixaDaNota* faixa = Faixa()) faixa->EsconderNumero(false);
    mNotaAntesDaEdicao = -1.0f;
    DevolverABarra();
    // Nas regras, o inimigo passa de uma etapa para a outra (mInimigoFica).
    if (!mInimigoFica) DesmontarOInimigo();
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
    for (Vermelho& v : mVermelhos) {
        // Fora do FPS, o vermelho volta a andar o que foi escrito nele.
        if (Vivo(v.p)) Andar(v.p, v.v);
    }
    mVermelhos.clear();
    mFps = RegrasTrocadas::kFpsDeReferencia;
    mFolgaDoFoco = RegrasTrocadas::Folga{};
    mArranques = {};
    mHistoricoDoFps.clear();
    mProximaAmostra = 0.0f;
    mRelogioSemDt = 0.0f;
    mProximaEspiral = 0.0f;
    mGiroDaEspiral = 0.0f;
    mProximaParede = 0.0f;
    mVaoDaParede = -1.0f;
    for (Descolado& d : mDescolados) {
        Matar(d.colisor);
        Matar(d.desenho);
    }
    mDescolados.clear();
    mSalvo = Salvo{};
    Matar(mSave);
    mFuga = RegrasTrocadas::Fuga{};
    mUltimoLoad = -1.0f;
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
    mGodMode = false;
    mJanelaAberta = false;
    mJanelaAte = -1.0f;
    mOndaDaFrota = 0;
    mFrotaChegaEm = -1.0f;
    mPassosDaFrota = 0.0f;
    mProximoHotfix = 0.0f;
    mHotfixLugar = -1;
    Esconder(mEtiquetasDaFrota);
    mBugsDerrubados = 0;
    mProximaQueda = 0.0f;
    for (LinhaDeCredito& l : mCreditos) MatarTodos(l.blocos);
    mCreditos.clear();
    mLinhasFeitas = 0;
    mProximaLinha = 0.0f;
    Esconder(mEtiquetasDoInimigo);
    Esconder(mEtiquetasDoSave);
    Esconder(mEtiquetasDoDialogo);
    Esconder(mEtiquetasDosCreditos);
    mProximoAnel = 0.0f;
    mProximoLeque = 0.0f;
}

void Ferreira::Desmontar() {
    // A troca de fase leva tudo, o inimigo tambem.
    mInimigoFica = false;
    DesmontarEtapa();
    // A gravidade e da fase inteira: so a troca de fase (ou o fim da luta) a desliga.
    SoltarAGravidade();
    mVirada = -1;
    if (mLinhas) mLinhas->Limpar();
    if (mLinhasDoChao) mLinhasDoChao->Limpar();
    if (mLinhasDoContador) mLinhasDoContador->Limpar();
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
    ReescreverEtapas(fase, etapas);
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
        mLinhasDoContador = ator->AddComponent<DrawLinhasComponent>(kOrdemDoContador);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();
    if (mLinhasDoChao) mLinhasDoChao->Limpar();
    if (mLinhasDoContador) mLinhasDoContador->Limpar();

    if (mBarraAcertada && mQuebrada < 0.0f) QuebrarABarra();
    if (mQuebrada >= 0.0f) AtualizarQuebra(deltaTime);
    else if (fase == HudHostil::kFase) AtualizarHud(t, deltaTime);
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
    const Degrau& g = mDegrauDoHud;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    TrocouDeEtapa(etapa);
    Varrer(t, 0.3f, 9.0f);

    if (etapa == kEtapaDaAbertura) {
        AtualizarAbertura(te);
        return;
    }
    if (etapa == kEtapaDoInimigo) {
        AtualizarInimigo(te, deltaTime, ComoOInimigo{});
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
    FaixaDaNota* faixa = Faixa();
    const Vector2 placa = faixa ? faixa->LugarDoNumero() : Vector2(Campo().x + Campo().w / 2.0f, Campo().y + Campo().h);
    // Uma vez por luta: se a fase 1 voltar (o ciclo das fases), a nota ja foi reescrita.
    if (mNotaEditada) {
        if (faixa) faixa->EsconderNumero(false);
        Placar(kFase, kEtapaDaAbertura, "nota = 15: agora a luta e de verdade");
        return;
    }
    if (mNotaAntesDaEdicao < 0.0f) mNotaAntesDaEdicao = Nota();
    const NumeroDaAbertura n = NumeroEm(te, placa, GetPosition(), mNotaAntesDaEdicao);

    if (n.momento == Momento::Devolvido) {
        // O numero novo encaixa na placa: dali em diante, a nota e 15.
        mNotaEditada = true;
        if (Battle* b = Batalha()) {
            b->DefinirNota(kNotaDaAbertura);
            b->Faiscar(placa, Hud::Faisca::Coleta);
        }
        if (faixa) faixa->EsconderNumero(false);
        mClarao = 0.2f;
        mCorDoClarao = kMagenta;
        Avisar("hud.nota = 15. Agora sim: bem-vindo a INF 216", 1.8f);
        SomDoDisparo();
        return;
    }
    if (faixa) faixa->EsconderNumero(n.momento != Momento::NaPlaca || Aceso(te, 0.08f));

    // O numero vai na camada de cima, como o contador do FPS: o jogo inteiro e dele.
    DrawLinhasComponent* doCampo = mLinhas;
    if (mLinhasDoContador) mLinhas = mLinhasDoContador;
    char texto[16];
    std::snprintf(texto, sizeof(texto), "%.2f", static_cast<double>(n.valor));
    const float altura = 22.0f * n.tamanho;
    // A largura de um caractere e a do ponto, como SeteSegmentos as escreve, para centrar o numero.
    float largura = 0.0f;
    for (const char* ch = texto; *ch; ++ch) largura += *ch == '.' ? altura / 4.0f : altura * 0.5f + std::max(2.0f, altura / 8.0f) * 1.6f;
    Vector2 canto = n.onde - Vector2(largura / 2.0f, altura / 2.0f);
    if (n.momento == Momento::NaPlaca) canto = canto + Vector2(3.0f * Math::Sin(mRelogio * 70.0f), 0.0f);
    const bool piscando = n.momento == Momento::Reescrevendo && Aceso(te, 0.09f);
    const SDL_Color cor = n.momento == Momento::Reescrevendo ? (piscando ? SDL_Color{255, 60, 230, 255} : SDL_Color{255, 240, 255, 255})
                                                             : SDL_Color{240, 244, 255, 255};
    if (n.momento != Momento::NaPlaca) {
        Preencher(canto - Vector2(10.0f, 8.0f), canto + Vector2(largura + 10.0f, altura + 8.0f), SDL_Color{8, 10, 20, 225});
        Retangulo(canto - Vector2(10.0f, 8.0f), canto + Vector2(largura + 10.0f, altura + 8.0f), kMagenta);
    }
    SeteSegmentos(texto, canto, altura, cor);
    if (n.momento == Momento::Reescrevendo) {
        // O cursor de quem edita, piscando depois do numero.
        if (Aceso(te, 0.25f)) Preencher(canto + Vector2(largura + 4.0f, 0.0f), canto + Vector2(largura + 8.0f, altura), kMagenta);
    }
    mLinhas = doCampo;
    // O fio: o professor puxa o numero da HUD ate ele, e o empurra de volta.
    if (n.momento == Momento::Indo || n.momento == Momento::Voltando || n.momento == Momento::NaPlaca) {
        Tracejada(GetPosition() + Vector2(0.0f, 40.0f), n.onde, kMagenta, 10.0f);
    }
    switch (n.momento) {
        case Momento::NaPlaca:      Placar(kFase, kEtapaDaAbertura, "hud.nota: este n\xc3\xbamero \xc3\xa9 meu"); break;
        case Momento::Indo:         Placar(kFase, kEtapaDaAbertura, "hud.nota.pegar()"); break;
        case Momento::Reescrevendo: Placar(kFase, kEtapaDaAbertura, "nota = " + Inteiro(mNotaAntesDaEdicao) + " -> nota = " + Inteiro(n.valor)); break;
        case Momento::Voltando:     Placar(kFase, kEtapaDaAbertura, "hud.nota.devolver(15)"); break;
        case Momento::Devolvido:    break;
    }
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
    mPoseDaBarra = pose;

    // O que machuca: um segmento sem cara embaixo de cada pedaco cheio do que a barra MOSTRA - a
    // nota que se ve, e nao a que ela ainda vai alcancar.
    const float mostrada = faixa->Mostrada();
    const int n = std::max(1, static_cast<int>(pose.comprimento / kSegmento));
    if (static_cast<int>(mBarra.size()) != n) {
        MatarTodos(mBarra);
        mBarra.assign(static_cast<size_t>(n), nullptr);
    }
    mSegmentosDaBarra = n;
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

void Ferreira::QuebrarABarra() {
    using namespace HudHostil;
    mBarraAcertada = false;
    mQuebrada = 0.0f;
    Battle* b = Batalha();
    FaixaDaNota* faixa = Faixa();
    const float mostrada = faixa ? faixa->Mostrada() : Nota();
    mCacos = Cacos(mPoseDaBarra, mSegmentosDaBarra, mostrada);
    mNotaAntesDaQuebra = Nota();
    // A barra sai de campo, e com ela o que o professor tinha atirado: agora so os cacos caem.
    MatarTodos(mBarra);
    for (BossProjectile*& p : mTiros) Matar(p);
    mTiros.clear();
    if (faixa) {
        mCorDosCacos = faixa->CorDoCheio();
        faixa->Quebrar();
    }
    if (b) {
        for (int k = 0; k < mSegmentosDaBarra; k += 2) {
            b->Faiscar(CentroDoSegmento(mPoseDaBarra, k, mSegmentosDaBarra), Hud::Faisca::Acerto);
        }
        // Os cacos nao machucam: o que sobrar no campo tambem nao, ate a luta acabar.
        b->CapturarAcertos(kQuebraDaBarra + 0.5f, [](Player&) {});
    }
    mClarao = 0.3f;
    mCorDoClarao = SDL_Color{255, 40, 60, 255};
    Avisar("hud.nota.destruir(): voc\xc3\xaa quebrou a sua pr\xc3\xb3pria nota!", kQuebraDaBarra + 1.0f);
    SomDoDisparo();
}

void Ferreira::AtualizarQuebra(const float deltaTime) {
    using namespace HudHostil;
    mQuebrada += deltaTime;
    const SDL_Color cheio = mCorDosCacos;
    const SDL_Color vazio{120, 120, 150, 255};
    for (const Caco& c : mCacos) {
        const Vector2 centro = OndeDoCaco(c, mQuebrada);
        if (!NoCampo(centro, 40.0f)) continue;
        const float a = Math::ToRadians(c.giro * mQuebrada);
        const Vector2 u = Vector2(Math::Cos(a), Math::Sin(a)) * (c.lado / 2.0f);
        const Vector2 v(-u.y, u.x);
        const SDL_Color cor = c.cheio ? cheio : vazio;
        Linha(centro - u - v, centro + u - v, cor);
        Linha(centro + u - v, centro + u + v, cor);
        Linha(centro + u + v, centro - u + v, cor);
        Linha(centro - u + v, centro - u - v, cor);
        Linha(centro - u - v, centro + u + v, cor);
    }
    Placar(kFase, static_cast<size_t>(std::max(0, mEtapaAnterior)), "hud.nota = null");
    Battle* b = Batalha();
    if (!b || !b->EmCombate()) return;
    if (QuebraAcabou(mQuebrada)) {
        b->DestruirANota(kFraseDaNotaDestruida);
    } else {
        b->DefinirNota(NotaNaQuebra(mNotaAntesDaQuebra, mQuebrada));
    }
}

// ---- Fase 1, a ultima etapa: o inimigo e a IA dele ----

int Ferreira::BlocosVivos() const {
    int n = 0;
    for (const BossProjectile* p : mCorpo) n += Vivo(p) ? 1 : 0;
    return n;
}

void Ferreira::MontarOInimigo() {
    using namespace IaDoInimigo;
    MatarTodos(mCorpo);
    for (const Vector2& lugar : LugaresDosBlocos()) {
        BossProjectile* p = Peca(HudHostil::kGatilhoDoInimigo, mCentroDoInimigo + lugar, "Textura");
        // Nascendo, os blocos ainda se juntam: nao machucam.
        if (p) p->DefinirEnfeite(true);
        mCorpo.push_back(p);
    }
    for (BossProjectile*& o : mOlhos) Matar(o);
    PorOsOlhos();
    mCorpoMontado = true;
}

void Ferreira::PorOsOlhos() {
    using namespace IaDoInimigo;
    const auto lugares = LugaresDosOlhos();
    for (size_t k = 0; k < mOlhos.size() && k < lugares.size(); ++k) {
        if (Vivo(mOlhos[k])) continue;
        mOlhos[k] = Peca(HudHostil::kGatilhoDoOlho, mCentroDoInimigo + lugares[k], "Olho");
        if (mOlhos[k]) mOlhos[k]->DefinirEnfeite(!Machuca(mIa.estado));
    }
}

void Ferreira::DesmontarOInimigo() {
    // O que ele pagou e ainda nao entrou na nota entra agora.
    if (mPremioDoInimigo > 0.0f) {
        if (Battle* b = Batalha()) b->Conquistar(mPremioDoInimigo);
        mPremioDoInimigo = 0.0f;
    }
    MatarTodos(mCorpo);
    for (BossProjectile*& o : mOlhos) Matar(o);
    Matar(mPoder);
    mCorpoMontado = false;
    mIa = IaDoInimigo::Ia{};
    mVelocidadeDoInimigo = Vector2::Zero;
    mOlhoAtingido = false;
    mApanhava = {};
    mProximoPoder = -1.0f;
    mMarca.clear();
    mMarcaAte = -1.0f;
}

void Ferreira::AoTrocarDeEstado(const IaDoInimigo::Estado de, const IaDoInimigo::Estado para, const float te) {
    using namespace IaDoInimigo;
    Battle* b = Batalha();
    if (para == Estado::Alerta) {
        mMarca = "!";
        mMarcaAte = mRelogio + kAlerta;
        SomDoDisparo();
    }
    if (de == Estado::Persegue && para == Estado::Patrulha) {
        mMarca = "?";
        mMarcaAte = mRelogio + 1.0f;
    }
    if (para == Estado::Atordoado) {
        mClarao = 0.1f;
        mCorDoClarao = SDL_Color{255, 230, 120, 255};
    }
    // Os olhos voltam quando ele acorda.
    if (de == Estado::Atordoado) PorOsOlhos();
    if (para == Estado::Fugindo) {
        mClarao = 0.15f;
        mCorDoClarao = SDL_Color{255, 214, 90, 255};
        Avisar(mFase == RegrasTrocadas::kFase ? "regras.inverter(): o fantasma virou moeda - pegue!"
                                               : "powerUp.usar(): agora ELE foge de voc\xc3\xaa!",
               1.3f);
        SomDoDisparo();
    }
    if (de == Estado::Fugindo) {
        // O power-up acabou (ou ele foi derrotado): o proximo vem depois de um respiro.
        mProximoPoder = te + mDegrauDoInimigo.entrePoderes;
        PorOsOlhos();
    }
    if (para == Estado::Derrotado) {
        if (b) b->Conquistar(mPremioDoInimigo + kPremioPorDerrotar);
        mPremioDoInimigo = 0.0f;
        for (BossProjectile*& o : mOlhos) {
            if (Vivo(o) && b) b->Faiscar(o->GetPosition(), Hud::Faisca::Coleta);
            Matar(o);
        }
        MatarTodos(mCorpo);
        mCorpoMontado = false;
        mClarao = 0.25f;
        mCorDoClarao = SDL_Color{255, 214, 90, 255};
        Avisar("inimigo.destruir(): + " + Inteiro(kPremioPorDerrotar) + " de b\xc3\xb4nus", 1.6f);
        SomDoDisparo();
    }
}

void Ferreira::AtualizarInimigo(const float te, const float deltaTime, const ComoOInimigo& como) {
    using namespace IaDoInimigo;
    const IaDoInimigo::Degrau& g = mDegrauDoInimigo;
    const JogosDigitais::Campo campo = CampoPuro();
    const auto ronda = Ronda(campo);
    Battle* b = Batalha();
    const std::vector<Player*> alunos = Jogadores();

    // O alvo da IA: o aluno mais perto.
    const Player* alvo = nullptr;
    float distancia = 1e9f;
    for (const Player* a : alunos) {
        const float d = (a->GetPosition() - mCentroDoInimigo).Length();
        if (d < distancia) {
            distancia = d;
            alvo = a;
        }
    }
    const Vector2 ondeAluno = alvo ? alvo->GetPosition() : PosicaoDoAluno();

    // Nascendo, o corpo aparece no canto mais longe do aluno.
    if (mIa.estado == Estado::Nascendo && !mCorpoMontado) {
        mCantoDaRonda = Ninho(campo, ondeAluno);
        mCentroDoInimigo = ronda[static_cast<size_t>(mCantoDaRonda)];
        distancia = alunos.empty() ? 1e9f : (ondeAluno - mCentroDoInimigo).Length();
        MontarOInimigo();
        mClarao = 0.08f;
        mCorDoClarao = SDL_Color{255, 60, 230, 255};
        SomDoDisparo();
    }
    if (como.fase1 && mProximoPoder < 0.0f) mProximoPoder = g.primeiroPoder;

    // O power-up: nasce longe dele e espera; quem encosta faz ele fugir. Enquanto ele nasce, ou
    // depois de derrotado, o power-up espera.
    bool pegou = false;
    if (Vivo(mPoder)) {
        const bool valeAgora = mIa.estado != Estado::Nascendo && mIa.estado != Estado::Derrotado;
        for (const Player* a : alunos) {
            if (!valeAgora || (a->GetPosition() - mPoder->GetPosition()).Length() > RaioDe(a) + kToque + 6.0f) continue;
            pegou = true;
            if (b) b->Faiscar(mPoder->GetPosition(), Hud::Faisca::Coleta);
            Matar(mPoder);
            break;
        }
    } else if (como.fase1 && te >= mProximoPoder && mIa.estado != Estado::Fugindo &&
               te < HudHostil::kDuracoes[HudHostil::kEtapaDoInimigo] - 1.5f) {
        mPoder = Peca(HudHostil::kGatilhoDoPoder, LugarDoPoder(mSorteio, campo, mCentroDoInimigo), "Poder");
        if (mPoder) mPoder->DefinirEnfeite(true);
        mProximoPoder = 1e9f;
    }

    // O corpo acabou de acertar alguem: o aluno que comecou a piscar agora, encostado nele.
    bool acertou = false;
    for (size_t i = 0; i < alunos.size() && i < mApanhava.size(); ++i) {
        const bool apanha = alunos[i]->AcabouDeApanhar();
        const Vector2 d = alunos[i]->GetPosition() - mCentroDoInimigo;
        if (apanha && !mApanhava[i] && std::fabs(d.x) <= kMeiaLargura + 10.0f && std::fabs(d.y) <= kMeiaAltura + 10.0f) {
            acertou = true;
        }
        mApanhava[i] = apanha;
    }

    // A IA percebe e decide.
    Percepcao p;
    p.distancia = distancia;
    // O susto de fora (a inversao) faz o mesmo que o power-up, com o tempo dele.
    p.pegouOPoder = pegou || como.susto > 0.0f;
    p.susto = pegou ? 0.0f : como.susto;
    p.olhoAtingido = mOlhoAtingido;
    p.acertouOAluno = acertou && Machuca(mIa.estado);
    p.blocos = mCorpoMontado ? BlocosVivos() : 0;
    mOlhoAtingido = false;
    const Estado antes = mIa.estado;
    if (Passo(mIa, p, deltaTime, g)) {
        AoTrocarDeEstado(antes, mIa.estado, te);
        // Quem acertou o aluno recua: a ronda segue para o canto mais longe dele.
        if (p.acertouOAluno && mIa.estado == Estado::Patrulha) {
            mCantoDaRonda = (Ninho(campo, ondeAluno) + 3) % 4;
            mMarca.clear();
        }
    }

    // Onde ele anda: a ronda pelos cantos, atras do aluno, ou para longe dele.
    Vector2 rumo = Vector2::Zero;
    if (mIa.estado == Estado::Patrulha) {
        const Vector2 proximo = ronda[static_cast<size_t>((mCantoDaRonda + 1) % 4)];
        if ((proximo - mCentroDoInimigo).Length() < 14.0f) mCantoDaRonda = (mCantoDaRonda + 1) % 4;
        rumo = JogosDigitais::Rumo(mCentroDoInimigo, ronda[static_cast<size_t>((mCantoDaRonda + 1) % 4)]);
    } else if (mIa.estado == Estado::Persegue) {
        rumo = JogosDigitais::Rumo(mCentroDoInimigo, ondeAluno);
    } else if (mIa.estado == Estado::Fugindo) {
        const JogosDigitais::Campo arena{ronda[0].x, ronda[0].y, ronda[2].x - ronda[0].x, ronda[2].y - ronda[0].y};
        rumo = JogosDigitais::Fugir(mCentroDoInimigo, ondeAluno, arena, 10.0f);
    }
    // No FPS ele anda sem deltaTime: o passo dele e o do quadro.
    mVelocidadeDoInimigo = rumo * (Rapidez(mIa, g) * como.passo);
    if (mCorpoMontado) mCentroDoInimigo = NaArena(mCentroDoInimigo + mVelocidadeDoInimigo * deltaTime, campo);

    // O corpo e os olhos no lugar. Nascendo, os blocos vem do meio para o lugar; no alerta, ele treme.
    const bool fugindo = mIa.estado == Estado::Fugindo;
    const bool pisca = Pisca(mIa, mRelogio);
    const bool machuca = Machuca(mIa.estado);
    const float junta = mIa.estado == Estado::Nascendo ? std::clamp(mIa.noEstado / kNascendo, 0.0f, 1.0f) : 1.0f;
    // O fantasma flutua - o corpo sobe e desce, e a barra ondula embaixo.
    const Vector2 tremor = (mIa.estado == Estado::Alerta ? Vector2(4.0f * Math::Sin(mRelogio * 70.0f), 0.0f) : Vector2::Zero) +
                           Vector2(0.0f, Flutua(mRelogio));
    const auto lugares = LugaresDosBlocos();
    for (size_t k = 0; k < mCorpo.size() && k < lugares.size(); ++k) {
        BossProjectile* bloco = mCorpo[k];
        if (!bloco) continue;
        const Vector2 onda(0.0f, NaBarra(lugares[k]) ? Onda(Coluna(lugares[k]), mRelogio) : 0.0f);
        bloco->SetPosition(mCentroDoInimigo + tremor + onda + lugares[k] * (0.2f + 0.8f * junta));
        // Nas regras ele vive mais que um tiro (Projectile::kTempoDeVidaMaximo): o corpo nao expira.
        bloco->RenovarTempoDeVida();
        Andar(bloco, mVelocidadeDoInimigo);
        Cara(bloco, fugindo && !pisca ? "Moeda" : "Textura");
        bloco->DefinirEnfeite(!machuca);
    }
    const auto lugaresDosOlhos = LugaresDosOlhos();
    for (size_t k = 0; k < mOlhos.size() && k < lugaresDosOlhos.size(); ++k) {
        BossProjectile* olho = mOlhos[k];
        if (!olho) continue;
        olho->SetPosition(mCentroDoInimigo + tremor + lugaresDosOlhos[k] * (0.2f + 0.8f * junta));
        olho->RenovarTempoDeVida();
        Andar(olho, mVelocidadeDoInimigo);
        const float dx = ondeAluno.x - olho->GetPosition().x;
        const bool tonto = fugindo || mIa.estado == Estado::Atordoado;
        Cara(olho, tonto ? "Assustado" : (dx < -40.0f ? "OlhoEsquerda" : (dx > 40.0f ? "OlhoDireita" : "Olho")));
        // Fugindo, o olho e moeda tambem: o tiro passa.
        olho->DefinirEnfeite(!machuca);
    }

    // Fugindo, cada bloco que o aluno encosta e dele.
    if (fugindo) {
        for (BossProjectile*& bloco : mCorpo) {
            if (!bloco) continue;
            for (const Player* a : alunos) {
                if ((bloco->GetPosition() - a->GetPosition()).Length() > RaioDe(a) + kToque) continue;
                if (b) b->Faiscar(bloco->GetPosition(), Hud::Faisca::Coleta);
                mPremioDoInimigo += kPremioPorBloco;
                Matar(bloco);
                break;
            }
        }
        if (mPremioDoInimigo >= 0.75f) {
            if (b) b->Conquistar(mPremioDoInimigo);
            mPremioDoInimigo = 0.0f;
        }
    }

    // A IA a mostra, como no modo debug de quem faz o jogo: o raio de visao na ronda, a linha ate o
    // alvo na perseguicao, o tempo do power-up na fuga, as estrelas do atordoado.
    const Vector2 eu = mCentroDoInimigo;
    if (mIa.estado == Estado::Patrulha) {
        for (int k = 0; k < 40; k += 2) {
            const float a0 = Math::TwoPi * static_cast<float>(k) / 40.0f, a1 = Math::TwoPi * static_cast<float>(k + 1) / 40.0f;
            Linha(eu + Vector2(Math::Cos(a0), Math::Sin(a0)) * g.raioDeVisao, eu + Vector2(Math::Cos(a1), Math::Sin(a1)) * g.raioDeVisao,
                  kMagentaFraco);
        }
    } else if (mIa.estado == Estado::Persegue && alvo) {
        Tracejada(eu, ondeAluno, kMagentaFraco, 10.0f);
    } else if (fugindo) {
        const float u = std::clamp(mIa.susto / std::max(g.susto, 0.1f), 0.0f, 1.0f);
        const Vector2 a(eu.x - kMeiaLargura, eu.y + kMeiaAltura + 10.0f), z(eu.x + kMeiaLargura, eu.y + kMeiaAltura + 16.0f);
        Retangulo(a, z, kOuro);
        Preencher(a, Vector2(Mistura(a.x, z.x, u), z.y), kOuro);
        for (const Player* aluno : alunos) Circulo(aluno->GetPosition(), 24.0f + 2.0f * Math::Sin(mRelogio * 10.0f), kOuro, 16);
    } else if (mIa.estado == Estado::Atordoado) {
        for (int k = 0; k < 3; ++k) {
            const float a = mRelogio * 5.0f + Math::TwoPi * static_cast<float>(k) / 3.0f;
            Circulo(eu + Vector2(Math::Cos(a) * 46.0f, -kMeiaAltura - 8.0f + Math::Sin(a) * 9.0f), 5.0f, kOuro, 6);
        }
    } else if (mIa.estado == Estado::Derrotado) {
        // O respawn: o contorno do corpo no canto onde ele vai nascer.
        const Vector2 ninho = ronda[static_cast<size_t>(Ninho(campo, ondeAluno))];
        if (Aceso(te, 0.25f)) {
            const Vector2 a = ninho - Vector2(kMeiaLargura, kMeiaAltura), z = ninho + Vector2(kMeiaLargura, kMeiaAltura);
            Tracejada(a, Vector2(z.x, a.y), kMagenta);
            Tracejada(Vector2(z.x, a.y), z, kMagenta);
            Tracejada(z, Vector2(a.x, z.y), kMagenta);
            Tracejada(Vector2(a.x, z.y), a, kMagenta);
        }
    }
    if (Vivo(mPoder)) Circulo(mPoder->GetPosition(), 20.0f + 4.0f * Math::Sin(mRelogio * 8.0f), kOuro, 14);

    // O estado em cima dele (ou o "!" e o "?" de um instante), e o rotulo do power-up.
    const bool comCorpo = mCorpoMontado && mIa.estado != Estado::Derrotado;
    EtiquetaDeCodigo* estado = Etiqueta(mEtiquetasDoInimigo, 0, 150, 26);
    if (estado && comCorpo) {
        const bool marca = mRelogio < mMarcaAte && !mMarca.empty();
        const SDL_Color tinta = marca || mIa.estado == Estado::Persegue || mIa.estado == Estado::Alerta
                                    ? SDL_Color{255, 110, 110, 255}
                                    : (fugindo ? SDL_Color{255, 214, 90, 255}
                                               : (mIa.estado == Estado::Atordoado ? SDL_Color{255, 240, 150, 255}
                                                                                   : SDL_Color{150, 255, 170, 255}));
        estado->Pintar(SDL_Color{255, 60, 230, 160}, SDL_Color{10, 12, 24, 215}, tinta);
        Escrever(estado, marca ? mMarca : std::string(Nome(mIa.estado)), eu + Vector2(0.0f, -kMeiaAltura - 22.0f));
    } else if (estado) {
        estado->Mostrar(false);
    }
    EtiquetaDeCodigo* rotulo = Etiqueta(mEtiquetasDoInimigo, 1, 120, 24);
    if (rotulo && Vivo(mPoder)) {
        rotulo->Pintar(SDL_Color{255, 214, 90, 160}, SDL_Color{10, 12, 24, 215}, SDL_Color{255, 214, 90, 255});
        Escrever(rotulo, "POWER-UP", mPoder->GetPosition() + Vector2(0.0f, -34.0f));
    } else if (rotulo) {
        rotulo->Mostrar(false);
    }

    // Na fase 1, o professor continua: os leques, menos que no resto da prova. Nas regras, a etapa
    // e quem atira, e o placar e dela.
    if (!como.fase1) return;
    if (te >= 1.0f && te >= mProximoLeque) {
        mProximoLeque = te + mDegrauDoHud.entreLeques * 1.4f;
        Leque(HudHostil::kGatilhoDoLeque, mDegrauDoHud.tirosDoLeque, 26.0f, mDegrauDoHud.velocidade, "Tiro");
    }

    std::string texto;
    switch (mIa.estado) {
        case Estado::Nascendo:  texto = "new Inimigo(): a IA dele \xc3\xa9 uma m\xc3\xa1quina de estados"; break;
        case Estado::Patrulha:  texto = "PATRULHA: fique fora do raio de vis\xc3\xa3o dele"; break;
        case Estado::Alerta:    texto = "ALERTA (!): ele te viu - CORRA!"; break;
        case Estado::Persegue:
            texto = Vivo(mPoder) ? "PERSEGUE: pegue o POWER-UP!" : "PERSEGUE: atire no OLHO dele, o ponto fraco";
            break;
        case Estado::Atordoado: texto = "ATORDOADO: o olho era o ponto fraco"; break;
        case Estado::Fugindo:
            texto = pisca || mIa.susto <= kPiscaNoFim ? "FUGINDO... o power-up est\xc3\xa1 acabando!"
                                                      : "FUGINDO: encoste nos blocos - cada um \xc3\xa9 nota!";
            break;
        case Estado::Derrotado:
            texto = "inimigo.destruir(): respawn em " + std::to_string(static_cast<int>(std::ceil(kRespawn - mIa.noEstado)));
            break;
    }
    Placar(HudHostil::kFase, HudHostil::kEtapaDoInimigo, texto);
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
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    // O fantasma da fase 1 volta e passa de uma etapa para a outra; o save fica sem ele.
    if (etapa >= kInimigoAte && mInimigoFica && mCorpoMontado) {
        if (Battle* b = Batalha()) {
            for (BossProjectile* bloco : mCorpo) {
                if (Vivo(bloco)) b->Faiscar(bloco->GetPosition(), Hud::Faisca::Dissolvido);
            }
        }
        Avisar("inimigo.desativar(): o fantasma sai de cena - por enquanto", 1.4f);
    }
    mInimigoFica = etapa < kInimigoAte;
    TrocouDeEtapa(etapa);
    Varrer(t, 0.32f, 8.0f);
    if (etapa == kEtapaDaInversao) AtualizarInversao(te);
    else if (etapa == kEtapaDoFogoAmigo) AtualizarFogoAmigo(te);
    else if (etapa == kEtapaDoFps) AtualizarFps(te, deltaTime);
    else AtualizarSaveLoad(te, deltaTime);
    if (etapa < kInimigoAte) {
        ComoOInimigo como;
        como.fase1 = false;
        // Invertidas as regras, o tiro e moeda - e o fantasma tambem: ele foge ate elas voltarem.
        if (etapa == kEtapaDaInversao && RegraEm(te) == Regra::Invertida && mIa.estado != IaDoInimigo::Estado::Fugindo) {
            como.susto = std::max(0.5f, kVoltou - te);
        }
        // No FPS, ele anda sem deltaTime.
        if (etapa == kEtapaDoFps) como.passo = SemDeltaTime(mFps);
        AtualizarInimigo(t, deltaTime, como);
    }
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

float Ferreira::SeteSegmentos(const std::string& texto, const Vector2 canto, const float altura, const SDL_Color cor) {
    // Cada caractere tem meia altura de largura e um traco de um oitavo dela.
    const float w = altura * 0.5f, e = std::max(2.0f, altura / 8.0f), meio = altura / 2.0f;
    float x = canto.x;
    const float y = canto.y;
    for (const char ch : texto) {
        if (ch == ' ') {
            x += w * 0.6f;
            continue;
        }
        if (ch == '.') {
            Preencher(Vector2(x, y + altura - e), Vector2(x + e, y + altura), cor);
            x += altura / 4.0f;
            continue;
        }
        const auto s = RegrasTrocadas::Segmentos(ch);
        if (s[0]) Preencher(Vector2(x + e, y), Vector2(x + w - e, y + e), cor);
        if (s[1]) Preencher(Vector2(x + w - e, y + e), Vector2(x + w, y + meio - e / 2.0f), cor);
        if (s[2]) Preencher(Vector2(x + w - e, y + meio + e / 2.0f), Vector2(x + w, y + altura - e), cor);
        if (s[3]) Preencher(Vector2(x + e, y + altura - e), Vector2(x + w - e, y + altura), cor);
        if (s[4]) Preencher(Vector2(x, y + meio + e / 2.0f), Vector2(x + e, y + altura - e), cor);
        if (s[5]) Preencher(Vector2(x, y + e), Vector2(x + e, y + meio - e / 2.0f), cor);
        if (s[6]) Preencher(Vector2(x + e, y + meio - e / 2.0f), Vector2(x + w - e, y + meio + e / 2.0f), cor);
        x += w + e * 1.6f;
    }
    return x - canto.x;
}

void Ferreira::AtualizarFps(const float te, const float deltaTime) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const SDL_FRect c = Campo();
    const std::vector<Player*> alunos = Jogadores();

    // O FPS e do aluno: quem segura o foco liga o modo economia. Em dupla, ele so cai inteiro com os
    // dois focados.
    int focados = 0;
    for (const Player* a : alunos) focados += a->Focado() ? 1 : 0;
    const float fracao = alunos.empty() ? 0.0f : static_cast<float>(focados) / static_cast<float>(alunos.size());
    // Quem solta o foco (depois de segura-lo um pouco) arranca: o impulso para correr ate o vao.
    for (size_t i = 0; i < alunos.size() && i < mArranques.size(); ++i) {
        if (Impulsiona(mArranques[i], alunos[i]->Focado(), deltaTime)) {
            alunos[i]->Impulsionar(kImpulso, kFatorDoImpulso);
            if (Battle* b = Batalha()) b->Faiscar(alunos[i]->GetPosition(), Hud::Faisca::Coleta);
        }
    }
    // Soltar o foco tem um respiro: o FPS segura kFolgaAoSoltar antes de subir.
    const float valendo = FocadosComFolga(mFolgaDoFoco, fracao, deltaTime);
    if (te >= kComecaOFps) mFps = PassoDoFps(mFps, FpsAlvo(valendo, g.fpsMaximo), g.fpsMaximo, deltaTime);
    const float fator = SemDeltaTime(mFps);
    mRelogioSemDt += deltaTime * fator;
    if (te >= mProximaAmostra) {
        mProximaAmostra = te + 0.05f;
        mHistoricoDoFps.push_back(mFps);
        if (mHistoricoDoFps.size() > kAmostrasDoGrafico) mHistoricoDoFps.erase(mHistoricoDoFps.begin());
    }

    // O vermelho: a espiral de dois bracos, feita sem deltaTime - dispara e gira pelo relogio dela, e
    // anda o que esta escrito nela vezes o FPS. O desenho e sempre o mesmo; so o tempo dele muda.
    if (te >= kComecaOFps) {
        if (mProximaEspiral <= 0.0f) mProximaEspiral = mRelogioSemDt;
        for (int k = 0; k < 4 && mRelogioSemDt >= mProximaEspiral; ++k) {
            mProximaEspiral += g.entreEspirais;
            mGiroDaEspiral = std::fmod(mGiroDaEspiral + g.giroDaEspiral, 360.0f);
            auto par = DispararEm(kGatilhoDoVermelho, GetPosition() + Vector2(0.0f, 30.0f), 2, mGiroDaEspiral, 180.0f,
                                  g.velocidadeVermelha);
            DarVelocidade(par, mGiroDaEspiral, 180.0f, g.velocidadeVermelha);
            for (BossProjectile* p : par) {
                Cara(p, "SemDt");
                mVermelhos.push_back({p, VelocidadeDe(p)});
            }
        }
    }
    // A cada quadro o vermelho anda o que o FPS manda, e o rastro diz o quanto.
    for (Vermelho& v : mVermelhos) {
        if (!Vivo(v.p)) continue;
        Andar(v.p, v.v * fator);
        const float escrita = v.v.Length();
        if (escrita < 1.0f) continue;
        const Vector2 onde = v.p->GetPosition();
        Linha(onde - v.v * (std::min(70.0f, escrita * fator * 0.08f) / escrita), onde, kRastro);
    }

    // O azul: a parede com um vao, que desce com deltaTime - na mesma velocidade em qualquer FPS. Para
    // chegar ao vao e preciso correr, e quem corre solta o foco.
    if (te >= kComecaOFps + 0.8f && te >= mProximaParede) {
        mProximaParede = te + g.entreParedes;
        // O vao novo: longe do anterior (focado nao se chega), mas ao alcance de quem solta o foco.
        const float longe = c.w / 3.0f;
        mVaoDaParede = VaoDaParede(mSorteio, mVaoDaParede, c.x + 10.0f, c.x + c.w - 10.0f, longe,
                                   VaoAlcancavel(kAlunoSemFoco, g.entreParedes, longe));
        for (float x = c.x + kEntreTirosDaParede / 2.0f; x < c.x + c.w; x += kEntreTirosDaParede) {
            if (std::fabs(x - mVaoDaParede) < kVaoDaParede / 2.0f) continue;
            Um(kGatilhoDoAzul, Vector2(x, c.y + 150.0f), "ComDt", Vector2(0.0f, g.velocidadeAzul));
        }
        SomDoDisparo();
    }

    // Quem segura o foco ganha o anel frio: o jogo esta em camera lenta por causa dele. Quem arranca
    // deixa um rastro de velocidade, para tras do rumo.
    for (const Player* a : alunos) {
        if (a->Focado()) Circulo(a->GetPosition(), 24.0f + 2.0f * Math::Sin(mRelogio * 6.0f), kFrio, 16);
        if (!a->Impulsionado()) continue;
        const auto* corpo = a->GetComponent<RigidBodyComponent>();
        const Vector2 v = corpo ? corpo->GetVelocity() : Vector2::Zero;
        if (v.Length() < 1.0f) continue;
        const Vector2 rumo = v * (1.0f / v.Length()), lado(-rumo.y, rumo.x);
        for (int k = -1; k <= 1; ++k) {
            const Vector2 de = a->GetPosition() + lado * (9.0f * static_cast<float>(k)) - rumo * 12.0f;
            Linha(de, de - rumo * 34.0f, kFrio);
        }
    }

    // O contador, no canto de cima a direita, como o de quem mede o jogo: frio em camera lenta, quente
    // quando o vermelho corre. Embaixo, o grafico dos ultimos segundos.
    const float u = NoGrafico(mFps, g.fpsMaximo);
    const auto mistura = [](const SDL_Color a, const SDL_Color b, const float k) {
        const float q = std::clamp(k, 0.0f, 1.0f);
        const auto m = [q](const Uint8 x, const Uint8 y) {
            return static_cast<Uint8>(std::lround(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * q));
        };
        return SDL_Color{m(a.r, b.r), m(a.g, b.g), m(a.b, b.b), m(a.a, b.a)};
    };
    const SDL_Color morno{240, 244, 255, 240}, quente{255, 90, 70, 245};
    const SDL_Color cor = u < 0.5f ? mistura(kFrio, morno, u * 2.0f) : mistura(morno, quente, (u - 0.5f) * 2.0f);
    const Vector2 canto(c.x + c.w - 262.0f, c.y + 18.0f);
    // O contador vai na camada de cima: o professor passa por baixo dele.
    DrawLinhasComponent* doCampo = mLinhas;
    if (mLinhasDoContador) mLinhas = mLinhasDoContador;
    Preencher(canto - Vector2(12.0f, 10.0f), canto + Vector2(250.0f, 112.0f), SDL_Color{8, 10, 20, 235});
    Retangulo(canto - Vector2(12.0f, 10.0f), canto + Vector2(250.0f, 112.0f), SDL_Color{cor.r, cor.g, cor.b, 120});
    const float largura = SeteSegmentos(std::to_string(static_cast<int>(std::lround(mFps))), canto, 46.0f, cor);
    SeteSegmentos("FPS", canto + Vector2(largura + 10.0f, 20.0f), 26.0f, cor);
    const Vector2 g0(canto.x, canto.y + 60.0f), g1(canto.x + 238.0f, canto.y + 100.0f);
    Retangulo(g0, g1, SDL_Color{150, 160, 200, 110});
    const float y60 = g1.y - (g1.y - g0.y) * NoGrafico(kFpsDeReferencia, g.fpsMaximo);
    Tracejada(Vector2(g0.x, y60), Vector2(g1.x, y60), SDL_Color{240, 244, 255, 80}, 6.0f);
    const float passo = (g1.x - g0.x) / static_cast<float>(kAmostrasDoGrafico - 1);
    for (size_t k = 1; k < mHistoricoDoFps.size(); ++k) {
        const float a0 = g1.y - (g1.y - g0.y) * NoGrafico(mHistoricoDoFps[k - 1], g.fpsMaximo);
        const float a1 = g1.y - (g1.y - g0.y) * NoGrafico(mHistoricoDoFps[k], g.fpsMaximo);
        Linha(Vector2(g0.x + passo * static_cast<float>(k - 1), a0), Vector2(g0.x + passo * static_cast<float>(k), a1), cor);
    }
    mLinhas = doCampo;

    if (te < kComecaOFps) Placar(kFase, kEtapaDoFps, "fps.limite = off: segure o FOCO e o jogo cai para 30 FPS");
    else Placar(kFase, kEtapaDoFps, "VERMELHO sem deltaTime: corre com o FPS. AZUL com deltaTime: n\xc3\xa3o muda");
}

void Ferreira::AtualizarSaveLoad(const float te, const float deltaTime) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const JogosDigitais::Campo campo = CampoPuro();
    const std::vector<Player*> alunos = Jogadores();

    if (te >= 0.5f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneisDoMotor;
        for (BossProjectile* p : Anel(kGatilhoDoAnel, g.tirosDoAnelDoMotor, g.velocidadeDoMotor, "Tiro")) mTiros.push_back(p);
    }
    if (te >= 0.9f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLequesDoMotor;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, g.tirosDoLequeDoMotor, 28.0f, g.velocidadeDoMotor + 70.0f, "Tiro")) mTiros.push_back(p);
    }

    const float agora = Nota();
    // Onde o disquete anda: o campo sem a faixa do professor, com a folga do raio dele. A fuga conhece
    // essas bordas, e escorrega por elas.
    const float folga = kRaioDoSave + 4.0f;
    const JogosDigitais::Campo area{campo.x + folga, campo.y + 150.0f, campo.w - 2.0f * folga, campo.h - 150.0f - folga};
    // O save: a nota de agora e onde cada aluno esta. O disquete nasce um pouco acima do aluno.
    if (!mSalvo.tem && HoraDeSalvar(te, mUltimoLoad)) {
        mSalvo = Salvo{};
        mFuga = RegrasTrocadas::Fuga{};
        mSalvo.tem = true;
        mSalvo.nota = agora;
        for (const Player* a : alunos) mSalvo.alunos.push_back(a->GetPosition());
        const Vector2 aluno = alunos.empty() ? PosicaoDoAluno() : alunos.front()->GetPosition();
        Matar(mSave);
        mSave = Peca(kGatilhoDoDisquete, JogosDigitais::Prender(aluno + Vector2(0.0f, -110.0f), area, 20.0f, 20.0f), "Save");
        if (mSave) mSave->DefinirEnfeite(true);
        mClarao = 0.2f;
        mCorDoClarao = kSave;
        Avisar("quicksave(): o save guardou a nota " + Inteiro(mSalvo.nota), 1.4f);
        SomDoDisparo();
    }
    if (mSalvo.tem && !mSave) mSalvo.tem = false;   // o disquete sumiu por outro caminho

    if (mSalvo.tem) {
        mSalvo.ha += deltaTime;
        // O aluno mais perto: e dele que o save foge, ou e ele que o save persegue.
        const Player* alvo = nullptr;
        float distancia = 1e9f;
        for (const Player* a : alunos) {
            const float d = (a->GetPosition() - mSave->GetPosition()).Length();
            if (d < distancia) {
                distancia = d;
                alvo = a;
            }
        }
        const Humor humor = HumorDoSave(mSalvo.nota, agora);
        const Vector2 eu = mSave->GetPosition();
        const Vector2 ondeAluno = alvo ? alvo->GetPosition() : PosicaoDoAluno();
        // O tiro do aluno que acerta o disquete fica nele: o drive engasga (RegrasTrocadas::PassoDoSave).
        int tiros = 0;
        if (const Battle* batalha = Batalha(); batalha && batalha->GerenteDeProjeteis()) {
            for (const auto& tiro : batalha->GerenteDeProjeteis()->GetPlayerProjectiles()) {
                if (!tiro || tiro->GetState() != ActorState::Active || tiro->Murchando()) continue;
                if ((tiro->GetPosition() - eu).Length() > kRaioDoSave + 6.0f) continue;
                ++tiros;
                tiro->Kill();
            }
        }
        // A fuga e vencivel: encurralado num canto ele trava, e o tiro o faz andar aos trancos.
        const Vector2 passo = mSalvo.ha >= kGravando
                                  ? PassoDoSave(mFuga, humor, eu, ondeAluno, area, RapidezDoSave(mSalvo.nota, agora), tiros, deltaTime)
                                  : Vector2::Zero;
        mSave->SetPosition(JogosDigitais::Prender(eu + passo, area, 0.0f, 0.0f));
        Parar(mSave);
        // Gravando, ele pisca; depois, a cara diz o humor: verde foge (vale mais), vermelho persegue.
        const bool gravando = mSalvo.ha < kGravando;
        Cara(mSave, gravando ? (Aceso(mSalvo.ha, 0.1f) ? "Save" : "Foge")
                             : (humor == Humor::Foge ? "Foge" : (humor == Humor::Persegue ? "Persegue" : "Save")));

        // Onde o load leva: o lugar de cada aluno no save, e a linha ate ele.
        for (size_t i = 0; i < mSalvo.alunos.size(); ++i) {
            const Vector2 onde = mSalvo.alunos[i];
            Circulo(onde, 18.0f, kSave, 14);
            Tracejada(onde + Vector2(-12.0f, 0.0f), onde + Vector2(12.0f, 0.0f), kSave, 4.0f);
            Tracejada(onde + Vector2(0.0f, -12.0f), onde + Vector2(0.0f, 12.0f), kSave, 4.0f);
        }
        const SDL_Color cor = humor == Humor::Foge ? SDL_Color{110, 230, 150, 220}
                                                   : (humor == Humor::Persegue ? SDL_Color{255, 100, 100, 220} : kSave);
        Circulo(mSave->GetPosition(), 26.0f + 2.0f * Math::Sin(mRelogio * 9.0f), cor, 16);
        if (mFuga.travado > 0.0f) {
            // Encurralado: os cantos de uma gaiola em volta dele - a hora de pega-lo.
            const Vector2 s = mSave->GetPosition();
            for (int k = 0; k < 4; ++k) {
                const float sx = k % 2 == 0 ? -1.0f : 1.0f, sy = k < 2 ? -1.0f : 1.0f;
                const Vector2 q = s + Vector2(sx * 32.0f, sy * 32.0f);
                Linha(q, q - Vector2(sx * 13.0f, 0.0f), kOuro);
                Linha(q, q - Vector2(0.0f, sy * 13.0f), kOuro);
            }
        } else if (mFuga.engasgo > 0.0f) {
            // Engasgado: o drive lendo, um arco que gira em volta.
            const Vector2 s = mSave->GetPosition();
            for (int k = 0; k < 6; ++k) {
                const float a0 = mRelogio * 14.0f + static_cast<float>(k) * 0.35f;
                Linha(s + Vector2(Math::Cos(a0), Math::Sin(a0)) * 32.0f, s + Vector2(Math::Cos(a0 + 0.3f), Math::Sin(a0 + 0.3f)) * 32.0f,
                      kSave);
            }
        }

        // A nota que ele guarda, em cima dele, na cor do humor.
        if (EtiquetaDeCodigo* e = Etiqueta(mEtiquetasDoSave, 0, 110, 24)) {
            e->Pintar(cor, SDL_Color{10, 12, 24, 215}, SDL_Color{cor.r, cor.g, cor.b, 255});
            Escrever(e, "SAVE " + Inteiro(mSalvo.nota), mSave->GetPosition() + Vector2(0.0f, -36.0f));
        }

        // O toque e o quick load.
        if (!gravando) {
            for (const Player* a : alunos) {
                if (!Encostou(mSave->GetPosition(), a->GetPosition(), RaioDe(a))) continue;
                Carregar(te);
                break;
            }
        }
        if (mSalvo.tem) {
            const std::string notas = "o save tem " + Inteiro(mSalvo.nota) + ", voc\xc3\xaa tem " + Inteiro(agora);
            switch (humor) {
                case Humor::Foge: Placar(kFase, 3, notas + ": ele FOGE - encurrale num canto, ou atire nele!"); break;
                case Humor::Persegue: Placar(kFase, 3, notas + ": ele te PERSEGUE - fuja!"); break;
                case Humor::Parado: Placar(kFase, 3, notas + ": ele espera"); break;
            }
        }
    }
    if (!mSalvo.tem) {
        Esconder(mEtiquetasDoSave);
        Placar(kFase, 3, te > kUltimoSave ? "sem mais saves: s\xc3\xb3 desvie" : "quicksave() a caminho: o save vai guardar a sua nota");
    }
}

void Ferreira::Carregar(const float te) {
    Battle* b = Batalha();
    const std::vector<Player*> alunos = Jogadores();
    for (size_t i = 0; i < alunos.size() && i < mSalvo.alunos.size(); ++i) {
        alunos[i]->SetPosition(mSalvo.alunos[i]);
        alunos[i]->Proteger(0.6f);
    }
    mFuga = RegrasTrocadas::Fuga{};
    const float antes = Nota();
    if (b) b->DefinirNota(mSalvo.nota);
    Matar(mSave);
    mSalvo.tem = false;
    mUltimoLoad = te;
    Esconder(mEtiquetasDoSave);
    mClarao = 0.25f;
    mCorDoClarao = SDL_Color{255, 255, 255, 255};
    Avisar("quickload(): a nota foi de " + Inteiro(antes) + " para " + Inteiro(mSalvo.nota), 1.6f);
    SomDoDisparo();
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
    else if (etapa == 2) AtualizarFrota(te, deltaTime);
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

void Ferreira::AtualizarFrota(const float te, const float deltaTime) {
    using namespace Lancamento;
    const Degrau& g = mDegrauDoLancamento;
    const SDL_FRect c = Campo();
    Battle* b = Batalha();
    const Vector2 topo(c.x + c.w / 2.0f, c.y + 170.0f);
    const float folga = (c.w - static_cast<float>(kColunasDaFrota - 1) * kEspacoX) / 2.0f - 40.0f;
    const int total = kFileiras * kColunasDaFrota;
    const auto indice = [](const int fileira, const int coluna) { return static_cast<size_t>(fileira * kColunasDaFrota + coluna); };
    const Vector2 eu = GetPosition();
    const std::string onda = std::to_string(mOndaDaFrota + 1);

    // A chegada de uma onda: os lugares piscam e os bugs aparecem - ja com o god mode ligado.
    if (!mFrotaMontada && !mJanelaAberta) {
        mGodMode = true;
        if (mFrotaChegaEm < 0.0f) mFrotaChegaEm = te + kChegadaDaFrota;
        if (Aceso(te, 0.12f)) {
            for (int f = 0; f < kFileiras; ++f) {
                for (int col = 0; col < kColunasDaFrota; ++col) Circulo(LugarNaFrota(f, col, topo, Marcha{}), 12.0f, kDebugFraco, 8);
            }
        }
        if (te >= mFrotaChegaEm) {
            mFrotaMontada = true;
            mPassosDaFrota = 0.0f;
            mProximoHotfix = te + g.hotfix;
            mFrota.assign(static_cast<size_t>(total), nullptr);
            for (int f = 0; f < kFileiras; ++f) {
                for (int col = 0; col < kColunasDaFrota; ++col) {
                    mFrota[indice(f, col)] = Peca(kGatilhoDoBug, LugarNaFrota(f, col, topo, Marcha{}), "Bug");
                }
            }
            if (mOndaDaFrota > 0) Avisar("new Wave(" + onda + "): mais r\xc3\xa1pida, e o god mode de volta", 1.6f);
            SomDoDisparo();
        }
        Placar(kFase, 2, mOndaDaFrota == 0 ? "genero = nave: os bugs do lan\xc3\xa7" "amento chegaram"
                                           : "onda " + onda + " a caminho");
        return;
    }

    // A marcha: de um lado ao outro, um degrau para baixo a cada batida na borda - e mais rapida quanto
    // menos bugs sobram, e a cada onda.
    int vivos = 0;
    for (const BossProjectile* p : mFrota) vivos += p ? 1 : 0;
    if (vivos > 0) mPassosDaFrota += deltaTime / EntrePassos(g.entrePassos, vivos, total, mOndaDaFrota);
    const Marcha m = MarchaEm(static_cast<int>(mPassosDaFrota), folga);
    std::vector<size_t> deBaixo;
    std::vector<int> vazios;
    for (int col = 0; col < kColunasDaFrota; ++col) {
        bool achou = false;
        for (int f = kFileiras - 1; f >= 0; --f) {
            BossProjectile* p = mFrota[indice(f, col)];
            if (!p) {
                vazios.push_back(static_cast<int>(indice(f, col)));
                continue;
            }
            p->SetPosition(LugarNaFrota(f, col, topo, m));
            p->RenovarTempoDeVida();
            Parar(p);
            if (!achou) deBaixo.push_back(indice(f, col));
            achou = true;
        }
    }
    // O bug de baixo de uma coluna solta um tiro.
    if (!deBaixo.empty() && te >= mProximaQueda) {
        mProximaQueda = te + g.entreQuedas * std::pow(0.85f, static_cast<float>(mOndaDaFrota));
        const size_t qual = deBaixo[static_cast<size_t>(mSorteio.Inteiro(0, static_cast<int>(deBaixo.size()) - 1))];
        Um(kGatilhoDaQueda, mFrota[qual]->GetPosition() + Vector2(0.0f, 18.0f), "Tiro", Vector2(0.0f, g.velocidadeDaQueda));
    }

    // O god mode: cada bug vivo o segura por um fio, e a gaiola do modo debug fica em volta dele.
    EtiquetaDeCodigo* rotulo = Etiqueta(mEtiquetasDaFrota, 0, 180, 26);
    if (vivos > 0) {
        mGodMode = true;
        for (const BossProjectile* p : mFrota) {
            if (p) Tracejada(eu + Vector2(0.0f, 30.0f), p->GetPosition(), kDebugFraco, 7.0f);
        }
        Circulo(eu, 74.0f + 3.0f * Math::Sin(mRelogio * 7.0f), kDebug, 28);
        Circulo(eu, 80.0f, kDebugFraco, 28);
        // Em god mode ele protege a frota: um leque mirado de tanto em tanto, mais rapido a cada onda.
        if (te >= mProximoLeque) {
            mProximoLeque = te + g.entreLequesDaFrota * std::pow(0.85f, static_cast<float>(mOndaDaFrota));
            Leque(kGatilhoDoTiro, g.tirosDoLequeDaFrota, 34.0f, g.velocidade + 20.0f, "Tiro");
        }
        if (rotulo) {
            rotulo->Pintar(SDL_Color{120, 255, 120, 160}, SDL_Color{10, 12, 24, 215}, SDL_Color{150, 255, 170, 255});
            Escrever(rotulo, "godMode = true", eu + Vector2(eu.x > Campo().x + Campo().w - 260.0f ? -170.0f : 170.0f, 10.0f));
        }
    }

    // O hotfix: um lugar vazio pisca, o fio do professor vai ate ele, e o bug volta.
    if (vivos > 0 && vivos < total && mHotfixLugar < 0 && te >= mProximoHotfix) {
        mHotfixLugar = LugarDoHotfix(vazios, mSorteio);
        mHotfixEm = te + kAvisoDoHotfix;
    }
    if (mHotfixLugar >= 0) {
        const int f = mHotfixLugar / kColunasDaFrota, col = mHotfixLugar % kColunasDaFrota;
        const Vector2 onde = LugarNaFrota(f, col, topo, m);
        if (vivos == 0 || mFrota[static_cast<size_t>(mHotfixLugar)]) {
            mHotfixLugar = -1;   // a frota acabou antes do patch: ele nao tem onde aplicar
        } else if (te < mHotfixEm) {
            Tracejada(eu + Vector2(0.0f, 30.0f), onde, kMagenta, 6.0f);
            if (Aceso(te, 0.07f)) Circulo(onde, 16.0f, kMagenta, 10);
        } else {
            mFrota[static_cast<size_t>(mHotfixLugar)] = Peca(kGatilhoDoBug, onde, "Bug");
            if (b) b->Faiscar(onde, Hud::Faisca::Coleta);
            mHotfixLugar = -1;
            mProximoHotfix = te + g.hotfix;
            ++vivos;
        }
    }

    // Zerada a frota, o god mode cai: a janela.
    if (vivos == 0 && !mJanelaAberta) {
        mJanelaAberta = true;
        mGodMode = false;
        mJanelaAte = te + g.janela;
        if (b) b->Conquistar(kPremioDaFrotaLimpa);
        mClarao = 0.2f;
        mCorDoClarao = kOuro;
        Avisar("0 bugs: godMode = false - ATIRE NELE, cada acerto vale o dobro!", 1.8f);
        SomDoDisparo();
    }
    if (mJanelaAberta) {
        // Sem frota, ele se defende atirando; a barra embaixo dele e o tempo da janela.
        if (te >= mProximoLeque) {
            mProximoLeque = te + 0.9f;
            Leque(kGatilhoDoTiro, 4, 30.0f, g.velocidade + 40.0f, "Tiro");
        }
        const float u = std::clamp((mJanelaAte - te) / std::max(g.janela, 0.1f), 0.0f, 1.0f);
        const Vector2 a(eu.x - 70.0f, eu.y + 66.0f), z(eu.x + 70.0f, eu.y + 74.0f);
        Retangulo(a, z, kOuro);
        Preencher(a, Vector2(Mistura(a.x, z.x, u), z.y), kOuro);
        if (Aceso(te, 0.15f)) Circulo(eu, 70.0f, kPerigo, 24);
        if (rotulo) {
            rotulo->Pintar(SDL_Color{255, 214, 90, 160}, SDL_Color{10, 12, 24, 215}, SDL_Color{255, 214, 90, 255});
            Escrever(rotulo, "godMode = false", eu + Vector2(eu.x > Campo().x + Campo().w - 260.0f ? -170.0f : 170.0f, 10.0f));
        }
        if (te >= mJanelaAte) {
            // A onda seguinte: mais rapida, e o god mode de volta.
            mJanelaAberta = false;
            mFrotaMontada = false;
            mFrotaChegaEm = -1.0f;
            ++mOndaDaFrota;
            mGodMode = true;
        }
        Placar(kFase, 2, "godMode = false: ATIRE NO PROFESSOR (x2)!");
        return;
    }
    Placar(kFase, 2, "godMode = true enquanto houver bug: faltam " + std::to_string(vivos) + " (onda " + onda + ")");
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
    // Um olho do inimigo, o ponto fraco: ele fica atordoado (a IA percebe no quadro seguinte).
    for (BossProjectile*& o : mOlhos) {
        if (o != qual) continue;
        o = nullptr;
        mOlhoAtingido = true;
        if (b) b->Conquistar(IaDoInimigo::kPremioPorOlho);
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
