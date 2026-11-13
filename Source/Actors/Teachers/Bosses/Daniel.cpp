//
// O chefe da INF 425 - Ciencia de Dados, a primeira optativa.
//

#include "Daniel.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../Player/Player.h"
#include "BossesProjectiles/BossProjectile.h"

Daniel::Daniel(Scene* scene) : Boss(scene) {
}

void Daniel::SortearALuta() {
    mLutaSorteada = true;
    mSorteio = SorteioDaFase(Preparacao::kFase);
    const Dificuldade::Nivel n = NivelDeDificuldade();
    mDegrauDaPreparacao = Preparacao::DegrauDo(n);
    mDegrauDoAgrupamento = Agrupamento::DegrauDo(n);
    mDegrauDaClassificacao = Classificacao::DegrauDo(n);
    mDegrauDaExplicacao = Explicacao::DegrauDo(n);
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
    if (!velocidade) Parar(p);
    return p;
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
    if (!p) return;
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
}

std::vector<Vector2> Daniel::Alunos() const {
    std::vector<Vector2> onde;
    if (const Battle* b = Batalha()) {
        for (const Player* p : b->Jogadores()) {
            if (p) onde.push_back(p->GetPosition());
        }
    }
    return onde;
}

void Daniel::Linha(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Acrescentar(de, ate, cor);
}

void Daniel::Tracejada(const Vector2 de, const Vector2 ate, const SDL_Color cor, const float traco) {
    const Vector2 d = ate - de;
    const float comprimento = d.Length();
    if (comprimento < 1.0f) return;
    const Vector2 u = d * (1.0f / comprimento);
    for (float s = 0.0f; s < comprimento; s += 2.0f * traco) {
        Linha(de + u * s, de + u * std::min(comprimento, s + traco), cor);
    }
}

void Daniel::Circulo(const Vector2 centro, const float raio, const SDL_Color cor, const int lados) {
    for (int k = 0; k < lados; ++k) {
        const float a0 = Math::TwoPi * static_cast<float>(k) / static_cast<float>(lados);
        const float a1 = Math::TwoPi * static_cast<float>(k + 1) / static_cast<float>(lados);
        Linha(centro + Vector2(Math::Cos(a0), Math::Sin(a0)) * raio, centro + Vector2(Math::Cos(a1), Math::Sin(a1)) * raio, cor);
    }
}

void Daniel::Retangulo(const Vector2 de, const Vector2 ate, const SDL_Color cor, const bool tracejado) {
    const Vector2 cantos[4] = {de, Vector2(ate.x, de.y), ate, Vector2(de.x, ate.y)};
    for (int k = 0; k < 4; ++k) {
        if (tracejado) Tracejada(cantos[k], cantos[(k + 1) % 4], cor, 6.0f);
        else Linha(cantos[k], cantos[(k + 1) % 4], cor);
    }
}

void Daniel::PlacarDaEtapa(const std::string& fase, const size_t etapa, const std::string& texto) {
    auto etapas = EtapasDa(fase);
    if (etapa >= etapas.size() || etapas[etapa].second == texto) return;
    etapas[etapa].second = texto;
    RegistrarEtapas(fase, etapas);
}

void Daniel::Avisar(const std::string& texto, const float duracao) {
    mAviso = texto;
    mAvisoAte = mRelogio + duracao;
}

void Daniel::Placar(const std::string& fase, const size_t etapa, const std::string& texto) {
    const std::string& vale = mRelogio < mAvisoAte ? mAviso : texto;
    if (!vale.empty()) PlacarDaEtapa(fase, etapa, vale);
}

std::optional<Vector2> Daniel::AlvoDoMovimento() const {
    return mAlvo;
}

void Daniel::Varrer(const float t, const float amplitude, const float periodo) {
    const SDL_FRect c = Campo();
    mAlvo = Vector2(c.x + c.w * (0.5f + amplitude * Math::Sin(Math::TwoPi * t / periodo)), c.y + 62.0f);
}

Vector2 Daniel::OndeNaIda(const Ida& ida, const float t) {
    if (ida.desde < 0.0f || ida.dura <= 0.0f) return ida.para;
    const float u = std::clamp((t - ida.desde) / ida.dura, 0.0f, 1.0f);
    const float suave = u * u * (3.0f - 2.0f * u);
    return ida.de + (ida.para - ida.de) * suave;
}

void Daniel::Desmontar() {
    DesmontarSorteio();
    DesmontarPreparacao();
    DesmontarAgrupamento();
    DesmontarClassificacao();
    DesmontarExplicacao();
    if (mLinhas) mLinhas->Limpar();
    mAlvo.reset();
    mAvisoAte = -1.0f;
}

void Daniel::AoTerminarABatalha() {
    Desmontar();
}

void Daniel::OnUpdate(const float deltaTime) {
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

    if (fase == Preparacao::kFase) AtualizarPreparacao(t, deltaTime);
    else if (fase == Agrupamento::kFase) AtualizarAgrupamento(t, deltaTime);
    else if (fase == Classificacao::kFase) AtualizarClassificacao(t, deltaTime);
    else if (fase == Explicacao::kFase) AtualizarExplicacao(t, deltaTime);
}

void Daniel::OnDerrubou(BossProjectile* qual) {
    // A carta derrubada e a resposta do sorteado.
    for (size_t k = 0; k < mSorteado.cartas.size(); ++k) {
        if (mSorteado.cartas[k] == qual) {
            Responder(static_cast<int>(k));
            return;
        }
    }
}

bool Daniel::AceitaDano() const {
    return !(mSorteado.feito && !mSorteado.respondido);
}

void Daniel::AoSerPerguntado() {
    mPerguntas.push_back(mRelogio);
    while (!mPerguntas.empty() && mRelogio - mPerguntas.front() > Explicacao::kJanelaDasPerguntas) mPerguntas.pop_front();
}

// ---- O sorteio ----

std::string Daniel::NomeDe(const int quem) const {
    if (quem < 0) return Chamada::Colega(mSorteado.colega);
    return "J" + std::to_string(quem + 1);
}

void Daniel::DesmontarSorteio() {
    for (BossProjectile*& c : mSorteado.cartas) Matar(c);
    mSorteado = Sorteado{};
    mProximaGaroa = 0.0f;
}

void Daniel::Responder(const int carta) {
    if (mSorteado.respondido || carta < 0 || carta >= Chamada::kOpcoes) return;
    mSorteado.respondido = true;
    const Chamada::Pergunta& p = Chamada::Revisao(mSorteado.aula)[static_cast<size_t>(mSorteado.pergunta)];
    const Vector2 onde = mSorteado.cartas[static_cast<size_t>(carta)] ? mSorteado.cartas[static_cast<size_t>(carta)]->GetPosition()
                                                                      : GetPosition();
    mSorteado.cartas[static_cast<size_t>(carta)] = nullptr;   // ja caiu: o tiro do aluno a derrubou
    if (mSorteado.opcaoDaCarta[static_cast<size_t>(carta)] == 0) {
        if (auto* b = Batalha()) b->Conquistar(Chamada::kPremio);
        // \xc2\xb7 e o ponto do meio.
        mSorteado.placar = std::string("Isso! ") + p.opcoes[0] + "  \xc2\xb7  +" + std::to_string(static_cast<int>(Chamada::kPremio));
    } else {
        for (BossProjectile* c : DispararEm(Chamada::kGatilhoDoEngano, onde, 0, mSorteio.Real(0.0f, 36.0f))) Cara(c, "Engano");
        // \xc3\xa3 e o a com til, \xc3\xa9 o e agudo.
        mSorteado.placar = std::string("N\xc3\xa3o \xc3\xa9 bem isso: era ") + p.opcoes[0];
    }
    for (BossProjectile*& c : mSorteado.cartas) Matar(c);
}

void Daniel::AtualizarSorteio(const float t, const int aula, const std::string& fase, const char* caraDaGaroa) {
    const SDL_FRect campo = Campo();
    const int alunos = Batalha() ? static_cast<int>(Batalha()->Jogadores().size()) : 1;
    if (!mSorteado.feito) {
        mSorteado.feito = true;
        mSorteado.aula = aula;
        mSorteado.quem = Chamada::Sortear(mSorteio, alunos);
        mSorteado.pergunta = mSorteio.Inteiro(0, Chamada::kPerguntasPorAula - 1);
        mSorteado.colega = mSorteio.Inteiro(0, Chamada::kColegas - 1);
        // A ordem das cartas na tela: Fisher-Yates.
        for (int i = Chamada::kOpcoes - 1; i > 0; --i) {
            std::swap(mSorteado.opcaoDaCarta[static_cast<size_t>(i)],
                      mSorteado.opcaoDaCarta[static_cast<size_t>(mSorteio.Inteiro(0, i))]);
        }
        mProximaGaroa = 0.6f;
    }
    const Chamada::Pergunta& pergunta = Chamada::Revisao(aula)[static_cast<size_t>(mSorteado.pergunta)];

    // A roleta: os nomes da turma passando, cada vez mais devagar.
    if (t < Chamada::kRoleta) {
        std::vector<std::string> nomes;
        for (int a = 0; a < alunos; ++a) nomes.push_back("J" + std::to_string(a + 1));
        for (int k = 0; k < Chamada::kColegas; ++k) nomes.push_back(Chamada::Colega(k));
        const std::string& nome = nomes[static_cast<size_t>(Chamada::NomeNaRoleta(t, static_cast<int>(nomes.size())))];
        PlacarDaEtapa(fase, 0, "Sorteio: quem explica a aula passada?  \xc2\xbb  " + nome);
    } else if (!mSorteado.perguntou) {
        mSorteado.perguntou = true;
        SomDoDisparo();
        if (mSorteado.quem < 0) {
            mSorteado.respondido = true;
            mSorteado.placar = NomeDe(-1) + " explica: " + pergunta.opcoes[0];
        } else {
            // \xc3\xaa e o e circunflexo.
            const std::string quem = alunos > 1 ? NomeDe(mSorteado.quem) + ", eu escolho voc\xc3\xaa! "
                                                : std::string("Eu escolho voc\xc3\xaa! ");
            mSorteado.placar = quem + pergunta.enunciado;
            for (int k = 0; k < Chamada::kOpcoes; ++k) {
                const Vector2 onde(campo.x + campo.w * (0.2f + 0.3f * static_cast<float>(k)), campo.y + campo.h * 0.38f);
                BossProjectile* c = Um(Chamada::kGatilhoDaResposta, onde,
                                       Chamada::Cara(aula, mSorteado.pergunta, mSorteado.opcaoDaCarta[static_cast<size_t>(k)]),
                                       Vector2(0.0f, 9.0f));
                mSorteado.cartas[static_cast<size_t>(k)] = c;
            }
        }
    }

    // O tempo de responder acabou: as tres cartas estouram.
    if (mSorteado.perguntou && !mSorteado.respondido && t >= Chamada::kRoleta + Chamada::kParaResponder) {
        mSorteado.respondido = true;
        for (BossProjectile*& c : mSorteado.cartas) {
            if (Vivo(c)) {
                for (BossProjectile* e : DispararEm(Chamada::kGatilhoDoEngano, c->GetPosition(), 0, mSorteio.Real(0.0f, 36.0f))) {
                    Cara(e, "Engano");
                }
            }
            Matar(c);
        }
        mSorteado.placar = std::string("O tempo acabou: era ") + pergunta.opcoes[0];
    }
    // O Ponto Extra levou as cartas: ninguem respondeu, e ninguem perde.
    if (mSorteado.perguntou && !mSorteado.respondido) {
        bool alguma = false;
        for (const BossProjectile* c : mSorteado.cartas) alguma = alguma || Vivo(c);
        if (!alguma) {
            mSorteado.respondido = true;
            mSorteado.placar = std::string("A resposta era: ") + pergunta.opcoes[0];
        }
    }
    if (mSorteado.perguntou) PlacarDaEtapa(fase, 0, mSorteado.placar);

    // A garoa: enquanto a turma espera, os dados caem balancando.
    if (t >= mProximaGaroa) {
        mProximaGaroa = t + Dificuldade::Escolher(NivelDeDificuldade(), 0.1f, 0.085f);
        const Vector2 onde(mSorteio.Real(campo.x + 30.0f, campo.x + campo.w - 30.0f), campo.y + 110.0f);
        if (BossProjectile* g = Um(kGatilhoDaGaroa, onde, caraDaGaroa, Vector2(0.0f, 185.0f))) {
            g->DefinirClasse(-1);
        }
    }
    Varrer(t, 0.12f, 6.0f);
}

// ---- Fase 1: a preparacao ----

SDL_FRect Daniel::Grafico() const {
    const SDL_FRect c = Campo();
    // O grafico vai ate o chao: o aluno anda dentro dele.
    return SDL_FRect{c.x + 46.0f, c.y + 140.0f, c.w - 84.0f, c.h - 160.0f};
}

Vector2 Daniel::NoCampo(const Vector2 g) const {
    const SDL_FRect r = Grafico();
    return Vector2(r.x + g.x * r.w, r.y + (1.0f - g.y) * r.h);
}

Vector2 Daniel::NoGrafico(const Vector2 p) const {
    const SDL_FRect r = Grafico();
    return Vector2((p.x - r.x) / std::max(r.w, 1.0f), 1.0f - (p.y - r.y) / std::max(r.h, 1.0f));
}

Vector2 Daniel::LugarDaFonte(const int k) const {
    const SDL_FRect c = Campo();
    switch (k) {
        case 0: return Vector2(c.x + 26.0f, c.y + c.h * 0.55f);          // o sensor, a esquerda
        case 1: return Vector2(c.x + c.w * 0.28f, c.y + 112.0f);         // a planilha, no alto
        default: return Vector2(c.x + c.w - 26.0f, c.y + c.h * 0.45f);   // a API, a direita
    }
}

void Daniel::DesmontarPreparacao() {
    for (DadoNoGrafico& d : mDados) Matar(d.p);
    mDados.clear();
    for (BossProjectile*& f : mFontes) Matar(f);
    mEspeciesSorteadas = false;
    mProximaColeta = 0.0f;
    mColetados = 0;
    mProximoPasso = 0;
    mTransformando.reset();
    mOutliersMarcados = false;
    mEstourou = false;
    mImputou = false;
}

void Daniel::AtualizarPreparacao(const float t, const float deltaTime) {
    using namespace Preparacao;
    (void)deltaTime;
    const Degrau& g = mDegrauDaPreparacao;
    const int etapa = EtapaNoTempo(t);
    if (!mEspeciesSorteadas) {
        mEspeciesSorteadas = true;
        mEspecies = Especies(mSorteio);
    }
    mDados.erase(std::remove_if(mDados.begin(), mDados.end(), [](const DadoNoGrafico& d) { return !Vivo(d.p); }), mDados.end());

    // As fontes, nas bordas.
    for (size_t k = 0; k < mFontes.size(); ++k) {
        if (!Vivo(mFontes[k])) {
            mFontes[k] = Um(kGatilhoDaFonte, LugarDaFonte(static_cast<int>(k)), kFontes[k]);
            if (mFontes[k]) mFontes[k]->DefinirEnfeite(true);
        }
        if (mFontes[k]) mFontes[k]->SetPosition(LugarDaFonte(static_cast<int>(k)));
    }

    // A coleta: cada dado voa da fonte ate o lugar dele no grafico, e fica. Na limpeza ela segue,
    // mais devagar; na normalizacao, para.
    const float cadencia = etapa == 0 ? g.entreColetas : g.entreColetas * 2.2f;
    if (etapa < 2 && t >= mProximaColeta && static_cast<int>(mDados.size()) < g.maisDados) {
        mProximaColeta = t + cadencia;
        DadoNoGrafico d;
        d.dado = Coletar(mSorteio, mEspecies, g.fracaoOutlier, g.fracaoNula);
        const Vector2 de = LugarDaFonte(d.dado.fonte), para = NoCampo(d.dado.onde);
        const float distancia = (para - de).Length();
        const std::string cara = d.dado.tipo == Tipo::Outlier ? kCaraDoOutlier
                                 : d.dado.tipo == Tipo::Nulo  ? kCaraDoNulo
                                                              : kCaras[static_cast<size_t>(d.dado.especie)];
        d.p = Um(kGatilhoDoDado, de, cara, (para - de) * (g.velocidadeDaColeta / std::max(distancia, 1.0f)));
        d.chega = t + distancia / g.velocidadeDaColeta;
        if (d.p) {
            d.p->DefinirResistencia(g.resistencia);
            mDados.push_back(d);
            if (++mColetados % 4 == 1) SomDoDisparo();
        }
    }

    // Quem chegou para no lugar; quem esta numa ida anda nela.
    for (DadoNoGrafico& d : mDados) {
        if (!d.chegou && t >= d.chega) {
            d.chegou = true;
            Parar(d.p);
            d.p->SetPosition(NoCampo(d.dado.onde));
        }
        if (d.ida.desde >= 0.0f && t >= d.ida.desde) {
            d.p->SetPosition(OndeNaIda(d.ida, t));
            if (Chegou(d.ida, t)) {
                d.dado.onde = NoGrafico(d.ida.para);
                d.ida.desde = -1.0f;
            }
        }
    }

    // ---- A limpeza ----
    // O z-score acha os outliers um pouco antes: eles piscam, cercados, e estouram.
    if (!mOutliersMarcados && t >= kEstouro - 0.7f) {
        mOutliersMarcados = true;
        std::vector<Vector2> onde;
        std::vector<DadoNoGrafico*> quem;
        for (DadoNoGrafico& d : mDados) {
            if (!d.chegou || d.dado.tipo == Tipo::Nulo) continue;
            onde.push_back(d.dado.onde);
            quem.push_back(&d);
        }
        const std::vector<bool> marcados = Outliers(onde, kLimiteDoOutlier);
        int n = 0;
        for (size_t k = 0; k < quem.size(); ++k) {
            if (!marcados[k]) continue;
            quem[k]->marcado = true;
            ++n;
        }
        Avisar("z-score: " + std::to_string(n) + (n == 1 ? " outlier" : " outliers") + " a mais de 2 desvios da m\xc3\xa9" "dia");
    }
    if (mOutliersMarcados && !mEstourou) {
        const bool aceso = static_cast<int>(t / 0.1f) % 2 == 0;
        for (const DadoNoGrafico& d : mDados) {
            if (d.marcado && aceso) Circulo(d.p->GetPosition(), 14.0f, kPerigo, 12);
        }
    }
    if (!mEstourou && t >= kEstouro) {
        mEstourou = true;
        for (DadoNoGrafico& d : mDados) {
            if (!d.marcado) continue;
            DispararEm(kGatilhoDoEstouro, d.p->GetPosition(), 0, mSorteio.Real(0.0f, 45.0f));
            Matar(d.p);
        }
    }
    // A imputacao: o nulo mostra para onde vai - a media dos outros -, e vai.
    if (t >= kImputacao - 0.7f && !mImputou) {
        std::vector<Vector2> onde;
        for (const DadoNoGrafico& d : mDados) {
            if (d.chegou && d.dado.tipo != Tipo::Nulo && Vivo(d.p)) onde.push_back(d.p->GetPosition());
        }
        mMediaDaImputacao = onde.empty() ? NoCampo(Vector2(0.5f, 0.5f)) : Media(onde);
        for (const DadoNoGrafico& d : mDados) {
            if (d.dado.tipo == Tipo::Nulo && d.chegou && Vivo(d.p)) Tracejada(d.p->GetPosition(), mMediaDaImputacao, kSeta, 5.0f);
        }
        if (t >= kImputacao) {
            mImputou = true;
            int n = 0;
            for (DadoNoGrafico& d : mDados) {
                if (d.dado.tipo != Tipo::Nulo || !d.chegou || !Vivo(d.p)) continue;
                d.dado.tipo = Tipo::Comum;
                d.ida = Ida{d.p->GetPosition(), mMediaDaImputacao + Vector2(mSorteio.Real(-16.0f, 16.0f), mSorteio.Real(-16.0f, 16.0f)), t, 0.45f};
                Cara(d.p, "Cinza");
                ++n;
            }
            SomDoDisparo();
            // \xc3\xa7 e o c cedilha.
            Avisar("Imputa\xc3\xa7\xc3\xa3o: " + std::to_string(n) + (n == 1 ? " nulo vira" : " nulos viram") + " a m\xc3\xa9" "dia");
        }
    }

    // ---- A normalizacao ----
    // Cada transformacao: a seta de cada dado ate o lugar novo (e a caixa do min-max), e depois a ida.
    while (mProximoPasso < kPassos.size() && t >= kPassos[mProximoPasso].quando) {
        const Passo passo = kPassos[mProximoPasso++];
        std::vector<Vector2> onde;
        std::vector<DadoNoGrafico*> quem;
        for (DadoNoGrafico& d : mDados) {
            if (!d.chegou || !Vivo(d.p)) continue;
            onde.push_back(NoGrafico(d.p->GetPosition()));
            quem.push_back(&d);
        }
        Transformando tr;
        tr.qual = passo.qual;
        tr.avisoDesde = t;
        tr.comeca = t + g.aviso;
        std::vector<Vector2> novos;
        if (passo.qual == Transformacao::MinMax) {
            // A caixa e o canto do chao em que o aluno esta: o grafico desaba em cima dele.
            tr.caixa = CaixaNoCanto(CantoDoAluno(NoGrafico(PosicaoDoAluno()).x), kLadoDaCaixa);
            novos = MinMax(onde, tr.caixa);
        } else {
            novos = Padronizar(onde, Vector2(0.5f, 0.45f), Vector2(0.2f, 0.21f));
        }
        for (size_t k = 0; k < quem.size(); ++k) {
            const Vector2 n(std::clamp(novos[k].x, 0.0f, 1.0f), std::clamp(novos[k].y, 0.0f, 1.0f));
            quem[k]->ida = Ida{quem[k]->p->GetPosition(), NoCampo(n), tr.comeca, g.transformacao};
        }
        mTransformando = tr;
    }
    if (mTransformando && t < mTransformando->comeca + g.transformacao) {
        if (t < mTransformando->comeca) {
            for (const DadoNoGrafico& d : mDados) {
                if (d.ida.desde >= 0.0f && Vivo(d.p)) Linha(d.p->GetPosition(), d.ida.para, kSeta);
            }
            if (mTransformando->qual == Transformacao::MinMax) {
                Retangulo(NoCampo(Vector2(mTransformando->caixa.de.x, mTransformando->caixa.ate.y)),
                          NoCampo(Vector2(mTransformando->caixa.ate.x, mTransformando->caixa.de.y)), kSeta, true);
            }
        } else if (t < mTransformando->comeca + 0.05f) {
            SomDoDisparo();
        }
    }

    // Os eixos do grafico: o ataque embaixo, a defesa a esquerda.
    const SDL_FRect r = Grafico();
    Linha(Vector2(r.x, r.y), Vector2(r.x, r.y + r.h), kEixo);
    Linha(Vector2(r.x, r.y + r.h), Vector2(r.x + r.w, r.y + r.h), kEixo);
    for (int k = 1; k <= 4; ++k) {
        const float x = r.x + r.w * static_cast<float>(k) / 5.0f, y = r.y + r.h * static_cast<float>(k) / 5.0f;
        Linha(Vector2(x, r.y + r.h), Vector2(x, r.y + r.h + 5.0f), kEixo);
        Linha(Vector2(r.x - 5.0f, y), Vector2(r.x, y), kEixo);
    }

    // O placar da etapa.
    std::string texto;
    if (etapa == 0) {
        int outliers = 0, nulos = 0;
        for (const DadoNoGrafico& d : mDados) {
            if (d.dado.tipo == Tipo::Outlier) ++outliers;
            if (d.dado.tipo == Tipo::Nulo) ++nulos;
        }
        texto = "Coleta: " + std::to_string(mDados.size()) + " dados  \xc2\xb7  " + std::to_string(outliers) +
                " outliers  \xc2\xb7  " + std::to_string(nulos) + " nulos   (x: ataque, y: defesa)";
    } else if (etapa == 1) {
        texto = "Limpeza: o z-score acha o outlier; o nulo vira a m\xc3\xa9" "dia";
    } else if (mTransformando) {
        texto = mTransformando->qual == Transformacao::MinMax
                    ? "Min-max: o gr\xc3\xa1" "fico inteiro cabe na caixa do canto"
                    : "Padroniza\xc3\xa7\xc3\xa3o: m\xc3\xa9" "dia no meio, um desvio de cada lado";
    }
    Placar(kFase, static_cast<size_t>(etapa), texto);
    Varrer(t, 0.28f, 10.0f);
}

// ---- Fase 2: os atributos e os grupos ----

void Daniel::DesmontarAgrupamento() {
    for (PontoDaNuvem& p : mNuvem) Matar(p.p);
    mNuvem.clear();
    for (BossProjectile*& c : mCentroides) Matar(c);
    mCentroides.clear();
    mCentros.clear();
    mIdasDosCentros.clear();
    mAnelPendente.clear();
    mNuvemCriada = false;
    mProjetou = false;
    mGirou = false;
    mAgrupou = false;
    mIteracao = 0;
    mCotovelo = false;
    mInerciaInicial = 0.0f;
}

void Daniel::AtualizarAgrupamento(const float t, const float deltaTime) {
    using namespace Agrupamento;
    (void)deltaTime;
    const SDL_FRect campo = Campo();
    const Degrau& g = mDegrauDoAgrupamento;
    const int etapa = EtapaNoTempo(t);
    if (etapa == 0) {
        AtualizarSorteio(t, 0, kFase, "Cinza");
        return;
    }
    if (mSorteado.feito) DesmontarSorteio();
    mNuvem.erase(std::remove_if(mNuvem.begin(), mNuvem.end(), [](const PontoDaNuvem& p) { return !Vivo(p.p); }), mNuvem.end());
    const float inicioDaPca = kDuracoes[0], inicioDasKMedias = kDuracoes[0] + kDuracoes[1];
    const Vector2 meio(campo.x + campo.w * 0.5f, campo.y + campo.h * 0.52f);

    // A nuvem: alongada, numa direcao sorteada, e cada ponto sai do professor.
    if (!mNuvemCriada) {
        mNuvemCriada = true;
        const float angulo = mSorteio.Real(0.0f, Math::Pi);
        const Vector2 longo(Math::Cos(angulo), Math::Sin(angulo)), curto(-longo.y, longo.x);
        mSentidoDoGiro = mSorteio.Inteiro(0, 1) == 0 ? -1.0f : 1.0f;
        for (int k = 0; k < g.pontos; ++k) {
            const auto sino = [this]() { return (mSorteio.Real(-1.0f, 1.0f) + mSorteio.Real(-1.0f, 1.0f) + mSorteio.Real(-1.0f, 1.0f)) * 0.58f; };
            Vector2 alvo = meio + longo * (campo.w * 0.26f * sino()) + curto * (campo.w * 0.05f * sino());
            alvo.x = std::clamp(alvo.x, campo.x + 30.0f, campo.x + campo.w - 30.0f);
            alvo.y = std::clamp(alvo.y, campo.y + 140.0f, campo.y + campo.h - 60.0f);
            PontoDaNuvem p;
            p.p = Um(kGatilhoDoPonto, GetPosition(), "Cinza");
            if (!p.p) continue;
            p.ida = Ida{GetPosition(), alvo, inicioDaPca, 0.7f};
            mNuvem.push_back(p);
        }
        SomDoDisparo();
    }
    // O eixo: a componente principal dos lugares de chegada; a seta de cada ponto ate ela.
    if (t >= inicioDaPca + 1.2f && !mProjetou) {
        std::vector<Vector2> onde;
        for (const PontoDaNuvem& p : mNuvem) onde.push_back(p.ida.para);
        mEixo = ComponentePrincipal(onde);
        for (PontoDaNuvem& p : mNuvem) p.projetado = Projetar(mEixo, p.ida.para);
        if (t >= inicioDaPca + 1.9f) {
            mProjetou = true;
            for (PontoDaNuvem& p : mNuvem) p.ida = Ida{p.p->GetPosition(), p.projetado, t, 0.5f};
            SomDoDisparo();
        }
    }
    const float giroDe = inicioDaPca + 2.6f, giroAte = inicioDaPca + 5.2f;
    float giro = 0.0f;
    if (t >= giroDe) {
        const float u = std::clamp((t - giroDe) / (giroAte - giroDe), 0.0f, 1.0f);
        giro = mSentidoDoGiro * g.giroDoEixo * u * u * (3.0f - 2.0f * u);
        if (!mGirou) {
            mGirou = true;
            SomDoDisparo();
        }
    }

    // Os grupos: os pontos vao para tres manchas, e os centroides aparecem.
    if (t >= inicioDaPca + 5.4f && !mAgrupou) {
        mAgrupou = true;
        std::array<Vector2, kGrupos> manchas{};
        for (int k = 0; k < kGrupos; ++k) {
            const float a = Math::TwoPi * static_cast<float>(k) / kGrupos + mSorteio.Real(-0.4f, 0.4f);
            manchas[static_cast<size_t>(k)] = meio + Vector2(Math::Cos(a) * campo.w * 0.27f, Math::Sin(a) * campo.h * 0.2f);
        }
        for (size_t k = 0; k < mNuvem.size(); ++k) {
            const Vector2 alvo = manchas[k % kGrupos] + Vector2(mSorteio.Real(-48.0f, 48.0f), mSorteio.Real(-40.0f, 40.0f));
            mNuvem[k].ida = Ida{mNuvem[k].p->GetPosition(), alvo, t, 0.6f};
        }
    }

    // Cada ponto: na ida dele, ou girando com a fileira.
    for (PontoDaNuvem& p : mNuvem) {
        if (p.ida.desde >= 0.0f && t >= p.ida.desde) {
            p.p->SetPosition(OndeNaIda(p.ida, t));
            if (Chegou(p.ida, t)) p.ida.desde = -1.0f;
        } else if (mProjetou && !mAgrupou && t >= giroDe) {
            p.p->SetPosition(Girar(p.projetado, mEixo.centro, giro));
        }
    }
    if (t >= inicioDaPca + 1.2f && t < inicioDaPca + 5.4f) {
        const Vector2 d = Vector2::Normalize(Girar(mEixo.centro + mEixo.direcao, mEixo.centro, giro) - mEixo.centro);
        Linha(mEixo.centro - d * campo.w, mEixo.centro + d * campo.w, kEixo);
        if (!mProjetou) {
            for (const PontoDaNuvem& p : mNuvem) Tracejada(p.ida.para, p.projetado, kSeta, 4.0f);
        }
    }

    // ---- As k-medias ----
    if (t >= inicioDasKMedias) {
        if (mCentros.empty()) {
            for (int k = 0; k < kGrupos; ++k) {
                mCentros.emplace_back(mSorteio.Real(campo.x + 60.0f, campo.x + campo.w - 60.0f),
                                      mSorteio.Real(campo.y + 160.0f, campo.y + campo.h - 120.0f));
            }
            mIdasDosCentros.assign(mCentros.size(), Ida{});
            mAnelPendente.assign(mCentros.size(), false);
            mProximaIteracao = t + 0.5f;
        }
        // O cotovelo: o grupo maior se divide, e o k sobe.
        if (!mCotovelo && t >= inicioDasKMedias + kCotovelo && mCentros.size() < kCores.size()) {
            mCotovelo = true;
            std::vector<Vector2> pontos;
            for (const PontoDaNuvem& p : mNuvem) pontos.push_back(p.p->GetPosition());
            const std::vector<int> grupos = Atribuir(pontos, mCentros);
            std::vector<int> contagem(mCentros.size(), 0);
            for (const int gr : grupos) if (gr >= 0) ++contagem[static_cast<size_t>(gr)];
            const size_t maior = static_cast<size_t>(std::max_element(contagem.begin(), contagem.end()) - contagem.begin());
            const float antes = Inercia(pontos, grupos, mCentros);
            mCentros.push_back(mCentros[maior] + Vector2(mSorteio.Real(-70.0f, 70.0f), mSorteio.Real(30.0f, 70.0f)));
            mIdasDosCentros.emplace_back();
            mAnelPendente.push_back(false);
            const float depois = Inercia(pontos, Atribuir(pontos, mCentros), mCentros);
            // \xc3\xa9 e o e agudo.
            Avisar("Cotovelo: k = " + std::to_string(mCentros.size()) + ", a in\xc3\xa9rcia cai " +
                   std::to_string(static_cast<int>(100.0f * (1.0f - depois / std::max(antes, 1.0f)))) + "%", 1.8f);
        }
        // Cada centroide tem o seu projetil; o Ponto Extra o leva, e ele volta.
        mCentroides.resize(mCentros.size(), nullptr);
        for (size_t k = 0; k < mCentros.size(); ++k) {
            if (!Vivo(mCentroides[k])) mCentroides[k] = Um(kGatilhoDoCentroide, mCentros[k], "C" + std::to_string(k));
        }
        // A iteracao: a atribuicao pinta os pontos; a atualizacao manda o centroide para a media - e o
        // aluno conta.
        if (t >= mProximaIteracao) {
            mProximaIteracao = t + g.entreIteracoes;
            ++mIteracao;
            std::vector<Vector2> pontos;
            for (const PontoDaNuvem& p : mNuvem) pontos.push_back(p.p->GetPosition());
            const std::vector<int> grupos = Atribuir(pontos, mCentros);
            const std::vector<Vector2> novos = Atualizar(pontos, grupos, mCentros, Alunos(), g.pesoDoAluno);
            mInercia = Inercia(pontos, grupos, novos);
            if (mIteracao == 1) mInerciaInicial = std::max(mInercia, 1.0f);
            for (size_t k = 0; k < mCentros.size(); ++k) {
                mIdasDosCentros[k] = Ida{mCentros[k], novos[k], t, kDeslize};
                mAnelPendente[k] = (novos[k] - mCentros[k]).Length() > 10.0f;
            }
            for (size_t k = 0; k < mNuvem.size(); ++k) {
                const int gr = grupos[k];
                if (gr < 0) continue;
                Cara(mNuvem[k].p, Preparacao::kCaras[static_cast<size_t>(gr) % Preparacao::kCaras.size()]);
                const Vector2 alvo = pontos[k] + (novos[static_cast<size_t>(gr)] - pontos[k]) * kAperto;
                mNuvem[k].ida = Ida{pontos[k], alvo, t, kDeslize};
            }
        }
        for (size_t k = 0; k < mCentros.size(); ++k) {
            Ida& ida = mIdasDosCentros[k];
            if (ida.desde < 0.0f) continue;
            mCentros[k] = OndeNaIda(ida, t);
            if (Chegou(ida, t)) {
                ida.desde = -1.0f;
                mCentros[k] = ida.para;
                if (mAnelPendente[k]) {
                    mAnelPendente[k] = false;
                    DispararEm(kGatilhoDoAnel, mCentros[k], g.tirosDoAnel, mSorteio.Real(0.0f, 45.0f),
                               360.0f - 360.0f / static_cast<float>(g.tirosDoAnel));
                }
            }
        }
        for (size_t k = 0; k < mCentros.size(); ++k) {
            if (mCentroides[k]) mCentroides[k]->SetPosition(mCentros[k]);
        }
        // O Voronoi: a celula de cada centroide; a do aluno, mais forte, com a linha ate quem o persegue.
        const Vector2 de(campo.x, campo.y + 100.0f), ate(campo.x + campo.w, campo.y + campo.h);
        const std::vector<Vector2> alunos = Alunos();
        std::vector<int> doAluno;
        for (const Vector2& a : alunos) doAluno.push_back(MaisPerto(mCentros, a));
        for (size_t k = 0; k < mCentros.size(); ++k) {
            const std::vector<Vector2> celula = Celula(mCentros, static_cast<int>(k), de, ate);
            SDL_Color cor = kCores[k % kCores.size()];
            const bool deAlguem = std::find(doAluno.begin(), doAluno.end(), static_cast<int>(k)) != doAluno.end();
            cor.a = deAlguem ? 170 : 70;
            for (size_t v = 0; v < celula.size(); ++v) {
                const Vector2 a = celula[v], b = celula[(v + 1) % celula.size()];
                // Nao desenha a borda do campo: so as fronteiras entre celulas.
                const bool naBorda = (std::fabs(a.x - b.x) < 0.5f && (std::fabs(a.x - de.x) < 0.5f || std::fabs(a.x - ate.x) < 0.5f)) ||
                                     (std::fabs(a.y - b.y) < 0.5f && (std::fabs(a.y - de.y) < 0.5f || std::fabs(a.y - ate.y) < 0.5f));
                if (!naBorda) Linha(a, b, cor);
            }
        }
        for (size_t a = 0; a < alunos.size(); ++a) {
            if (doAluno[a] < 0) continue;
            SDL_Color cor = kCores[static_cast<size_t>(doAluno[a]) % kCores.size()];
            cor.a = 120;
            Tracejada(alunos[a], mCentros[static_cast<size_t>(doAluno[a])], cor, 6.0f);
        }
    }

    // O placar.
    std::string texto;
    if (etapa == 1) {
        texto = !mProjetou ? "PCA: a componente principal \xc3\xa9 a dire\xc3\xa7\xc3\xa3o de maior vari\xc3\xa2ncia"
                           : "Proje\xc3\xa7\xc3\xa3o: de duas dimens\xc3\xb5" "es para uma - e a fileira gira";
    } else {
        const int caiu = mInerciaInicial > 0.0f ? static_cast<int>(std::lround(100.0f * mInercia / mInerciaInicial)) : 100;
        texto = "k = " + std::to_string(mCentros.size()) + "  \xc2\xb7  itera\xc3\xa7\xc3\xa3o " + std::to_string(mIteracao) +
                "  \xc2\xb7  in\xc3\xa9rcia em " + std::to_string(caiu) + "% da primeira  \xc2\xb7  o seu centr\xc3\xb3" "ide vem a voc\xc3\xaa";
    }
    Placar(kFase, static_cast<size_t>(etapa), texto);
    Varrer(t, 0.22f, 9.0f);
}

// ---- Fase 3: a classificacao ----

Vector2 Daniel::Normalizado(const Vector2 p) const {
    const SDL_FRect c = Campo();
    return Vector2((p.x - (c.x + c.w * 0.5f)) / (c.w * 0.5f), (p.y - (c.y + c.h * 0.5f)) / (c.h * 0.5f));
}

Vector2 Daniel::DoNormalizado(const Vector2 n) const {
    const SDL_FRect c = Campo();
    return Vector2(c.x + c.w * 0.5f + n.x * c.w * 0.5f, c.y + c.h * 0.5f + n.y * c.h * 0.5f);
}

int Daniel::ClasseEm(const Vector2 p, const int etapa) const {
    if (etapa == 1) return Classificacao::Lado(mModelo, Normalizado(p));
    if (etapa == 2 && !mComite.empty()) return Classificacao::Voto(mComite, Normalizado(p));
    return -1;
}

void Daniel::LimparClasses() {
    if (const Battle* b = Batalha()) {
        for (Player* p : b->Jogadores()) {
            if (p) p->DefinirClasse(-1);
        }
    }
}

void Daniel::DesmontarClassificacao() {
    LimparClasses();
    for (BossProjectile*& a : mArvores) Matar(a);
    mComite.clear();
    mArvoreNova.reset();
    mProximoModelo.reset();
    mPerceptronComecou = false;
    mComiteComecou = false;
    mEpoca = 0;
    mClasseDaRajada = -1;
}

void Daniel::AtualizarClassificacao(const float t, const float deltaTime) {
    using namespace Classificacao;
    (void)deltaTime;
    const SDL_FRect campo = Campo();
    const Degrau& g = mDegrauDaClassificacao;
    const int etapa = EtapaNoTempo(t);
    if (etapa == 0) {
        AtualizarSorteio(t, 1, kFase, "Cinza");
        return;
    }
    if (mSorteado.feito) DesmontarSorteio();

    // ---- O perceptron: a reta aprende com os erros, e os dados mudam (o drift) ----
    if (etapa == 1) {
        if (!mPerceptronComecou) {
            mPerceptronComecou = true;
            mAnguloDaVerdade = mSorteio.Real(0.0f, 360.0f);
            mVerdade = RetaPeloAngulo(mAnguloDaVerdade, mSorteio.Real(-0.2f, 0.2f));
            mModelo = RetaPeloAngulo(mAnguloDaVerdade + mSorteio.Real(70.0f, 110.0f), 0.0f);
            mProximaEpoca = t + g.entreEpocas;
            mProximaRajada = t + 0.4f;
        }
        if (!mProximoModelo && t >= mProximaEpoca - g.aviso) {
            mAnguloDaVerdade += (mSorteio.Inteiro(0, 1) == 0 ? -1.0f : 1.0f) * mSorteio.Real(15.0f, 35.0f);
            mVerdade = RetaPeloAngulo(mAnguloDaVerdade, mSorteio.Real(-0.25f, 0.25f));
            const Epoca e = Treinar(mModelo, mVerdade, mSorteio, kAmostrasPorEpoca, kTaxa);
            mProximoModelo = e.modelo;
            mErrosDaEpoca = e.erros;
        }
        if (mProximoModelo) {
            Vector2 de, ate;
            if (Segmento(*mProximoModelo, de, ate)) Tracejada(DoNormalizado(de), DoNormalizado(ate), kSeta, 10.0f);
            if (t >= mProximaEpoca) {
                mModelo = *mProximoModelo;
                mProximoModelo.reset();
                mProximaEpoca += g.entreEpocas;
                ++mEpoca;
                SomDoDisparo();
            }
        }
        Vector2 de, ate;
        if (Segmento(mModelo, de, ate)) {
            Linha(DoNormalizado(de), DoNormalizado(ate), kFronteira);
            Linha(DoNormalizado(de) + Vector2(0.0f, 1.5f), DoNormalizado(ate) + Vector2(0.0f, 1.5f), kFronteira);
        }
    }

    // ---- O comite: tres arvores votam; uma e replantada de tempos em tempos ----
    if (etapa == 2) {
        if (!mComiteComecou) {
            mComiteComecou = true;
            mCortes = Vector2(mSorteio.Real(-0.3f, 0.3f), mSorteio.Real(-0.2f, 0.3f));
            mComite.clear();
            for (int k = 0; k < kArvores; ++k) mComite.push_back(Plantar(mSorteio, mCortes, 0.3f, 0.2f));
            mProximoReplantio = t + g.entreReplantios;
            SomDoDisparo();
        }
        for (size_t k = 0; k < mArvores.size(); ++k) {
            const Vector2 onde(campo.x + campo.w * (0.3f + 0.2f * static_cast<float>(k)), campo.y + 118.0f);
            if (!Vivo(mArvores[k])) {
                mArvores[k] = Um(kGatilhoDaArvore, onde, "A" + std::to_string(k));
                if (mArvores[k]) mArvores[k]->DefinirEnfeite(true);
            }
            if (mArvores[k]) mArvores[k]->SetPosition(onde);
        }
        if (!mArvoreNova && t >= mProximoReplantio - g.aviso) {
            mCortes.x = std::clamp(mCortes.x + mSorteio.Real(-0.2f, 0.2f), -0.5f, 0.5f);
            mCortes.y = std::clamp(mCortes.y + mSorteio.Real(-0.2f, 0.2f), -0.4f, 0.5f);
            mArvoreNova = Plantar(mSorteio, mCortes, 0.3f, 0.2f);
        }
        // Os cortes de cada arvore, fracos na cor dela; o da replantada, tracejado.
        const auto cortes = [&](const Arvore& a, SDL_Color cor, const bool tracejado) {
            const Vector2 alto = DoNormalizado(Vector2(a.corteX, -0.75f)), baixo = DoNormalizado(Vector2(a.corteX, 1.0f));
            if (tracejado) Tracejada(alto, baixo, cor, 8.0f); else Linha(alto, baixo, cor);
            for (int lado = 0; lado < 2; ++lado) {
                const float x0 = lado == 0 ? -1.0f : a.corteX, x1 = lado == 0 ? a.corteX : 1.0f;
                const Vector2 e = DoNormalizado(Vector2(x0, a.corteY[static_cast<size_t>(lado)])),
                              d = DoNormalizado(Vector2(x1, a.corteY[static_cast<size_t>(lado)]));
                if (tracejado) Tracejada(e, d, cor, 8.0f); else Linha(e, d, cor);
            }
        };
        for (size_t k = 0; k < mComite.size(); ++k) {
            SDL_Color cor = kCoresDasArvores[k % kCoresDasArvores.size()];
            cor.a = 90;
            cortes(mComite[k], cor, false);
        }
        if (mArvoreNova) {
            cortes(*mArvoreNova, kCoresDasArvores[static_cast<size_t>(mProximaArvore) % kCoresDasArvores.size()], true);
            if (t >= mProximoReplantio) {
                mComite[static_cast<size_t>(mProximaArvore)] = *mArvoreNova;
                mArvoreNova.reset();
                mProximaArvore = (mProximaArvore + 1) % kArvores;
                mProximoReplantio += g.entreReplantios;
                SomDoDisparo();
            }
        }
    }

    // O mapa das classes: uma cruz de cada cor, de tanto em tanto, pelo campo.
    for (float y = campo.y + 150.0f; y < campo.y + campo.h - 20.0f; y += 58.0f) {
        for (float x = campo.x + 30.0f; x < campo.x + campo.w - 10.0f; x += 58.0f) {
            const int c = ClasseEm(Vector2(x, y), etapa);
            if (c < 0) continue;
            SDL_Color cor = kCores[static_cast<size_t>(c)];
            cor.a = 80;
            Linha(Vector2(x - 4.0f, y), Vector2(x + 4.0f, y), cor);
            Linha(Vector2(x, y - 4.0f), Vector2(x, y + 4.0f), cor);
        }
    }

    // A classe de cada aluno: o lado da fronteira em que ele esta. O anel dele mostra.
    if (const Battle* b = Batalha()) {
        for (Player* p : b->Jogadores()) {
            if (!p) continue;
            const int c = ClasseEm(p->GetPosition(), etapa);
            p->DefinirClasse(c);
            if (c >= 0) Circulo(p->GetPosition(), 20.0f, kCores[static_cast<size_t>(c)], 14);
        }
    }

    // Os dados de treino chovem do alto, rotulados, das duas classes: so a outra cor acerta.
    if (t >= mProximaChuva) {
        mProximaChuva = t + g.entreChuvas;
        const int c = mSorteio.Inteiro(0, kClasses - 1);
        const Vector2 onde(mSorteio.Real(campo.x + 20.0f, campo.x + campo.w - 20.0f), campo.y + 112.0f);
        // Os dados de treino saem pela garoa (calada): a mesma amostra, que cai balancando.
        if (BossProjectile* p = Um(kGatilhoDaGaroa, onde, kCaras[static_cast<size_t>(c)], Vector2(0.0f, 200.0f))) {
            p->DefinirClasse(c);
        }
    }

    // As rajadas: uma classe por vez, miradas no aluno; o anel do professor ja mostra a da proxima.
    if (mClasseDaRajada < 0) mClasseDaRajada = ClasseDaRajada(mSorteio, -1);
    Circulo(GetPosition(), 34.0f + 3.0f * Math::Sin(mRelogio * 9.0f), kCores[static_cast<size_t>(mClasseDaRajada)], 18);
    if (t >= mProximaRajada) {
        mProximaRajada = t + g.entreRajadas;
        const Vector2 de = GetPosition() + Vector2(0.0f, 40.0f);
        const int c = mClasseDaRajada;
        // A rajada mira um pouco a frente do aluno: quem so continua andando nao escapa.
        const float voo = (PosicaoDoAluno() - de).Length() / std::max(g.velocidade, 1.0f);
        const Vector2 rumo = GetPredictedPlayerDirection(std::min(0.25f * voo, 0.5f));
        const float mira = rumo.LengthSq() > 1e-6f ? Math::ToDegrees(Math::Atan2(rumo.y, rumo.x)) : AnguloAteOAluno(de);
        for (BossProjectile* p : DispararEm(kGatilhoDaAmostra, de, g.tirosDaRajada, mira, std::nullopt, g.velocidade)) {
            p->DefinirClasse(c);
            Cara(p, kCaras[static_cast<size_t>(c)]);
        }
        mClasseDaRajada = ClasseDaRajada(mSorteio, c);
    }

    // O placar.
    std::string texto;
    // \xc3\xa9 e o e agudo, \xc3\xa3 o a com til, \xc3\xaa o e circunflexo, \xc3\xa1 o a agudo.
    const std::string proxima = mClasseDaRajada == 0 ? "fogo" : "\xc3\xa1gua";
    if (etapa == 1) {
        texto = "\xc3\x89poca " + std::to_string(mEpoca) + ": " + std::to_string(mErrosDaEpoca) +
                " erros  \xc2\xb7  a pr\xc3\xb3xima rajada \xc3\xa9 de " + proxima;
    } else {
        texto = "Comit\xc3\xaa: tr\xc3\xaas \xc3\xa1rvores votam  \xc2\xb7  a pr\xc3\xb3xima rajada \xc3\xa9 de " + proxima;
    }
    Placar(kFase, static_cast<size_t>(etapa), texto);
    Varrer(t, 0.3f, 7.0f);
}

// ---- A repescagem: a explicacao ----

int Daniel::PerguntasRecentes() const {
    int n = 0;
    for (const float q : mPerguntas) {
        if (mRelogio - q <= Explicacao::kJanelaDasPerguntas) ++n;
    }
    return n;
}

void Daniel::DesmontarExplicacao() {
    Matar(mCaixaPreta);
    mPalpite.reset();
    mProximoPalpite = 0.0f;
    mBarras.clear();
    mContribuicoes.clear();
    mProximaCascata = 0.0f;
    mPrevisaoEm = -1.0f;
    for (Solto& s : mParados) s.p = nullptr;
    mParados.clear();
}

void Daniel::AtualizarExplicacao(const float t, const float deltaTime) {
    using namespace Explicacao;
    (void)deltaTime;
    const SDL_FRect campo = Campo();
    const Degrau& g = mDegrauDaExplicacao;
    const int etapa = EtapaNoTempo(t);
    if (etapa == 0) {
        AtualizarSorteio(t, 2, kFase, "Palpite");
        return;
    }
    if (mSorteado.feito) DesmontarSorteio();
    const Dificuldade::Nivel nivel = NivelDeDificuldade();

    // ---- A caixa-preta: quem pergunta ve o palpite mais cedo ----
    if (etapa == 1) {
        if (!Vivo(mCaixaPreta)) {
            mCaixaPreta = Um(kGatilhoDaCaixa, GetPosition(), "Caixa");
            if (mCaixaPreta) mCaixaPreta->DefinirEnfeite(true);
        }
        if (mCaixaPreta) mCaixaPreta->SetPosition(GetPosition());
        if (!mPalpite) {
            PalpiteDaVez p;
            p.tipo = static_cast<Palpite>(mSorteio.Inteiro(0, kPalpites - 1));
            p.sai = std::max(t + 0.3f, mProximoPalpite) + g.entrePalpites;
            mProximoPalpite = p.sai;
            mPalpite = p;
        }
        PalpiteDaVez& p = *mPalpite;
        mAntecedenciaDaVez = Antecedencia(PerguntasRecentes(), nivel);
        const Vector2 de = GetPosition() + Vector2(0.0f, 30.0f);
        if (!p.avisado && t >= p.sai - mAntecedenciaDaVez) {
            // O aviso trava o palpite: a linha mostra exatamente o que vai sair.
            p.avisado = true;
            const int tiros = p.tipo == Palpite::Anel ? g.tirosDoAnel : p.tipo == Palpite::Leque ? g.tirosDoLeque : g.tirosDoAnel;
            p.rumos = Rumos(p.tipo, AnguloAteOAluno(de), mSorteio.Real(0.0f, 360.0f), tiros);
        }
        if (p.avisado) {
            for (const float r : p.rumos) {
                const Vector2 u(Math::Cos(Math::ToRadians(r)), Math::Sin(Math::ToRadians(r)));
                Tracejada(de, de + u * 230.0f, kSeta, 7.0f);
            }
        }
        if (t >= p.sai) {
            if (!p.avisado) p.rumos = Rumos(p.tipo, AnguloAteOAluno(de), mSorteio.Real(0.0f, 360.0f), g.tirosDoAnel);
            for (const float r : p.rumos) {
                const Vector2 u(Math::Cos(Math::ToRadians(r)), Math::Sin(Math::ToRadians(r)));
                Um(kGatilhoDoPalpite, de, "Palpite", u * g.velocidade);
            }
            SomDoDisparo();
            mPalpite.reset();
        }
        // A antecedencia com duas casas e virgula: "0,42".
        const int centesimos = static_cast<int>(std::lround(mAntecedenciaDaVez * 100.0f));
        const std::string antes = std::to_string(centesimos / 100) + "," + (centesimos % 100 < 10 ? "0" : "") +
                                  std::to_string(centesimos % 100);
        Placar(kFase, 1, "Caixa-preta: " + std::to_string(PerguntasRecentes()) + " perguntas  \xc2\xb7  o palpite avisa " +
                             antes + " s antes");
    }

    // ---- O SHAP: a cascata das contribuicoes ----
    if (etapa == 2) {
        Matar(mCaixaPreta);
        const float base = campo.x + campo.w * 0.5f;
        const float topo = campo.y + campo.h * 0.28f, passo = campo.h * 0.095f;
        if (mBarras.empty() && t >= mProximaCascata) {
            mContribuicoes = Contribuicoes(mSorteio, static_cast<int>(kAtributos.size()), 50.0f, 150.0f, campo.w * 0.5f - 50.0f);
            const std::vector<Barra> cascata = Cascata(base, mContribuicoes);
            for (size_t k = 0; k < cascata.size(); ++k) {
                mBarras.push_back(BarraEmCampo{cascata[k], topo + passo * static_cast<float>(k),
                                               t + g.entreBarras * static_cast<float>(k), 0});
            }
            mPrevisaoEm = t + g.entreBarras * static_cast<float>(cascata.size()) + g.avisoDaBarra + 0.5f;
        }
        // O valor base: a linha do meio.
        Tracejada(Vector2(base, topo - 30.0f), Vector2(base, topo + passo * static_cast<float>(kAtributos.size())), kBase, 6.0f);
        constexpr float kEntreTiros = 15.0f, kEnchimento = 900.0f;
        for (BarraEmCampo& b : mBarras) {
            if (t < b.aparece) continue;
            const bool positiva = b.barra.ate >= b.barra.de;
            const SDL_Color cor = positiva ? SDL_Color{255, 90, 90, 170} : SDL_Color{80, 150, 255, 170};
            const float comprimento = std::fabs(b.barra.ate - b.barra.de);
            const int total = std::max(1, static_cast<int>(comprimento / kEntreTiros));
            if (t < b.aparece + g.avisoDaBarra) {
                Linha(Vector2(b.barra.de, b.y), Vector2(b.barra.ate, b.y), cor);
                Linha(Vector2(b.barra.de, b.y - 6.0f), Vector2(b.barra.de, b.y + 6.0f), cor);
                continue;
            }
            // Os tiros nascem do comeco para o fim da barra, depressa; param, e depois caem.
            const int devem = std::min(total, 1 + static_cast<int>((t - b.aparece - g.avisoDaBarra) * kEnchimento / kEntreTiros));
            if (b.cheios == 0 && devem > 0) SomDoDisparo();
            while (b.cheios < devem) {
                const float x = b.barra.de + (positiva ? 1.0f : -1.0f) * kEntreTiros * (static_cast<float>(b.cheios) + 0.5f);
                // A barra tem duas fileiras: e uma faixa, e nao um fio.
                for (const float dy : {-6.0f, 6.0f}) {
                    if (BossProjectile* p = Um(kGatilhoDaBarra, Vector2(x, b.y + dy), positiva ? "BarraMais" : "BarraMenos")) {
                        mParados.push_back(Solto{p, t + 0.6f});
                    }
                }
                ++b.cheios;
            }
        }
        // O fim da cascata: a previsao estoura.
        if (mPrevisaoEm >= 0.0f && t >= mPrevisaoEm && !mBarras.empty()) {
            const BarraEmCampo& ultima = mBarras.back();
            for (BossProjectile* p : DispararEm(kGatilhoDaPrevisao, Vector2(ultima.barra.ate, ultima.y + 40.0f), g.tirosDaPrevisao,
                                                mSorteio.Real(0.0f, 30.0f), 360.0f - 360.0f / static_cast<float>(g.tirosDaPrevisao))) {
                Cara(p, "Previsao");
            }
            mPrevisaoEm = -1.0f;
            mProximaCascata = t + 1.1f;
            mBarras.clear();
        }
        for (Solto& s : mParados) {
            if (s.p && t >= s.cai) {
                if (Vivo(s.p)) {
                    if (auto* rb = s.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, 195.0f));
                }
                s.p = nullptr;
            }
        }
        mParados.erase(std::remove_if(mParados.begin(), mParados.end(), [](const Solto& s) { return s.p == nullptr; }),
                       mParados.end());
        if (mContribuicoes.empty()) {
            Placar(kFase, 2, "SHAP: quanto cada atributo empurra a previs\xc3\xa3o, a partir do valor base");
        } else {
            const float previsao = std::accumulate(mContribuicoes.begin(), mContribuicoes.end(), 0.0f);
            const std::string fx = std::to_string(static_cast<int>(std::lround(previsao / 4.0f)));
            Placar(kFase, 2, Legenda(mContribuicoes, 4.0f) + "   \xc2\xbb  f(x) = base " + (previsao >= 0.0f ? "+" : "") + fx);
        }
    }
    Varrer(t, 0.2f, 8.0f);
}
