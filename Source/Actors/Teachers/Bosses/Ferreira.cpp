//
// O chefe da INF 216 - Projeto e Implementacao de Jogos Digitais, a optativa secreta. O que as
// fases dividem mora aqui; cada fase, no seu arquivo (FerreiraInterface, FerreiraFisica,
// FerreiraRegras e FerreiraLancamento).
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

#include "FerreiraComum.h"

using namespace FerreiraComum;

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

float Ferreira::AnguloAteOAluno(const Vector2 de) const {
    const Vector2 d = PosicaoDoAluno() - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
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
    for (CaixaDaCamera& cx : mCaixasDaCamera) Matar(cx.p);
    mCaixasDaCamera.clear();
    mProximaCaixa = 0.0f;
    mCamera = Vector2::Zero;
    mDeslizaDe.clear();
    mCameraAvisada = false;
    mCameraLigada = false;
    Esconder(mEtiquetasDaCamera);
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
