//
// A chefe da INF 221 - Engenharia de Software.
//

#include "MariaLucia.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Movements/Escada.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../Player/Player.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {
    /// A fase dos padroes: o Observer e o Decorator, as duas etapas dela em fases.json.
    constexpr const char* kFaseDosPadroes = Observer::kFaseDosPadroes;
    constexpr size_t kEtapaDoDecorator = 1;
    /// O que o placar mostra de um aviso (a mudanca que estourou, a bala que nao matou).
    constexpr float kAviso = 1.3f;
}

MariaLucia::MariaLucia(Scene* scene) : Boss(scene) {
}

void MariaLucia::SortearALuta() {
    mLutaSorteada = true;
    mSorteio = SorteioDaFase(Scrum::kFase);
    Variacao::Sorteio observer = SorteioDaFase(kFaseDosPadroes);
    mSujeitoDaLuta = Observer::Sortear(observer, Observer::DegrauDo(NivelDeDificuldade()));
    Variacao::Sorteio bala = SorteioDaFase(kFaseDaBala);
    mOrdemDasPromessas = BalaDePrata::Sortear(bala);
    const Dificuldade::Nivel n = NivelDeDificuldade();
    mDegrauDaCascata = Cascata::DegrauDo(n);
    mDegrauDoDecorator = Decorator::DegrauDo(n);
    mDegrauDaBala = BalaDePrata::DegrauDo(n);
}

// ---- O que as fases dividem ----

std::vector<BossProjectile*> MariaLucia::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
                                                    const std::optional<float> centro,
                                                    const std::optional<float> abertura,
                                                    const std::optional<float> velocidade, const int alvo) {
    mDisparandoEm = true;
    mOndeDisparar = onde;
    mQuantos = quantos;
    mCentroEm = centro;
    mAberturaEm = abertura;
    mVelocidadeEm = velocidade;
    mCriados.clear();
    Disparar(gatilho, alvo);
    mDisparandoEm = false;
    mQuantos = 0;
    mCentroEm.reset();
    mAberturaEm.reset();
    mVelocidadeEm.reset();
    return mCriados;
}

BossProjectile* MariaLucia::Um(const char* gatilho, const Vector2 onde, const std::string& cara, const float escala,
                               const std::optional<Vector2> velocidade) {
    std::optional<float> centro, rapidez;
    if (velocidade) {
        centro = Math::ToDegrees(Math::Atan2(velocidade->y, velocidade->x));
        rapidez = velocidade->Length();
    }
    const auto criados = DispararEm(gatilho, onde, 1, centro, 0.0f, rapidez);
    if (criados.empty()) return nullptr;
    BossProjectile* p = criados.front();
    for (size_t i = 1; i < criados.size(); ++i) criados[i]->Kill();
    p->SetPosition(onde);
    p->SetScale(escala);
    Cara(p, cara);
    return p;
}

void MariaLucia::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
    if (!mDisparandoEm) return;
    params.firePosition = mOndeDisparar;
    if (mQuantos > 0) params.numProjectiles = mQuantos;
    if (mCentroEm) params.centralAngle = *mCentroEm;
    if (mAberturaEm) params.angle = *mAberturaEm;
    if (mVelocidadeEm) params.projectileSpeed = *mVelocidadeEm;
}

void MariaLucia::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                      const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

float MariaLucia::AnguloAte(const Vector2 de, const Vector2 para) const {
    const Vector2 d = para - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

bool MariaLucia::Vivo(const BossProjectile* p) {
    return p && p->GetState() == ActorState::Active;
}

void MariaLucia::Matar(BossProjectile*& p) {
    if (Vivo(p)) p->Kill();
    p = nullptr;
}

void MariaLucia::Cara(BossProjectile* p, const std::string& cara) {
    if (!p || cara.empty()) return;
    if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) anim->SetAnimation(cara);
}

void MariaLucia::Linha(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Acrescentar(de, ate, cor);
}

void MariaLucia::PlacarDaEtapa(const std::string& fase, const size_t etapa, const std::string& texto) {
    auto etapas = EtapasDa(fase);
    if (etapa >= etapas.size() || etapas[etapa].second == texto) return;
    etapas[etapa].second = texto;
    RegistrarEtapas(fase, etapas);
}

void MariaLucia::PlacarDaFase(const std::string& fase, const std::string& texto) {
    if (AnuncioDaFase(fase) != texto) RegistrarAnuncio(fase, texto);
}

std::optional<Vector2> MariaLucia::AlvoDoMovimento() const {
    return mAlvo;
}

void MariaLucia::Desmontar() {
    for (BossProjectile*& p : mPlacas) Matar(p);
    for (Mudanca& m : mMudancas) Matar(m.p);
    mMudancas.clear();
    mEtapaDaCascata = -1;
    for (Historia& h : mHistorias) Matar(h.p);
    mHistorias.clear();
    for (BossProjectile*& p : mEntregues) Matar(p);
    mEntregues.clear();
    for (BossProjectile*& p : mTitulos) Matar(p);
    Matar(mCursor);
    mSprintDaVez = -2;
    for (Olho& o : mOlhos) Matar(o.p);
    mOlhos.clear();
    for (Decorada& d : mDecoradas) Matar(d.p);
    mDecoradas.clear();
    Matar(mLobo);
    for (Bala& b : mBalas) Matar(b.p);
    mBalas.clear();
    mAvisoAte = -1.0f;
    if (mLinhas) mLinhas->Limpar();
    mAlvo.reset();
}

void MariaLucia::AoTerminarABatalha() {
    Desmontar();
}

void MariaLucia::OnUpdate(const float deltaTime) {
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

    if (fase == Cascata::kFaseDaCascata) AtualizarCascata(t);
    else if (fase == Scrum::kFase) AtualizarScrum(t);
    else if (fase == kFaseDosPadroes) AtualizarPadroes(t, deltaTime);
    else if (fase == kFaseDaBala) AtualizarLobisomem(t, deltaTime);
}

void MariaLucia::OnDerrubou(BossProjectile* qual) {
    // O Decorator: o tiro tira o decorador de fora, e a bala segue com uma camada a menos.
    for (auto it = mDecoradas.begin(); it != mDecoradas.end(); ++it) {
        if (it->p != qual) continue;
        const Decorada antes = *it;
        mDecoradas.erase(it);
        if (antes.camadas <= 0) return;
        Vector2 v = antes.velocidade;
        if (antes.camadas == 1) v = v * (1.0f / Decorator::kFatorDaRapida);   // sai a Rapida
        if (BossProjectile* p = Um(Decorator::kGatilhoDaBala, qual->GetPosition(), Decorator::Cara(antes.camadas - 1), 1.0f, v)) {
            mDecoradas.push_back(Decorada{p, antes.camadas - 1, std::numeric_limits<float>::max(), true, v});
        }
        return;
    }
    // A bala de prata que o aluno derrubou: o teste com usuario a reprovou antes de ela chegar.
    for (auto it = mBalas.begin(); it != mBalas.end(); ++it) {
        if (it->p != qual) continue;
        mAvisoNoPlacar = BalaDePrata::Testada(Promessa(it->promessa));
        mAvisoAte = mRelogio + kAviso;
        mBalas.erase(it);
        return;
    }
}

// ---- Fase 1: a cascata ----

Vector2 MariaLucia::Caixa(const int k) const {
    Escada::Geometria g;
    g.largura = mScene && mScene->GetGame() ? static_cast<float>(mScene->GetGame()->GetWindowWidth()) : 1200.0f;
    return Escada::Degrau(k, static_cast<int>(Cascata::kEtapas.size()), g) + Vector2(0.0f, Cascata::kAbaixoDoDegrau);
}

void MariaLucia::AtualizarCascata(const float t) {
    using namespace Cascata;
    const SDL_FRect campo = Campo();
    const int etapa = EtapaNoTempo(t);
    const int n = static_cast<int>(kEtapas.size());

    // As caixas, uma por etapa; a da vez acesa. O Ponto Extra as leva, e elas voltam.
    for (int k = 0; k < n; ++k) {
        BossProjectile*& placa = mPlacas[static_cast<size_t>(k)];
        if (!Vivo(placa)) {
            placa = Um(kGatilhoDaPlaca, Caixa(k), "");
            if (placa) placa->DefinirEnfeite(true);
        }
        Cara(placa, (k == etapa ? "A" : "P") + std::to_string(k));
        if (k + 1 < n) Linha(Caixa(k) + Vector2(0.0f, 16.0f), Caixa(k + 1) + Vector2(0.0f, -16.0f), kAgua);
    }
    if (etapa != mEtapaDaCascata) {
        mEtapaDaCascata = etapa;
        mProximaQueda = t;
        if (mProximaMudanca < kPrimeiraMudanca) mProximaMudanca = kPrimeiraMudanca;
    }
    if (etapa >= n) return;

    // A cachoeira: o que a etapa produz cai da caixa dela, cada vez mais depressa.
    if (t >= mProximaQueda) {
        mProximaQueda = t + EntreQuedas(etapa, NivelDeDificuldade());
        // Pelas bordas da caixa, uma de cada vez: o vao do meio fica livre.
        mLadoDaQueda = -mLadoDaQueda;
        const float x = mLadoDaQueda * (kBordaDaQueda + mSorteio.Real(0.0f, kEspalhoDaQueda));
        const Vector2 onde = Caixa(etapa) + Vector2(x, 14.0f);
        const float rapidez = VelocidadeDaQueda(etapa) * mDegrauDaCascata.quedaMaisRapida;
        const float desvio = Math::ToRadians(90.0f + mSorteio.Real(-10.0f, 10.0f));
        Um(kGatilhoDaQueda, onde, CaraDaEtapa(etapa), 1.0f, Vector2(Math::Cos(desvio), Math::Sin(desvio)) * rapidez);
    }

    // A mudanca de escopo: o cliente pede de fora, o pedido vai para a primeira caixa e desce.
    if (t >= mProximaMudanca && etapa >= 1) {
        mProximaMudanca = t + mDegrauDaCascata.entreMudancas;
        const auto sorteio = static_cast<std::uint32_t>(mSorteio.Inteiro(0, std::numeric_limits<int>::max()));
        const Vector2 origem = Vector2(campo.x, campo.y) +
                               OrigemDoPedido(sorteio, campo.w, campo.h);
        Vector2 ida = Caixa(0) - origem;
        if (ida.LengthSq() > 0.0f) ida.Normalize();
        if (BossProjectile* p = Um(kGatilhoDoPedido, origem, CaraDaEtapa(0), 1.15f, ida * kVelocidadeDoPedido)) {
            mMudancas.push_back(Mudanca{p, -1, t, origem});
        }
    }
    mMudancas.erase(std::remove_if(mMudancas.begin(), mMudancas.end(), [](const Mudanca& m) { return !Vivo(m.p); }),
                    mMudancas.end());
    for (Mudanca& m : mMudancas) {
        if (m.caixa < 0) {
            // Voando ate os requisitos.
            if ((m.p->GetPosition() - Caixa(0)).LengthSq() < 18.0f * 18.0f ||
                Vector2::Dot(Caixa(0) - m.p->GetPosition(), Caixa(0) - m.de) <= 0.0f) {
                m.caixa = 0;
                m.desde = t;
                m.de = Caixa(0);
                m.p->SetPosition(Caixa(0));
                if (auto* rb = m.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
                DispararEm(kGatilhoDoCusto, Caixa(0), TirosDoCusto(0));
            }
            continue;
        }
        // Na caixa 'caixa': chegou na da etapa da vez, estoura; senao, desce para a seguinte.
        if (m.caixa >= etapa) {
            const int estouro = std::min(Custo(m.caixa) + 1, 7);
            DispararEm(kGatilhoDoEstouro, m.p->GetPosition(), estouro, AnguloAte(m.p->GetPosition(), PosicaoDoAluno()),
                       static_cast<float>(9 * (estouro - 1)));
            mAvisoNoPlacar = PlacarDaMudanca(m.caixa);
            mAvisoAte = mRelogio + kAviso;
            m.p->Kill();
            continue;
        }
        const float u = std::clamp((t - m.desde) / kPorCaixa, 0.0f, 1.0f);
        const Vector2 de = Caixa(m.caixa), para = Caixa(m.caixa + 1);
        // A descida: um arco, como agua caindo de um degrau ao outro.
        const Vector2 aqui = de + (para - de) * u + Vector2(0.0f, -30.0f * std::sin(u * Math::Pi));
        m.p->SetPosition(aqui);
        if (auto* rb = m.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        if (u >= 1.0f) {
            ++m.caixa;
            m.desde = t;
            Cara(m.p, CaraDaEtapa(m.caixa));
            DispararEm(kGatilhoDoCusto, para, TirosDoCusto(m.caixa));
        }
    }

    const std::string texto = mRelogio < mAvisoAte ? mAvisoNoPlacar : std::string();
    if (!texto.empty()) PlacarDaFase(kFaseDaCascata, texto);
    else PlacarDaFase(kFaseDaCascata, "Cascata");
}

// ---- Fase 2: o Scrum ----

void MariaLucia::Quadro(float& esquerda, float& largura, float& topo) const {
    const SDL_FRect c = Campo();
    esquerda = c.x + c.w * 0.1f;
    largura = c.w * 0.72f;
    topo = c.y + 118.0f;
}

void MariaLucia::AtualizarScrum(const float t) {
    using namespace Scrum;
    const SDL_FRect campo = Campo();
    float esquerda = 0.0f, largura = 0.0f, topo = 0.0f;
    Quadro(esquerda, largura, topo);
    const float fundo = topo + kTopoDasVagas + kEntreHistorias * 6.0f;

    // O quadro: as colunas e os titulos.
    for (int c = 0; c <= kColunas; ++c) {
        const float x = esquerda + largura * static_cast<float>(c) / static_cast<float>(kColunas);
        Linha(Vector2(x, topo), Vector2(x, fundo), kColuna);
    }
    Linha(Vector2(esquerda, topo), Vector2(esquerda + largura, topo), kColuna);
    for (int c = 0; c < kColunas; ++c) {
        BossProjectile*& titulo = mTitulos[static_cast<size_t>(c)];
        if (!Vivo(titulo)) {
            titulo = Um(kGatilhoDoTitulo, Vector2(Vaga(c, 0, esquerda, largura, topo).x, topo + 14.0f),
                        kTitulos[static_cast<size_t>(c)]);
            if (titulo) titulo->DefinirEnfeite(true);
        }
    }
    if (!Vivo(mCursor)) {
        mCursor = Um(kGatilhoDoCursor, GetPosition(), "Cursor", 1.2f);
        if (mCursor) mCursor->DefinirEnfeite(true);
        mOndeOCursor = GetPosition();
    }

    const int sprint = SprintNoTempo(t);
    mHistorias.erase(std::remove_if(mHistorias.begin(), mHistorias.end(), [](const Historia& h) { return !Vivo(h.p); }),
                     mHistorias.end());
    if (sprint != mSprintDaVez) {
        mSprintDaVez = sprint;
        mDailiesDaVez = 0;
        if (sprint >= 0 && sprint < kSprints) {
            // O planning: as historias da sprint caem na A FAZER, com os pontos do poker.
            const std::vector<int> pontos = Pontos(mSorteio, Historias(sprint, NivelDeDificuldade()));
            mPontosDaSprint = 0;
            int ordem = 0;
            for (const int p : pontos) {
                const Vector2 vaga = Vaga(0, ordem, esquerda, largura, topo);
                if (BossProjectile* h = Um(kGatilhoDaHistoria, vaga + Vector2(0.0f, -60.0f), Scrum::Cara(p), 1.15f)) {
                    h->DefinirResistencia(1 + p / 3);
                    mHistorias.push_back(Historia{h, p, 0, ordem, t + kEntreTrabalhos});
                    mPontosDaSprint += p;
                }
                ++ordem;
            }
            mFaltavam = {mPontosDaSprint};
            mOndeOCursor = Vaga(0, 0, esquerda, largura, topo);
        } else if (sprint == kSprints) {
            PlacarDaEtapa(kFase, static_cast<size_t>(kSprints) + 1, "Retrospectiva: o que aprendemos?");
        }
    }

    if (sprint >= 0 && sprint < kSprints) {
        const int dailies = Dailies(t - InicioDaSprint(sprint));
        while (mDailiesDaVez < dailies) {
            ++mDailiesDaVez;
            // A daily: o quadro anda uma coluna; o que estava FEITO e entregue.
            DispararEm(kGatilhoDaDaily, GetPosition());
            bool moveu = false;
            for (Historia& h : mHistorias) {
                if (h.coluna >= kColunas - 1) {
                    if (auto* rb = h.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, 230.0f));
                    mEntregues.push_back(h.p);
                    h.p = nullptr;
                    continue;
                }
                if (!moveu) {
                    mOndeOCursor = h.p->GetPosition();
                    moveu = true;
                }
                ++h.coluna;
            }
            mHistorias.erase(std::remove_if(mHistorias.begin(), mHistorias.end(), [](const Historia& h) { return h.p == nullptr; }),
                             mHistorias.end());
            mCliqueAte = t + 0.3f;
            int faltam = 0;
            for (const Historia& h : mHistorias) faltam += h.pontos;
            mFaltavam.push_back(faltam);
        }
        // Cada historia vai para a vaga dela: a pilha da coluna, na ordem de chegada.
        std::array<int, kColunas> naColuna{};
        std::vector<Historia*> ordenadas;
        for (Historia& h : mHistorias) ordenadas.push_back(&h);
        std::sort(ordenadas.begin(), ordenadas.end(), [](const Historia* a, const Historia* b) { return a->ordem < b->ordem; });
        for (Historia* h : ordenadas) {
            const Vector2 vaga = Vaga(h->coluna, naColuna[static_cast<size_t>(h->coluna)]++, esquerda, largura, topo);
            const Vector2 aqui = h->p->GetPosition();
            h->p->SetPosition(aqui + (vaga - aqui) * 0.16f);
            if (auto* rb = h->p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
            // Na FAZENDO, a historia trabalha: atira no aluno, e a mais pesada atira mais.
            if (h->coluna == 1 && t >= h->proximoTrabalho) {
                h->proximoTrabalho = t + kEntreTrabalhos;
                const int tiros = TirosDaHistoria(h->pontos);
                DispararEm(kGatilhoDoTrabalho, aqui, tiros, AnguloAte(aqui, PosicaoDoAluno()), static_cast<float>(12 * (tiros - 1)));
            }
        }
        int faltam = 0;
        for (const Historia& h : mHistorias) faltam += h.pontos;
        PlacarDaEtapa(kFase, static_cast<size_t>(sprint) + 1, Placar(sprint, mDailiesDaVez, faltam));
    }

    // A entrega: o incremento cai no campo do aluno e estoura embaixo.
    const float linhaDaEntrega = campo.y + campo.h * 0.52f;
    for (BossProjectile*& p : mEntregues) {
        if (!Vivo(p)) { p = nullptr; continue; }
        if (p->GetPosition().y < linhaDaEntrega) continue;
        DispararEm(kGatilhoDaEntrega, p->GetPosition());
        p->Kill();
        p = nullptr;
    }
    mEntregues.erase(std::remove(mEntregues.begin(), mEntregues.end(), nullptr), mEntregues.end());

    // O cursor da professora: vai ate a historia que andou, e clica.
    if (mCursor) {
        const Vector2 aqui = mCursor->GetPosition();
        mCursor->SetPosition(aqui + (mOndeOCursor + Vector2(10.0f, 12.0f) - aqui) * 0.2f);
        Cara(mCursor, t < mCliqueAte ? "Clique" : "Cursor");
    }

    // O burndown, no canto: os pontos que faltavam no planning e em cada daily.
    const float gx = campo.x + campo.w - 150.0f, gy = campo.y + 120.0f, gw = 120.0f, gh = 70.0f;
    Linha(Vector2(gx, gy), Vector2(gx, gy + gh), kEixo);
    Linha(Vector2(gx, gy + gh), Vector2(gx + gw, gy + gh), kEixo);
    if (mPontosDaSprint > 0 && mFaltavam.size() >= 2) {
        for (size_t k = 1; k < mFaltavam.size(); ++k) {
            const auto ponto = [&](const size_t i) {
                return Vector2(gx + gw * static_cast<float>(i) / 3.0f,
                               gy + gh - gh * static_cast<float>(mFaltavam[i]) / static_cast<float>(mPontosDaSprint));
            };
            Linha(ponto(k - 1), ponto(k), kBurndown);
        }
    }
    mAlvo = Vector2(campo.x + campo.w * 0.5f, campo.y + 62.0f);
}

// ---- Fase 3: os padroes de projeto ----

void MariaLucia::AtualizarPadroes(const float t, const float deltaTime) {
    const auto* batalha = Batalha();
    if (!batalha) return;
    const auto& alunos = batalha->Jogadores();
    const Vector2 aluno = PosicaoDoAluno();

    if (t < Observer::kFimDoObserver) {
        // O Observer: os olhos se inscrevem um a um e orbitam o aluno, cada vez mais perto.
        mOlhos.erase(std::remove_if(mOlhos.begin(), mOlhos.end(), [](const Olho& o) { return !Vivo(o.p); }), mOlhos.end());
        if (static_cast<int>(mOlhos.size()) < Observer::kMaisObservadores && t >= mProximaInscricao) {
            mProximaInscricao = t + Observer::kEntreInscricoes;
            if (BossProjectile* olho = Um(Observer::kGatilhoDoObservador, GetPosition() + Vector2(0.0f, 40.0f), Observer::kOlhaFrente, 1.3f)) {
                mOlhos.push_back(Olho{olho, t});
            }
        }
        const int n = static_cast<int>(mOlhos.size());
        for (int k = 0; k < n; ++k) {
            BossProjectile* olho = mOlhos[static_cast<size_t>(k)].p;
            const Vector2 lugar = Observer::NaOrbita(aluno, k, n, t, Observer::Raio(t));
            const Vector2 aqui = olho->GetPosition();
            olho->SetPosition(aqui + (lugar - aqui) * std::min(1.0f, 6.0f * deltaTime));
            if (auto* rb = olho->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
        // O sujeito: cada aluno avisa quando o estado dele muda, e todos os olhos respondem.
        if (mSujeitos.size() != alunos.size()) mSujeitos.assign(alunos.size(), mSujeitoDaLuta);
        mPiscando = std::max(0.0f, mPiscando - deltaTime);
        for (size_t i = 0; i < alunos.size(); ++i) {
            const Player* quem = alunos[i];
            if (!quem) continue;
            const auto* rb = quem->GetComponent<RigidBodyComponent>();
            const auto aviso = Observer::Avancar(mSujeitos[i], rb ? rb->GetVelocity() : Vector2::Zero, deltaTime);
            if (aviso == Observer::Aviso::Nenhum) continue;
            const bool consulta = aviso == Observer::Aviso::Consulta;
            for (const Olho& o : mOlhos) {
                DispararEm(consulta ? Observer::kGatilhoDaConsulta : Observer::kGatilhoDaNotificacao, o.p->GetPosition(), 0,
                           AnguloAte(o.p->GetPosition(), quem->GetPosition()), std::nullopt, std::nullopt, static_cast<int>(i));
            }
            mPiscando = 0.2f;
            mAvisoNoPlacar = consulta ? "polling: aluno.getEstado()" : "notifyAll()";
            mAvisoAte = mRelogio + 0.8f;
        }
        for (const Olho& o : mOlhos) {
            for (const Player* quem : alunos) {
                if (quem) Linha(o.p->GetPosition(), quem->GetPosition(), mPiscando > 0.0f ? kNotificando : kAssinatura);
            }
            const int lado = Observer::Olhar(o.p->GetPosition(), aluno);
            Cara(o.p, mPiscando > 0.0f ? Observer::kPisca
                                       : (lado < 0 ? Observer::kOlhaEsquerda : lado > 0 ? Observer::kOlhaDireita : Observer::kOlhaFrente));
        }
        PlacarDaEtapa(kFaseDosPadroes, 0, mRelogio < mAvisoAte ? mAvisoNoPlacar
                                                                : "Observer: " + std::to_string(mOlhos.size()) + " inscritos");
        mAlvo = Vector2(Campo().x + Campo().w / 2.0f, Campo().y + 70.0f);
        return;
    }

    // O fim do Observer: os olhos cancelam a inscricao.
    for (Olho& o : mOlhos) Matar(o.p);
    mOlhos.clear();

    // O Decorator: um componente por vez, de um lado e do outro dela, e cada um ganha decoradores no
    // caminho.
    mDecoradas.erase(std::remove_if(mDecoradas.begin(), mDecoradas.end(), [](const Decorada& d) { return !Vivo(d.p); }),
                     mDecoradas.end());
    if (t >= mProximoComponente) {
        mProximoComponente = std::max(t, Observer::kFimDoObserver) + mDegrauDoDecorator.entreComponentes;
        mLadoDoComponente = -mLadoDoComponente;
        const int lado = mLadoDoComponente > 0.0f ? 1 : -1;
        const Vector2 de = GetPosition() + Vector2(static_cast<float>(lado) * Decorator::kAoLado, 20.0f);
        const float a = Math::ToRadians(Decorator::Saida(lado));
        const Vector2 v(Math::Cos(a) * Decorator::kVelocidadeDoComponente, Math::Sin(a) * Decorator::kVelocidadeDoComponente);
        if (BossProjectile* p = Um(Decorator::kGatilhoDaBala, de, Decorator::Cara(0), 1.0f, v)) {
            mDecoradas.push_back(Decorada{p, 0, t + Decorator::kEntreCamadas, false, v});
        }
    }
    std::vector<Decorada> novas;
    for (Decorada& d : mDecoradas) {
        if (!d.descascada && d.camadas < Decorator::kCamadas && t >= d.proxima) {
            d.proxima = t + Decorator::kEntreCamadas;
            const Decorator::Camada c = Decorator::kOrdem[static_cast<size_t>(d.camadas)];
            ++d.camadas;
            if (c == Decorator::Camada::Rapida) d.velocidade = d.velocidade * Decorator::kFatorDaRapida;
            if (c == Decorator::Camada::Teleguiada) d.guiaAte = t + Decorator::kGuia;
            if (c == Decorator::Camada::Dividida) {
                // Vira duas: a outra metade sai com as mesmas camadas, abrindo para o outro lado.
                const float meia = Math::ToRadians(Decorator::kAberturaDaDividida / 2.0f);
                const float a = Math::Atan2(d.velocidade.y, d.velocidade.x);
                const float r = d.velocidade.Length();
                const Vector2 v1(Math::Cos(a + meia) * r, Math::Sin(a + meia) * r);
                const Vector2 v2(Math::Cos(a - meia) * r, Math::Sin(a - meia) * r);
                d.velocidade = v1;
                if (BossProjectile* outra = Um(Decorator::kGatilhoDaDivisao, d.p->GetPosition(), Decorator::Cara(d.camadas), 1.0f, v2)) {
                    novas.push_back(Decorada{outra, d.camadas, d.proxima, false, v2});
                }
            }
            Cara(d.p, Decorator::Cara(d.camadas));
        }
        if (d.camadas >= Decorator::kCamadas && !d.descascada && t < d.guiaAte) {
            // A Teleguiada: vira para o aluno por kGuia segundos, e depois segue reta - quem anda
            // escapa dela.
            const float agora = Math::Atan2(d.velocidade.y, d.velocidade.x);
            const float rumo = Decorator::Guiar(agora, aluno - d.p->GetPosition(), deltaTime);
            const float r = d.velocidade.Length();
            d.velocidade = Vector2(Math::Cos(rumo) * r, Math::Sin(rumo) * r);
        }
        if (auto* rb = d.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(d.velocidade);
    }
    for (const Decorada& d : novas) mDecoradas.push_back(d);
    // O placar: a composicao da bala mais decorada em campo.
    int camadas = -1;
    for (const Decorada& d : mDecoradas) camadas = std::max(camadas, d.camadas);
    PlacarDaEtapa(kFaseDosPadroes, kEtapaDoDecorator, camadas < 0 ? std::string("Decorator") : Decorator::Placar(camadas));
    mAlvo = Vector2(Campo().x + Campo().w / 2.0f + 220.0f * std::sin(t * 0.6f), Campo().y + 70.0f);
}

// ---- A repescagem: o lobisomem ----

const std::string& MariaLucia::Promessa(const int k) const {
    const auto& promessas = BalaDePrata::Promessas();
    const int i = mOrdemDasPromessas.empty() ? k : mOrdemDasPromessas[static_cast<size_t>(k) % mOrdemDasPromessas.size()];
    return promessas[static_cast<size_t>(i) % promessas.size()];
}

void MariaLucia::AtualizarLobisomem(const float t, const float deltaTime) {
    using namespace BalaDePrata;
    const SDL_FRect campo = Campo();

    // O lobisomem: some com o Ponto Extra, e volta - a complexidade essencial nao some.
    if (!Vivo(mLobo)) {
        if (mLobo) {
            mLobo = nullptr;
            mLoboVolta = t + kVolta;
        }
        if (t >= mLoboVolta) {
            mLobo = Um(kGatilhoDoLobisomem, Vector2(campo.x + campo.w / 2.0f, campo.y + 170.0f), "Lobo",
                       EscalaDoLobisomem(mBalasNoLobo));
            if (mLobo) mLobo->DefinirBlindado(true);
            mLoboNasceu = t;
            mProximoUivo = t + kEntreUivos;
        }
    }
    if (Vivo(mLobo)) {
        Vector2 aqui = PassoDoLobisomem(mLobo->GetPosition(), PosicaoDoAluno(), RapidezDoLobisomem(t - mLoboNasceu), deltaTime);
        aqui.x = std::clamp(aqui.x, campo.x + 40.0f, campo.x + campo.w - 40.0f);
        aqui.y = std::clamp(aqui.y, campo.y + 120.0f, campo.y + campo.h - 40.0f);
        mLobo->SetPosition(aqui);
        if (auto* rb = mLobo->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        if (t >= mProximoUivo) {
            mProximoUivo = t + kEntreUivos;
            DispararEm(kGatilhoDoUivo, aqui);
            mUivandoAte = t + 0.45f;
        }
        Cara(mLobo, t < mUivandoAte ? "Uivo" : "Lobo");
    }

    // As balas de prata: a professora atira as promessas, e elas perseguem o aluno - cada uma sai
    // para um lado da mira e faz a curva.
    const Vector2 aluno = PosicaoDoAluno();
    if (t >= mProximaBala && Vivo(mLobo)) {
        mProximaBala = t + mDegrauDaBala.entreBalas;
        const Vector2 de = GetPosition() + Vector2(0.0f, 30.0f);
        const float lado = (mProximaPromessa % 2 == 0) ? 1.0f : -1.0f;
        const float a = Math::ToRadians(AnguloAte(de, aluno) + lado * kDesvioDaSaida);
        if (BossProjectile* b = Um(kGatilhoDaBala, de, "", 1.0f, Vector2(Math::Cos(a), Math::Sin(a)) * kVelocidadeDaBala)) {
            mBalas.push_back(Bala{b, mProximaPromessa, t + kGuiaDaBala});
            mAvisoNoPlacar = Promessa(mProximaPromessa) + "?";
            mAvisoAte = mRelogio + kAviso;
        }
        ++mProximaPromessa;
    }
    mBalas.erase(std::remove_if(mBalas.begin(), mBalas.end(), [](const Bala& b) { return !Vivo(b.p); }), mBalas.end());
    for (Bala& b : mBalas) {
        if (t >= b.guiaAte) continue;
        auto* rb = b.p->GetComponent<RigidBodyComponent>();
        if (!rb) continue;
        const Vector2 v = rb->GetVelocity();
        const float rumo = Guiar(Math::Atan2(v.y, v.x), aluno - b.p->GetPosition(), mDegrauDaBala.giro, deltaTime);
        rb->SetVelocity(Vector2(Math::Cos(rumo), Math::Sin(rumo)) * kVelocidadeDaBala);
    }
    for (auto it = mBalas.begin(); it != mBalas.end();) {
        if (!Vivo(mLobo) || !Atingiu(it->p->GetPosition(), mLobo->GetPosition(), mLobo->GetScale())) { ++it; continue; }
        // A bala chega e nao mata: se desfaz em bugs, e o lobisomem cresce.
        const auto* rb = it->p->GetComponent<RigidBodyComponent>();
        const Vector2 v = rb ? rb->GetVelocity() : Vector2(0.0f, 1.0f);
        DispararEm(kGatilhoDoEstilhaco, mLobo->GetPosition(), 0, Math::ToDegrees(Math::Atan2(v.y, v.x)));
        it->p->Kill();
        ++mBalasNoLobo;
        mLobo->SetScale(EscalaDoLobisomem(mBalasNoLobo));
        mAvisoNoPlacar = NaoMatou(Promessa(it->promessa));
        mAvisoAte = mRelogio + kAviso;
        it = mBalas.erase(it);
    }
    PlacarDaFase(kFaseDaBala, mRelogio < mAvisoAte ? mAvisoNoPlacar : std::string("N\xc3\xa3o h\xc3\xa1 bala de prata"));
    mAlvo.reset();
}
