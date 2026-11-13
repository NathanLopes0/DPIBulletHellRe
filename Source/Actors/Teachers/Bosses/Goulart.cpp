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
        Avisar("VM " + std::to_string(it->id) + " desligada: a nuvem sobe outra");
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

void Goulart::DesmontarEscalonador() {
    for (auto& [pid, p] : mCartoes) Matar(p);
    mCartoes.clear();
    for (BossProjectile*& p : mTitulos) Matar(p);
    mRR.reset();
    mGantt.clear();
    mUltimoPid = 0;
    mForks = 0;
    mInstrucoes = 0;
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
    if (t > 0.5f) Avisar("fork(): nasce " + Escalonador::Cara(pid));
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
        BossProjectile* cartao = mCartoes.count(a.pid) ? mCartoes[a.pid] : nullptr;
        switch (a.evento) {
            case Evento::Despachou:
                mProximaInstrucao = t + 0.15f;
                mGantt.push_back(Fatia{a.pid, t, -1.0f});
                break;
            case Evento::Preemptou:
                DispararEm(kGatilhoDaTroca, LugarNaCpu());
                FecharFatia(a.pid, t);
                break;
            case Evento::PediuES:
                DispararEm(kGatilhoDaTroca, LugarNaCpu());
                FecharFatia(a.pid, t);
                Avisar(Escalonador::Cara(a.pid) + " pediu E/S: vai esperar o disco");
                break;
            case Evento::Acordou:
                if (Vivo(cartao)) DispararEm(kGatilhoDaInterrupcao, cartao->GetPosition());
                // \xc3\xa7\xc3\xa3 e "ca" com cedilha e til.
                Avisar("interrup\xc3\xa7\xc3\xa3o: " + Escalonador::Cara(a.pid) + " volta para a fila");
                break;
            case Evento::Terminou:
                DispararEm(kGatilhoDoTermino, LugarNaCpu());
                FecharFatia(a.pid, t);
                if (cartao) Matar(mCartoes[a.pid]);
                mCartoes.erase(a.pid);
                Avisar("exit(0): " + Escalonador::Cara(a.pid) + " terminou");
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
        // O que esta na CPU roda embaixo do professor: o tiro passa por ele (quem mira no professor
        // nao o mata sem querer). O kill -9 e nos que esperam, nas filas.
        if (p) p->DefinirEnfeite(mRR->Rodando() && *mRR->Rodando() == pid);
        Levar(p, lugar, deltaTime);
    }

    // O que roda atira, cada tipo do seu jeito.
    if (mRR->Rodando() && t >= mProximaInstrucao) {
        const Processo* p = mRR->Achar(*mRR->Rodando());
        const Vector2 cpu = LugarNaCpu();
        if (p) {
            mProximaInstrucao = t + EntreInstrucoes(p->tipo) * mDegrauDoEscalonador.cadencia;
            ++mInstrucoes;
            if (p->tipo == Tipo::Cpu) {
                // O limitado por CPU: um leque de quatro, com a fresta no meio.
                DispararEm(kGatilhoDaInstrucao, cpu, 4, AnguloAteOAluno(cpu), 42.0f);
            } else if (p->tipo == Tipo::EntradaSaida) {
                // O limitado por E/S: uma rajada de pares, curta - logo ele vai esperar o disco.
                DispararEm(kGatilhoDaInstrucao, cpu, 2, AnguloAteOAluno(cpu), 12.0f);
            } else {
                // As tres threads: tres regadores ao mesmo tempo, cada um no seu compasso.
                for (int k = 0; k < 3; ++k) {
                    const Vector2 de = cpu + Vector2(-64.0f + 64.0f * static_cast<float>(k), 6.0f);
                    const float fase = static_cast<float>(mInstrucoes) * 0.55f + static_cast<float>(k) * 2.1f;
                    DispararEm(kGatilhoDaInstrucao, de, 1, 90.0f + 32.0f * std::sin(fase), 0.0f);
                }
            }
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
    Linha(Vector2(gx + gw * std::clamp(t / Fase::kDuracao, 0.0f, 1.0f), gy - 4.0f),
          Vector2(gx + gw * std::clamp(t / Fase::kDuracao, 0.0f, 1.0f), gy + 12.0f), kPonteiro);

    PlacarDaFase(kFase, mRelogio < mAvisoAte ? mAviso : Placar(*mRR));
    // A CPU anda de um lado ao outro no alto.
    mAlvo = Vector2(campo.x + campo.w * (0.5f + 0.18f * Math::Sin(Math::TwoPi * t / 7.0f)), campo.y + 62.0f);
}

// ---- Fase 2: a paginacao ----

Vector2 Goulart::CentroDoRelogio() const {
    const SDL_FRect c = Campo();
    return Vector2(c.x + c.w * 0.5f, c.y + 300.0f);
}

Vector2 Goulart::LugarDaMoldura(const int m) const {
    const int n = mMemoria ? static_cast<int>(mMemoria->Molduras().size()) : 1;
    const float a = -Math::Pi / 2.0f + Math::TwoPi * static_cast<float>(m) / static_cast<float>(std::max(n, 1));
    return CentroDoRelogio() + Vector2(Math::Cos(a), Math::Sin(a)) * 122.0f;
}

Vector2 Goulart::LugarDaEntrada(const int p) const {
    const SDL_FRect c = Campo();
    return Vector2(c.x + 66.0f, c.y + 150.0f + 40.0f * static_cast<float>(p));
}

void Goulart::DesmontarPaginacao() {
    for (BossProjectile*& p : mMolduras) Matar(p);
    mMolduras.clear();
    for (BossProjectile*& p : mEntradas) Matar(p);
    for (Chegando& c : mChegando) Matar(c.p);
    mChegando.clear();
    mMemoria.reset();
    mEtapaDaPaginacao = -1;
    mUltimasFaltas.clear();
    mVarredura.reset();
    mUltimaReferencia.clear();
}

void Goulart::AtualizarPaginacao(const float t, const float deltaTime) {
    using namespace Paginacao;
    (void)deltaTime;
    const SDL_FRect campo = Campo();
    if (!mMemoria) {
        mMemoria.emplace(mDegrauDaPaginacao.molduras);
        mBaseDoConjunto = mSorteio.Inteiro(0, kPaginas - 1);
        mProximaReferencia = t + 0.8f;
    }
    const int etapa = EtapaNoTempo(t);
    if (etapa != mEtapaDaPaginacao) {
        mEtapaDaPaginacao = etapa;
        mConjunto = ConjuntoDeTrabalho(etapa, static_cast<int>(mMemoria->Molduras().size()), mBaseDoConjunto);
        mCursor = -1;
    }
    const auto& molduras = mMemoria->Molduras();
    const int n = static_cast<int>(molduras.size());

    // A memoria fisica e a tabela de paginas, como o relogio as deixou.
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
        if (m >= 0) Linha(LugarDaEntrada(pg) + Vector2(26.0f, 0.0f), LugarDaMoldura(m), kMapeamento);
    }

    // A referencia da vez: acerto, ou falta de pagina.
    if (t >= mProximaReferencia) {
        mProximaReferencia = t + EntreReferencias(etapa, NivelDeDificuldade());
        const int pagina = ProximaReferencia(mSorteio, mConjunto, mDegrauDaPaginacao.fora, mCursor, etapa == 2);
        const std::uint32_t endereco = Montar(pagina, mSorteio.Inteiro(0, (1 << kBitsDoDeslocamento) - 1));
        const Referencia r = mMemoria->Referenciar(pagina);
        mUltimasFaltas.push_back(!r.acerto);
        while (static_cast<int>(mUltimasFaltas.size()) > kJanelaDoThrashing) mUltimasFaltas.pop_front();
        const Vector2 lugar = LugarDaMoldura(r.moldura);
        if (r.acerto) {
            DispararEm(kGatilhoDoAcesso, lugar, 0, AnguloAteOAluno(lugar));
            mUltimaReferencia = PlacarDoAcerto(endereco, r.moldura);
        } else {
            // A falta e um trap para o sistema: o professor busca a pagina no disco.
            DispararEm(kGatilhoDaFalta, GetPosition() + Vector2(0.0f, 50.0f));
            mVarredura = Varredura{r.segundaChance, r.moldura, r.vitima, t, false};
            mUltimaReferencia = PlacarDaFalta(endereco, r.vitima, r.moldura);
            const Vector2 de = GetPosition() + Vector2(0.0f, 50.0f);
            const float voo = 0.35f;
            if (BossProjectile* chega = Um(kGatilhoDaPagina, de, "Pg" + std::to_string(pagina), 1.0f, (lugar - de) * (1.0f / voo))) {
                chega->DefinirEnfeite(true);
                mChegando.push_back(Chegando{chega, t + voo});
            }
        }
    }
    for (Chegando& c : mChegando) if (t >= c.ate) Matar(c.p);
    mChegando.erase(std::remove_if(mChegando.begin(), mChegando.end(), [](const Chegando& c) { return !c.p; }), mChegando.end());

    // O ponteiro do relogio: parado na moldura da vez, ou varrendo - a segunda chance, passo a passo.
    int apontada = mMemoria->Ponteiro();
    if (mVarredura) {
        const int passo = static_cast<int>((t - mVarredura->desde) / kPassoDoPonteiro);
        if (passo < static_cast<int>(mVarredura->passos.size())) {
            apontada = mVarredura->passos[static_cast<size_t>(passo)];
        } else {
            apontada = mVarredura->moldura;
            if (!mVarredura->despejou) {
                mVarredura->despejou = true;
                // A pagina vitima e despejada no aluno.
                if (mVarredura->vitima >= 0) {
                    const Vector2 de = LugarDaMoldura(mVarredura->moldura);
                    const float a = Math::ToRadians(AnguloAteOAluno(de));
                    Um(kGatilhoDaVitima, de, "Pg" + std::to_string(mVarredura->vitima), 1.0f,
                       Vector2(Math::Cos(a), Math::Sin(a)) * 190.0f);
                }
            }
            if (passo > static_cast<int>(mVarredura->passos.size()) + 3) mVarredura.reset();
        }
    }
    Linha(CentroDoRelogio(), LugarDaMoldura(apontada), kPonteiro);
    for (int m = 0; m < n; ++m) Linha(CentroDoRelogio(), CentroDoRelogio() + (LugarDaMoldura(m) - CentroDoRelogio()) * 0.18f, kEixo);

    std::string placar = mUltimaReferencia;
    if (etapa == 2 && !mUltimasFaltas.empty()) {
        const int faltas = static_cast<int>(std::count(mUltimasFaltas.begin(), mUltimasFaltas.end(), true));
        placar = PlacarDoThrashing(faltas, static_cast<int>(mUltimasFaltas.size()));
    }
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
            mPedidos.push_back(Pedido{p, c, t, t + kFome});
        }
    }

    // O braco vai ao alvo do algoritmo e fica com ele ate atende-lo - sem isso, dois pedidos quase a
    // mesma distancia o fariam tremer no meio. E le o que estiver embaixo dele (no FCFS, so o da vez).
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
    for (auto it = mPedidos.begin(); it != mPedidos.end();) {
        const float x = XDoCilindro(it->cilindro, XDaTrilha(), LarguraDaTrilha());
        const bool embaixo = std::fabs(x - GetPosition().x) < kAlcance;
        if (!embaixo || (algoritmo == Algoritmo::FCFS && it->cilindro != alvo)) {
            ++it;
            continue;
        }
        DispararEm(kGatilhoDaLeitura, it->p->GetPosition(), 0, 90.0f);
        // O quanto o braco andou e o caminho entre os pedidos que ele atendeu, na ordem em que os
        // atendeu - o mesmo caminho que o FCFS faria na ordem de chegada.
        mAndou += std::abs(it->cilindro - mUltimoCilindro);
        mUltimoCilindro = it->cilindro;
        mAtendidos.emplace_back(it->chegou, it->cilindro);
        Matar(it->p);
        it = mPedidos.erase(it);
    }

    // Quem espera demais passa fome - e grita.
    for (Pedido& p : mPedidos) {
        const float espera = t - p.chegou;
        Cara(p.p, espera >= kFome ? "Faminto" : espera >= kFome / 2.0f ? "Velho" : "Novo");
        if (espera >= kFome && t >= p.proximoGrito) {
            p.proximoGrito = t + kEntreGritos;
            DispararEm(kGatilhoDaFome, p.p->GetPosition());
        }
    }

    // A trilha, as marcas dos cilindros e o braco descendo do professor ate ela.
    Linha(Vector2(XDaTrilha(), YDaTrilha()), Vector2(XDaTrilha() + LarguraDaTrilha(), YDaTrilha()), kTrilha);
    for (int c = 0; c < kCilindros; c += 25) {
        const float x = XDoCilindro(c, XDaTrilha(), LarguraDaTrilha());
        Linha(Vector2(x, YDaTrilha() - 5.0f), Vector2(x, YDaTrilha() + 5.0f), kTrilha);
    }
    Linha(GetPosition() + Vector2(0.0f, 40.0f), Vector2(GetPosition().x, YDaTrilha()), kBraco);

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
    for (Vm& v : mVms) Matar(v.p);
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
    (void)deltaTime;
    const int etapa = EtapaNoTempo(t);
    const float cadencia = mDegrauDaNuvem.cadencia;

    // As VMs que sumiram (o Ponto Extra as leva) saem da conta, e a nuvem repoe.
    const size_t antes = mVms.size();
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
                mVms.push_back(Vm{p, id, host, -1, 0.0f, t + 0.6f + defasagem, t + 1.0f + defasagem});
                if (etapa == 2 && queridas > mVmsAntes) Avisar(PlacarDaEscala(static_cast<int>(mVms.size()) - 1, static_cast<int>(mVms.size())));
            }
        }
    }
    mVmsAntes = std::max(mVmsAntes, queridas);

    // A migracao ao vivo: uma VM por vez escorrega para um host livre, sem parar de rodar.
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
        v.p->SetPosition(onde);
        if (auto* rb = v.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        Retangulo(onde, kLarguraDaVm, kAlturaDaVm, kCores[static_cast<size_t>((v.id - 1) % 4)]);

        // O kernel do convidado roda: um anel.
        if (t >= v.proximoKernel) {
            v.proximoKernel = t + kEntreKernels * cadencia;
            DispararEm(kGatilhoDoKernel, onde);
        }
        // A instrucao privilegiada: um trap ate o hipervisor (parada, a VM; migrando, espera).
        if (t >= v.proximoTrap && v.destino < 0) {
            v.proximoTrap = t + kEntreTraps * cadencia;
            if (BossProjectile* trap = Um(kGatilhoDoTrap, onde, "", 1.0f, (hipervisor - onde) * (1.0f / kVooDoTrap))) {
                trap->DefinirEnfeite(true);
                mTraps.push_back(Trap{trap, t + kVooDoTrap});
                Avisar(PlacarDoTrap(v.id), 0.9f);
            }
        }
    }

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
