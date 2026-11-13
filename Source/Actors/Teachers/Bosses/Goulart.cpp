//
// O chefe da INF 451 - Sistemas Operacionais.
//

#include "Goulart.h"

#include <algorithm>
#include <cmath>

#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "BossesProjectiles/BossProjectile.h"

Goulart::Goulart(Scene* scene) : Boss(scene) {
}

void Goulart::SortearALuta() {
    mLutaSorteada = true;
    mSorteio = SorteioDaFase(Escalonador::kFase);
    const Dificuldade::Nivel n = NivelDeDificuldade();
    mDegrauDoEscalonador = Escalonador::DegrauDo(n);
    mDegrauDaPaginacao = Paginacao::DegrauDo(n);
    mDegrauDoDisco = Disco::DegrauDo(n);
    mDegrauDaNuvem = Virtualizacao::DegrauDo(n);
}

// ---- O que as fases dividem ----

std::vector<BossProjectile*> Goulart::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
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

BossProjectile* Goulart::Um(const char* gatilho, const Vector2 onde, const std::string& cara, const float escala,
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

void Goulart::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
    if (!mDisparandoEm) return;
    params.firePosition = mOndeDisparar;
    if (mQuantos > 0) params.numProjectiles = mQuantos;
    if (mCentroEm) params.centralAngle = *mCentroEm;
    if (mAberturaEm) params.angle = *mAberturaEm;
    if (mVelocidadeEm) params.projectileSpeed = *mVelocidadeEm;
}

void Goulart::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                   const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

float Goulart::AnguloAteOAluno(const Vector2 de) const {
    const Vector2 d = PosicaoDoAluno() - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

bool Goulart::Vivo(const BossProjectile* p) {
    return p && p->GetState() == ActorState::Active;
}

void Goulart::Matar(BossProjectile*& p) {
    if (Vivo(p)) p->Kill();
    p = nullptr;
}

void Goulart::Cara(BossProjectile* p, const std::string& cara) {
    if (!p || cara.empty()) return;
    if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) anim->SetAnimation(cara);
}

void Goulart::Levar(BossProjectile* p, const Vector2 para, const float deltaTime, const float rapidez) {
    if (!p) return;
    const Vector2 aqui = p->GetPosition();
    p->SetPosition(aqui + (para - aqui) * std::min(1.0f, rapidez * deltaTime));
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
}

void Goulart::Linha(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Acrescentar(de, ate, cor);
}

void Goulart::Retangulo(const Vector2 centro, const float largura, const float altura, const SDL_Color cor) {
    const float l = largura / 2.0f, a = altura / 2.0f;
    Linha(centro + Vector2(-l, -a), centro + Vector2(l, -a), cor);
    Linha(centro + Vector2(l, -a), centro + Vector2(l, a), cor);
    Linha(centro + Vector2(l, a), centro + Vector2(-l, a), cor);
    Linha(centro + Vector2(-l, a), centro + Vector2(-l, -a), cor);
}

void Goulart::Faixa(const float de, const float ate, const float y, const int linhas, const SDL_Color cor) {
    for (int i = 0; i < linhas; ++i) Linha(Vector2(de, y + 1.5f * static_cast<float>(i)), Vector2(ate, y + 1.5f * static_cast<float>(i)), cor);
}

void Goulart::PlacarDaEtapa(const std::string& fase, const size_t etapa, const std::string& texto) {
    auto etapas = EtapasDa(fase);
    if (etapa >= etapas.size() || etapas[etapa].second == texto) return;
    etapas[etapa].second = texto;
    RegistrarEtapas(fase, etapas);
}

void Goulart::PlacarDaFase(const std::string& fase, const std::string& texto) {
    if (AnuncioDaFase(fase) != texto) RegistrarAnuncio(fase, texto);
}

void Goulart::Avisar(const std::string& texto, const float duracao) {
    mAviso = texto;
    mAvisoAte = mRelogio + duracao;
}

std::optional<Vector2> Goulart::AlvoDoMovimento() const {
    return mAlvo;
}

void Goulart::Desmontar() {
    DesmontarEscalonador();
    DesmontarPaginacao();
    DesmontarDisco();
    DesmontarNuvem();
    if (mLinhas) mLinhas->Limpar();
    mAlvo.reset();
    mAvisoAte = -1.0f;
}

void Goulart::AoTerminarABatalha() {
    Desmontar();
}

void Goulart::OnUpdate(const float deltaTime) {
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

    if (fase == Escalonador::kFase) AtualizarEscalonador(t, deltaTime);
    else if (fase == Paginacao::kFase) AtualizarPaginacao(t, deltaTime);
    else if (fase == Disco::kFase) AtualizarDisco(t, deltaTime);
    else if (fase == Virtualizacao::kFase) AtualizarNuvem(t, deltaTime);
}

void Goulart::OnDerrubou(BossProjectile* qual) {
    // kill -9: o cartao derrubado e o processo morto, de onde estiver.
    for (auto it = mCartoes.begin(); it != mCartoes.end(); ++it) {
        if (it->second != qual) continue;
        const int pid = it->first;
        mCartoes.erase(it);
        if (mRR && mRR->Matar(pid)) {
            FecharFatia(pid, GetComponent<FSMComponent>() ? GetComponent<FSMComponent>()->GetStateTime() : 0.0f);
            Avisar("kill -9 " + Escalonador::Cara(pid));
        }
        return;
    }
    // A leitura cancelada: o pedido sai da trilha.
    for (auto it = mPedidos.begin(); it != mPedidos.end(); ++it) {
        if (it->p != qual) continue;
        mPedidos.erase(it);
        Avisar("E/S cancelada", 0.8f);
        return;
    }
    // A VM desligada: a nuvem repoe outra daqui a pouco.
    for (auto it = mVms.begin(); it != mVms.end(); ++it) {
        if (it->p != qual) continue;
        Avisar("VM " + std::to_string(it->id) + " desligada: os tiros dela somem, e a nuvem sobe outra");
        for (Preso& pr : it->presos) Matar(pr.p);
        mVms.erase(it);
        mReporDepois = (GetComponent<FSMComponent>() ? GetComponent<FSMComponent>()->GetStateTime() : 0.0f) +
                       Virtualizacao::kReposicao;
        return;
    }
}

// ---- Fase 1: o escalonador ----

Vector2 Goulart::LugarNaCpu() const {
    return GetPosition() + Vector2(0.0f, 84.0f);
}

Vector2 Goulart::LugarNaFila(const int i) const {
    const SDL_FRect c = Campo();
    return Vector2(c.x + 62.0f, c.y + 182.0f + 46.0f * static_cast<float>(i));
}

Vector2 Goulart::LugarNaES(const int i) const {
    const SDL_FRect c = Campo();
    return Vector2(c.x + c.w - 62.0f, c.y + 182.0f + 46.0f * static_cast<float>(i));
}

Vector2 Goulart::LugarDoTitulo(const int k) const {
    const SDL_FRect c = Campo();
    return Vector2(k == 0 ? c.x + 62.0f : c.x + c.w - 62.0f, c.y + 146.0f);
}

void Goulart::Faixa(const int f, const SDL_Color cor, const bool tracejada) {
    const SDL_FRect c = Campo();
    const float x0 = Escalonador::InicioDaFaixa(f, c.x, c.w), x1 = x0 + Escalonador::LarguraDaFaixa(c.w);
    const float topo = c.y + 112.0f, fundo = c.y + c.h;
    for (const float x : {x0 + 2.0f, x1 - 2.0f}) {
        if (!tracejada) {
            Linha(Vector2(x, topo), Vector2(x, fundo), cor);
            continue;
        }
        for (float y = topo; y < fundo; y += 22.0f) Linha(Vector2(x, y), Vector2(x, std::min(fundo, y + 11.0f)), cor);
    }
}

void Goulart::DesmontarEscalonador() {
    for (auto& [pid, p] : mCartoes) Matar(p);
    mCartoes.clear();
    for (BossProjectile*& p : mTitulos) Matar(p);
    mRR.reset();
    mGantt.clear();
    mUltimoPid = 0;
    mForks = 0;
}

void Goulart::Forkar(const float t) {
    using namespace Escalonador;
    const int pid = ++mUltimoPid;
    const Tipo tipo = TipoDoFork(mForks++);
    mRR->Chegar(Novo(pid, tipo));
    // O filho nasce na CPU, de onde o pai deu o fork, e vai para o fim da fila.
    if (BossProjectile* p = Um(kGatilhoDoProcesso, LugarNaCpu(), Escalonador::Cara(pid))) {
        p->DefinirResistencia(3);
        mCartoes[pid] = p;
    }
    if (t > 0.5f) Avisar("fork(): nasce " + Escalonador::Cara(pid) + ", que chove na faixa " + std::to_string(FaixaDo(pid) + 1));
}

void Goulart::FecharFatia(const int pid, const float t) {
    for (auto it = mGantt.rbegin(); it != mGantt.rend(); ++it) {
        if (it->pid == pid && it->ate < 0.0f) {
            it->ate = t;
            return;
        }
    }
}

void Goulart::AtualizarEscalonador(const float t, const float deltaTime) {
    using namespace Escalonador;
    const SDL_FRect campo = Campo();
    if (!mRR) {
        mRR.emplace(mDegrauDoEscalonador.quantum);
        for (int k = 0; k < 3; ++k) Forkar(t);
        mProximoFork = t + mDegrauDoEscalonador.entreForks;
    }
    for (size_t k = 0; k < mTitulos.size(); ++k) {
        if (Vivo(mTitulos[k])) continue;
        mTitulos[k] = Um(kGatilhoDoTitulo, LugarDoTitulo(static_cast<int>(k)), kTitulos[k]);
        if (mTitulos[k]) mTitulos[k]->DefinirEnfeite(true);
    }
    if (t >= mProximoFork) {
        mProximoFork = t + mDegrauDoEscalonador.entreForks;
        if (mRR->Quantos() < mDegrauDoEscalonador.processos) Forkar(t);
    }

    // O relogio do escalonador, e o que ele fez.
    for (const Aviso& a : mRR->Avancar(t)) {
        switch (a.evento) {
            case Evento::Despachou:
                mGantt.push_back(Fatia{a.pid, t, -1.0f});
                // A chuva e calada ("som": false): o som e o da faixa que comeca a chover.
                SomDoDisparo();
                break;
            case Evento::Preemptou:
                FecharFatia(a.pid, t);
                break;
            case Evento::PediuES:
                FecharFatia(a.pid, t);
                Avisar(Escalonador::Cara(a.pid) + " pediu E/S: a faixa dele para de chover");
                break;
            case Evento::Acordou:
                // \xc3\xa7\xc3\xa3 e "ca" com cedilha e til.
                Avisar("interrup\xc3\xa7\xc3\xa3o: " + Escalonador::Cara(a.pid) + " volta para a fila");
                break;
            case Evento::Terminou:
                FecharFatia(a.pid, t);
                if (mCartoes.count(a.pid)) Matar(mCartoes[a.pid]);
                mCartoes.erase(a.pid);
                Avisar("exit(0): " + Escalonador::Cara(a.pid) + " terminou, e a faixa dele seca");
                break;
        }
    }

    // Os cartoes: cada processo vivo tem o seu (o Ponto Extra os leva, e eles voltam), e cada um
    // anda ate a vaga do estado em que esta.
    std::map<int, Vector2> vaga;
    if (mRR->Rodando()) vaga[*mRR->Rodando()] = LugarNaCpu();
    for (size_t i = 0; i < mRR->Prontos().size(); ++i) vaga[mRR->Prontos()[i]] = LugarNaFila(static_cast<int>(i));
    for (size_t i = 0; i < mRR->Bloqueados().size(); ++i) vaga[mRR->Bloqueados()[i]] = LugarNaES(static_cast<int>(i));
    for (const auto& [pid, lugar] : vaga) {
        BossProjectile*& p = mCartoes[pid];
        if (!Vivo(p)) {
            p = Um(kGatilhoDoProcesso, lugar, Escalonador::Cara(pid));
            if (p) p->DefinirResistencia(3);
        }
        // O que esta na CPU roda embaixo do professor: o tiro passa por ele. O kill -9 e nos que
        // esperam, nas filas.
        if (p) p->DefinirEnfeite(mRR->Rodando() && *mRR->Rodando() == pid);
        Levar(p, lugar, deltaTime);
    }

    // As faixas: a de quem roda chove, a do proximo da fila fica marcada, as outras sao so divisorias.
    for (int f = 0; f <= kFaixas; ++f) {
        const float x = InicioDaFaixa(0, campo.x, campo.w) + LarguraDaFaixa(campo.w) * static_cast<float>(f);
        Linha(Vector2(x, campo.y + 112.0f), Vector2(x, campo.y + campo.h), kDivisoria);
    }
    std::vector<int> chovendo;
    SDL_Color corDaVez = kPerigo;
    if (mRR->Rodando()) {
        if (const Processo* p = mRR->Achar(*mRR->Rodando())) {
            chovendo = FaixasQueRodam(*p);
            corDaVez = kCores[static_cast<size_t>((p->pid - 1) % 9)];
        }
    }
    if (!mRR->Prontos().empty()) {
        if (const Processo* p = mRR->Achar(mRR->Prontos().front())) {
            for (const int f : FaixasQueRodam(*p)) {
                if (std::find(chovendo.begin(), chovendo.end(), f) == chovendo.end()) Faixa(f, kProxima, true);
            }
        }
    }
    for (const int f : chovendo) Faixa(f, corDaVez, false);
    if (!chovendo.empty() && t >= mProximaGota) {
        mProximaGota = t + kEntreGotas * mDegrauDoEscalonador.cadencia;
        for (const int f : chovendo) {
            const float x0 = InicioDaFaixa(f, campo.x, campo.w) + 12.0f;
            const float x1 = x0 + LarguraDaFaixa(campo.w) - 24.0f;
            Um(kGatilhoDaInstrucao, Vector2(mSorteio.Real(x0, x1), campo.y + 114.0f), "", 1.0f,
               Vector2(0.0f, kVelocidadeDaChuva * mDegrauDoEscalonador.chuva));
        }
    }

    // O grafico de Gantt no alto: cada fatia de CPU na cor do processo, e a de agora crescendo.
    const float gx = campo.x + 90.0f, gw = campo.w - 180.0f, gy = campo.y + 100.0f;
    Linha(Vector2(gx, gy + 8.0f), Vector2(gx + gw, gy + 8.0f), kEixo);
    for (const Fatia& f : mGantt) {
        const float de = gx + gw * std::clamp(f.de / Fase::kDuracao, 0.0f, 1.0f);
        const float ate = gx + gw * std::clamp((f.ate < 0.0f ? t : f.ate) / Fase::kDuracao, 0.0f, 1.0f);
        Faixa(de, std::max(de + 1.0f, ate), gy, 4, kCores[static_cast<size_t>((f.pid - 1) % 9)]);
    }
    const float agora = gx + gw * std::clamp(t / Fase::kDuracao, 0.0f, 1.0f);
    Linha(Vector2(agora, gy - 4.0f), Vector2(agora, gy + 12.0f), kPonteiro);

    PlacarDaFase(kFase, mRelogio < mAvisoAte ? mAviso : Placar(*mRR));
    // A CPU anda devagar de um lado ao outro no alto: quem quer acerta-la escolhe a faixa.
    mAlvo = Vector2(campo.x + campo.w * (0.5f + 0.2f * Math::Sin(Math::TwoPi * t / 9.0f)), campo.y + 62.0f);
}

// ---- Fase 2: a paginacao ----

Vector2 Goulart::CentroDoRelogio() const {
    const SDL_FRect c = Campo();
    return Vector2(c.x + c.w * 0.5f, c.y + 250.0f);
}

Vector2 Goulart::LugarDaMoldura(const int m) const {
    const int n = mMemoria ? static_cast<int>(mMemoria->Molduras().size()) : 1;
    const float a = -Math::Pi / 2.0f + Math::TwoPi * static_cast<float>(m) / static_cast<float>(std::max(n, 1));
    return CentroDoRelogio() + Vector2(Math::Cos(a), Math::Sin(a)) * 96.0f;
}

Vector2 Goulart::LugarDaEntrada(const int p) const {
    // A entrada da tabela fica no chao, no pe da coluna da pagina.
    const SDL_FRect c = Campo();
    return Vector2(Paginacao::InicioDaPagina(p, c.x, c.w) + Paginacao::LarguraDaPagina(c.w) / 2.0f, c.y + c.h - 18.0f);
}

void Goulart::Despejar(const int p, const float agora, const float inicio) {
    mCortinas.push_back(Cortina{p, agora, inicio, inicio + Paginacao::kDespejo, inicio});
}

void Goulart::DesmontarPaginacao() {
    for (BossProjectile*& p : mMolduras) Matar(p);
    mMolduras.clear();
    for (BossProjectile*& p : mEntradas) Matar(p);
    mMemoria.reset();
    mEtapaDaPaginacao = -1;
    mPaginaDoAluno = -1;
    mPaginaDoProfessor = -1;
    mUltimasFaltas.clear();
    mVarredura.reset();
    mCortinas.clear();
    mUltimaReferencia.clear();
}

void Goulart::TrazerPagina(const int p, const int m) {
    const Vector2 de = GetPosition() + Vector2(0.0f, 50.0f);
    const float voo = 0.35f;
    if (BossProjectile* chega = Um(Paginacao::kGatilhoDaPagina, de, "Pg" + std::to_string(p), 1.0f,
                                   (LugarDaMoldura(m) - de) * (1.0f / voo))) {
        chega->DefinirEnfeite(true);
    }
}

void Goulart::AtualizarPaginacao(const float t, const float deltaTime) {
    using namespace Paginacao;
    (void)deltaTime;
    const SDL_FRect campo = Campo();
    const Dificuldade::Nivel nivel = NivelDeDificuldade();
    const int etapa = EtapaNoTempo(t);
    if (!mMemoria) {
        mMemoria.emplace(MoldurasNaEtapa(0, nivel));
        mProximoAcesso = t + 1.0f;
        mProximaDoProfessor = t + 2.0f;
    }
    if (etapa != mEtapaDaPaginacao) {
        // A memoria encolhe: as molduras de sobra saem, e as paginas delas desabam.
        if (mEtapaDaPaginacao >= 0) {
            for (const int p : mMemoria->Encolher(MoldurasNaEtapa(etapa, nivel))) Despejar(p, t, t + 0.5f);
            // \xc3\xb3 e o o com acento.
            Avisar("Outro processo pegou mem\xc3\xb3ria: " + std::to_string(mMemoria->Molduras().size()) + " molduras", 1.6f);
        }
        mEtapaDaPaginacao = etapa;
    }
    const auto& molduras = mMemoria->Molduras();
    const int n = static_cast<int>(molduras.size());

    // O aluno e a referencia: a coluna em que ele esta e a pagina lida - ao entrar nela, e de tempos
    // em tempos enquanto fica.
    const Vector2 aluno = PosicaoDoAluno();
    const int pagina = PaginaDoX(aluno.x, campo.x, campo.w);
    if (pagina != mPaginaDoAluno || t >= mProximaLeitura) {
        mPaginaDoAluno = pagina;
        mProximaLeitura = t + kEntreLeituras;
        const std::uint32_t endereco = Montar(pagina, mSorteio.Inteiro(0, (1 << kBitsDoDeslocamento) - 1));
        const Referencia r = mMemoria->Referenciar(pagina);
        mUltimasFaltas.push_back(!r.acerto);
        while (static_cast<int>(mUltimasFaltas.size()) > kJanelaDoThrashing) mUltimasFaltas.pop_front();
        if (r.acerto) {
            mUltimaReferencia = PlacarDoAcerto(endereco, r.moldura);
        } else {
            mUltimaReferencia = PlacarDaFalta(endereco, r.vitima, r.moldura);
            mFaltouEm = t;
            mVarredura = Varredura{r.segundaChance, r.moldura, t};
            // A vitima desaba quando o ponteiro chega nela: ate la, a coluna dela fica marcada.
            if (r.vitima >= 0) {
                Despejar(r.vitima, t, t + kPassoDoPonteiro * static_cast<float>(r.segundaChance.size() + 1) + 0.25f);
            }
            TrazerPagina(pagina, r.moldura);
        }
    }

    // O professor tambem le as paginas dele, pelo mesmo relogio: a falta dele despeja uma vitima onde
    // ela estiver. A do aluno tem o R aceso e quase sempre escapa - se nao escapar, desaba mais devagar.
    if (t >= mProximaDoProfessor) {
        mProximaDoProfessor = t + EntreLeiturasDoProfessor(mDegrauDaPaginacao, etapa);
        mPaginaDoProfessor = PaginaDoProfessor(mSorteio.Inteiro(0, kPaginas - 2), pagina);
        mProfessorLeuEm = t;
        const Referencia r = mMemoria->Referenciar(mPaginaDoProfessor);
        if (!r.acerto) {
            mVarredura = Varredura{r.segundaChance, r.moldura, t};
            if (r.vitima >= 0) {
                float inicio = t + kPassoDoPonteiro * static_cast<float>(r.segundaChance.size() + 1) + 0.25f;
                if (r.vitima == pagina) inicio = std::max(inicio, t + 0.8f);
                Despejar(r.vitima, t, inicio);
            }
            TrazerPagina(mPaginaDoProfessor, r.moldura);
        }
    }
    if (mPaginaDoProfessor >= 0 && t - mProfessorLeuEm < 0.45f) {
        Linha(GetPosition() + Vector2(0.0f, 40.0f), LugarDaEntrada(mPaginaDoProfessor) + Vector2(0.0f, -14.0f), kProxima);
    }

    // As cortinas: a coluna marcada, e depois os pedacos da pagina caindo de cima a baixo.
    for (Cortina& c : mCortinas) {
        const float x0 = InicioDaPagina(c.pagina, campo.x, campo.w), largura = LarguraDaPagina(campo.w);
        if (t < c.inicio) {
            const bool aceso = static_cast<int>((t - c.marcada) / 0.08f) % 2 == 0;
            if (aceso) {
                for (const float x : {x0 + 2.0f, x0 + largura - 2.0f}) {
                    Linha(Vector2(x, campo.y + 110.0f), Vector2(x, campo.y + campo.h), kPerigo);
                }
            }
            continue;
        }
        // Os pedacos sao calados: o som e o da coluna que comeca a cair.
        if (c.proximo == c.inicio) SomDoDisparo();
        while (t >= c.proximo && c.proximo < c.fim) {
            c.proximo += kEntrePedacos;
            Um(kGatilhoDaVitima, Vector2(mSorteio.Real(x0 + 10.0f, x0 + largura - 10.0f), campo.y + 112.0f),
               "Pg" + std::to_string(c.pagina), 1.0f, Vector2(0.0f, kVelocidadeDoDespejo));
        }
    }
    mCortinas.erase(std::remove_if(mCortinas.begin(), mCortinas.end(), [t](const Cortina& c) { return t >= c.fim; }),
                    mCortinas.end());

    // O professor tambem le: um leque mirado de tanto em tanto, que tira o aluno do lugar.
    if (t >= mProximoAcesso) {
        mProximoAcesso = t + mDegrauDaPaginacao.entreAcessos;
        const Vector2 de = GetPosition() + Vector2(0.0f, 50.0f);
        DispararEm(kGatilhoDoAcesso, de, 0, AnguloAteOAluno(de));
    }

    // As colunas do chao, a do aluno acesa (verde no acerto, vermelha logo depois de uma falta).
    for (int p = 0; p <= kPaginas; ++p) {
        const float x = InicioDaPagina(0, campo.x, campo.w) + LarguraDaPagina(campo.w) * static_cast<float>(p);
        Linha(Vector2(x, campo.y + 110.0f), Vector2(x, campo.y + campo.h), kDivisoria);
    }
    {
        const float x0 = InicioDaPagina(pagina, campo.x, campo.w), largura = LarguraDaPagina(campo.w);
        const SDL_Color cor = t - mFaltouEm < 0.35f ? kPerigo : kVoce;
        Linha(Vector2(x0 + 3.0f, campo.y + campo.h - 40.0f), Vector2(x0 + largura - 3.0f, campo.y + campo.h - 40.0f), cor);
    }

    // A memoria fisica (o relogio) e a tabela de paginas no chao, como o relogio as deixou.
    for (size_t m = static_cast<size_t>(n); m < mMolduras.size(); ++m) Matar(mMolduras[m]);
    mMolduras.resize(static_cast<size_t>(n), nullptr);
    for (int m = 0; m < n; ++m) {
        const Moldura& mo = molduras[static_cast<size_t>(m)];
        const std::string cara = mo.pagina < 0 ? std::string("Livre")
                                               : (mo.referenciada ? "R" : "P") + std::to_string(mo.pagina);
        BossProjectile*& p = mMolduras[static_cast<size_t>(m)];
        if (!Vivo(p)) {
            p = Um(kGatilhoDaMoldura, LugarDaMoldura(m), cara);
            if (p) p->DefinirEnfeite(true);
        }
        if (p) p->SetPosition(LugarDaMoldura(m));
        Cara(p, cara);
    }
    for (int pg = 0; pg < kPaginas; ++pg) {
        const int m = mMemoria->MolduraDa(pg);
        const std::string cara = (m >= 0 ? "V" : "I") + std::to_string(pg);
        BossProjectile*& p = mEntradas[static_cast<size_t>(pg)];
        if (!Vivo(p)) {
            p = Um(kGatilhoDaEntrada, LugarDaEntrada(pg), cara);
            if (p) p->DefinirEnfeite(true);
        }
        Cara(p, cara);
        // A entrada valida aponta a moldura: o mapeamento da MMU.
        if (m >= 0) Linha(LugarDaEntrada(pg) + Vector2(0.0f, -14.0f), LugarDaMoldura(m), kMapeamento);
    }

    // O ponteiro do relogio: parado na moldura da vez, ou varrendo - a segunda chance, passo a passo.
    int apontada = mMemoria->Ponteiro();
    if (mVarredura) {
        const int passo = static_cast<int>((t - mVarredura->desde) / kPassoDoPonteiro);
        if (passo < static_cast<int>(mVarredura->passos.size())) apontada = mVarredura->passos[static_cast<size_t>(passo)];
        else if (passo <= static_cast<int>(mVarredura->passos.size()) + 2) apontada = mVarredura->moldura;
        else mVarredura.reset();
    }
    if (apontada >= 0 && apontada < n) Linha(CentroDoRelogio(), LugarDaMoldura(apontada), kPonteiro);

    std::string placar = mUltimaReferencia;
    if (etapa == 2 && !mUltimasFaltas.empty()) {
        const int faltas = static_cast<int>(std::count(mUltimasFaltas.begin(), mUltimasFaltas.end(), true));
        placar = PlacarDoThrashing(faltas, static_cast<int>(mUltimasFaltas.size()));
    }
    if (mRelogio < mAvisoAte) placar = mAviso;
    if (!placar.empty()) PlacarDaEtapa(kFase, static_cast<size_t>(etapa), placar);
    mAlvo = Vector2(campo.x + campo.w * (0.5f + 0.24f * Math::Sin(Math::TwoPi * t / 8.0f)), campo.y + 62.0f);
}

// ---- Fase 3: o braco do disco ----

float Goulart::XDaTrilha() const {
    return Campo().x + Disco::kMargemDaTrilha;
}

float Goulart::LarguraDaTrilha() const {
    return Campo().w - 2.0f * Disco::kMargemDaTrilha;
}

float Goulart::YDaTrilha() const {
    return Campo().y + 146.0f;
}

void Goulart::DesmontarDisco() {
    for (Pedido& p : mPedidos) Matar(p.p);
    mPedidos.clear();
    mPedidoDoBraco = nullptr;
    mAlvoDoBraco = -1;
    mEtapaDoBraco = -1;
    mEtapaDoDisco = -1;
    mUltimoCilindro = -1;
    mAtendidos.clear();
    mLeituras.clear();
    mSentido = -1;
}

void Goulart::AtualizarDisco(const float t, const float deltaTime) {
    using namespace Disco;
    (void)deltaTime;
    const SDL_FRect campo = Campo();
    const int etapa = EtapaNoTempo(t);
    const Algoritmo algoritmo = kOrdem[static_cast<size_t>(etapa)];
    const int cabeca = CilindroDoX(GetPosition().x, XDaTrilha(), LarguraDaTrilha());
    if (etapa != mEtapaDoDisco) {
        mEtapaDoDisco = etapa;
        mAndou = 0;
        mCabecaNoInicio = cabeca;
        mAtendidos.clear();
        mUltimoCilindro = cabeca;
    }

    // Os pedidos caem na trilha, cada um num cilindro.
    mPedidos.erase(std::remove_if(mPedidos.begin(), mPedidos.end(), [](const Pedido& p) { return !Vivo(p.p); }), mPedidos.end());
    if (t >= mProximoPedido && static_cast<int>(mPedidos.size()) < mDegrauDoDisco.maisPedidos) {
        mProximoPedido = t + mDegrauDoDisco.entrePedidos;
        const int c = mSorteio.Inteiro(0, kCilindros - 1);
        if (BossProjectile* p = Um(kGatilhoDoPedido, Vector2(XDoCilindro(c, XDaTrilha(), LarguraDaTrilha()), YDaTrilha()), "Novo")) {
            p->DefinirResistencia(2);
            mPedidos.push_back(Pedido{p, c, t});
        }
    }

    // O braco vai ao alvo do algoritmo e fica com ele ate atende-lo - sem isso, dois pedidos quase a
    // mesma distancia o fariam tremer no meio.
    bool aindaVale = false;
    if (mPedidoDoBraco) {
        for (const Pedido& p : mPedidos) if (p.p == mPedidoDoBraco) aindaVale = true;
    } else if (mAlvoDoBraco >= 0) {
        // A ponta do elevador: vale ate chegar nela (tao perto quanto o professor chega), e la o
        // sentido vira.
        aindaVale = std::abs(cabeca - mAlvoDoBraco) > kPertoDaPonta;
        if (!aindaVale && algoritmo == Algoritmo::SCAN) mSentido = -mSentido;
    }
    if (!aindaVale || mEtapaDoBraco != etapa) {
        std::vector<int> cilindros;
        for (const Pedido& p : mPedidos) cilindros.push_back(p.cilindro);
        mAlvoDoBraco = Alvo(cabeca, cilindros, algoritmo, mSentido);
        mPedidoDoBraco = nullptr;
        for (const Pedido& p : mPedidos) if (p.cilindro == mAlvoDoBraco) { mPedidoDoBraco = p.p; break; }
        mEtapaDoBraco = etapa;
    }
    const int alvo = mAlvoDoBraco;

    // O que estiver embaixo do braco e lido (no FCFS, so o da vez), e o bloco cai na coluna dele.
    for (auto it = mPedidos.begin(); it != mPedidos.end();) {
        const float x = XDoCilindro(it->cilindro, XDaTrilha(), LarguraDaTrilha());
        const bool embaixo = std::fabs(x - GetPosition().x) < kAlcance;
        if (!embaixo || (algoritmo == Algoritmo::FCFS && it->cilindro != alvo)) {
            ++it;
            continue;
        }
        const bool faminto = t - it->chegou >= kFome;
        mLeituras.push_back(Leitura{x, faminto ? 30.0f : 18.0f, t + (faminto ? 1.0f : 0.55f), t});
        SomDoDisparo();   // os blocos sao calados: o som e o da coluna lida
        mAndou += std::abs(it->cilindro - mUltimoCilindro);
        mUltimoCilindro = it->cilindro;
        mAtendidos.emplace_back(it->chegou, it->cilindro);
        if (faminto) Avisar("o pedido faminto enfim foi lido: o bloco dele vem largo", 1.0f);
        Matar(it->p);
        it = mPedidos.erase(it);
    }
    for (Leitura& l : mLeituras) {
        while (t >= l.proximo && l.proximo < l.fim) {
            l.proximo += 0.045f;
            Um(kGatilhoDaLeitura, Vector2(mSorteio.Real(l.x - l.meia, l.x + l.meia), YDaTrilha() + 14.0f), "", 1.0f,
               Vector2(0.0f, mDegrauDoDisco.queda));
        }
    }
    mLeituras.erase(std::remove_if(mLeituras.begin(), mLeituras.end(), [t](const Leitura& l) { return t >= l.fim; }),
                    mLeituras.end());

    // Quem espera demais passa fome: fica vermelho - e o bloco dele vira o mais largo.
    for (Pedido& p : mPedidos) {
        const float espera = t - p.chegou;
        Cara(p.p, espera >= kFome ? "Faminto" : espera >= kFome / 2.0f ? "Velho" : "Novo");
    }

    // A trilha, as marcas dos cilindros, o braco descendo do professor ate ela - e o pedido que ele
    // vai ler: a linha ate ele, e a coluna dele tracejada ate o chao.
    Linha(Vector2(XDaTrilha(), YDaTrilha()), Vector2(XDaTrilha() + LarguraDaTrilha(), YDaTrilha()), kTrilha);
    for (int c = 0; c < kCilindros; c += 25) {
        const float x = XDoCilindro(c, XDaTrilha(), LarguraDaTrilha());
        Linha(Vector2(x, YDaTrilha() - 5.0f), Vector2(x, YDaTrilha() + 5.0f), kTrilha);
    }
    Linha(GetPosition() + Vector2(0.0f, 40.0f), Vector2(GetPosition().x, YDaTrilha()), kBraco);
    if (mPedidoDoBraco && Vivo(mPedidoDoBraco)) {
        const Vector2 lugar = mPedidoDoBraco->GetPosition();
        Linha(Vector2(GetPosition().x, YDaTrilha()), lugar, kProxima);
        for (float y = YDaTrilha() + 16.0f; y < campo.y + campo.h; y += 22.0f) {
            Linha(Vector2(lugar.x, y), Vector2(lugar.x, std::min(campo.y + campo.h, y + 11.0f)), kProxima);
        }
    }

    // O placar: o quanto o braco andou, e o quanto o FCFS andaria com os mesmos pedidos.
    std::vector<std::pair<float, int>> ordem = mAtendidos;
    std::sort(ordem.begin(), ordem.end());
    std::vector<int> naChegada;
    for (const auto& [quando, c] : ordem) naChegada.push_back(c);
    const int fcfs = Movimento(mCabecaNoInicio, naChegada, Algoritmo::FCFS);
    PlacarDaEtapa(kFase, static_cast<size_t>(etapa),
                  mRelogio < mAvisoAte ? mAviso : Placar(algoritmo, mAndou, static_cast<int>(mAtendidos.size()), fcfs));

    if (alvo >= 0) mAlvo = Vector2(XDoCilindro(alvo, XDaTrilha(), LarguraDaTrilha()), campo.y + 62.0f);
    else if (!mAlvo) mAlvo = Vector2(GetPosition().x, campo.y + 62.0f);
}

// ---- A repescagem: a nuvem ----

Vector2 Goulart::LugarDoHost(const int k) const {
    const SDL_FRect c = Campo();
    return Virtualizacao::Host(k, c.x, c.y, c.w, c.h);
}

void Goulart::DesmontarNuvem() {
    for (Vm& v : mVms) {
        Matar(v.p);
        for (Preso& pr : v.presos) Matar(pr.p);
    }
    mVms.clear();
    for (Trap& tr : mTraps) Matar(tr.p);
    mTraps.clear();
    mProximoIdDeVm = 1;
    mVmsAntes = 0;
    mReporDepois = 0.0f;
    mProximaMigracao = Virtualizacao::kDuracoes[0];
}

void Goulart::AtualizarNuvem(const float t, const float deltaTime) {
    using namespace Virtualizacao;
    const int etapa = EtapaNoTempo(t);
    const float cadencia = mDegrauDaNuvem.cadencia;

    // As VMs que sumiram (o Ponto Extra as leva) saem da conta com os tiros delas, e a nuvem repoe.
    const size_t antes = mVms.size();
    for (Vm& v : mVms) {
        if (Vivo(v.p)) continue;
        for (Preso& pr : v.presos) Matar(pr.p);
    }
    mVms.erase(std::remove_if(mVms.begin(), mVms.end(), [](const Vm& v) { return !Vivo(v.p); }), mVms.end());
    if (mVms.size() < antes) mReporDepois = std::max(mReporDepois, t + kReposicao);

    const int queridas = VmsNoTempo(t, mDegrauDaNuvem.maisVms);
    if (static_cast<int>(mVms.size()) < queridas && t >= mReporDepois) {
        std::vector<bool> ocupado(kHosts, false);
        for (const Vm& v : mVms) {
            ocupado[static_cast<size_t>(v.host)] = true;
            if (v.destino >= 0) ocupado[static_cast<size_t>(v.destino)] = true;
        }
        const int host = HostLivre(ocupado, static_cast<int>(mVms.size()) * 2 - 1);
        if (host >= 0) {
            const int id = mProximoIdDeVm++;
            if (BossProjectile* p = Um(kGatilhoDaVm, LugarDoHost(host), "Vm" + std::to_string((id - 1) % 4 + 1))) {
                p->DefinirResistencia(4);
                const float defasagem = 0.37f * static_cast<float>(id % 4);
                mVms.push_back(Vm{p, id, host, -1, 0.0f, t + 0.3f, t + 1.0f + defasagem, LugarDoHost(host), {}});
                if (etapa == 2 && queridas > mVmsAntes) Avisar(PlacarDaEscala(static_cast<int>(mVms.size()) - 1, static_cast<int>(mVms.size())));
            }
        }
    }
    mVmsAntes = std::max(mVmsAntes, queridas);

    // A migracao ao vivo: uma VM por vez escorrega para um host livre, com tudo o que tem dentro.
    if (etapa >= 1 && t >= mProximaMigracao && !mVms.empty()) {
        mProximaMigracao = t + kEntreMigracoes;
        std::vector<bool> ocupado(kHosts, false);
        for (const Vm& v : mVms) {
            ocupado[static_cast<size_t>(v.host)] = true;
            if (v.destino >= 0) ocupado[static_cast<size_t>(v.destino)] = true;
        }
        Vm& v = mVms[static_cast<size_t>(mMigrarAVez++ % static_cast<int>(mVms.size()))];
        const int destino = v.destino < 0 ? HostLivre(ocupado, v.host) : -1;
        if (destino >= 0) {
            v.destino = destino;
            v.migraDesde = t;
            v.p->DefinirBlindado(true);
            Avisar(PlacarDaMigracao(v.id, v.host, destino));
        }
    }

    const Vector2 hipervisor = GetPosition();
    for (Vm& v : mVms) {
        Vector2 onde = LugarDoHost(v.host);
        if (v.destino >= 0) {
            const float u = (t - v.migraDesde) / kMigracao;
            onde = NaMigracao(LugarDoHost(v.host), LugarDoHost(v.destino), u);
            Linha(LugarDoHost(v.host), LugarDoHost(v.destino), kRastro);
            if (u >= 1.0f) {
                v.host = v.destino;
                v.destino = -1;
                v.p->DefinirBlindado(false);
                onde = LugarDoHost(v.host);
            }
        }
        const Vector2 andou = onde - v.onde;
        v.onde = onde;
        v.p->SetPosition(onde);
        if (auto* rb = v.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        const SDL_Color cor = kCores[static_cast<size_t>((v.id - 1) % 4)];
        Retangulo(onde, kLarguraDaVm, kAlturaDaVm, cor);
        Retangulo(onde, kLarguraDaVm - 6.0f, kAlturaDaVm - 6.0f, cor);

        // O kernel do convidado: os tiros dele nascem dentro da caixa e quicam nas paredes - a VM isola.
        v.presos.erase(std::remove_if(v.presos.begin(), v.presos.end(), [](const Preso& pr) { return !Vivo(pr.p); }),
                       v.presos.end());
        if (t >= v.proximoKernel && static_cast<int>(v.presos.size()) < kTirosPorVm) {
            v.proximoKernel = t + kEntreKernels * cadencia;
            const float a = mSorteio.Real(0.0f, Math::TwoPi);
            const Vector2 vel(Math::Cos(a) * kVelocidadeDoKernel, Math::Sin(a) * kVelocidadeDoKernel);
            if (BossProjectile* p = Um(kGatilhoDoKernel, onde, "", 1.0f, vel)) v.presos.push_back(Preso{p, vel});
        }
        for (Preso& pr : v.presos) {
            Vector2 aqui = pr.p->GetPosition() + andou;   // a caixa migrou: o que esta dentro vai junto
            Quicar(aqui, pr.velocidade, onde, kLarguraDaVm, kAlturaDaVm, 8.0f);
            pr.p->SetPosition(aqui);
            if (auto* rb = pr.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(pr.velocidade);
        }

        // A instrucao privilegiada: o trap, o unico que sai da caixa, voa ate o hipervisor.
        if (t >= v.proximoTrap && v.destino < 0) {
            v.proximoTrap = t + kEntreTraps * cadencia;
            if (BossProjectile* trap = Um(kGatilhoDoTrap, onde, "", 1.0f, (hipervisor - onde) * (1.0f / kVooDoTrap))) {
                trap->DefinirEnfeite(true);
                mTraps.push_back(Trap{trap, t + kVooDoTrap});
                Avisar(PlacarDoTrap(v.id), 0.9f);
            }
        }
    }
    (void)deltaTime;

    // O trap que chegou: o hipervisor emula a instrucao - e atira no aluno.
    for (Trap& tr : mTraps) {
        if (Vivo(tr.p)) Linha(tr.p->GetPosition(), hipervisor, kRastro);
        if (t < tr.chega) continue;
        Matar(tr.p);
        DispararEm(kGatilhoDaEmulacao, hipervisor + Vector2(0.0f, 40.0f), 0, AnguloAteOAluno(hipervisor));
        tr.chega = 1.0e9f;
    }
    mTraps.erase(std::remove_if(mTraps.begin(), mTraps.end(), [](const Trap& tr) { return !tr.p; }), mTraps.end());

    PlacarDaEtapa(kFase, static_cast<size_t>(etapa), mRelogio < mAvisoAte ? mAviso : Placar(static_cast<int>(mVms.size())));
    mAlvo.reset();
}
