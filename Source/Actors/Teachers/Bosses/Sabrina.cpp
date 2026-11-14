//
// A chefe da INF 491 - Topicos Especiais: Bioinformatica, a terceira optativa.
//

#include "Sabrina.h"

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
#include "BossesProjectiles/BossProjectile.h"

namespace {
    using Pareamento::Base;
    using Pareamento::Familia;

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

    std::string Graus(const float t) {
        return std::to_string(static_cast<int>(std::lround(t))) + " \xc2\xb0" "C";
    }

    std::string Letras(const std::vector<Base>& v, const size_t de, const size_t quantas) {
        std::string s;
        for (size_t k = de; k < v.size() && k < de + quantas; ++k) s += Pareamento::Letra(v[k]);
        return s;
    }

    std::string LetraDa(const Base b) {
        return std::string(1, Pareamento::Letra(b));
    }

    const char* CaraDaFamilia(const Familia f) {
        switch (f) {
            case Familia::Hidrofobico: return "Hidrofobico";
            case Familia::Positivo: return "Positivo";
            case Familia::Negativo: return "Negativo";
            default: return "Polar";
        }
    }

    SDL_Color CorDaFamilia(const Familia f, const Uint8 alfa) {
        switch (f) {
            case Familia::Hidrofobico: return SDL_Color{255, 210, 90, alfa};
            case Familia::Positivo: return SDL_Color{110, 160, 255, alfa};
            case Familia::Negativo: return SDL_Color{255, 110, 110, alfa};
            default: return SDL_Color{120, 230, 160, alfa};
        }
    }

    Familia FamiliaQualquer(Variacao::Sorteio& s) {
        // Quase metade hidrofobica, como nas proteinas de verdade.
        const float u = s.Real(0.0f, 1.0f);
        if (u < 0.45f) return Familia::Hidrofobico;
        if (u < 0.75f) return Familia::Polar;
        if (u < 0.88f) return Familia::Positivo;
        return Familia::Negativo;
    }

    Vector2 Girado(const Vector2 v, const float graus) {
        const float a = Math::ToRadians(graus), c = Math::Cos(a), s = Math::Sin(a);
        return Vector2(v.x * c - v.y * s, v.x * s + v.y * c);
    }

    float Suave(const float u) {
        const float x = std::clamp(u, 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    }
}

Sabrina::Sabrina(Scene* scene) : Boss(scene) {
}

void Sabrina::SortearALuta() {
    mLutaSorteada = true;
    mSorteio = SorteioDaFase(Helice::kFase);
    const Dificuldade::Nivel n = NivelDeDificuldade();
    mDegrauDaHelice = Helice::DegrauDo(n);
    mDegrauDoCodigo = Pareamento::DegrauDo(n);
    mDegrauDaProteina = Dobramento::DegrauDo(n);
    mDegrauDoSequenciamento = Sequenciamento::DegrauDo(n);
}

// ---- O que as fases dividem ----

std::vector<BossProjectile*> Sabrina::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
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

BossProjectile* Sabrina::Um(const char* gatilho, const Vector2 onde, const std::string& cara,
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

BossProjectile* Sabrina::Enfeite(const char* gatilho, const Vector2 onde, const std::string& cara) {
    BossProjectile* p = Um(gatilho, onde, cara);
    if (p) {
        p->DefinirEnfeite(true);
        p->DefinirMargemDeSaida(3.0f, 1.0f);
    }
    return p;
}

std::vector<BossProjectile*> Sabrina::Bases(const char* gatilho, const Vector2 de, const Base b, const int quantos,
                                            const float centro, const float abertura, const float velocidade,
                                            const bool comClasse) {
    auto criados = DispararEm(gatilho, de, quantos, centro, abertura, velocidade);
    for (BossProjectile* p : criados) {
        Cara(p, Pareamento::Cara(b));
        p->DefinirClasse(comClasse ? Pareamento::ClasseDoTiro(b) : -1);
    }
    return criados;
}

void Sabrina::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
    if (!mDisparandoEm) return;
    params.firePosition = mOndeDisparar;
    if (mQuantos > 0) params.numProjectiles = mQuantos;
    if (mCentroEm) params.centralAngle = *mCentroEm;
    if (mAberturaEm) params.angle = *mAberturaEm;
    if (mVelocidadeEm) params.projectileSpeed = *mVelocidadeEm;
}

void Sabrina::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                   const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

float Sabrina::AnguloAteOAluno(const Vector2 de) const {
    return AnguloAte(de, PosicaoDoAluno());
}

float Sabrina::AnguloAte(const Vector2 de, const Vector2 para) {
    const Vector2 d = para - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

bool Sabrina::Vivo(const BossProjectile* p) {
    return p && p->GetState() == ActorState::Active;
}

void Sabrina::Matar(BossProjectile*& p) {
    if (Vivo(p)) p->Kill();
    p = nullptr;
}

void Sabrina::Cara(BossProjectile* p, const std::string& cara) {
    if (!p || cara.empty()) return;
    if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) anim->SetAnimation(cara);
}

void Sabrina::Parar(BossProjectile* p) {
    if (!p) return;
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
}

void Sabrina::Mover(BossProjectile* p, const Vector2 onde, const Vector2 velocidade) {
    if (!p) return;
    p->SetPosition(onde);
    // A velocidade so serve ao raspao e ao desenho: a posicao quem manda e a professora.
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(velocidade);
}

std::vector<Player*> Sabrina::Jogadores() const {
    std::vector<Player*> v;
    if (const Battle* b = Batalha()) {
        for (Player* p : b->Jogadores()) {
            if (p) v.push_back(p);
        }
    }
    return v;
}

bool Sabrina::NoCampo(const Vector2 p, const float folga) const {
    const SDL_FRect c = Campo();
    return p.x >= c.x - folga && p.x <= c.x + c.w + folga && p.y >= c.y - folga && p.y <= c.y + c.h + folga;
}

void Sabrina::Linha(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Acrescentar(de, ate, cor);
}

void Sabrina::Tracejada(const Vector2 de, const Vector2 ate, const SDL_Color cor, const float traco) {
    const Vector2 d = ate - de;
    const float comprimento = d.Length();
    if (comprimento < 1.0f) return;
    const Vector2 u = d * (1.0f / comprimento);
    for (float s = 0.0f; s < comprimento; s += 2.0f * traco) {
        Linha(de + u * s, de + u * std::min(comprimento, s + traco), cor);
    }
}

void Sabrina::Circulo(const Vector2 centro, const float raio, const SDL_Color cor, const int lados) {
    for (int k = 0; k < lados; ++k) {
        const float a0 = Math::TwoPi * static_cast<float>(k) / static_cast<float>(lados);
        const float a1 = Math::TwoPi * static_cast<float>(k + 1) / static_cast<float>(lados);
        Linha(centro + Vector2(Math::Cos(a0), Math::Sin(a0)) * raio, centro + Vector2(Math::Cos(a1), Math::Sin(a1)) * raio, cor);
    }
}

void Sabrina::Retangulo(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    const Vector2 cantos[4] = {de, Vector2(ate.x, de.y), ate, Vector2(de.x, ate.y)};
    for (int k = 0; k < 4; ++k) Linha(cantos[k], cantos[(k + 1) % 4], cor);
}

void Sabrina::Preencher(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Preencher(de, ate, cor);
}

void Sabrina::Termometro(const float agora, const float minima, const float maxima) {
    const SDL_FRect c = Campo();
    const float x = c.x + c.w - 30.0f, topo = c.y + 150.0f, fundo = c.y + 520.0f;
    const float u = std::clamp((agora - minima) / std::max(1.0f, maxima - minima), 0.0f, 1.0f);
    const auto mistura = [&](const Uint8 a, const Uint8 b) {
        return static_cast<Uint8>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * u);
    };
    const SDL_Color cor = {mistura(kFrio.r, kCalor.r), mistura(kFrio.g, kCalor.g), mistura(kFrio.b, kCalor.b), 220};
    Preencher(Vector2(x - 5.0f, fundo - (fundo - topo) * u), Vector2(x + 5.0f, fundo), cor);
    Retangulo(Vector2(x - 7.0f, topo - 2.0f), Vector2(x + 7.0f, fundo), kEsqueleto);
    Circulo(Vector2(x, fundo + 11.0f), 11.0f, cor, 14);
    // As marcas de dez em dez graus.
    for (float g = std::ceil(minima / 10.0f) * 10.0f; g <= maxima; g += 10.0f) {
        const float y = fundo - (fundo - topo) * (g - minima) / std::max(1.0f, maxima - minima);
        Linha(Vector2(x - 12.0f, y), Vector2(x - 7.0f, y), kEsqueleto);
    }
}

void Sabrina::PlacarDaEtapa(const std::string& fase, const size_t etapa, const std::string& texto) {
    auto etapas = EtapasDa(fase);
    if (etapa >= etapas.size() || etapas[etapa].second == texto) return;
    etapas[etapa].second = texto;
    RegistrarEtapas(fase, etapas);
}

void Sabrina::Avisar(const std::string& texto, const float duracao) {
    mAviso = texto;
    mAvisoAte = mRelogio + duracao;
}

void Sabrina::Placar(const std::string& fase, const size_t etapa, const std::string& texto) {
    const std::string& vale = mRelogio < mAvisoAte ? mAviso : texto;
    if (!vale.empty()) PlacarDaEtapa(fase, etapa, vale);
}

std::optional<Vector2> Sabrina::AlvoDoMovimento() const {
    return mAlvo;
}

void Sabrina::Varrer(const float t, const float amplitude, const float periodo) {
    const SDL_FRect c = Campo();
    mAlvo = Vector2(c.x + c.w * (0.5f + amplitude * Math::Sin(Math::TwoPi * t / periodo)), c.y + 62.0f);
}

SDL_Color Sabrina::CorDa(const Base b, const Uint8 alfa) {
    switch (b) {
        case Base::A: return SDL_Color{110, 220, 120, alfa};
        case Base::C: return SDL_Color{100, 170, 255, alfa};
        case Base::G: return SDL_Color{255, 200, 70, alfa};
        default: return SDL_Color{255, 100, 100, alfa};
    }
}

void Sabrina::Posicionar(Conta& c, const Helice::NaTela& tela, const bool frente, const char* gatilho,
                         const std::string& cara, const float deltaTime, const SDL_Color contorno) {
    const Vector2 antes = c.posta ? c.onde : tela.onde;
    c.onde = tela.onde;
    c.posta = true;
    if (!frente || !NoCampo(tela.onde, -4.0f)) {
        // Atras: o contorno apagado, por onde se passa.
        Matar(c.p);
        c.frente = false;
        if (NoCampo(tela.onde, 0.0f)) Circulo(tela.onde, 6.5f * tela.escala, contorno, 8);
        return;
    }
    if (!Vivo(c.p)) {
        c.p = Um(gatilho, tela.onde, cara);
        if (c.p) c.p->DefinirMargemDeSaida(2.0f, 1.0f);
    }
    c.frente = true;
    if (!c.p) return;
    Cara(c.p, cara);
    const Vector2 v = deltaTime > 0.0f ? (tela.onde - antes) * (1.0f / deltaTime) : Vector2::Zero;
    Mover(c.p, tela.onde, v);
    c.p->SetScale(tela.escala);
}

void Sabrina::Desmontar() {
    DesmontarHelice();
    DesmontarCodigo();
    DesmontarProteina();
    DesmontarSequenciamento();
    if (mLinhas) mLinhas->Limpar();
    mAlvo.reset();
    mAvisoAte = -1.0f;
    mEtapaAnterior = -1;
}

void Sabrina::AoTerminarABatalha() {
    Desmontar();
}

void Sabrina::OnUpdate(const float deltaTime) {
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

    if (fase == Helice::kFase) AtualizarHelice(t, deltaTime);
    else if (fase == Pareamento::kFase) AtualizarCodigo(t, deltaTime);
    else if (fase == Dobramento::kFase) AtualizarProteina(t, deltaTime);
    else if (fase == Sequenciamento::kFase) AtualizarSequenciamento(t, deltaTime);
}

void Sabrina::OnDerrubou(BossProjectile* qual) {
    // A protease: o residuo atingido corta a cadeia.
    for (size_t i = 0; i < mCadeia.size(); ++i) {
        if (mCadeia[i].p != qual) continue;
        mCadeia[i].p = nullptr;   // ja caiu: o tiro do aluno o derrubou
        CortarACadeia(i);
        return;
    }
    // O primer derrubado: aquela fita nao se copia neste ciclo.
    for (Copia& c : mCopias) {
        if (c.primer != qual) continue;
        c.primer = nullptr;
        c.primerCaiu = true;
        ++mPrimersDerrubados;
        return;
    }
    // A ponta aparada do contig.
    if (!mContig.empty()) {
        const bool tras = qual == mContig.front() && mAtras > 0;
        const bool frente = qual == mContig.back() && mFrente > 0;
        if (tras || frente) {
            if (tras) {
                mContig.erase(mContig.begin());
                --mAtras;
            } else {
                mContig.pop_back();
                --mFrente;
            }
            MontarOContig();
            return;
        }
    }
    // O genoma: no alvo, a Cas9 corta; fora dele, a conta volta - sem o guia, ela nao corta.
    for (FitaDoGenoma& f : mGenomas) {
        for (size_t k = 0; k < f.contas.size(); ++k) {
            if (f.contas[k] != qual) continue;
            const int i = static_cast<int>(k);
            const bool noAlvo = i >= f.genoma.alvo && i < f.genoma.alvo + static_cast<int>(mGuia.size());
            const Vector2 onde = qual->GetPosition();
            f.contas[k] = nullptr;
            if (noAlvo && !f.cortada) {
                f.cortada = true;
                for (int j = f.genoma.alvo; j < f.genoma.alvo + static_cast<int>(mGuia.size()); ++j) {
                    Matar(f.contas[static_cast<size_t>(j)]);
                }
                for (int j = f.genoma.alvo - 1; j < f.genoma.alvo + static_cast<int>(mGuia.size()); ++j) {
                    if (j >= 0 && j < static_cast<int>(f.elos.size())) Matar(f.elos[static_cast<size_t>(j)]);
                }
                ++mEdicoes;
                if (Battle* b = Batalha()) b->Conquistar(2.0f);
                Avisar("Corte no alvo! A Cas9 abriu a fita  \xc2\xb7  +2", 1.4f);
                Matar(mTesoura);
                mTesoura = Enfeite(Sequenciamento::kGatilhoDaTesoura, onde, "Tesoura");
                mTesouraAte = mRelogio + 0.6f;
            } else if (!noAlvo) {
                BossProjectile* p = Um(Sequenciamento::kGatilhoDoGenoma, onde, "G" + LetraDa(f.genoma.fita[k]));
                if (p) {
                    p->DefinirDerrubavel(true);
                    p->DefinirBlindado(false);
                    p->DefinirResistencia(1);
                }
                f.contas[k] = p;
                // Uma vez por fita: o placar e onde o guia esta escrito.
                if (!f.avisou) Avisar("Sem o guia, a Cas9 n\xc3\xa3o corta", 0.9f);
                f.avisou = true;
            }
            return;
        }
    }
}

// ---- Fase 1: a dupla helice ----

void Sabrina::DesmontarHelice() {
    for (auto& fita : mContas) {
        for (Conta& c : fita) Matar(c.p);
        fita.clear();
    }
    mMolde.clear();
    mFormaDaHelice = -1;
    mGiro = 0.0f;
    mGiroAntes = 0.0f;
    mSoltouEm.clear();
    mSoltos = 0;
    mProximoLequeDaHelice = 0.0f;
}

void Sabrina::AtualizarHelice(const float t, const float deltaTime) {
    using namespace Helice;
    const Degrau& g = mDegrauDaHelice;
    const SDL_FRect c = Campo();
    const int etapa = EtapaNoTempo(t);
    const float inicio = etapa == 0 ? 0.0f : etapa == 1 ? kDuracoes[0] : kDuracoes[0] + kDuracoes[1];
    const float tEtapa = t - inicio;
    Varrer(t, 0.3f, 8.0f);

    // A forma: de pe na helice e na desnaturacao, deitada na torcao.
    const int forma = etapa == 1 ? 1 : 0;
    const int pares = forma == 0 ? g.pares : std::max(2, static_cast<int>((c.w - 16.0f) / g.passoDeitada) + 1);
    const int porPar = forma == 0 ? g.contasPorPar : g.contasPorParDeitada;
    const int contas = (pares - 1) * porPar + 1;
    if (forma != mFormaDaHelice || etapa != mEtapaAnterior) {
        for (auto& fita : mContas) {
            for (Conta& k : fita) Matar(k.p);
            fita.assign(static_cast<size_t>(contas), Conta{});
        }
        mFormaDaHelice = forma;
        mSoltouEm.assign(static_cast<size_t>(pares), -1.0f);
        mSoltos = 0;
        mEtapaAnterior = etapa;
    }
    if (static_cast<int>(mMolde.size()) < pares) mMolde = Sortear(mSorteio, std::max(pares, 48));

    float giro = g.giro;
    if (etapa == 1) giro = g.giroDeitada;
    if (etapa == 2) giro = g.giro + (g.giroQuente - g.giro) * std::clamp(tEtapa / kDuracoes[2], 0.0f, 1.0f);
    mGiroAntes = mGiro;
    mGiro += giro * deltaTime;

    if (forma == 0) {
        const float alto = c.y + 150.0f;
        float eixo = c.x + 0.5f * c.w;
        if (etapa == 0) eixo += g.varredura * c.w * Math::Sin(Math::TwoPi * tEtapa / g.periodo);
        else eixo += 0.1f * c.w * Math::Sin(Math::TwoPi * tEtapa / kDuracoes[2]);
        const Vector2 centro(eixo, alto + 0.5f * static_cast<float>(pares - 1) * g.passo);

        // A desnaturacao: o calor sobe e cada par solta na temperatura dele.
        float temperatura = 0.0f;
        const std::vector<Base> fita(mMolde.begin(), mMolde.begin() + pares);
        if (etapa == 2) {
            temperatura = Temperatura(tEtapa, kDuracoes[2], g.temperaturaInicial, g.temperaturaFinal);
            for (int k = 0; k < pares; ++k) {
                if (mSoltouEm[static_cast<size_t>(k)] >= 0.0f || temperatura < Fusao(fita, k)) continue;
                mSoltouEm[static_cast<size_t>(k)] = mRelogio;
                ++mSoltos;
                // O par solta e as duas bases saem, cada uma da sua fita, no aluno.
                for (int f = 0; f < 2; ++f) {
                    const Base b = f == 0 ? fita[static_cast<size_t>(k)] : Pareamento::Complementar(fita[static_cast<size_t>(k)], false);
                    const Vector2 de = mContas[f][static_cast<size_t>(k * porPar)].onde;
                    Bases(kGatilhoDaBase, de, b, g.tirosDaFusao, AnguloAteOAluno(de) + mSorteio.Real(-9.0f, 9.0f),
                          g.aberturaDaFusao, g.velocidadeDaFusao, false);
                }
                if (mSoltos % 3 == 1) SomDoDisparo();
            }
            Termometro(temperatura, 60.0f, 105.0f);
        }
        const auto raioDoPar = [&](const int k) {
            const float soltou = mSoltouEm[static_cast<size_t>(std::clamp(k, 0, pares - 1))];
            if (etapa != 2 || soltou < 0.0f) return g.raio;
            return RaioDoPar(mRelogio - soltou, g.raio, g.inchaco, g.tempoDoInchaco);
        };

        // As duas fitas: so a frente machuca.
        for (int f = 0; f < 2; ++f) {
            for (int j = 0; j < contas; ++j) {
                const float s = static_cast<float>(j) / static_cast<float>(porPar);
                const int k0 = static_cast<int>(std::floor(s));
                const float u = s - static_cast<float>(k0);
                const float raio = raioDoPar(k0) + (raioDoPar(k0 + 1) - raioDoPar(k0)) * u;
                Ponto3 p = DePe(s, raio, g.passo, Angulo(f, s, mGiro, g.torcao, g.sulco));
                p.y -= 0.5f * static_cast<float>(pares - 1) * g.passo;
                Posicionar(mContas[f][static_cast<size_t>(j)], Projetar(p, centro, kDistanciaDaCamera),
                           NaFrente(p.z, raio, g.limiar), kGatilhoDaFita, f == 0 ? "Fita0" : "Fita1", deltaTime, kFundo);
            }
        }

        // Os degraus: cada par, metade na cor de cada base, com as pontes de hidrogenio no meio.
        for (int k = 0; k < pares; ++k) {
            if (etapa == 2 && mSoltouEm[static_cast<size_t>(k)] >= 0.0f) continue;
            const Vector2 a = mContas[0][static_cast<size_t>(k * porPar)].onde;
            const Vector2 b = mContas[1][static_cast<size_t>(k * porPar)].onde;
            const Vector2 meio = (a + b) * 0.5f;
            const Base base = fita[static_cast<size_t>(k)];
            Linha(a, meio, CorDa(base, 170));
            Linha(meio, b, CorDa(Pareamento::Complementar(base, false), 170));
            const Vector2 d = b - a;
            const float l = d.Length();
            if (l < 12.0f) continue;
            const Vector2 u = d * (1.0f / l), n(-u.y, u.x);
            const int pontes = Pareamento::Pontes(base);
            for (int q = 0; q < pontes; ++q) {
                const Vector2 m = meio + u * ((static_cast<float>(q) - 0.5f * static_cast<float>(pontes - 1)) * 5.0f);
                Linha(m - n * 4.0f, m + n * 4.0f, kPonte);
            }
        }

        // Na helice, a fita molde solta a base de cada par que passa pela borda.
        if (etapa == 0) {
            for (int k = 0; k < pares; ++k) {
                if (k % std::max(1, g.aCadaPar) != 0) continue;
                for (int f = 0; f < (g.duasFitasAtiram ? 2 : 1); ++f) {
                    const float antes = Angulo(f, static_cast<float>(k), mGiroAntes, g.torcao, g.sulco);
                    const float agora = Angulo(f, static_cast<float>(k), mGiro, g.torcao, g.sulco);
                    const int lado = Helice::Borda(antes, agora);
                    if (lado == 0) continue;
                    const Base b = f == 0 ? fita[static_cast<size_t>(k)] : Pareamento::Complementar(fita[static_cast<size_t>(k)], false);
                    const Vector2 de = mContas[f][static_cast<size_t>(k * porPar)].onde;
                    Bases(kGatilhoDaBase, de, b, 1, (lado > 0 ? 16.0f : 164.0f) + mSorteio.Real(-6.0f, 6.0f), 0.0f,
                          g.velocidadeDaBase, false);
                }
            }
            Placar(kFase, 0, "Dupla h\xc3\xa9lice: s\xc3\xb3 a frente machuca \xe2\x80\x94 passe pelo fundo");
        } else {
            Placar(kFase, 2, "Desnatura\xc3\xa7\xc3\xa3o: " + Graus(temperatura) + "  \xc2\xb7  " + std::to_string(mSoltos) + " de " +
                                 std::to_string(pares) + " pares soltos");
        }
        return;
    }

    // ---- A torcao: a helice deitada desce sobre o aluno como uma rosca ----
    const float topo = c.y + 150.0f, chao = c.y + c.h - 26.0f;
    const float desce = 0.5f * (1.0f - Math::Cos(Math::TwoPi * tEtapa / g.periodoDaDescida));
    const Vector2 centro(c.x + 0.5f * c.w, topo + (chao - topo) * desce);
    const float torcao = g.torcaoInicial + (g.torcaoFinal - g.torcaoInicial) * std::clamp(tEtapa / kDuracoes[1], 0.0f, 1.0f);
    for (int f = 0; f < 2; ++f) {
        for (int j = 0; j < contas; ++j) {
            const float s = static_cast<float>(j) / static_cast<float>(porPar);
            Ponto3 p = Deitada(s, g.raioDeitada, g.passoDeitada, Angulo(f, s, mGiro, torcao, g.sulco));
            p.x -= 0.5f * static_cast<float>(pares - 1) * g.passoDeitada;
            Posicionar(mContas[f][static_cast<size_t>(j)], Projetar(p, centro, kDistanciaDaCamera),
                       NaFrente(p.z, g.raioDeitada, g.limiar), kGatilhoDaFita, f == 0 ? "Fita0" : "Fita1", deltaTime, kFundo);
        }
    }
    for (int k = 0; k < pares; k += 2) {
        const Vector2 a = mContas[0][static_cast<size_t>(k * porPar)].onde;
        const Vector2 b = mContas[1][static_cast<size_t>(k * porPar)].onde;
        const Base base = mMolde[static_cast<size_t>(k)];
        Linha(a, (a + b) * 0.5f, CorDa(base, 120));
        Linha((a + b) * 0.5f, b, CorDa(Pareamento::Complementar(base, false), 120));
    }
    // A professora atira por cima, em leque.
    if (t >= mProximoLequeDaHelice) {
        mProximoLequeDaHelice = t + g.entreLeques;
        const Vector2 de = GetPosition() + Vector2(0.0f, 34.0f);
        const Base b = Pareamento::kDna[static_cast<size_t>(mSorteio.Inteiro(0, 3))];
        Bases(kGatilhoDoLeque, de, b, g.tirosDoLeque, AnguloAteOAluno(de), 24.0f, g.velocidadeDoLeque, false);
    }
    const float voltas = torcao * static_cast<float>(pares - 1) / Math::TwoPi;
    Placar(kFase, 1, "Tor\xc3\xa7\xc3\xa3o: " + Decimal(voltas, 1) + " voltas  \xc2\xb7  atravesse pela fita de tr\xc3\xa1s");
}

// ---- Fase 2: o codigo ----

void Sabrina::DesmontarCodigo() {
    for (Selo& s : mSelos) Matar(s.p);
    mSelos.clear();
    for (Player* p : Jogadores()) p->DefinirClasse(-1);
    for (Capsula& c : mCapsulas) Matar(c.p);
    mCapsulas.clear();
    mOrbita = 0.0f;
    mAbsorvidas = 0;
    if (mPremioGuardado > 0.0f) {
        if (Battle* b = Batalha()) b->Conquistar(mPremioGuardado);
    }
    mPremioGuardado = 0.0f;
    mProximaOnda = 0.0f;
    mOndas = 0;
    mFitaMolde.clear();
    for (BossProjectile*& p : mLetrasDoMolde) Matar(p);
    mLetrasDoMolde.clear();
    for (BossProjectile*& p : mLetrasDoRna) Matar(p);
    mLetrasDoRna.clear();
    mLidas = 0;
    Matar(mMaquina);
    mMensageiro.clear();
    for (BossProjectile*& p : mLetrasDoMensageiro) Matar(p);
    mLetrasDoMensageiro.clear();
    mCodons = 0;
    mTeleguiados.clear();
    for (BossProjectile*& p : mPolipeptideo) Matar(p);
    mPolipeptideo.clear();
    mOndeDoPolipeptideo.clear();
}

void Sabrina::AtualizarBases(const float deltaTime, const bool rna) {
    using namespace Pareamento;
    const Degrau& g = mDegrauDoCodigo;
    const SDL_FRect c = Campo();
    const auto alunos = Jogadores();
    if (mSelos.size() != alunos.size()) {
        for (Selo& s : mSelos) Matar(s.p);
        mSelos.assign(alunos.size(), Selo{});
        // Na dupla, cada um comeca com uma base diferente.
        const int primeira = mSorteio.Inteiro(0, 3);
        for (size_t i = 0; i < mSelos.size(); ++i) mSelos[i].classe = (primeira + 2 * static_cast<int>(i)) % 4;
    }

    // As capsulas de nucleotideo livre giram na metade de baixo.
    if (mCapsulas.empty()) {
        mCapsulas.assign(4, Capsula{});
        for (int k = 0; k < 4; ++k) mCapsulas[static_cast<size_t>(k)].classe = k;
    }
    mOrbita += g.rapidezDaOrbita * deltaTime;
    const Vector2 centro(c.x + 0.5f * c.w, c.y + 0.69f * c.h), raios(0.38f * c.w, 0.15f * c.h);
    for (int k = 0; k < 4; ++k) {
        Capsula& cap = mCapsulas[static_cast<size_t>(k)];
        const Vector2 onde = NaOrbita(k, 4, mOrbita, centro, raios, 1.0f);
        const std::string cara = "C" + LetraDa(DaClasse(cap.classe, rna));
        if (cap.voltaEm > mRelogio) {
            Matar(cap.p);
            continue;
        }
        if (!Vivo(cap.p)) cap.p = Enfeite(kGatilhoDaCapsula, onde, cara);
        if (cap.p) {
            cap.p->SetPosition(onde);
            Cara(cap.p, cara);
        }
    }

    ProjectileManager* gerente = Batalha() ? Batalha()->GerenteDeProjeteis() : nullptr;
    for (size_t i = 0; i < alunos.size(); ++i) {
        Player* aluno = alunos[i];
        Selo& selo = mSelos[i];
        const Vector2 pa = aluno->GetPosition();
        // Encostar numa capsula troca a base.
        for (Capsula& cap : mCapsulas) {
            if (!Vivo(cap.p) || (cap.p->GetPosition() - pa).Length() > g.toqueDaCapsula) continue;
            if (cap.classe != selo.classe) {
                selo.classe = cap.classe;
                selo.trocouEm = mRelogio;
            }
            Matar(cap.p);
            cap.voltaEm = mRelogio + g.voltaDaCapsula;
            if (Battle* b = Batalha()) b->Faiscar(pa, Hud::Faisca::Coleta);
        }
        aluno->DefinirClasse(selo.classe);
        const Base base = DaClasse(selo.classe, rna);

        // O selo segue o aluno, e pula quando a base troca.
        const float pulo = std::max(0.0f, 1.0f - (mRelogio - selo.trocouEm) / 0.35f);
        const Vector2 ondeDoSelo = pa + Vector2(22.0f, -26.0f - 12.0f * pulo);
        const std::string cara = "S" + LetraDa(base);
        if (!Vivo(selo.p)) selo.p = Enfeite(kGatilhoDoSelo, ondeDoSelo, cara);
        if (selo.p) {
            selo.p->SetPosition(ondeDoSelo);
            Cara(selo.p, cara);
            selo.p->SetScale(1.0f + 0.5f * pulo);
        }
        Circulo(pa, 18.0f + 6.0f * pulo, CorDa(base, 210), 16);

        // As pontes de hidrogenio ate os tiros que pareiam, e a absorcao dos que encostam.
        if (!gerente) continue;
        const auto* colisor = aluno->GetComponent<CircleColliderComponent>();
        const float raioDoAluno = colisor ? colisor->GetRadius() : 6.0f;
        int pontes = 0;
        for (const auto& tiro : gerente->GetBossProjectiles()) {
            if (tiro->GetState() != ActorState::Active || tiro->Enfeite()) continue;
            if (tiro->Classe() < 0 || tiro->Classe() != selo.classe) continue;
            const auto* outro = tiro->GetComponent<CircleColliderComponent>();
            const float r = outro ? outro->GetRadius() : 6.0f;
            const float distancia = (tiro->GetPosition() - pa).Length();
            if (distancia < raioDoAluno + r + 4.0f) {
                if (Battle* b = Batalha()) b->Faiscar(tiro->GetPosition(), Hud::Faisca::Dissolvido);
                tiro->Kill();
                ++mAbsorvidas;
                mPremioGuardado += kPremioDoPar;
                continue;
            }
            if (distancia < 120.0f && pontes < 6) {
                ++pontes;
                Tracejada(tiro->GetPosition(), pa, kPonte, 4.0f);
            }
        }
    }
    if (mPremioGuardado >= 0.6f) {
        if (Battle* b = Batalha()) b->Conquistar(mPremioGuardado);
        mPremioGuardado = 0.0f;
    }
}

void Sabrina::AtualizarCodigo(const float t, const float deltaTime) {
    using namespace Pareamento;
    const Degrau& g = mDegrauDoCodigo;
    const SDL_FRect c = Campo();
    const int etapa = EtapaNoTempo(t);
    const float inicio = etapa == 0 ? 0.0f : etapa == 1 ? kDuracoes[0] : kDuracoes[0] + kDuracoes[1];
    const float tEtapa = t - inicio;
    const bool rna = etapa > 0;
    Varrer(t, 0.32f, 7.5f);
    AtualizarBases(deltaTime, rna);
    const float fitaY = c.y + 124.0f;

    // ---- O pareamento: as ondas de uma base so ----
    if (etapa == 0) {
        if (mOndas == 0 && mProximaOnda <= 0.0f) {
            mDominante = DaClasse(mSorteio.Inteiro(0, 3), false);
            mProximaOnda = 1.0f;
        }
        Circulo(GetPosition(), 34.0f + 3.0f * Math::Sin(mRelogio * 8.0f), CorDa(mDominante), 18);
        if (t >= mProximaOnda) {
            mProximaOnda = t + g.entreOndas;
            const int n = g.basesPorLinha;
            const auto bases = Onda(mSorteio, mDominante, n * g.linhasDaOnda, g.fracaoDominante, false);
            const float de = c.x + 24.0f, ate = c.x + c.w - 24.0f;
            const float passo = (ate - de) / static_cast<float>(std::max(1, n - 1));
            for (int linha = 0; linha < g.linhasDaOnda; ++linha) {
                for (int k = 0; k < n; ++k) {
                    const float x = de + passo * (static_cast<float>(k) + (linha % 2 == 1 ? 0.5f : 0.0f));
                    if (x > ate) continue;
                    const Base b = bases[static_cast<size_t>(linha * n + k)];
                    const Vector2 onde(x, c.y + 104.0f - static_cast<float>(linha) * g.entreLinhas);
                    if (BossProjectile* p = Um(kGatilhoDaOnda, onde, Pareamento::Cara(b), Vector2(0.0f, g.velocidadeDaOnda))) {
                        p->DefinirClasse(ClasseDoTiro(b));
                    }
                }
            }
            ++mOndas;
            SomDoDisparo();
            mDominante = Outra(mSorteio, mDominante, false);
        }
        Placar(kFase, 0, "Pareamento: a pr\xc3\xb3xima onda \xc3\xa9 de " + LetraDa(mDominante) + ", que pareia com " +
                             LetraDa(Complementar(mDominante, false)));
        return;
    }

    // ---- A transcricao: a polimerase le o molde e solta o RNA ----
    if (etapa == 1) {
        const int n = g.basesDoMolde;
        const float de = c.x + 40.0f, passo = (c.w - 80.0f) / static_cast<float>(std::max(1, n - 1));
        if (mFitaMolde.empty()) {
            mFitaMolde = Molde(Sequencia(mSorteio, n, g.repetir, true));
            mLidas = 0;
            Avisar("No RNA, T vira U: o seu selo de T virou U", 1.8f);
        }
        mLetrasDoMolde.resize(static_cast<size_t>(n), nullptr);
        for (int k = 0; k < n; ++k) {
            BossProjectile*& l = mLetrasDoMolde[static_cast<size_t>(k)];
            const Vector2 onde(de + passo * static_cast<float>(k), fitaY);
            if (!Vivo(l)) l = Enfeite(kGatilhoDaLetra, onde, "L" + LetraDa(mFitaMolde[static_cast<size_t>(k)]));
        }
        const int lendo = std::min(n - 1, static_cast<int>(tEtapa / g.porBase));
        while (mLidas <= lendo) {
            const int k = mLidas++;
            const Base doRna = Transcrever(mFitaMolde[static_cast<size_t>(k)]);
            const Vector2 onde(de + passo * static_cast<float>(k), fitaY + 26.0f);
            Bases(kGatilhoDoRna, onde, doRna, g.tirosPorBase, AnguloAteOAluno(onde), g.aberturaPorBase, g.velocidadeDoRna, true);
            mLetrasDoRna.push_back(Enfeite(kGatilhoDaLetra, onde, "L" + LetraDa(doRna)));
            if (k % 3 == 0) SomDoDisparo();
        }
        const float xDaPolimerase = de + passo * std::min(static_cast<float>(n - 1), tEtapa / g.porBase);
        if (!Vivo(mMaquina)) mMaquina = Enfeite(kGatilhoDaMaquina, Vector2(xDaPolimerase, fitaY + 8.0f), "Polimerase");
        if (mMaquina) {
            mMaquina->SetPosition(Vector2(xDaPolimerase, fitaY + 8.0f));
            Cara(mMaquina, "Polimerase");
        }
        // O esqueleto do molde, e a janela do que vem: quem le a frente sabe que base pegar.
        Linha(Vector2(de - 14.0f, fitaY - 13.0f), Vector2(de + passo * static_cast<float>(n - 1) + 14.0f, fitaY - 13.0f), kEsqueleto);
        if (lendo + 1 < n) {
            const int ate = std::min(n - 1, lendo + 4);
            Retangulo(Vector2(de + passo * static_cast<float>(lendo + 1) - 13.0f, fitaY - 13.0f),
                      Vector2(de + passo * static_cast<float>(ate) + 13.0f, fitaY + 13.0f), kOuro);
            Placar(kFase, 1, "Transcri\xc3\xa7\xc3\xa3o: vem " + Letras(mFitaMolde, static_cast<size_t>(lendo + 1), 4) +
                                 "  \xc2\xb7  pegue a base do molde (T vira U)");
        } else {
            Placar(kFase, 1, "Transcri\xc3\xa7\xc3\xa3o: o RNA est\xc3\xa1 pronto");
        }
        return;
    }

    // ---- A traducao: o ribossomo le os codons ----
    if (mMensageiro.empty()) {
        mMensageiro = Gene(mSorteio, 5);
        const auto segundo = Gene(mSorteio, 6);
        mMensageiro.insert(mMensageiro.end(), segundo.begin(), segundo.end());
        mCodons = 0;
        for (BossProjectile*& p : mLetrasDoMolde) Matar(p);
        mLetrasDoMolde.clear();
        for (BossProjectile*& p : mLetrasDoRna) Matar(p);
        mLetrasDoRna.clear();
        Matar(mMaquina);
    }
    const int n = static_cast<int>(mMensageiro.size());
    const int codons = n / 3;
    const float de = c.x + 40.0f, passo = (c.w - 80.0f) / static_cast<float>(std::max(1, n - 1));
    mLetrasDoMensageiro.resize(static_cast<size_t>(n), nullptr);
    for (int k = 0; k < n; ++k) {
        BossProjectile*& l = mLetrasDoMensageiro[static_cast<size_t>(k)];
        if (!Vivo(l)) l = Enfeite(kGatilhoDaLetra, Vector2(de + passo * static_cast<float>(k), fitaY),
                                  "L" + LetraDa(mMensageiro[static_cast<size_t>(k)]));
    }
    for (int k = 1; k < codons; ++k) {
        const float x = de + passo * (static_cast<float>(3 * k) - 0.5f);
        Linha(Vector2(x, fitaY - 14.0f), Vector2(x, fitaY + 14.0f), kEsqueleto);
    }
    const int lendo = std::min(codons - 1, static_cast<int>(tEtapa / g.porCodon));
    const Vector2 ribossomo(de + passo * (3.0f * std::min(static_cast<float>(codons - 1), tEtapa / g.porCodon) + 1.0f),
                            fitaY + 10.0f);
    while (mCodons <= lendo) {
        const int k = mCodons++;
        const Base b[3] = {mMensageiro[static_cast<size_t>(3 * k)], mMensageiro[static_cast<size_t>(3 * k + 1)],
                           mMensageiro[static_cast<size_t>(3 * k + 2)]};
        const char aa = Traduzir(b[0], b[1], b[2]);
        const Familia fam = FamiliaDe(aa);
        mUltimoAminoacido = aa;
        const Vector2 onde(de + passo * static_cast<float>(3 * k + 1), fitaY + 30.0f);
        const float mira = AnguloAteOAluno(onde);
        const int m = g.tirosPorBaseNaTraducao;
        switch (fam) {
            case Familia::Hidrofobico:
                // Foge da agua: vem reto, junto e rapido.
                for (int q = 0; q < 3; ++q) {
                    Bases(kGatilhoDoCodon, onde, b[q], m, mira + 4.0f * static_cast<float>(q - 1), 5.0f, g.velocidadeHidrofobica, true);
                }
                break;
            case Familia::Polar:
                for (int q = 0; q < 3; ++q) {
                    Bases(kGatilhoDoCodon, onde, b[q], m, mira + g.aberturaPolar / 3.0f * static_cast<float>(q - 1),
                          g.aberturaPolar / 3.0f, g.velocidadePolar, true);
                }
                break;
            case Familia::Positivo:
                // A carga positiva persegue o aluno.
                for (int q = 0; q < 3; ++q) {
                    for (BossProjectile* p : Bases(kGatilhoDoCodon, onde, b[q], m, mira + 28.0f * static_cast<float>(q - 1), 12.0f,
                                                   g.velocidadePositiva, true)) {
                        mTeleguiados.push_back({p, mRelogio + 1.6f});
                    }
                }
                break;
            case Familia::Negativo: {
                // A carga negativa espalha em anel.
                const float giro = mSorteio.Real(0.0f, 360.0f);
                for (int q = 0; q < g.anelNegativo; ++q) {
                    Bases(kGatilhoDoCodon, onde, b[q % 3], 1, giro + 360.0f * static_cast<float>(q) / static_cast<float>(g.anelNegativo),
                          0.0f, g.velocidadeNegativa, true);
                }
                break;
            }
            case Familia::Parada:
                // A cadeia se solta e cai no aluno.
                for (size_t q = 0; q < mPolipeptideo.size(); ++q) {
                    BossProjectile* p = mPolipeptideo[q];
                    if (!Vivo(p)) continue;
                    Vector2 rumo = PosicaoDoAluno() - p->GetPosition();
                    if (rumo.LengthSq() < 1.0f) rumo = Vector2(0.0f, 1.0f);
                    rumo.Normalize();
                    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Girado(rumo, mSorteio.Real(-10.0f, 10.0f)) * g.quedaDaCadeia);
                }
                mPolipeptideo.clear();
                mOndeDoPolipeptideo.clear();
                Avisar("C\xc3\xb3" "don de parada: a cadeia se solta!", 1.2f);
                break;
        }
        if (fam != Familia::Parada) {
            const Vector2 ponta = mOndeDoPolipeptideo.empty() ? ribossomo + Vector2(0.0f, 24.0f) : mOndeDoPolipeptideo.back() + Vector2(0.0f, 14.0f);
            if (BossProjectile* p = Um(kGatilhoDaCadeia, ponta, CaraDaFamilia(fam))) {
                mPolipeptideo.push_back(p);
                mOndeDoPolipeptideo.push_back(ponta);
            }
        }
        SomDoDisparo();
    }
    if (!Vivo(mMaquina)) mMaquina = Enfeite(kGatilhoDaMaquina, ribossomo, "Ribossomo");
    if (mMaquina) {
        mMaquina->SetPosition(ribossomo);
        Cara(mMaquina, "Ribossomo");
    }
    Retangulo(Vector2(de + passo * static_cast<float>(3 * lendo) - 13.0f, fitaY - 14.0f),
              Vector2(de + passo * static_cast<float>(3 * lendo + 2) + 13.0f, fitaY + 14.0f), kOuro);

    // A cadeia nascente pende do ribossomo e vem atras dele.
    if (!mOndeDoPolipeptideo.empty()) {
        for (size_t q = 1; q < mOndeDoPolipeptideo.size(); ++q) mOndeDoPolipeptideo[q].y += 60.0f * deltaTime;
        Dobramento::Arrastar(mOndeDoPolipeptideo, ribossomo + Vector2(0.0f, 26.0f), 15.0f);
        for (size_t q = 0; q < mPolipeptideo.size(); ++q) {
            if (q > 0) Linha(mOndeDoPolipeptideo[q - 1], mOndeDoPolipeptideo[q], kLigacao);
            if (Vivo(mPolipeptideo[q])) Mover(mPolipeptideo[q], mOndeDoPolipeptideo[q], Vector2::Zero);
        }
    }
    // Os positivos perseguem o aluno por um tempo.
    for (Teleguiado& tg : mTeleguiados) {
        if (!Vivo(tg.p) || mRelogio > tg.ate) {
            tg.p = nullptr;
            continue;
        }
        if (auto* rb = tg.p->GetComponent<RigidBodyComponent>()) {
            const Vector2 v = rb->GetVelocity();
            const float rapidez = v.Length();
            const float agora = Math::Atan2(v.y, v.x);
            const float vira = Math::VirarPara(agora, PosicaoDoAluno() - tg.p->GetPosition(), g.giroPositivo * deltaTime);
            rb->SetVelocity(Vector2(Math::Cos(vira), Math::Sin(vira)) * rapidez);
        }
    }
    mTeleguiados.erase(std::remove_if(mTeleguiados.begin(), mTeleguiados.end(), [](const Teleguiado& tg) { return tg.p == nullptr; }),
                       mTeleguiados.end());

    const Base* codon = &mMensageiro[static_cast<size_t>(3 * lendo)];
    const char aa = Traduzir(codon[0], codon[1], codon[2]);
    Placar(kFase, 2, "Tradu\xc3\xa7\xc3\xa3o: " + Letras(mMensageiro, static_cast<size_t>(3 * lendo), 3) + " \xe2\x86\x92 " + NomeDo(aa) + " (" +
                         NomeDa(FamiliaDe(aa)) + ")");
}

// ---- Fase 3: a proteina ----

void Sabrina::DesmontarProteina() {
    for (Residuo& r : mCadeia) Matar(r.p);
    mCadeia.clear();
    mOndeDaCadeia.clear();
    mProximoResiduo = 0.0f;
    mProximaCadeia = -1.0f;
    mProximoLequeDaCadeia = 0.0f;
    mCortes = 0;
    for (Solto& s : mSoltosDaCadeia) Matar(s.p);
    mSoltosDaCadeia.clear();
    mGlobuloMontado = false;
    mDobrado.clear();
    mFamilias.clear();
    mDeOnde.clear();
    for (Conta& a : mAtomos) Matar(a.p);
    mAtomos.clear();
    mProximoBorrifo = 0.0f;
    for (BossProjectile*& p : mParede) Matar(p);
    mParede.clear();
    for (Gota& gota : mGotas) Matar(gota.p);
    mGotas.clear();
    mProximaGota = 0.0f;
    mProximoInibidor = 0.0f;
    mEncaixado = 0.0f;
    mComecoDoEncaixe = -1.0f;
    if (mPremioGuardado > 0.0f) {
        if (Battle* b = Batalha()) b->Conquistar(mPremioGuardado);
    }
    mPremioGuardado = 0.0f;
}

void Sabrina::CortarACadeia(const size_t i) {
    const Dobramento::Degrau& g = mDegrauDaProteina;
    if (i >= mCadeia.size()) return;
    // Os residuos de [de, ate) se soltam e se desfazem; o atingido some ali mesmo.
    const auto soltar = [&](const size_t de, const size_t ate) {
        for (size_t k = de; k < ate; ++k) {
            const Vector2 onde = mOndeDaCadeia[k];
            const bool atingido = mCadeia[k].p == nullptr || k == i;
            Matar(mCadeia[k].p);
            if (atingido) continue;
            const float a = mSorteio.Real(0.0f, Math::TwoPi);
            if (BossProjectile* p = Um(Dobramento::kGatilhoDoSolto, onde, "Solto", Vector2(Math::Cos(a), Math::Sin(a)) * 50.0f)) {
                mSoltosDaCadeia.push_back({p, mRelogio + g.vidaDosSoltos});
            }
        }
    };
    // A protease corta no residuo atingido, e o pedaco menor se desfaz: acertar a cabeca so a
    // encurta; acertar o meio leva metade embora.
    const size_t n = mCadeia.size();
    if (i >= n - 1 - i) {
        soltar(i, n);
        mCadeia.resize(i);
        mOndeDaCadeia.resize(i);
    } else {
        soltar(0, i + 1);
        mCadeia.erase(mCadeia.begin(), mCadeia.begin() + static_cast<std::ptrdiff_t>(i + 1));
        mOndeDaCadeia.erase(mOndeDaCadeia.begin(), mOndeDaCadeia.begin() + static_cast<std::ptrdiff_t>(i + 1));
    }
    ++mCortes;
    // Curta demais, ela toda se desfaz, e o ribossomo solta a proxima.
    if (mCadeia.size() < 3) {
        soltar(0, mCadeia.size());
        mCadeia.clear();
        mOndeDaCadeia.clear();
        mProximaCadeia = mRelogio + g.voltaDaCadeia;
    }
}

void Sabrina::AtualizarProteina(const float t, const float deltaTime) {
    using namespace Dobramento;
    const Degrau& g = mDegrauDaProteina;
    const SDL_FRect c = Campo();
    const int etapa = EtapaNoTempo(t);
    Varrer(t, 0.3f, 7.0f);
    const Vector2 aluno = PosicaoDoAluno();

    // Os pedacos soltos somem com o tempo.
    for (Solto& s : mSoltosDaCadeia) {
        if (Vivo(s.p) && mRelogio >= s.morre) Matar(s.p);
    }
    mSoltosDaCadeia.erase(std::remove_if(mSoltosDaCadeia.begin(), mSoltosDaCadeia.end(), [](const Solto& s) { return !Vivo(s.p); }),
                          mSoltosDaCadeia.end());

    // ---- A cadeia: o polipeptideo persegue o aluno, e o tiro e a protease ----
    if (etapa == 0) {
        if (mCadeia.empty() && (mProximaCadeia < 0.0f || mRelogio >= mProximaCadeia)) {
            // Cada cadeia sai de um canto do alto, o mais longe do aluno: fora da linha de tiro dele.
            const bool esquerda = aluno.x > c.x + 0.5f * c.w;
            const Vector2 de(esquerda ? c.x + 30.0f : c.x + c.w - 30.0f, c.y + 120.0f);
            for (int k = 0; k < g.residuosIniciais; ++k) {
                const Vector2 onde = de + Vector2(esquerda ? -3.0f : 3.0f, -3.0f) * static_cast<float>(k);
                Residuo r;
                r.familia = FamiliaQualquer(mSorteio);
                r.p = Um(kGatilhoDoResiduo, onde, CaraDaFamilia(r.familia));
                if (r.p) r.p->DefinirResistencia(g.resistencia);
                mCadeia.push_back(r);
                mOndeDaCadeia.push_back(onde);
            }
            Vector2 rumo = aluno - de;
            if (rumo.LengthSq() < 1.0f) rumo = Vector2(0.0f, 1.0f);
            rumo.Normalize();
            mRumo = rumo;
            mProximaCadeia = -1.0f;
            mProximoResiduo = mRelogio + g.entreResiduos;
        }
        if (!mCadeia.empty()) {
            // A cabeca vira para o aluno, no maximo 'giro' por segundo.
            const float agora = Math::Atan2(mRumo.y, mRumo.x);
            const float vira = Math::VirarPara(agora, aluno - mOndeDaCadeia.front(), g.giro * deltaTime);
            mRumo = Vector2(Math::Cos(vira), Math::Sin(vira));
            Vector2 cabeca = mOndeDaCadeia.front() + mRumo * g.rapidez * deltaTime;
            cabeca.x = std::clamp(cabeca.x, c.x + 14.0f, c.x + c.w - 14.0f);
            cabeca.y = std::clamp(cabeca.y, c.y + 90.0f, c.y + c.h - 14.0f);
            Arrastar(mOndeDaCadeia, cabeca, g.espacamento);
            // Cresce um aminoacido por vez, no fim.
            if (mRelogio >= mProximoResiduo && static_cast<int>(mCadeia.size()) < g.residuosMaximos && mOndeDaCadeia.size() >= 2) {
                mProximoResiduo = mRelogio + g.entreResiduos;
                const Vector2 ultimo = mOndeDaCadeia.back();
                Vector2 d = ultimo - mOndeDaCadeia[mOndeDaCadeia.size() - 2];
                if (d.LengthSq() < 1e-4f) d = Vector2(0.0f, -1.0f);
                d.Normalize();
                const Vector2 onde = ultimo + d * g.espacamento;
                Residuo r;
                r.familia = FamiliaQualquer(mSorteio);
                r.p = Um(kGatilhoDoResiduo, onde, CaraDaFamilia(r.familia));
                if (r.p) r.p->DefinirResistencia(g.resistencia);
                mCadeia.push_back(r);
                mOndeDaCadeia.push_back(onde);
            }
            // O residuo que sumiu sem o tiro (a onda do Ponto Extra) tambem corta.
            for (size_t i = 0; i < mCadeia.size(); ++i) {
                if (!Vivo(mCadeia[i].p)) {
                    mCadeia[i].p = nullptr;
                    CortarACadeia(i);
                    break;
                }
            }
            for (size_t i = 0; i < mCadeia.size(); ++i) {
                if (i > 0) Linha(mOndeDaCadeia[i - 1], mOndeDaCadeia[i], kLigacao);
                Mover(mCadeia[i].p, mOndeDaCadeia[i], mRumo * g.rapidez);
            }
        }
        if (mRelogio >= mProximoLequeDaCadeia) {
            mProximoLequeDaCadeia = mRelogio + g.entreLeques;
            const Vector2 de = GetPosition() + Vector2(0.0f, 34.0f);
            for (BossProjectile* p : DispararEm(kGatilhoDoLeque, de, g.tirosDoLeque, AnguloAteOAluno(de), 30.0f, g.velocidadeDoLeque)) {
                Cara(p, CaraDaFamilia(FamiliaQualquer(mSorteio)));
            }
        }
        Placar(kFase, 0, "Cadeia: " + std::to_string(mCadeia.size()) + " res\xc3\xad" "duos  \xc2\xb7  o seu tiro \xc3\xa9 a protease");
        return;
    }

    // ---- O dobramento: a cadeia colapsa num globulo, que rola ----
    if (etapa == 1) {
        if (!mGlobuloMontado) {
            mGlobuloMontado = true;
            const int n = g.residuosDoGlobulo;
            std::vector<bool> hidrofobico;
            mFamilias.clear();
            mDeOnde.clear();
            for (int k = 0; k < n; ++k) {
                const bool daCadeia = k < static_cast<int>(mCadeia.size());
                const Familia f = daCadeia ? mCadeia[static_cast<size_t>(k)].familia : FamiliaQualquer(mSorteio);
                mFamilias.push_back(f);
                hidrofobico.push_back(f == Familia::Hidrofobico);
                const Vector2 onde = daCadeia ? mOndeDaCadeia[static_cast<size_t>(k)] : GetPosition() + Vector2(0.0f, 44.0f);
                mDeOnde.push_back(onde);
            }
            mDobrado = Globulo(hidrofobico, g.nucleo);
            for (Residuo& r : mCadeia) Matar(r.p);
            mCadeia.clear();
            mOndeDaCadeia.clear();
            mAtomos.assign(static_cast<size_t>(n), Conta{});
            // O globulo se forma no alto do campo: a cadeia voa ate la e se dobra.
            mCentroDoGlobulo = Vector2(c.x + 0.5f * c.w, c.y + 0.4f * c.h);
            const float a = mSorteio.Real(0.0f, Math::TwoPi);
            mVelocidadeDoGlobulo = Vector2(Math::Cos(a), Math::Sin(a)) * g.rapidezDoGlobulo;
            mComecoDoGlobulo = mRelogio;
            mProximoBorrifo = mRelogio + g.colapso;
            mGiroX = 0.0f;
            mGiroY = 0.0f;
        }
        mGiroY += g.giroY * deltaTime;
        mGiroX += g.giroX * deltaTime;
        const float u = std::clamp((mRelogio - mComecoDoGlobulo) / std::max(0.01f, g.colapso), 0.0f, 1.0f);
        if (u >= 1.0f) {
            // O globulo rola atras do aluno: a velocidade vira para ele, devagar, e quica nas paredes.
            const float rapidez = mVelocidadeDoGlobulo.Length();
            const float rumo = Math::VirarPara(Math::Atan2(mVelocidadeDoGlobulo.y, mVelocidadeDoGlobulo.x),
                                               aluno - mCentroDoGlobulo, g.perseguicao * deltaTime);
            mVelocidadeDoGlobulo = Vector2(Math::Cos(rumo), Math::Sin(rumo)) * rapidez;
            mCentroDoGlobulo = Quicar(mCentroDoGlobulo, mVelocidadeDoGlobulo, deltaTime, g.raioDoGlobulo,
                                      Vector2(c.x, c.y + 110.0f), Vector2(c.x + c.w, c.y + c.h));
        }
        int escondidos = 0;
        for (size_t k = 0; k < mAtomos.size(); ++k) {
            const Helice::Ponto3& m = mDobrado[k];
            Helice::Ponto3 p{m.x * g.raioDoGlobulo, m.y * g.raioDoGlobulo, m.z * g.raioDoGlobulo};
            p = Helice::GirarEmY(p, mGiroY);
            p = Helice::GirarEmX(p, mGiroX);
            Helice::NaTela tela = Helice::Projetar(p, mCentroDoGlobulo, kDistanciaDaCamera);
            if (u < 1.0f) tela.onde = mDeOnde[k] + (tela.onde - mDeOnde[k]) * Suave(u);
            const bool frente = Helice::NaFrente(p.z, g.raioDoGlobulo, g.limiar);
            if (mFamilias[k] == Familia::Hidrofobico) ++escondidos;
            Posicionar(mAtomos[k], tela, frente, kGatilhoDoAtomo, CaraDaFamilia(mFamilias[k]), deltaTime,
                       CorDaFamilia(mFamilias[k], 90));
        }
        // As ligacoes da cadeia (bolas e varetas).
        for (size_t k = 1; k < mAtomos.size(); ++k) Linha(mAtomos[k - 1].onde, mAtomos[k].onde, kLigacao);
        // Os polares da frente borrifam agua para fora.
        if (u >= 1.0f && mRelogio >= mProximoBorrifo) {
            mProximoBorrifo = mRelogio + g.entreBorrifos;
            for (size_t k = 0; k < mAtomos.size(); ++k) {
                if (!mAtomos[k].frente || mFamilias[k] == Familia::Hidrofobico) continue;
                const Vector2 de = mAtomos[k].onde;
                const float a = AnguloAte(mCentroDoGlobulo, de);
                for (BossProjectile* p : DispararEm(kGatilhoDaAgua, de, g.tirosDoBorrifo, a, 12.0f * static_cast<float>(g.tirosDoBorrifo - 1),
                                                    g.velocidadeDoBorrifo)) {
                    Cara(p, "Agua");
                }
            }
            SomDoDisparo();
        }
        Placar(kFase, 1, "Dobramento: " + std::to_string(escondidos) + " hidrof\xc3\xb3" "bicos no n\xc3\xba" "cleo");
        return;
    }

    // ---- O encaixe: o sitio ativo protege - e prende o tiro ----
    if (mComecoDoEncaixe < 0.0f) {
        mComecoDoEncaixe = mRelogio;
        for (Conta& a : mAtomos) Matar(a.p);
        mAtomos.clear();
        mGlobuloMontado = false;
        mEncaixado = 0.0f;
        mProximaGota = mRelogio;
        mProximoInibidor = mRelogio + 1.0f;
    }
    const float te = mRelogio - mComecoDoEncaixe;
    const Vector2 centro(c.x + c.w * (0.5f + g.passeioDoSitio * Math::Sin(Math::TwoPi * te / 8.0f)), c.y + 0.6f * c.h);
    // A boca comeca para baixo (o lado de onde nada vem) e gira.
    const float boca = 0.5f * Math::Pi + g.giroDaBoca * te;
    const auto contas = Sitio(centro, g.raioDoSitio, g.contasDoSitio, boca, g.abertura);
    mParede.resize(contas.size(), nullptr);
    for (size_t k = 0; k < contas.size(); ++k) {
        if (!Vivo(mParede[k])) mParede[k] = Um(kGatilhoDoSitio, contas[k], "Sitio");
        Mover(mParede[k], contas[k], Vector2::Zero);
    }
    Circulo(centro, g.raioDoSitio - 16.0f, SDL_Color{kSitio.r, kSitio.g, kSitio.b, 70}, 24);
    const Vector2 dirDaBoca(Math::Cos(boca), Math::Sin(boca));
    Tracejada(centro + dirDaBoca * (g.raioDoSitio - 16.0f), centro + dirDaBoca * (g.raioDoSitio + 26.0f), kSitio, 4.0f);

    // A agua: gotas em movimento browniano, do alto.
    if (mRelogio >= mProximaGota) {
        mProximaGota = mRelogio + g.entreGotas;
        const Vector2 onde(mSorteio.Real(c.x + 12.0f, c.x + c.w - 12.0f), c.y + 100.0f);
        const Vector2 v(mSorteio.Real(-30.0f, 30.0f), g.velocidadeDaGota);
        if (BossProjectile* p = Um(kGatilhoDaAgua, onde, "Agua", v)) mGotas.push_back({p, v});
    }
    for (Gota& gota : mGotas) {
        if (!Vivo(gota.p)) {
            gota.p = nullptr;
            continue;
        }
        const float a = mSorteio.Real(0.0f, Math::TwoPi);
        gota.v = Browniano(gota.v, Vector2(0.0f, g.velocidadeDaGota), Vector2(Math::Cos(a), Math::Sin(a)) * g.chute, deltaTime, 1.5f);
        if (auto* rb = gota.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(gota.v);
    }
    mGotas.erase(std::remove_if(mGotas.begin(), mGotas.end(), [](const Gota& gota) { return gota.p == nullptr; }), mGotas.end());

    // Quem esta encaixado ganha afinidade.
    bool algum = false;
    for (Player* p : Jogadores()) {
        if (!Encaixado(p->GetPosition(), centro, g.raioDoSitio, 13.0f)) continue;
        algum = true;
        Circulo(p->GetPosition(), 21.0f, kOuro, 14);
    }

    // Os inibidores competem pela ligacao: com o aluno encaixado, vao no sitio; fora dele, no aluno.
    if (mRelogio >= mProximoInibidor) {
        mProximoInibidor = mRelogio + g.entreInibidores;
        const Vector2 de = GetPosition() + Vector2(0.0f, 34.0f);
        const float mira = algum ? AnguloAte(de, centro) : AnguloAteOAluno(de);
        for (BossProjectile* p : DispararEm(kGatilhoDoInibidor, de, g.tirosDoInibidor, mira, 14.0f, g.velocidadeDoInibidor)) {
            Cara(p, "Inibidor");
        }
        SomDoDisparo();
    }
    if (algum) {
        mEncaixado += deltaTime;
        mPremioGuardado += g.premioPorSegundo * deltaTime;
        if (mPremioGuardado >= 0.5f) {
            if (Battle* b = Batalha()) b->Conquistar(mPremioGuardado);
            mPremioGuardado = 0.0f;
        }
    }
    Placar(kFase, 2, algum ? "Encaixe: \xce\x94G = \xe2\x88\x92" + Decimal(-DeltaG(mEncaixado), 1) + " kcal/mol  \xc2\xb7  o tiro s\xc3\xb3 sai pela boca"
                           : std::string("Encaixe: entre no s\xc3\xadtio ativo pela boca"));
}

// ---- A repescagem: o sequenciamento ----

void Sabrina::DesmontarSequenciamento() {
    for (Copia& c : mCopias) {
        Matar(c.p);
        Matar(c.primer);
    }
    mCopias.clear();
    mCicloAntes = -1;
    mPassoAntes = Sequenciamento::Passo::Estende;
    mPrimersDerrubados = 0;
    for (BossProjectile*& p : mContig) Matar(p);
    mContig.clear();
    mAtras = 0;
    mFrente = 0;
    mGiroDoContig = 0.0f;
    mProximaRajada = 0.0f;
    for (BossProjectile*& p : mReads) Matar(p);
    mReads.clear();
    mGuia.clear();
    for (FitaDoGenoma& f : mGenomas) {
        for (BossProjectile*& p : f.contas) Matar(p);
        for (BossProjectile*& p : f.elos) Matar(p);
    }
    mGenomas.clear();
    mProximaFita = 0.0f;
    mEdicoes = 0;
    Matar(mTesoura);
    mTesouraAte = -1.0f;
}

void Sabrina::MontarOContig() {
    // As pontas se deixam aparar (contorno vermelho); o meio e o consenso, e o tiro morre nele.
    const int n = static_cast<int>(mContig.size());
    for (int k = 0; k < n; ++k) {
        BossProjectile* p = mContig[static_cast<size_t>(k)];
        if (!Vivo(p)) continue;
        const bool ponta = (k == 0 && mAtras > 0) || (k == n - 1 && mFrente > 0);
        p->DefinirDerrubavel(ponta);
        p->DefinirBlindado(!ponta);
        p->DefinirResistencia(1);
        Cara(p, ponta ? "Ponta" : "Contig");
    }
}

void Sabrina::AtualizarSequenciamento(const float t, const float deltaTime) {
    using namespace Sequenciamento;
    const Degrau& g = mDegrauDoSequenciamento;
    const SDL_FRect c = Campo();
    const int etapa = EtapaNoTempo(t);
    Varrer(t, 0.3f, 7.0f);
    if (Vivo(mTesoura) && mRelogio >= mTesouraAte) Matar(mTesoura);

    // ---- A PCR: cada ciclo dobra as copias, e o tiro derruba o primer ----
    if (etapa == 0) {
        if (mCopias.empty() && mCicloAntes < 0) {
            for (int k = 0; k < g.moldes; ++k) {
                Copia cp;
                const float a = mSorteio.Real(0.0f, Math::TwoPi);
                cp.v = Vector2(Math::Cos(a), Math::Sin(a)) * mSorteio.Real(g.rapidezMinima, g.rapidezMaxima);
                const Vector2 onde(c.x + c.w * (0.5f + 0.25f * (static_cast<float>(k) - 0.5f * static_cast<float>(g.moldes - 1))),
                                   c.y + 0.4f * c.h);
                cp.p = Um(kGatilhoDaCopia, onde, "Dupla");
                if (cp.p) cp.p->DefinirRotacaoPelaVelocidade(true);
                mCopias.push_back(cp);
            }
        }
        const Ciclo ciclo = CicloNoTempo(t, g.periodo);
        if (ciclo.numero != mCicloAntes || ciclo.passo != mPassoAntes) {
            mCicloAntes = ciclo.numero;
            mPassoAntes = ciclo.passo;
            if (ciclo.passo == Passo::Desnatura) {
                // 95 graus: a dupla fita se separa em duas.
                std::vector<Copia> novas;
                int total = static_cast<int>(mCopias.size());
                for (Copia& cp : mCopias) {
                    if (!cp.dupla || !Vivo(cp.p) || total >= g.teto) continue;
                    cp.dupla = false;
                    Cara(cp.p, "Simples");
                    Copia irma;
                    irma.dupla = false;
                    irma.v = Girado(cp.v, 55.0f);
                    cp.v = Girado(cp.v, -55.0f);
                    irma.p = Um(kGatilhoDaCopia, cp.p->GetPosition(), "Simples");
                    if (irma.p) {
                        irma.p->DefinirRotacaoPelaVelocidade(true);
                        novas.push_back(irma);
                        ++total;
                    }
                }
                mCopias.insert(mCopias.end(), novas.begin(), novas.end());
            } else if (ciclo.passo == Passo::Anela) {
                // 55 graus: o primer encosta em cada fita simples.
                for (Copia& cp : mCopias) {
                    if (cp.dupla || !Vivo(cp.p) || Vivo(cp.primer)) continue;
                    cp.primer = Um(kGatilhoDoPrimer, cp.p->GetPosition(), "Primer");
                    if (cp.primer) cp.primer->DefinirResistencia(g.resistenciaDoPrimer);
                    cp.primerCaiu = false;
                }
            } else {
                // 72 graus: a polimerase estende quem tem primer.
                for (Copia& cp : mCopias) {
                    if (cp.dupla || !Vivo(cp.p)) continue;
                    if (Vivo(cp.primer)) {
                        Matar(cp.primer);
                        cp.dupla = true;
                        Cara(cp.p, "Dupla");
                    }
                }
                SomDoDisparo();
            }
        }
        for (Copia& cp : mCopias) {
            if (!Vivo(cp.p)) {
                cp.p = nullptr;
                Matar(cp.primer);
                continue;
            }
            const Vector2 onde = Dobramento::Quicar(cp.p->GetPosition(), cp.v, deltaTime, 14.0f, Vector2(c.x, c.y + 100.0f),
                                                    Vector2(c.x + c.w, c.y + c.h));
            Mover(cp.p, onde, cp.v);
            if (Vivo(cp.primer)) {
                Vector2 lado(-cp.v.y, cp.v.x);
                if (lado.LengthSq() > 1e-4f) lado.Normalize();
                Mover(cp.primer, onde + lado * 13.0f, cp.v);
            }
        }
        mCopias.erase(std::remove_if(mCopias.begin(), mCopias.end(), [](const Copia& cp) { return cp.p == nullptr; }), mCopias.end());
        Termometro(ciclo.temperatura, 40.0f, 100.0f);
        Placar(kFase, 0, "PCR ciclo " + std::to_string(ciclo.numero + 1) + " (" + Graus(ciclo.temperatura) + "): " +
                             std::to_string(mCopias.size()) + " c\xc3\xb3pias  \xc2\xb7  derrube os primers");
        return;
    }

    // ---- O shotgun: os reads montam o contig, e as pontas se aparam ----
    if (etapa == 1) {
        if (mContig.empty()) {
            for (Copia& cp : mCopias) {
                Matar(cp.p);
                Matar(cp.primer);
            }
            mCopias.clear();
            mAtras = g.pontaInicial;
            mFrente = g.pontaInicial;
            mGiroDoContig = mSorteio.Real(0.0f, Math::TwoPi);
            for (int k = 0; k < mAtras + mFrente + 1; ++k) mContig.push_back(Um(kGatilhoDoContig, GetPosition(), "Contig"));
            MontarOContig();
            mProximaRajada = mRelogio + 0.6f;
        }
        mGiroDoContig += g.giro * deltaTime;
        const Vector2 centro(c.x + 0.5f * c.w, c.y + 0.42f * c.h);
        // Uma conta do meio que sumiu (a onda do Ponto Extra) volta; a ponta que sumiu e aparada.
        for (size_t k = 0; k < mContig.size(); ++k) {
            if (Vivo(mContig[k])) continue;
            const bool tras = k == 0 && mAtras > 0, frente = k + 1 == mContig.size() && mFrente > 0;
            if (tras) {
                mContig.erase(mContig.begin());
                --mAtras;
            } else if (frente) {
                mContig.pop_back();
                --mFrente;
            } else {
                mContig[k] = Um(kGatilhoDoContig, centro, "Contig");
            }
            MontarOContig();
            break;
        }
        const auto contas = Contig(centro, mAtras, mFrente, g.espacamento, mGiroDoContig);
        for (size_t k = 0; k < mContig.size() && k < contas.size(); ++k) {
            const Vector2 r = contas[k] - centro;
            Mover(mContig[k], contas[k], Vector2(-r.y, r.x) * g.giro);
        }
        if (contas.size() >= 2) Linha(contas.front(), contas.back(), kEsqueleto);

        // A rajada de reads, em leque.
        if (mRelogio >= mProximaRajada) {
            mProximaRajada = mRelogio + g.entreRajadas;
            const Vector2 de = GetPosition() + Vector2(0.0f, 36.0f);
            for (BossProjectile* p : DispararEm(kGatilhoDoRead, de, g.tirosDaRajada, AnguloAteOAluno(de), g.aberturaDaRajada,
                                                g.velocidadeDoRead)) {
                Cara(p, "Read");
                p->DefinirRotacaoPelaVelocidade(true);
                mReads.push_back(p);
            }
            SomDoDisparo();
        }
        // O read que encosta no contig se monta nele, na ponta mais perto.
        for (BossProjectile*& read : mReads) {
            if (!Vivo(read)) {
                read = nullptr;
                continue;
            }
            const int lado = Encosta(read->GetPosition(), centro, mAtras, mFrente, g.espacamento, mGiroDoContig, g.alcanceDoContig);
            if (lado == 0) continue;
            Matar(read);
            if (lado < 0 && mAtras < g.pontaMaxima) {
                ++mAtras;
                mContig.insert(mContig.begin(), Um(kGatilhoDoContig, centro, "Contig"));
            } else if (lado > 0 && mFrente < g.pontaMaxima) {
                ++mFrente;
                mContig.push_back(Um(kGatilhoDoContig, centro, "Contig"));
            }
            MontarOContig();
        }
        mReads.erase(std::remove(mReads.begin(), mReads.end(), nullptr), mReads.end());
        Placar(kFase, 1, "Shotgun: contig de " + std::to_string(mAtras + mFrente + 1) + " kb  \xc2\xb7  apare as pontas");
        return;
    }

    // ---- O CRISPR: a fita desce como parede, e so o alvo do guia se corta ----
    if (mGuia.empty()) {
        for (BossProjectile*& p : mContig) Matar(p);
        mContig.clear();
        for (BossProjectile*& p : mReads) Matar(p);
        mReads.clear();
        mGuia = Guia(mSorteio, g.tamanhoDoGuia);
        mProximaFita = t + 0.2f;
    }
    const int gtam = static_cast<int>(mGuia.size());
    if (t >= mProximaFita) {
        mProximaFita = t + g.entreFitas;
        FitaDoGenoma f;
        f.genoma = Montar(mSorteio, g.basesDoGenoma, mGuia, g.iscas);
        f.y = c.y + 104.0f;
        const int n = static_cast<int>(f.genoma.fita.size());
        const float passo = (c.w - 44.0f) / static_cast<float>(std::max(1, n - 1));
        for (int k = 0; k < n; ++k) {
            const bool alvo = k >= f.genoma.alvo && k < f.genoma.alvo + gtam;
            BossProjectile* p = Um(kGatilhoDoGenoma, Vector2(c.x + 22.0f + passo * static_cast<float>(k), f.y),
                                   "G" + LetraDa(f.genoma.fita[static_cast<size_t>(k)]));
            if (p) {
                // Toda conta tem o contorno de quem se derruba: so o alvo, porem, se corta (OnDerrubou).
                p->DefinirDerrubavel(true);
                p->DefinirBlindado(false);
                p->DefinirResistencia(alvo ? g.resistenciaDoAlvo : 1);
            }
            f.contas.push_back(p);
        }
        // Os elos do esqueleto, entre as letras: blindados (o tiro morre neles) e sem frestas.
        for (int k = 0; k + 1 < n; ++k) {
            BossProjectile* p = Um(kGatilhoDoGenoma, Vector2(c.x + 22.0f + passo * (static_cast<float>(k) + 0.5f), f.y), "Elo");
            if (p) {
                p->DefinirDerrubavel(false);
                p->DefinirBlindado(true);
            }
            f.elos.push_back(p);
        }
        mGenomas.push_back(std::move(f));
        SomDoDisparo();
    }
    for (FitaDoGenoma& f : mGenomas) {
        f.y += g.descida * deltaTime;
        const int n = static_cast<int>(f.contas.size());
        const float passo = (c.w - 44.0f) / static_cast<float>(std::max(1, n - 1));
        const auto x = [&](const int k) { return c.x + 22.0f + passo * static_cast<float>(k); };
        for (int k = 0; k < n; ++k) {
            BossProjectile*& p = f.contas[static_cast<size_t>(k)];
            // A conta que sumiu sem o tiro (a onda do Ponto Extra) deixa o buraco: a onda e uma saida.
            if (!Vivo(p)) {
                p = nullptr;
                continue;
            }
            Mover(p, Vector2(x(k), f.y), Vector2(0.0f, g.descida));
        }
        for (size_t k = 0; k < f.elos.size(); ++k) {
            BossProjectile*& p = f.elos[k];
            if (!Vivo(p)) {
                p = nullptr;
                continue;
            }
            Mover(p, Vector2(x(static_cast<int>(k)) + 0.5f * passo, f.y), Vector2(0.0f, g.descida));
        }
        // Os PAMs (N G G) brilham embaixo: e por eles que a Cas9 procura.
        for (int i = 0; i + 2 < n; ++i) {
            if (!Pam(f.genoma.fita, i)) continue;
            Linha(Vector2(x(i + 1) - 10.0f, f.y + 15.0f), Vector2(x(i + 2) + 10.0f, f.y + 15.0f), kPam);
        }
        // A Cas9 da professora corta fora do alvo, no meio do caminho: os estilhacos vem no aluno.
        if (!f.foraDoAlvo && f.y >= c.y + 0.4f * c.h) {
            f.foraDoAlvo = true;
            for (const int i : f.genoma.iscas) {
                const Vector2 de(x(i + gtam / 2), f.y + 12.0f);
                for (BossProjectile* p : DispararEm(kGatilhoDoEstilhaco, de, g.tirosForaDoAlvo, AnguloAteOAluno(de), 40.0f,
                                                    g.velocidadeForaDoAlvo)) {
                    Cara(p, "Estilhaco");
                }
            }
            if (!f.genoma.iscas.empty()) SomDoDisparo();
        }
    }
    // A fita que passou do chao se vai.
    for (FitaDoGenoma& f : mGenomas) {
        if (f.y <= c.y + c.h + 30.0f) continue;
        for (BossProjectile*& p : f.contas) Matar(p);
        for (BossProjectile*& p : f.elos) Matar(p);
        f.contas.clear();
        f.elos.clear();
    }
    mGenomas.erase(std::remove_if(mGenomas.begin(), mGenomas.end(), [](const FitaDoGenoma& f) { return f.contas.empty(); }),
                   mGenomas.end());
    Placar(kFase, 2, "CRISPR: guia " + Letras(mGuia, 0, mGuia.size()) + " + NGG  \xc2\xb7  corte s\xc3\xb3 no alvo");
}
