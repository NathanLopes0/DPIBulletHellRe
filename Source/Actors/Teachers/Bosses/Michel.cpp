//
// O chefe da INF 429 - Visao Computacional, a segunda optativa.
//

#include "Michel.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

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

namespace {
    /// Um numero com virgula: 1,32.
    std::string Decimal(const float x, const int casas) {
        char b[32];
        std::snprintf(b, sizeof b, "%.*f", casas, static_cast<double>(x));
        std::string s(b);
        for (char& ch : s) {
            if (ch == '.') ch = ',';
        }
        return s;
    }

    std::string Porcento(const float x) {
        return std::to_string(static_cast<int>(std::lround(std::clamp(x, 0.0f, 1.0f) * 100.0f))) + "%";
    }

    /// Os objetos da cena da fase 1, e a imagem de cada um (a mesma figura, de cabeca para baixo).
    constexpr std::array<const char*, 4> kObjetos = {"Casa", "Arvore", "Pessoa", "Carro"};
    constexpr std::array<const char*, 4> kImagens = {"CasaInvertida", "ArvoreInvertida", "PessoaInvertida",
                                                     "CarroInvertido"};
}

Michel::Michel(Scene* scene) : Boss(scene) {
}

void Michel::SortearALuta() {
    mLutaSorteada = true;
    mSorteio = SorteioDaFase(CameraEscura::kFase);
    const Dificuldade::Nivel n = NivelDeDificuldade();
    mDegrauDaCamera = CameraEscura::DegrauDo(n);
    mDegrauDaConvolucao = Convolucao::DegrauDo(n);
    mDegrauDaRede = RedeNeural::DegrauDo(n);
    mDegrauDoLaboratorio = Laboratorio::DegrauDo(n);
}

// ---- O que as fases dividem ----

BossProjectile* Michel::Um(const char* gatilho, const Vector2 onde, const std::string& cara,
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
    if (!velocidade) Parar(p);
    return p;
}

float Michel::AnguloAteOAluno(const Vector2 de) const {
    return AnguloAte(de, PosicaoDoAluno());
}

float Michel::AnguloAte(const Vector2 de, const Vector2 para) {
    const Vector2 d = para - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

void Michel::Mover(BossProjectile* p, const Vector2 onde, const Vector2 velocidade) {
    if (!p) return;
    p->SetPosition(onde);
    // A velocidade so serve ao raspao e ao desenho: a posicao quem manda e o professor.
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(velocidade);
}

std::vector<Vector2> Michel::Alunos() const {
    std::vector<Vector2> onde;
    if (const Battle* b = Batalha()) {
        for (const Player* p : b->Jogadores()) {
            if (p) onde.push_back(p->GetPosition());
        }
    }
    if (onde.empty()) onde.push_back(PosicaoDoAluno());
    return onde;
}

bool Michel::NoCampo(const Vector2 p, const float folga) const {
    const SDL_FRect c = Campo();
    return p.x >= c.x - folga && p.x <= c.x + c.w + folga && p.y >= c.y - folga && p.y <= c.y + c.h + folga;
}

void Michel::Retangulo(const Vector2 de, const Vector2 ate, const SDL_Color cor, const bool tracejado) {
    const Vector2 cantos[4] = {de, Vector2(ate.x, de.y), ate, Vector2(de.x, ate.y)};
    for (int k = 0; k < 4; ++k) {
        if (tracejado) Tracejada(cantos[k], cantos[(k + 1) % 4], cor, 6.0f);
        else Linha(cantos[k], cantos[(k + 1) % 4], cor);
    }
}

void Michel::Preencher(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Preencher(de, ate, cor);
}

void Michel::PlacarDaEtapa(const std::string& fase, const size_t etapa, const std::string& texto) {
    auto etapas = EtapasDa(fase);
    if (etapa >= etapas.size() || etapas[etapa].second == texto) return;
    etapas[etapa].second = texto;
    ReescreverEtapas(fase, etapas);
}

void Michel::Avisar(const std::string& texto, const float duracao) {
    mAviso = texto;
    mAvisoAte = mRelogio + duracao;
}

void Michel::Placar(const std::string& fase, const size_t etapa, const std::string& texto) {
    const std::string& vale = mRelogio < mAvisoAte ? mAviso : texto;
    if (!vale.empty()) PlacarDaEtapa(fase, etapa, vale);
}

std::optional<Vector2> Michel::AlvoDoMovimento() const {
    return mAlvo;
}

void Michel::Varrer(const float t, const float amplitude, const float periodo) {
    const SDL_FRect c = Campo();
    mAlvo = Vector2(c.x + c.w * (0.5f + amplitude * Math::Sin(Math::TwoPi * t / periodo)), c.y + 62.0f);
}

void Michel::Desmontar() {
    DesmontarCamera();
    DesmontarConvolucao();
    DesmontarRede();
    DesmontarLaboratorio();
    if (mLinhas) mLinhas->Limpar();
    mAlvo.reset();
    mAvisoAte = -1.0f;
    mEtapaAnterior = -1;
}

void Michel::AoTerminarABatalha() {
    Desmontar();
}

void Michel::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
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

    if (fase == CameraEscura::kFase) AtualizarCamera(t, deltaTime);
    else if (fase == Convolucao::kFase) AtualizarConvolucao(t, deltaTime);
    else if (fase == RedeNeural::kFase) AtualizarRede(t, deltaTime);
    else if (fase == Laboratorio::kFase) AtualizarLaboratorio(t, deltaTime);
}

void Michel::OnDerrubou(BossProjectile* qual) {
    // Um neuronio derrubado e o dropout: sai da rede por um tempo, e os pulsos dele se perdem.
    for (size_t k = 0; k < mNeuronios.size(); ++k) {
        for (size_t i = 0; i < mNeuronios[k].size(); ++i) {
            Neuronio& n = mNeuronios[k][i];
            if (n.p != qual) continue;
            n.p = nullptr;   // ja caiu: o tiro do aluno o derrubou
            n.soma = 0.0f;
            n.foraAte = mRelogio + mDegrauDaRede.dropout;
            Cortar(static_cast<int>(k), static_cast<int>(i));
            ++mDerrubados;
            if (Battle* b = Batalha()) {
                b->Conquistar(RedeNeural::kPremioDoDropout);
                b->Faiscar(qual->GetPosition(), Hud::Faisca::Derrubada);
            }
            Avisar("Dropout: um neurônio fora da rede por " + Decimal(mDegrauDaRede.dropout, 1) + " s", 1.0f);
            return;
        }
    }
}

// ---- Fase 1: a camera escura ----

void Michel::DesmontarCamera() {
    for (ObjetoNaCena& o : mCena) {
        Matar(o.objeto);
        for (BossProjectile*& i : o.imagens) Matar(i);
    }
    mCena.clear();
    Matar(mCamera);
    Matar(mReflexo);
    mAumento = 1.0f;
    mFocoComecou = false;
    mProximoArremesso = 0.0f;
    mArremessos = 0;
}

void Michel::AtualizarImagens(ObjetoNaCena& o, const int etapa, const float deltaTime) {
    using namespace CameraEscura;
    const Degrau& g = mDegrauDaCamera;
    const Vector2 p = o.objeto->GetPosition();
    const Vector2 centro = Imagem(p, mFuro, mAumento);
    Vector2 velocidade;
    if (const auto* rb = o.objeto->GetComponent<RigidBodyComponent>()) velocidade = rb->GetVelocity() * -mAumento;

    std::vector<Vector2> pontos = {centro};
    if (etapa == 2) {
        o.giro += 1.6f * deltaTime;
        const float raio = RaioDoBorrao(centro.y, mFoco, g.ganhoDoBorrao, g.borraoMaximo);
        if (raio >= 4.0f) pontos = Borrao(centro, raio, g.amostrasDoBorrao, o.giro);
    }
    const bool nitida = pontos.size() == 1;
    // O borrao aparece e some com a distancia ao foco: a quantidade de imagens muda.
    if (o.imagens.size() != pontos.size()) {
        for (BossProjectile*& i : o.imagens) Matar(i);
        o.imagens.assign(pontos.size(), nullptr);
    }
    for (size_t k = 0; k < pontos.size(); ++k) {
        BossProjectile*& im = o.imagens[k];
        // So existe imagem do lado de dentro da camera: abaixo do furo, e no campo.
        const bool dentro = NoCampo(pontos[k], -6.0f) && pontos[k].y > mFuro.y + 0.5f * kMargemDoFuro;
        if (!dentro) {
            Matar(im);
            continue;
        }
        if (!Vivo(im)) {
            im = Um(kGatilhoDaImagem, pontos[k], nitida ? kImagens[static_cast<size_t>(o.tipo)] : "Borrao");
            if (im) im->DefinirMargemDeSaida(1.0f, 1.0f);
        }
        Mover(im, pontos[k], velocidade);
    }
    // O raio de luz: do objeto, pelo furo, ate a imagem.
    if (NoCampo(centro, 0.0f)) {
        Linha(p, mFuro, kRaio);
        Linha(mFuro, centro, kRaio);
    }
    if (!nitida) Circulo(centro, (pontos.front() - centro).Length(), kRaio, 14);
}

void Michel::AtualizarCamera(const float t, const float deltaTime) {
    using namespace CameraEscura;
    const Degrau& g = mDegrauDaCamera;
    const SDL_FRect c = Campo();
    const int etapa = EtapaNoTempo(t);
    const float inicio = etapa == 0 ? 0.0f : etapa == 1 ? kDuracoes[0] : kDuracoes[0] + kDuracoes[1];
    const float tEtapa = t - inicio;
    Varrer(t, 0.28f, 7.5f);

    // O furo e o aumento. Com o aumento 1, a imagem do professor fica no chao: a imagem de tudo o que
    // ele arremessa nasce embaixo e sobe.
    Vector2 furo(c.x + c.w * 0.5f, c.y + c.h * 0.52f);
    float aumento = 1.0f;
    if (etapa == 1) {
        furo.x += g.passeioDoFuro * c.w * Math::Sin(Math::TwoPi * tEtapa / 4.4f);
        aumento = AumentoNoTempo(tEtapa, g.periodoDoFoco, g.aumentoMenor, g.aumentoMaior);
    } else if (etapa == 2) {
        aumento = 1.15f;
    }
    mFuro = furo;
    mAumento = aumento;
    const Vector2 aluno = PosicaoDoAluno();
    if (etapa == 2) {
        if (!mFocoComecou) {
            mFocoComecou = true;
            mFoco = c.y + c.h * 0.75f;
        }
        mFoco = Focar(mFoco, aluno.y, deltaTime, g.rapidezDoFoco);
    }

    // A camera no furo, e o reflexo do professor, de cabeca para baixo, do lado de dentro.
    if (!Vivo(mCamera)) {
        mCamera = Um(kGatilhoDoEnfeite, furo, "Camera");
        if (mCamera) mCamera->DefinirEnfeite(true);
    }
    if (mCamera) mCamera->SetPosition(furo);
    const Vector2 reflexo = Imagem(GetPosition(), furo, aumento);
    const Vector2 noChao(reflexo.x, std::min(reflexo.y, c.y + c.h - 28.0f));
    if (!Vivo(mReflexo)) {
        mReflexo = Um(kGatilhoDoEnfeite, noChao, "Reflexo");
        if (mReflexo) mReflexo->DefinirEnfeite(true);
    }
    if (mReflexo) {
        mReflexo->SetPosition(noChao);
        Cara(mReflexo, reflexo.y > c.y + c.h - 4.0f ? "ReflexoFora" : "Reflexo");
    }
    // A parede da camera, com o furo no meio, e o plano de foco.
    Tracejada(Vector2(c.x, furo.y), Vector2(furo.x - 26.0f, furo.y), kGrade, 10.0f);
    Tracejada(Vector2(furo.x + 26.0f, furo.y), Vector2(c.x + c.w, furo.y), kGrade, 10.0f);
    if (etapa == 2) Tracejada(Vector2(c.x, mFoco), Vector2(c.x + c.w, mFoco), kFantasma, 6.0f);

    // O arremesso: abaixo do furo, o professor mira na imagem do aluno (o ponto da cena cuja imagem
    // cai nele); acima, nele mesmo. A imagem so chega se o aluno estiver acima do reflexo: com o
    // aumento pequeno, o chao fica a salvo.
    if (t >= mProximoArremesso) {
        mProximoArremesso = t + g.entreArremessos;
        const Vector2 de = GetPosition() + Vector2(0.0f, 26.0f);
        const bool embaixo = aluno.y > furo.y + kMargemDoFuro;
        Vector2 alvo = embaixo ? ObjetoDa(aluno, furo, aumento) : aluno;
        if (alvo.y < de.y + 30.0f) alvo = Vector2(furo.x + (furo.x - aluno.x) / std::max(aumento, 0.1f), furo.y);
        const float centro = AnguloAte(de, alvo);
        const int n = g.leque;
        for (int k = 0; k < n; ++k) {
            const float u = n > 1 ? static_cast<float>(k) / static_cast<float>(n - 1) - 0.5f : 0.0f;
            const float a = Math::ToRadians(centro + g.aberturaDoLeque * u);
            ObjetoNaCena o;
            o.tipo = (mArremessos + k) % static_cast<int>(kObjetos.size());
            o.giro = mSorteio.Real(0.0f, Math::TwoPi);
            o.objeto = Um(kGatilhoDoObjeto, de, kObjetos[static_cast<size_t>(o.tipo)],
                          Vector2(Math::Cos(a), Math::Sin(a)) * g.velocidadeDoObjeto);
            if (o.objeto) mCena.push_back(o);
        }
        if (++mArremessos % 2 == 1) SomDoDisparo();
    }

    // Cada objeto anda na cena; quando chega a parede, entra na camera - e a imagem chega ao furo.
    for (ObjetoNaCena& o : mCena) {
        if (!Vivo(o.objeto)) {
            o.objeto = nullptr;
            for (BossProjectile*& i : o.imagens) Matar(i);
            continue;
        }
        const Vector2 p = o.objeto->GetPosition();
        if (p.y >= furo.y - kMargemDoFuro || !NoCampo(p, 20.0f)) {
            Matar(o.objeto);
            for (BossProjectile*& i : o.imagens) Matar(i);
            continue;
        }
        AtualizarImagens(o, etapa, deltaTime);
    }
    mCena.erase(std::remove_if(mCena.begin(), mCena.end(), [](const ObjetoNaCena& o) { return o.objeto == nullptr; }),
                mCena.end());

    if (etapa == 0) Placar(kFase, 0, "Câmera escura: a imagem sai de cabeça para baixo");
    else if (etapa == 1) Placar(kFase, 1, "Foco: a imagem com aumento de " + Decimal(aumento, 2) + "×");
    else Placar(kFase, 2, "Desfoque: o plano de foco segue você");
}

// ---- Fase 2: os descritores e a convolucao ----

Vector2 Michel::TamanhoDaCelula() const {
    const SDL_FRect c = Campo();
    return Vector2(c.w / static_cast<float>(Convolucao::kColunas), c.h / static_cast<float>(Convolucao::kLinhas));
}

Vector2 Michel::CentroDaCelula(const int col, const int lin) const {
    const SDL_FRect c = Campo();
    const Vector2 t = TamanhoDaCelula();
    return Vector2(c.x + (static_cast<float>(col) + 0.5f) * t.x, c.y + (static_cast<float>(lin) + 0.5f) * t.y);
}

std::pair<int, int> Michel::CelulaEm(const Vector2 p) const {
    const SDL_FRect c = Campo();
    const Vector2 t = TamanhoDaCelula();
    return {static_cast<int>(std::floor((p.x - c.x) / t.x)), static_cast<int>(std::floor((p.y - c.y) / t.y))};
}

void Michel::DesmontarConvolucao() {
    for (Fluxo& f : mFluxos) Matar(f.p);
    mFluxos.clear();
    mBrilho = Convolucao::Grade{};
    mAtivacao = Convolucao::Grade{};
    mBins.clear();
    mGiro = 0;
    mProximaAtualizacao = 0.0f;
    mProximoFluxo = 0.0f;
    mPassoDaJanela = -1;
    mProximoPasso = 0.0f;
    mPassoDoPooling = -1;
    mProximoPooling = 0.0f;
    mProximoLeque = 0.0f;
    mVetorDisparado = false;
    mUltimaResposta = 0.0f;
    mPassoDaBorda = -1;
    mProximaBorda = 0.0f;
    mUltimaBorda = 0.0f;
    mSegundoVetor = -1.0f;
    mTirosDoVetor = 0;
    mPendentes.clear();
}

void Michel::AtualizarConvolucao(const float t, const float deltaTime) {
    using namespace Convolucao;
    const Degrau& g = mDegrauDaConvolucao;
    const int etapa = EtapaNoTempo(t);
    const float t1 = kDuracoes[0], t2 = kDuracoes[0] + kDuracoes[1];
    const SDL_FRect c = Campo();
    const Vector2 tam = TamanhoDaCelula();
    if (mBrilho.v.empty()) {
        mBrilho = Vazia(kColunas, kLinhas);
        mAtivacao = Vazia(kColunas, kLinhas);
        mBins.assign(static_cast<size_t>(kColunas * kLinhas), 0);
    }
    Varrer(t, 0.33f, 6.2f);
    const Vector2 aluno = PosicaoDoAluno();
    const auto celula = [&](const int col, const int lin) {
        return std::make_pair(Vector2(c.x + static_cast<float>(col) * tam.x, c.y + static_cast<float>(lin) * tam.y),
                              Vector2(c.x + static_cast<float>(col + 1) * tam.x, c.y + static_cast<float>(lin + 1) * tam.y));
    };

    // A grade da imagem.
    for (int k = 1; k < kColunas; ++k) {
        const float x = c.x + static_cast<float>(k) * tam.x;
        Linha(Vector2(x, c.y), Vector2(x, c.y + c.h), kGrade);
    }
    for (int k = 1; k < kLinhas; ++k) {
        const float y = c.y + static_cast<float>(k) * tam.y;
        Linha(Vector2(c.x, y), Vector2(c.x + c.w, y), kGrade);
    }

    // ---- O gradiente: as setas levam os tiros ----
    if (etapa == 0) {
        if (t >= mProximaAtualizacao) {
            mProximaAtualizacao = t + g.atualizacaoDoDescritor;
            const int giro = t >= 0.5f * kDuracoes[0] ? kGiroDaImagem : 0;
            if (giro != mGiro) {
                mGiro = giro;
                Avisar("A imagem girou 90°: o descritor feito à mão não gira junto");
            }
            for (int l = 0; l < kLinhas; ++l) {
                for (int col = 0; col < kColunas; ++col) {
                    mBins[static_cast<size_t>(l * kColunas + col)] = BinDaCelula(CentroDaCelula(col, l), aluno, mGiro);
                }
            }
        }
        for (int l = 0; l < kLinhas; ++l) {
            for (int col = 0; col < kColunas; ++col) {
                const Vector2 m = CentroDaCelula(col, l);
                const Vector2 d = DirecaoDoBin(mBins[static_cast<size_t>(l * kColunas + col)]);
                const Vector2 ponta = m + d * 18.0f;
                Linha(m - d * 18.0f, ponta, kSeta);
                const Vector2 lado(-d.y, d.x);
                Linha(ponta, ponta - d * 7.0f + lado * 5.0f, kSeta);
                Linha(ponta, ponta - d * 7.0f - lado * 5.0f, kSeta);
            }
        }
        if (t >= mProximoFluxo) {
            mProximoFluxo = t + g.entreFluxos;
            const auto [ca, la] = CelulaEm(aluno);
            for (int tentativa = 0; tentativa < 10; ++tentativa) {
                const int col = mSorteio.Inteiro(0, kColunas - 1), l = mSorteio.Inteiro(0, kLinhas - 1);
                if (std::abs(col - ca) + std::abs(l - la) < 3) continue;
                const Vector2 onde = CentroDaCelula(col, l) +
                                     Vector2(mSorteio.Real(-0.3f, 0.3f) * tam.x, mSorteio.Real(-0.3f, 0.3f) * tam.y);
                const Vector2 v = DirecaoDoBin(mBins[static_cast<size_t>(l * kColunas + col)]) * g.velocidadeDoFluxo;
                if (BossProjectile* p = Um(kGatilhoDoFluxo, onde, "Seta", v)) {
                    p->DefinirRotacaoPelaVelocidade(true);
                    mFluxos.push_back({p, t + g.vidaDoFluxo});
                }
                break;
            }
        }
        Placar(kFase, 0, mGiro == 0 ? "Gradiente: 8 direções por célula, todas para você"
                                    : "O descritor não girou: as setas ficaram de lado");
    }
    // Os tiros do gradiente seguem a seta da celula em que estao (e morrem com o tempo).
    for (Fluxo& f : mFluxos) {
        if (!Vivo(f.p)) {
            f.p = nullptr;
            continue;
        }
        const auto [col, l] = CelulaEm(f.p->GetPosition());
        if (t >= f.morre || col < 0 || l < 0 || col >= kColunas || l >= kLinhas) {
            Matar(f.p);
            continue;
        }
        if (auto* rb = f.p->GetComponent<RigidBodyComponent>()) {
            const Vector2 quer = DirecaoDoBin(mBins[static_cast<size_t>(l * kColunas + col)]) * g.velocidadeDoFluxo;
            const Vector2 v = rb->GetVelocity();
            rb->SetVelocity(v + (quer - v) * std::min(1.0f, 7.0f * deltaTime));
        }
    }
    mFluxos.erase(std::remove_if(mFluxos.begin(), mFluxos.end(), [](const Fluxo& f) { return f.p == nullptr; }),
                  mFluxos.end());
    if (etapa == 0) return;

    // ---- A imagem que o aluno pinta ----
    if (const Battle* b = Batalha(); b && b->GerenteDeProjeteis()) {
        for (const auto& tiro : b->GerenteDeProjeteis()->GetPlayerProjectiles()) {
            if (!tiro || tiro->GetState() != ActorState::Active) continue;
            const auto [col, l] = CelulaEm(tiro->GetPosition());
            Pintar(mBrilho, col, l, g.brilhoPorSegundo * deltaTime, 1.0f);
        }
    }
    Esmaecer(mBrilho, deltaTime, g.meiaVida);
    Esmaecer(mAtivacao, deltaTime, 0.3f);
    for (int l = 0; l < kLinhas; ++l) {
        for (int col = 0; col < kColunas; ++col) {
            const auto [de, ate] = celula(col, l);
            const float brilho = mBrilho.Em(col, l);
            if (brilho > 0.03f) {
                Preencher(de + Vector2(2.0f, 2.0f), ate - Vector2(2.0f, 2.0f),
                          SDL_Color{kBrilho.r, kBrilho.g, kBrilho.b, static_cast<Uint8>(std::min(1.0f, brilho) * 92.0f)});
            }
            const float ativo = mAtivacao.Em(col, l);
            if (ativo > 0.03f) {
                const Vector2 meio = (de + ate) * 0.5f, lado = (ate - de) * 0.3f;
                Preencher(meio - lado, meio + lado,
                          SDL_Color{kAtivo.r, kAtivo.g, kAtivo.b, static_cast<Uint8>(std::min(1.0f, ativo * 4.0f) * 150.0f)});
            }
        }
    }
    if (t >= mProximoLeque) {
        mProximoLeque = t + g.entreLeques;
        DispararEm(kGatilhoDoLeque, GetPosition(), g.tirosDoLeque, AnguloAteOAluno(GetPosition()));
    }
    // As ativacoes que passaram do limiar piscam, e disparam.
    for (Ativacao& at : mPendentes) {
        const auto [de, a0] = celula(at.coluna, at.linha);
        const auto [d1, ate] = celula(std::min(kColunas - 1, at.coluna + at.lado - 1), std::min(kLinhas - 1, at.linha + at.lado - 1));
        (void)a0;
        (void)d1;
        if (t < at.quando) {
            const SDL_Color cor = at.borda ? kBorda : kAtivo;
            const Uint8 alfa = static_cast<int>(t / 0.07f) % 2 == 0 ? 120 : 50;
            Preencher(de, ate, SDL_Color{cor.r, cor.g, cor.b, alfa});
            Retangulo(de, ate, cor);
            continue;
        }
        DispararEm(kGatilhoDaAtivacao, at.de, at.tiros, AnguloAteOAluno(at.de), at.abertura, at.velocidade);
        for (int dl = 0; dl < at.lado; ++dl) {
            for (int dc = 0; dc < at.lado; ++dc) Pintar(mAtivacao, at.coluna + dc, at.linha + dl, at.valor, 1.0f);
        }
        SomDoDisparo();
        at.tiros = 0;
    }
    mPendentes.erase(std::remove_if(mPendentes.begin(), mPendentes.end(), [](const Ativacao& at) { return at.tiros == 0; }),
                     mPendentes.end());

    // ---- A convolucao: a janela 3x3 varre a imagem ----
    if (etapa == 1) {
        if (mPassoDaJanela < 0) mProximoPasso = t1 + 0.4f;
        if (t >= mProximoPasso) {
            mProximoPasso = t + 1.0f / g.passosPorSegundo;
            ++mPassoDaJanela;
            const auto [jc, jl] = Janela(mPassoDaJanela, kColunas, kLinhas);
            mUltimaResposta = Resposta(mBrilho, jc, jl, kGaussiano);
            const float a = Relu(mUltimaResposta, g.limiar);
            const int n = TirosDaAtivacao(a, g.tirosMinimos, g.tirosMaximos, 0.4f);
            if (n > 0) {
                // A ativacao se espalha em anel, a partir da celula - o vazio do anel aponta para longe do
                // aluno, e o mais denso, para ele. Antes, a celula pisca.
                mPendentes.push_back(Ativacao{CentroDaCelula(jc, jl), n, 300.0f * static_cast<float>(n - 1) / static_cast<float>(n),
                                              g.velocidadeDaAtivacao, t + g.avisoDaAtivacao, jc, jl, 1, a});
            }
        }
        const auto [jc, jl] = Janela(std::max(0, mPassoDaJanela), kColunas, kLinhas);
        const auto [de, ate0] = celula(std::max(0, jc - 1), std::max(0, jl - 1));
        const auto [de1, ate] = celula(std::min(kColunas - 1, jc + 1), std::min(kLinhas - 1, jl + 1));
        (void)ate0;
        (void)de1;
        Retangulo(de, ate, kJanela);
        const auto [cd, ca] = celula(jc, jl);
        Retangulo(cd + Vector2(3.0f, 3.0f), ca - Vector2(3.0f, 3.0f), kJanela);
        if (g.filtros < 2) {
            Placar(kFase, 1, "Filtro 3×3 na célula (" + std::to_string(jc + 1) + ", " + std::to_string(jl + 1) +
                                 "): resposta " + Decimal(mUltimaResposta, 2) + " · seus tiros são a imagem");
            return;
        }
        // O segundo filtro, o de borda: vem de tras para a frente, e responde ao lado do que se
        // acendeu. A janela dele e tracejada, na cor dele.
        if (mPassoDaBorda < 0) mProximaBorda = t1 + 0.4f + 0.5f / g.passosPorSegundo;
        if (t >= mProximaBorda) {
            mProximaBorda = t + 1.0f / g.passosPorSegundo;
            ++mPassoDaBorda;
            const auto [bc, bl] = JanelaDeVolta(mPassoDaBorda, kColunas, kLinhas);
            mUltimaBorda = Convolucao::Borda(mBrilho, bc, bl);
            const float a = Relu(mUltimaBorda, g.limiarDaBorda);
            const int n = TirosDaAtivacao(a, g.tirosMinimos, g.tirosMaximos, 0.4f);
            if (n > 0) {
                Ativacao at{CentroDaCelula(bc, bl), n, 300.0f * static_cast<float>(n - 1) / static_cast<float>(n),
                            g.velocidadeDaAtivacao, t + g.avisoDaAtivacao, bc, bl, 1, a};
                at.borda = true;
                mPendentes.push_back(at);
            }
        }
        const auto [bc, bl] = JanelaDeVolta(std::max(0, mPassoDaBorda), kColunas, kLinhas);
        const auto [bde, bate0] = celula(std::max(0, bc - 1), std::max(0, bl - 1));
        const auto [bde1, bate] = celula(std::min(kColunas - 1, bc + 1), std::min(kLinhas - 1, bl + 1));
        (void)bate0;
        (void)bde1;
        Retangulo(bde, bate, kBorda, true);
        const auto [bcd, bca] = celula(bc, bl);
        Retangulo(bcd + Vector2(3.0f, 3.0f), bca - Vector2(3.0f, 3.0f), kBorda);
        Placar(kFase, 1, "Dois filtros: o gaussiano (" + std::to_string(jc + 1) + ", " + std::to_string(jl + 1) + ") " +
                             Decimal(mUltimaResposta, 2) + " e o de borda (" + std::to_string(bc + 1) + ", " +
                             std::to_string(bl + 1) + ") " + Decimal(mUltimaBorda, 2));
        return;
    }

    // ---- O pooling: o maximo de cada bloco, e no fim o vetor ----
    const Grade pool = MaxPool(mBrilho);
    const float tPooling = t - t2;
    for (int l = 0; l < pool.linhas; ++l) {
        for (int col = 0; col < pool.colunas; ++col) {
            const auto [de, a0] = celula(2 * col, 2 * l);
            const auto [d1, ate] = celula(std::min(kColunas - 1, 2 * col + 1), std::min(kLinhas - 1, 2 * l + 1));
            (void)a0;
            (void)d1;
            Retangulo(de + Vector2(1.0f, 1.0f), ate - Vector2(1.0f, 1.0f), SDL_Color{kJanela.r, kJanela.g, kJanela.b, 60});
        }
    }
    if (tPooling < kVetor) {
        if (mPassoDoPooling < 0) mProximoPooling = t2 + 0.5f;
        if (t >= mProximoPooling) {
            mProximoPooling = t + 1.0f / g.passosDoPooling;
            ++mPassoDoPooling;
            const auto [pc, pl] = Janela(mPassoDoPooling, pool.colunas, pool.linhas);
            const float a = Relu(pool.Em(pc, pl), 2.0f * g.limiar);
            const int n = TirosDaAtivacao(a, g.tirosMinimos + 4, g.tirosMaximos + 5, 0.5f);
            if (n > 0) {
                const Vector2 de = (CentroDaCelula(2 * pc, 2 * pl) + CentroDaCelula(2 * pc + 1, 2 * pl + 1)) * 0.5f;
                mPendentes.push_back(Ativacao{de, n, 360.0f * static_cast<float>(n - 1) / static_cast<float>(n),
                                              0.85f * g.velocidadeDaAtivacao, t + g.avisoDaAtivacao, 2 * pc, 2 * pl, 2, a});
            }
        }
        const auto [pc, pl] = Janela(std::max(0, mPassoDoPooling), pool.colunas, pool.linhas);
        const auto [de, a0] = celula(2 * pc, 2 * pl);
        const auto [d1, ate] = celula(std::min(kColunas - 1, 2 * pc + 1), std::min(kLinhas - 1, 2 * pl + 1));
        (void)a0;
        (void)d1;
        Retangulo(de, ate, kJanela);
        Placar(kFase, 2, "Max pooling 2×2: o bloco (" + std::to_string(pc + 1) + ", " + std::to_string(pl + 1) +
                             ") vale o maior, " + Decimal(pool.Em(pc, pl), 2));
    } else {
        if (!mVetorDisparado) {
            mVetorDisparado = true;
            const float global = Global(mBrilho);
            const int n = std::max(10, static_cast<int>(std::lround(static_cast<float>(g.tirosDoVetor) *
                                                                   std::clamp(global / 0.3f, 0.3f, 1.0f))));
            DispararEm(kGatilhoDoVetor, GetPosition(), n, AnguloAteOAluno(GetPosition()),
                       360.0f * static_cast<float>(n - 1) / static_cast<float>(n));
            if (g.filtros > 1) {
                mSegundoVetor = t + kEntreOsVetores;
                mTirosDoVetor = n;
            }
            SomDoDisparo();
            Avisar("Pooling global: a imagem inteira vira um vetor de " + std::to_string(n), 2.0f);
        }
        // O segundo mapa vira o segundo anel: meio passo girado, no vao do primeiro.
        if (mSegundoVetor >= 0.0f && t >= mSegundoVetor && mTirosDoVetor > 0) {
            mSegundoVetor = -1.0f;
            const float n = static_cast<float>(mTirosDoVetor);
            DispararEm(kGatilhoDoVetor, GetPosition(), mTirosDoVetor, AnguloAteOAluno(GetPosition()) + 180.0f / n,
                       360.0f * (n - 1.0f) / n);
            SomDoDisparo();
        }
        Placar(kFase, 2, "Deep features: o que você acendeu virou o vetor");
    }
}

// ---- Fase 3: a rede ----

Vector2 Michel::LugarDoNeuronio(const int camada, const int i) const {
    using namespace RedeNeural;
    const SDL_FRect c = Campo();
    const int camadas = static_cast<int>(mRede.camadas.size());
    const int n = mRede.camadas[static_cast<size_t>(camada)];
    const float u = camadas > 1 ? static_cast<float>(camada) / static_cast<float>(camadas - 1) : 0.0f;
    // De parede a parede, e cada camada meio passo para um lado: as ligacoes cruzam o campo em
    // diagonal, e nao ha coluna livre de cima a baixo.
    const float lado = (camada % 2 == 0) ? -0.18f : 0.18f;
    const float x = c.x + c.w * (static_cast<float>(i) + 0.5f + lado) / static_cast<float>(n);
    return Vector2(std::clamp(x, c.x + 36.0f, c.x + c.w - 36.0f),
                   c.y + c.h * (kAlturaDaEntrada + (kAlturaDaSaida - kAlturaDaEntrada) * u));
}

void Michel::DesmontarRede() {
    for (auto& camada : mNeuronios) {
        for (Neuronio& n : camada) Matar(n.p);
    }
    mNeuronios.clear();
    for (Sinal& s : mSinais) Matar(s.p);
    mSinais.clear();
    mPorVir.clear();
    if (const Battle* b = Batalha()) {
        for (Player* p : b->Jogadores()) if (p) p->DefinirClasse(-1);
    }
    mRede = RedeNeural::Rede{};
    mEtapaDaRede = -1;
    mProximaEntrada = 0.0f;
    mProximoTreino = 0.0f;
    mAlvoDoTreino = -1;
    mTreinos = 0;
    mDerrubados = 0;
}

void Michel::MontarARede(const int etapa) {
    using namespace RedeNeural;
    const Degrau& g = mDegrauDaRede;
    for (auto& camada : mNeuronios) {
        for (Neuronio& n : camada) Matar(n.p);
    }
    for (Sinal& s : mSinais) Matar(s.p);
    mSinais.clear();
    mPorVir.clear();
    mRede = Montar(CamadasDa(etapa), mSorteio, g.pesoMenor, g.pesoMaior);
    if (etapa >= 2) mRede.atalhos = Atalhos(mRede.camadas, 2);
    mNeuronios.assign(mRede.camadas.size(), {});
    for (size_t k = 0; k < mRede.camadas.size(); ++k) {
        mNeuronios[k].assign(static_cast<size_t>(mRede.camadas[k]), Neuronio{});
    }
    mEtapaDaRede = etapa;
}

Vector2 Michel::NoCaminho(const Sinal& s, const float u) {
    if (!s.atalho) return s.de + (s.para - s.de) * u;
    // O atalho faz uma curva por fora: uma Bezier quadratica, com o controle para o lado.
    const Vector2 meio = (s.de + s.para) * 0.5f;
    const Vector2 d = s.para - s.de;
    Vector2 lado(-d.y, d.x);
    const float n = lado.Length();
    if (n > 1e-3f) lado = lado * (1.0f / n);
    const Vector2 controle = meio + lado * 70.0f;
    const float v = 1.0f - u;
    return s.de * (v * v) + controle * (2.0f * v * u) + s.para * (u * u);
}

void Michel::MandarSinal(const Vector2 de, const int camada, const int ate, const float peso, const bool atalho,
                         const int camadaDe, const int iDe, const bool leva) {
    if (mSinais.size() >= 160) return;
    Sinal s;
    s.camada = camada;
    s.ate = ate;
    s.peso = peso;
    s.de = de;
    s.para = LugarDoNeuronio(camada, ate);
    s.atalho = atalho;
    s.camadaDe = camadaDe;
    s.de_ = iDe;
    s.leva = leva;
    const float distancia = (s.para - s.de).Length() * (atalho ? 1.2f : 1.0f);
    s.sai = mRelogio;
    s.chega = mRelogio + std::max(0.15f, distancia / mDegrauDaRede.velocidadeDoPulso);
    s.p = Um(RedeNeural::kGatilhoDoSinal, de, atalho ? "Atalho" : "Sinal",
             (s.para - s.de) * (1.0f / (s.chega - s.sai)));
    if (s.p) mSinais.push_back(s);
}

void Michel::Disparou(const int camada, const int i) {
    using namespace RedeNeural;
    const Degrau& g = mDegrauDaRede;
    const int camadas = static_cast<int>(mRede.camadas.size());
    mNeuronios[static_cast<size_t>(camada)][static_cast<size_t>(i)].acesoAte = mRelogio + 0.18f;
    const Vector2 de = LugarDoNeuronio(camada, i);
    if (camada == camadas - 1) {
        DispararEm(kGatilhoDaSaida, de, g.tirosDaSaida, AnguloAteOAluno(de), g.aberturaDaSaida, g.velocidadeDaSaida);
        SomDoDisparo();
        return;
    }
    // Cada ligacao forte acende e, passado o aviso, solta um pulso: os vagoes, um atras do outro.
    const auto pulso = [&](const int camadaAte, const int ate, const float peso, const bool atalho) {
        for (int v = 0; v < std::max(1, g.vagoes); ++v) {
            mPorVir.push_back(PorVir{camada, i, camadaAte, ate, peso, atalho, v == 0,
                                     mRelogio + g.avisoDoPulso + g.entreVagoes * static_cast<float>(v)});
        }
    };
    for (const int j : Ligacoes(mRede, camada, i, g.ligacaoForte)) pulso(camada + 1, j, mRede.Peso(camada, i, j), false);
    for (const Atalho& a : mRede.atalhos) {
        if (a.camada != camada || a.de != i) continue;
        // O atalho leva o pulso inteiro: o bastante para o de la disparar.
        const bool saida = camada + a.pulo == camadas - 1;
        pulso(camada + a.pulo, a.ate, saida ? g.limiarDaSaida : g.limiar, true);
    }
}

bool Michel::Fora(const int k, const int i) const {
    if (k < 0 || i < 0 || k >= static_cast<int>(mNeuronios.size())) return false;
    const auto& camada = mNeuronios[static_cast<size_t>(k)];
    if (i >= static_cast<int>(camada.size())) return false;
    return camada[static_cast<size_t>(i)].foraAte >= 0.0f;
}

void Michel::Cortar(const int k, const int i) {
    using RedeNeural::Corta;
    for (Sinal& s : mSinais) {
        if (!Vivo(s.p) || !Corta(k, i, s.camadaDe, s.de_, s.camada, s.ate)) continue;
        if (Battle* b = Batalha()) b->Faiscar(s.p->GetPosition(), Hud::Faisca::Dissolvido);
        Matar(s.p);
    }
    mPorVir.erase(std::remove_if(mPorVir.begin(), mPorVir.end(),
                                 [&](const PorVir& p) { return Corta(k, i, p.camadaDe, p.de, p.camada, p.ate); }),
                  mPorVir.end());
}

void Michel::AtualizarRede(const float t, const float deltaTime) {
    using namespace RedeNeural;
    (void)deltaTime;
    const Degrau& g = mDegrauDaRede;
    const int etapa = EtapaNoTempo(t);
    const int forma = etapa >= 2 ? 2 : 0;
    if (mEtapaDaRede != forma) {
        MontarARede(forma);
        if (forma == 2) Avisar("Residual: a rede ficou funda, e os atalhos pulam duas camadas", 2.0f);
    }
    Varrer(t, 0.3f, 6.8f);
    const int camadas = static_cast<int>(mRede.camadas.size());

    // As ligacoes: as fortes levam pulso; as fracas mal se veem; as de um neuronio fora da rede se
    // apagam. Na retropropagacao, as que chegam na saida que aprende o aluno engrossam, vermelhas.
    const int alvo = etapa >= 1 ? mAlvoDoTreino : -1;
    for (int k = 0; k + 1 < camadas; ++k) {
        for (int i = 0; i < mRede.camadas[static_cast<size_t>(k)]; ++i) {
            for (int j = 0; j < mRede.camadas[static_cast<size_t>(k + 1)]; ++j) {
                const float w = mRede.Peso(k, i, j);
                const bool forte = w >= g.ligacaoForte;
                const Vector2 de = LugarDoNeuronio(k, i), para = LugarDoNeuronio(k + 1, j);
                if (Fora(k, i) || Fora(k + 1, j)) {
                    if (forte) Tracejada(de, para, SDL_Color{150, 150, 170, 40}, 6.0f);
                    continue;
                }
                const Uint8 a = static_cast<Uint8>(forte ? 50.0f + 130.0f * w : 16.0f);
                Linha(de, para, SDL_Color{kLigacao.r, kLigacao.g, kLigacao.b, a});
                if (forte && k + 1 == camadas - 1 && j == alvo) {
                    Vector2 lado(-(para - de).y, (para - de).x);
                    const float n = lado.Length();
                    if (n > 1e-3f) lado = lado * (1.5f / n);
                    const SDL_Color vermelho{kPerigo.r, kPerigo.g, kPerigo.b, static_cast<Uint8>(90.0f + 140.0f * w)};
                    Linha(de + lado, para + lado, vermelho);
                    Linha(de - lado, para - lado, vermelho);
                }
            }
        }
    }
    for (const Atalho& a : mRede.atalhos) {
        Sinal s;
        s.de = LugarDoNeuronio(a.camada, a.de);
        s.para = LugarDoNeuronio(a.camada + a.pulo, a.ate);
        s.atalho = true;
        const bool apagado = Fora(a.camada, a.de) || Fora(a.camada + a.pulo, a.ate);
        Vector2 antes = s.de;
        for (int k = 1; k <= 12; ++k) {
            const Vector2 agora = NoCaminho(s, static_cast<float>(k) / 12.0f);
            if (!apagado || k % 2 == 0) Linha(antes, agora, apagado ? SDL_Color{150, 150, 170, 50} : kAtalho);
            antes = agora;
        }
    }

    // O aviso dos pulsos: a ligacao que vai levar um acende, cada vez mais rapido ate ele sair.
    for (const PorVir& v : mPorVir) {
        if (!v.leva || v.sai <= mRelogio) continue;
        const float falta = v.sai - mRelogio;
        if (std::fmod(falta, falta < 0.2f ? 0.06f : 0.12f) < (falta < 0.2f ? 0.03f : 0.06f)) continue;
        Sinal s;
        s.de = LugarDoNeuronio(v.camadaDe, v.de);
        s.para = LugarDoNeuronio(v.camada, v.ate);
        s.atalho = v.atalho;
        const SDL_Color acesa = v.atalho ? SDL_Color{255, 225, 140, 230} : SDL_Color{225, 240, 255, 230};
        Vector2 antes = s.de;
        for (int k = 1; k <= (v.atalho ? 12 : 1); ++k) {
            const Vector2 agora = NoCaminho(s, static_cast<float>(k) / (v.atalho ? 12.0f : 1.0f));
            Vector2 lado(-(agora - antes).y, (agora - antes).x);
            const float n = lado.Length();
            if (n > 1e-3f) lado = lado * (1.5f / n);
            Linha(antes, agora, acesa);
            Linha(antes + lado, agora + lado, acesa);
            Linha(antes - lado, agora - lado, acesa);
            antes = agora;
        }
    }
    // Os que ja podem sair saem - se as duas pontas ainda estao na rede.
    for (const PorVir& v : mPorVir) {
        if (v.sai > mRelogio || Fora(v.camadaDe, v.de) || Fora(v.camada, v.ate)) continue;
        MandarSinal(LugarDoNeuronio(v.camadaDe, v.de), v.camada, v.ate, v.peso, v.atalho, v.camadaDe, v.de, v.leva);
    }
    mPorVir.erase(std::remove_if(mPorVir.begin(), mPorVir.end(), [&](const PorVir& v) { return v.sai <= mRelogio; }),
                  mPorVir.end());

    // Os neuronios: no lugar, acesos quando disparam; o derrubado volta depois do dropout.
    int fora = 0;
    for (int k = 0; k < camadas; ++k) {
        for (int i = 0; i < mRede.camadas[static_cast<size_t>(k)]; ++i) {
            Neuronio& n = mNeuronios[static_cast<size_t>(k)][static_cast<size_t>(i)];
            const Vector2 onde = LugarDoNeuronio(k, i);
            if (n.p && !Vivo(n.p)) {
                n.p = nullptr;   // o Ponto Extra tambem tira um neuronio da rede
                n.foraAte = mRelogio + g.dropout;
                Cortar(k, i);
            }
            if (n.foraAte >= 0.0f) {
                if (mRelogio < n.foraAte) {
                    ++fora;
                    Circulo(onde, 12.0f, SDL_Color{150, 150, 170, 110}, 10);
                    Linha(onde + Vector2(-8.0f, -8.0f), onde + Vector2(8.0f, 8.0f), SDL_Color{150, 150, 170, 110});
                    continue;
                }
                n.foraAte = -1.0f;
                n.soma = 0.0f;
            }
            if (!Vivo(n.p)) {
                n.p = Um(kGatilhoDoNeuronio, onde, "Neuronio");
                if (n.p) {
                    n.p->DefinirResistencia(g.resistencia);
                    n.p->DefinirClasse(kClasseDaRede);
                }
            }
            if (n.p) {
                n.p->SetPosition(onde);
                Cara(n.p, mRelogio < n.acesoAte ? "Aceso" : "Neuronio");
            }
        }
    }

    // O aluno anda pela rede: o neuronio tem a classe dele (nao machuca), e encostar o derruba.
    if (const Battle* b = Batalha()) {
        for (Player* aluno : b->Jogadores()) {
            if (!aluno) continue;
            aluno->DefinirClasse(kClasseDaRede);
            const auto* colisor = aluno->GetComponent<CircleColliderComponent>();
            const float alcance = (colisor ? colisor->GetRadius() : 6.0f) + 18.0f;
            for (auto& camada : mNeuronios) {
                for (Neuronio& n : camada) {
                    if (!Vivo(n.p) || (n.p->GetPosition() - aluno->GetPosition()).Length() > alcance) continue;
                    BossProjectile* qual = n.p;
                    qual->Kill();
                    OnDerrubou(qual);
                }
            }
        }
    }

    // A entrada: o professor manda um sinal a um neuronio da primeira camada.
    if (t >= mProximaEntrada) {
        mProximaEntrada = t + g.entreEntradas;
        MandarSinal(GetPosition() + Vector2(0.0f, 20.0f), 0, mSorteio.Inteiro(0, mRede.camadas.front() - 1), 1.0f, false);
    }

    // Os sinais andam; quem chega soma no neuronio, e o que passou do limiar dispara.
    std::vector<std::pair<int, int>> disparam;
    for (Sinal& s : mSinais) {
        if (!Vivo(s.p)) {
            s.p = nullptr;
            continue;
        }
        const float dura = std::max(1e-3f, s.chega - s.sai);
        const float u = std::clamp((mRelogio - s.sai) / dura, 0.0f, 1.0f);
        const Vector2 onde = NoCaminho(s, u);
        const Vector2 adiante = NoCaminho(s, std::min(1.0f, u + 0.05f));
        Mover(s.p, onde, (adiante - onde) * (1.0f / (0.05f * dura)));
        if (u < 1.0f) continue;
        Matar(s.p);
        Neuronio& n = mNeuronios[static_cast<size_t>(s.camada)][static_cast<size_t>(s.ate)];
        if (n.foraAte >= 0.0f) continue;   // o dropout: o sinal se perde
        if (!s.leva) continue;             // os vagoes de tras so passam: quem soma e o primeiro
        const float limiar = s.camada == 0 ? 1.0f : s.camada == camadas - 1 ? g.limiarDaSaida : g.limiar;
        if (Somar(n.soma, s.peso, limiar)) disparam.emplace_back(s.camada, s.ate);
    }
    mSinais.erase(std::remove_if(mSinais.begin(), mSinais.end(), [](const Sinal& s) { return s.p == nullptr; }),
                  mSinais.end());
    for (const auto& [k, i] : disparam) Disparou(k, i);

    // A retropropagacao: a saida em cima do aluno devia disparar, e as outras nao.
    if (etapa >= 1) {
        std::vector<float> xs;
        for (int i = 0; i < mRede.camadas.back(); ++i) xs.push_back(LugarDoNeuronio(camadas - 1, i).x);
        if (t >= mProximoTreino) {
            mProximoTreino = t + g.entreTreinos;
            mAlvoDoTreino = SaidaDoAluno(xs, PosicaoDoAluno().x);
            // A saida que aprenderia o aluno esta fora da rede: a epoca nao tem como aprender.
            if (!Fora(camadas - 1, mAlvoDoTreino)) {
                Retropropagar(mRede, mAlvoDoTreino, etapa == 1 ? g.taxa : 0.6f * g.taxa, g.pesoMenor, g.pesoMaior);
                ++mTreinos;
            }
        }
        if (mAlvoDoTreino >= 0 && mAlvoDoTreino < mRede.camadas.back()) {
            const Vector2 onde = LugarDoNeuronio(camadas - 1, mAlvoDoTreino);
            if (Fora(camadas - 1, mAlvoDoTreino)) {
                // A mira quebrada: a epoca falhou.
                for (int k = 0; k < 14; k += 2) {
                    const float a0 = Math::TwoPi * static_cast<float>(k) / 14.0f, a1 = Math::TwoPi * static_cast<float>(k + 1) / 14.0f;
                    Linha(onde + Vector2(Math::Cos(a0), Math::Sin(a0)) * 20.0f, onde + Vector2(Math::Cos(a1), Math::Sin(a1)) * 20.0f,
                          SDL_Color{150, 150, 170, 120});
                }
            } else if (static_cast<int>(t / 0.15f) % 2 == 0) {
                Circulo(onde, 20.0f, kPerigo, 14);
                Circulo(onde, 26.0f, SDL_Color{kPerigo.r, kPerigo.g, kPerigo.b, 120}, 14);
            }
        }
    }

    const std::string dropout = fora > 0 ? " · " + std::to_string(fora) + (fora == 1 ? " neurônio fora" : " neurônios fora")
                                         : std::string();
    if (etapa == 0) Placar(kFase, 0, "Neurônios: encoste ou atire, e ele sai da rede (dropout)" + dropout);
    else if (etapa == 1) Placar(kFase, 1, "Retropropagação, época " + std::to_string(mTreinos) + ": a rede aprende onde você está" + dropout);
    else Placar(kFase, 2, "Residual: o atalho pula duas camadas, e o dropout não o corta" + dropout);
}

// ---- A repescagem: o MaVIlab ----

Vector2 Michel::Normalizado(const Vector2 p) const {
    const SDL_FRect c = Campo();
    return Vector2((p.x - c.x) / std::max(c.w, 1.0f), (p.y - c.y) / std::max(c.h, 1.0f));
}

void Michel::DesmontarLaboratorio() {
    for (MascaraEmCampo& m : mMascaras) {
        for (BossProjectile*& p : m.pixels) Matar(p);
    }
    mMascaras.clear();
    mPassoDoDecoder = 0;
    mAtalhos.clear();
    mElos.clear();
    mElosAte = -1.0f;
    mProximoRuido = 0.0f;
    mMemorias.clear();
    mProximaPrevisao = 0.0f;
    mLstm = false;
    mRastros.clear();
    mProximoRastro = 0.0f;
    mMiras.clear();
    for (Colega& k : mColegas) Matar(k.p);
    mColegas.clear();
    mConfiancaDosAlunos.clear();
    mDisparosAntes.clear();
    mOndeAntes.clear();
    mRodada = -1;
    mDetectados.clear();
    mCercou = false;
    mRastreioAte.clear();
    mProximoRastreio = 0.0f;
}

void Michel::AtualizarLaboratorio(const float t, const float deltaTime) {
    using namespace Laboratorio;
    const Degrau& g = mDegrauDoLaboratorio;
    const SDL_FRect c = Campo();
    const int etapa = EtapaNoTempo(t);
    const float t1 = kDuracoes[0], t2 = kDuracoes[0] + kDuracoes[1];
    const std::vector<Vector2> alunos = Alunos();
    const Battle* batalha = Batalha();
    Varrer(t, 0.28f, 7.0f);

    // ---- A segmentacao: a mascara do decoder, da grade grossa a fina ----
    if (etapa == 0 && mPassoDoDecoder < kDecoder.size() && t >= kDecoder[mPassoDoDecoder].quando) {
        const PassoDoDecoder& d = kDecoder[mPassoDoDecoder++];
        std::vector<Vector2> agora;
        for (const Vector2& a : alunos) agora.push_back(Normalizado(a));
        MascaraEmCampo m;
        m.m = Segmentar(d.n, agora, mAtalhos, d.dilatacao);
        m.inicio = t;
        m.dispara = t + d.aviso * g.fatorDoAviso;
        // O atalho que se ve: do lugar do passo anterior ate o de agora.
        mElos.clear();
        for (size_t i = 0; i < mAtalhos.size() && i < alunos.size(); ++i) {
            mElos.emplace_back(Vector2(c.x + mAtalhos[i].x * c.w, c.y + mAtalhos[i].y * c.h), alunos[i]);
        }
        mElosAte = m.dispara;
        m.some = m.dispara + g.vidaDoPixel;
        m.espacamento = d.espacamento;
        const int celulas = m.m.Quantas();
        Avisar("Decoder " + std::to_string(d.n) + "×" + std::to_string(d.n) + ": " + std::to_string(celulas) +
                   (celulas == 1 ? " célula" : " células") + (mAtalhos.empty() ? "" : ", com o atalho do passo anterior"),
               d.aviso + 0.3f);
        mMascaras.push_back(std::move(m));
        mAtalhos = agora;
    }
    for (MascaraEmCampo& m : mMascaras) {
        const float w = c.w / static_cast<float>(m.m.n), h = c.h / static_cast<float>(m.m.n);
        if (!m.disparou) {
            for (int k = 1; k < m.m.n; ++k) {
                Linha(Vector2(c.x + w * static_cast<float>(k), c.y), Vector2(c.x + w * static_cast<float>(k), c.y + c.h), kGrade);
                Linha(Vector2(c.x, c.y + h * static_cast<float>(k)), Vector2(c.x + c.w, c.y + h * static_cast<float>(k)), kGrade);
            }
        }
        for (int l = 0; l < m.m.n; ++l) {
            for (int col = 0; col < m.m.n; ++col) {
                if (!m.m.Em(col, l)) continue;
                const Vector2 de(c.x + w * static_cast<float>(col), c.y + h * static_cast<float>(l));
                const Vector2 ate = de + Vector2(w, h);
                if (!m.disparou) {
                    // A celula enche do meio para fora, como uma barra de carregamento: cheia, vira
                    // tiro. No fim, a borda pisca.
                    const float u = std::clamp((t - m.inicio) / std::max(0.05f, m.dispara - m.inicio), 0.0f, 1.0f);
                    const Vector2 meio = (de + ate) * 0.5f, meia = (ate - de) * (0.5f * u);
                    Preencher(de, ate, SDL_Color{kPerigo.r, kPerigo.g, kPerigo.b, 26});
                    Preencher(meio - meia, meio + meia, SDL_Color{kPerigo.r, kPerigo.g, kPerigo.b, 110});
                    if (u < 0.75f || static_cast<int>(t / 0.06f) % 2 == 0) Retangulo(de, ate, kPerigo);
                    if (t >= m.dispara) {
                        for (const Vector2& p : Pixels(de, ate, m.espacamento)) {
                            if (BossProjectile* q = Um(kGatilhoDoPixel, p, "Pixel")) m.pixels.push_back(q);
                        }
                    }
                }
            }
        }
        if (!m.disparou && t >= m.dispara) {
            m.disparou = true;
            SomDoDisparo();
        }
        if (m.disparou && t >= m.some) {
            for (BossProjectile*& p : m.pixels) Matar(p);
            m.pixels.clear();
            m.m.n = 0;
        }
    }
    mMascaras.erase(std::remove_if(mMascaras.begin(), mMascaras.end(), [](const MascaraEmCampo& m) { return m.m.n == 0; }),
                    mMascaras.end());
    if (etapa == 0 && t < mElosAte) {
        for (const auto& [antes, agora] : mElos) {
            Tracejada(antes, agora, SDL_Color{kPerigo.r, kPerigo.g, kPerigo.b, 200}, 10.0f);
            Circulo(antes, 10.0f, SDL_Color{kPerigo.r, kPerigo.g, kPerigo.b, 200}, 10);
        }
    }
    if (etapa == 0) {
        if (t >= mProximoRuido) {
            mProximoRuido = t + 2.0f * g.entreRuidos;
            Um(kGatilhoDoRuido, Vector2(mSorteio.Real(c.x + 20.0f, c.x + c.w - 20.0f), c.y + 96.0f), "Ruido",
               Vector2(0.0f, 150.0f));
        }
        Placar(kFase, 0, "Segmentação: o encoder-decoder pinta onde você está");
        return;
    }

    // ---- A recorrente: a memoria do caminho, e a previsao ----
    if (mMemorias.size() != alunos.size()) mMemorias.assign(alunos.size(), Memoria{});
    for (size_t i = 0; i < alunos.size(); ++i) Lembrar(mMemorias[i], alunos[i], deltaTime, g.esquecimento);
    if (etapa == 1) {
        const float tEtapa = t - t1;
        const bool lstm = tEtapa >= 0.5f * kDuracoes[1];
        if (lstm && !mLstm) {
            mLstm = true;
            Avisar("LSTM: a memória longa lembra também da curva", 1.8f);
        }
        size_t alvo = 0;
        if (batalha) {
            const Player* quem = Aluno();
            for (size_t i = 0; i < batalha->Jogadores().size() && i < mMemorias.size(); ++i) {
                if (batalha->Jogadores()[i] == quem) alvo = i;
            }
        }
        // A memoria que se ve: o rastro de cada aluno, um ponto a cada decimo de segundo.
        if (mRastros.size() != alunos.size()) mRastros.assign(alunos.size(), {});
        if (t >= mProximoRastro) {
            mProximoRastro = t + 0.1f;
            for (size_t i = 0; i < alunos.size(); ++i) {
                mRastros[i].push_back(alunos[i]);
                if (mRastros[i].size() > 10) mRastros[i].erase(mRastros[i].begin());
            }
        }
        for (const auto& rastro : mRastros) {
            for (size_t k = 0; k < rastro.size(); ++k) {
                const auto a = static_cast<Uint8>(40.0f + 160.0f * static_cast<float>(k + 1) / static_cast<float>(rastro.size()));
                Circulo(rastro[k], 3.0f + 0.3f * static_cast<float>(k), SDL_Color{kFantasma.r, kFantasma.g, kFantasma.b, a}, 6);
            }
        }
        // O fantasma: o aluno onde a rede acha que ele vai estar - com a mira em cima. E la que os
        // tiros caem: quem muda de rumo escapa.
        for (const Memoria& m : mMemorias) {
            Vector2 antes = m.onde;
            for (int k = 1; k <= 8; ++k) {
                const Vector2 agora = Prever(m, 0.12f * static_cast<float>(k), lstm);
                if (k % 2 == 0) Linha(antes, agora, kFantasma);
                antes = agora;
            }
            const Vector2 f = antes;
            Circulo(f + Vector2(0.0f, -16.0f), 8.0f, kFantasma, 12);
            Retangulo(f + Vector2(-9.0f, -7.0f), f + Vector2(9.0f, 16.0f), kFantasma);
            Circulo(f, 26.0f, SDL_Color{kPerigo.r, kPerigo.g, kPerigo.b, 170}, 18);
            Linha(f + Vector2(-34.0f, 0.0f), f + Vector2(-18.0f, 0.0f), kPerigo);
            Linha(f + Vector2(18.0f, 0.0f), f + Vector2(34.0f, 0.0f), kPerigo);
            Linha(f + Vector2(0.0f, -34.0f), f + Vector2(0.0f, -18.0f), kPerigo);
            Linha(f + Vector2(0.0f, 18.0f), f + Vector2(0.0f, 34.0f), kPerigo);
        }
        for (const Mira& mira : mMiras) {
            if (t < mira.ate) Tracejada(mira.de, mira.para, SDL_Color{kPerigo.r, kPerigo.g, kPerigo.b, 150}, 12.0f);
        }
        mMiras.erase(std::remove_if(mMiras.begin(), mMiras.end(), [&](const Mira& m) { return t >= m.ate; }), mMiras.end());
        if (t >= mProximaPrevisao && alvo < mMemorias.size()) {
            mProximaPrevisao = t + g.entrePrevisoes;
            // A camera que atira: o professor, e as dos lados do laboratorio, uma de cada vez.
            static const std::array<Vector2, 3> kDe = {Vector2(0.5f, 0.0f), Vector2(0.0f, 0.4f), Vector2(1.0f, 0.4f)};
            const int vez = static_cast<int>(std::lround(tEtapa / std::max(g.entrePrevisoes, 0.05f)));
            const Vector2 r = kDe[static_cast<size_t>(vez % 3)];
            const Vector2 de = vez % 3 == 0 ? GetPosition() : Vector2(c.x + c.w * r.x, c.y + c.h * r.y);
            const Memoria& m = mMemorias[alvo];
            const float tempo = Encontro(m, de, g.velocidadeDaPrevisao, lstm);
            Vector2 para = Prever(m, tempo, lstm);
            para = Vector2(std::clamp(para.x, c.x + 10.0f, c.x + c.w - 10.0f), std::clamp(para.y, c.y + 10.0f, c.y + c.h - 10.0f));
            for (BossProjectile* p : DispararEm(kGatilhoDaPrevisao, de, g.tirosDaPrevisao, AnguloAte(de, para),
                                                g.aberturaDaPrevisao, g.velocidadeDaPrevisao)) {
                Cara(p, "Previsao");
            }
            mMiras.push_back(Mira{de, para, t + 0.35f});
            SomDoDisparo();
        }
        Placar(kFase, 1, lstm ? "LSTM: ela também lembra da curva - quebre o padrão"
                              : "Recorrente: a rede prevê onde você vai estar");
        return;
    }

    // ---- A deteccao: o aluno entre os colegas do MaVIlab ----
    const float tEtapa = t - t2;
    const float topo = c.y + c.h * 0.5f, chao = c.y + c.h - 22.0f;
    if (mColegas.empty()) {
        for (int k = 0; k < kColegas; ++k) {
            Colega colega;
            const Vector2 onde(mSorteio.Real(c.x + 40.0f, c.x + c.w - 40.0f), mSorteio.Real(topo + 20.0f, chao - 20.0f));
            colega.p = Um(kGatilhoDoColega, onde, "Colega" + std::to_string(k));
            if (colega.p) colega.p->DefinirEnfeite(true);
            mColegas.push_back(colega);
        }
        Avisar("MaVIlab: o detector procura você entre os colegas", 1.8f);
    }
    for (Colega& k : mColegas) {
        if (!Vivo(k.p)) continue;
        if (t >= k.muda) {
            k.muda = t + mSorteio.Real(0.5f, 1.5f);
            const bool agitado = mSorteio.Real(0.0f, 1.0f) < 0.55f;
            k.rapidez = agitado ? mSorteio.Real(120.0f, 300.0f) : 0.0f;
            k.tiros = agitado ? mSorteio.Real(2.0f, 7.0f) : mSorteio.Real(0.0f, 1.5f);
            const float a = mSorteio.Real(0.0f, Math::TwoPi);
            k.rumo = Vector2(Math::Cos(a), Math::Sin(a));
        }
        Vector2 onde = k.p->GetPosition() + k.rumo * (k.rapidez * deltaTime);
        if (onde.x < c.x + 20.0f || onde.x > c.x + c.w - 20.0f) k.rumo.x = -k.rumo.x;
        if (onde.y < topo || onde.y > chao) k.rumo.y = -k.rumo.y;
        onde = Vector2(std::clamp(onde.x, c.x + 20.0f, c.x + c.w - 20.0f), std::clamp(onde.y, topo, chao));
        k.p->SetPosition(onde);
        k.confianca = Confianca(k.confianca, k.rapidez, k.tiros, deltaTime);
    }
    if (batalha) {
        const auto& jogadores = batalha->Jogadores();
        if (mConfiancaDosAlunos.size() != jogadores.size()) {
            mConfiancaDosAlunos.assign(jogadores.size(), kConfiancaBase);
            mDisparosAntes.assign(jogadores.size(), 0);
            mOndeAntes.assign(jogadores.size(), Vector2::Zero);
            for (size_t i = 0; i < jogadores.size(); ++i) {
                if (!jogadores[i]) continue;
                mDisparosAntes[i] = jogadores[i]->Disparos();
                mOndeAntes[i] = jogadores[i]->GetPosition();
            }
        }
        for (size_t i = 0; i < jogadores.size(); ++i) {
            const Player* j = jogadores[i];
            if (!j || deltaTime <= 0.0f) continue;
            // O detector e sensivel ao aluno: no Dificil, o movimento e os tiros dele contam mais.
            const float movimento = g.sensibilidade * (j->GetPosition() - mOndeAntes[i]).Length() / deltaTime;
            const float tiros = g.sensibilidade * static_cast<float>(j->Disparos() - mDisparosAntes[i]) / deltaTime;
            mConfiancaDosAlunos[i] = Confianca(mConfiancaDosAlunos[i], movimento, tiros, deltaTime);
            mOndeAntes[i] = j->GetPosition();
            mDisparosAntes[i] = j->Disparos();
        }
    }

    // As caixas de todos: os colegas e os alunos.
    constexpr float kLado = 62.0f;
    std::vector<Caixa> caixas;
    std::vector<float> confiancas;
    std::vector<std::string> nomes;
    std::vector<int> qualAluno;   ///< -1: um colega
    for (size_t k = 0; k < mColegas.size(); ++k) {
        if (!Vivo(mColegas[k].p)) continue;
        caixas.push_back(CaixaEm(mColegas[k].p->GetPosition(), kLado));
        confiancas.push_back(mColegas[k].confianca);
        nomes.push_back(Laboratorio::Colega(static_cast<int>(k)));
        qualAluno.push_back(-1);
    }
    for (size_t i = 0; i < alunos.size() && i < mConfiancaDosAlunos.size(); ++i) {
        caixas.push_back(CaixaEm(alunos[i], kLado));
        confiancas.push_back(mConfiancaDosAlunos[i]);
        nomes.push_back(alunos.size() > 1 ? "J" + std::to_string(i + 1) : std::string("você"));
        qualAluno.push_back(static_cast<int>(i));
    }

    // A caixa de todo mundo, o tempo todo, com a barra da confianca em cima: verde e curta em quem esta
    // quieto, vermelha e cheia em quem corre e atira. A do aluno tem a borda inteira, branca: e assim
    // que ele se acha - e ve a propria barra subir quando atira.
    if (tEtapa >= 0.6f) {
        for (size_t k = 0; k < caixas.size(); ++k) {
            const float cf = std::clamp(confiancas[k], 0.0f, 1.0f);
            const SDL_Color cor{static_cast<Uint8>(90.0f + 165.0f * cf), static_cast<Uint8>(235.0f - 175.0f * cf), 110, 210};
            const bool aluno = qualAluno[k] >= 0;
            Retangulo(caixas[k].de, caixas[k].ate, aluno ? SDL_Color{240, 244, 255, 200} : SDL_Color{cor.r, cor.g, cor.b, 120}, !aluno);
            const Vector2 barra = caixas[k].de - Vector2(0.0f, 11.0f);
            Preencher(barra, barra + Vector2(kLado, 7.0f), SDL_Color{10, 12, 24, 200});
            Preencher(barra, barra + Vector2(kLado * cf, 7.0f), cor);
            Retangulo(barra, barra + Vector2(kLado, 7.0f), SDL_Color{200, 210, 230, 120});
        }
    }

    const float u = tEtapa - 0.6f;
    if (u >= 0.0f) {
        const int rodada = static_cast<int>(u / g.rodadaDoDetector);
        const float naRodada = u - static_cast<float>(rodada) * g.rodadaDoDetector;
        if (rodada != mRodada) {
            mRodada = rodada;
            mDetectados.clear();
            mCercou = false;
        }
        if (naRodada < g.procura) {
            // O detector olha: a varredura cruza o campo, e quando ela chega ao fim ele decide.
            const float x = c.x + c.w * std::clamp(naRodada / std::max(g.procura, 0.05f), 0.0f, 1.0f);
            Preencher(Vector2(x - 26.0f, topo - 10.0f), Vector2(x, chao + 18.0f), SDL_Color{kCaixa.r, kCaixa.g, kCaixa.b, 26});
            Linha(Vector2(x, topo - 10.0f), Vector2(x, chao + 18.0f), kCaixa);
            Placar(kFase, 2, "Detector: procurando você entre os do MaVIlab");
        } else {
            if (mDetectados.empty() && !caixas.empty()) {
                const int quantas = static_cast<int>(std::max<size_t>(1, alunos.size()));
                std::string texto = "Detectado: ";
                for (const int e : Nms(caixas, confiancas, 0.3f, quantas)) {
                    const size_t k = static_cast<size_t>(e);
                    mDetectados.push_back(Detectado{(caixas[k].de + caixas[k].ate) * 0.5f, nomes[k],
                                                    static_cast<int>(std::lround(confiancas[k] * 100.0f)), qualAluno[k]});
                    if (qualAluno[k] >= 0) ++mAlunosDetectados;
                    else ++mColegasDetectados;
                    if (texto.size() > 11) texto += ", ";
                    texto += nomes[k] + " (" + Porcento(confiancas[k]) + ")";
                }
                Avisar(texto, g.avisoDoCerco + 0.6f);
                SomDoDisparo();
            }
            const bool aceso = static_cast<int>(t / 0.1f) % 2 == 0;
            for (const Detectado& d : mDetectados) {
                const Caixa cx = CaixaEm(d.centro, kLado);
                if (!mCercou && aceso) Preencher(cx.de, cx.ate, SDL_Color{kPerigo.r, kPerigo.g, kPerigo.b, 60});
                Retangulo(cx.de, cx.ate, kPerigo);
            }
            if (!mCercou && naRodada >= g.procura + g.avisoDoCerco) {
                mCercou = true;
                // O aluno detectado passa a ser rastreado: a caixa vai com ele.
                if (mRastreioAte.size() != alunos.size()) mRastreioAte.assign(alunos.size(), -1.0f);
                for (const Detectado& d : mDetectados) {
                    if (d.aluno >= 0 && d.aluno < static_cast<int>(alunos.size())) {
                        mRastreioAte[static_cast<size_t>(d.aluno)] = mRelogio + g.rastreio;
                    }
                }
                for (const Detectado& d : mDetectados) {
                    const float giro = mSorteio.Real(0.0f, Math::TwoPi);
                    for (int k = 0; k < g.tirosDoCerco; ++k) {
                        const float a = giro + Math::TwoPi * static_cast<float>(k) / static_cast<float>(g.tirosDoCerco);
                        const Vector2 dir(Math::Cos(a), Math::Sin(a));
                        Um(kGatilhoDoCerco, d.centro + dir * g.raioDoCerco, "Cerco", dir * -(g.raioDoCerco / 0.7f));
                    }
                }
                SomDoDisparo();
            }
            Placar(kFase, 2, "Detector: o NMS ficou com a caixa mais confiante");
        }
    }
    // O rastreador: a caixa segue o aluno detectado, e os cantos atiram nele.
    const bool atira = mRelogio >= mProximoRastreio;
    if (atira) mProximoRastreio = mRelogio + g.entreRastreios;
    for (size_t i = 0; i < mRastreioAte.size() && i < alunos.size(); ++i) {
        if (mRelogio >= mRastreioAte[i]) continue;
        const Caixa cx = CaixaEm(alunos[i], g.ladoDoRastreio);
        Retangulo(cx.de, cx.ate, kPerigo);
        Linha(Vector2(alunos[i].x - 10.0f, alunos[i].y), Vector2(alunos[i].x + 10.0f, alunos[i].y), kPerigo);
        Linha(Vector2(alunos[i].x, alunos[i].y - 10.0f), Vector2(alunos[i].x, alunos[i].y + 10.0f), kPerigo);
        if (!atira) continue;
        // Dois cantos opostos por vez, alternando a diagonal.
        const bool diagonal = static_cast<int>(mRelogio / g.entreRastreios) % 2 == 0;
        const Vector2 cantos[2] = {diagonal ? cx.de : Vector2(cx.ate.x, cx.de.y), diagonal ? cx.ate : Vector2(cx.de.x, cx.ate.y)};
        for (const Vector2& de : cantos) {
            const Vector2 d = alunos[i] - de;
            const float n = std::max(1.0f, d.Length());
            Um(kGatilhoDoCerco, de, "Cerco", d * (170.0f / n));
        }
        SomDoDisparo();
    }
    // O ruido: uma garoa que pede que se ande um pouco.
    if (t >= mProximoRuido) {
        mProximoRuido = t + g.entreRuidos;
        Um(kGatilhoDoRuido, Vector2(mSorteio.Real(c.x + 20.0f, c.x + c.w - 20.0f), c.y + 96.0f), "Ruido", Vector2(0.0f, 160.0f));
    }
}
