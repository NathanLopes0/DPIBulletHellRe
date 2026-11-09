//
// O chefe da INF 340 - Linguagens de Programacao.
//

#include "Luizinho.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "../../../Motor/Random.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../Player/Player.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"
#include "PainelDeCodigo.h"

namespace {

    bool Vivo(const BossProjectile* p) { return p && p->GetState() == ActorState::Active; }

    std::uint32_t Sorteio() {
        return static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
    }

    void PorCara(BossProjectile* p, const std::string& cara) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }

    void Colidir(BossProjectile* p, const bool colide) {
        if (auto* c = p->GetComponent<CircleColliderComponent>()) c->SetEnabled(colide);
    }

    // Os paineis e os cantos de codigo do exame.
    constexpr SDL_Color kMolduraDoPainel{100, 120, 165, 190};
    constexpr SDL_Color kFundoDoPainel{24, 30, 48, 110};
    constexpr SDL_Color kMolduraDoCanto{120, 140, 180, 230};
    constexpr SDL_Color kFundoDoCanto{28, 32, 46, 225};
    constexpr SDL_Color kTintaDoCanto{214, 226, 250, 255};
    constexpr float kAlturaDoCanto = 26.0f;
    constexpr int kOrdemDosCantos = 57;

    // A linha do professor ate a conta da fase 2.
    constexpr SDL_Color kLinhaDaChamada{90, 200, 240, 220};

    /// A largura do canto que cabe o texto, na fonte do placar.
    float LarguraDoCanto(const std::string& texto) {
        return static_cast<float>(texto.size()) * 9.5f + 18.0f;
    }
}

Luizinho::Luizinho(Scene* scene) : Boss(scene) {
}

std::optional<Vector2> Luizinho::AlvoDoMovimento() const {
    return mAlvo;
}

void Luizinho::SortearALuta() {
    mLutaSorteada = true;
    const Dificuldade::Nivel nivel = NivelDeDificuldade();
    mDegrauDasFormas = Polimorfismo::DegrauDo(nivel);
    mDegrauDosTipos = Coercao::DegrauDo(nivel);
    mDegrauDaComplexidade = Complexidade::DegrauDo(nivel);
    mDegrauDasExcecoes = Excecao::DegrauDo(nivel);
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Polimorfismo::kFaseDasFormas);
        mPlanoDasFormas = Polimorfismo::Sortear(sorteio);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Coercao::kFaseDosTipos);
        mOrdemDosTipos = Coercao::OrdemDosComandos(sorteio);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Complexidade::kFaseDaComplexidade);
        mPlanoDaComplexidade = Complexidade::Sortear(sorteio);
    }
    mSorteioDasRodadas = SorteioDaFase(Excecao::kFaseDasExcecoes);
}

void Luizinho::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();

    Boss::OnUpdate(deltaTime);

    if (!mLinhas && mScene) {
        auto ator = std::make_unique<Actor>(mScene);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();

    AtualizarFormas();
    AtualizarTracos();
    AtualizarTipos();
    AtualizarComplexidade();
    AtualizarExcecoes();
}

void Luizinho::AoTerminarABatalha() {
    // A tela da nota fica limpa: os projeteis a batalha ja levou.
    for (EtiquetaDeCodigo* r : mRotulos) if (r) r->Mostrar(false);
    LimparConta();
    if (mLinhas) mLinhas->Limpar();
    if (mQuadroDoGrafico) mQuadroDoGrafico->Mostrar(false);
    if (mNomeDoGrafico) mNomeDoGrafico->Mostrar(false);
    for (const FaixaEmCampo& f : mFaixas) {
        if (f.painel) f.painel->Mostrar(false);
        if (f.canto) f.canto->Mostrar(false);
    }
}

void Luizinho::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O ataque de gatilho nasce onde o professor disse, no numero que ele pediu - e so ele.
    if (mDisparandoEm) {
        params.firePosition = mOndeDisparar;
        if (mQuantos > 0) params.numProjectiles = mQuantos;
        if (mCentroEm) params.centralAngle = *mCentroEm;
        if (mAberturaEm) params.angle = *mAberturaEm;
    }
}

void Luizinho::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                    const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

std::vector<BossProjectile*> Luizinho::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
                                                 const std::optional<float> centro,
                                                 const std::optional<float> abertura,
                                                 const float rapidez) {
    mDisparandoEm = true;
    mOndeDisparar = onde;
    mQuantos = quantos;
    mCentroEm = centro;
    mAberturaEm = abertura;
    mCriados.clear();
    Disparar(gatilho, -1, rapidez);
    mDisparandoEm = false;
    mQuantos = 0;
    mCentroEm.reset();
    mAberturaEm.reset();
    return mCriados;
}

float Luizinho::AnguloAteOAluno(const Vector2 de) const {
    const Vector2 d = PosicaoDoAluno() - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

// A fase 1: o polimorfismo.

namespace {
    /// A cor de cada classe: a do traco, a da moldura do rotulo e a do desenho em cima dele.
    SDL_Color CorDaClasse(const Polimorfismo::Classe c) {
        switch (c) {
            case Polimorfismo::Classe::Circulo: return {96, 210, 255, 255};
            case Polimorfismo::Classe::Quadrado: return {255, 214, 72, 255};
            case Polimorfismo::Classe::Triangulo: return {110, 230, 130, 255};
            case Polimorfismo::Classe::Estrela: return {240, 120, 230, 255};
        }
        return {220, 220, 230, 255};
    }
    constexpr SDL_Color kMolduraDaForma{150, 150, 170, 230};     // "Forma f = ?": o tipo ainda nao se sabe
    constexpr SDL_Color kFundoDoRotulo{24, 28, 42, 230};
    constexpr SDL_Color kTintaDoRotulo{226, 232, 250, 255};
    constexpr SDL_Color kLinhaDaReferencia{150, 200, 255, 200};

    float Suave(const float u) { return u * u * (3.0f - 2.0f * u); }
}

void Luizinho::AtualizarTracos() {
    for (TracoNoAr& t : mTracosNoAr) {
        if (!Vivo(t.p)) { t.p = nullptr; continue; }
        if (t.p->TempoDeVida() >= Polimorfismo::kVidaDoTraco) {
            t.p->Kill();
            t.p = nullptr;
            continue;
        }
        auto* rb = t.giro != 0.0f ? t.p->GetComponent<RigidBodyComponent>() : nullptr;
        if (!rb) continue;
        // A forma cresce na mesma escala e gira: cada ponto se afasta do centro e anda de lado.
        const Vector2 d = t.p->GetPosition() - t.centro;
        const float escala = Polimorfismo::kRaio + mDegrauDasFormas.velocidade * (t.p->TempoDeVida() - t.vidaAoPartir);
        rb->SetVelocity(d * (mDegrauDasFormas.velocidade / escala) + Vector2(-d.y, d.x) * Math::ToRadians(t.giro));
    }
    mTracosNoAr.erase(std::remove_if(mTracosNoAr.begin(), mTracosNoAr.end(), [](const TracoNoAr& t) { return !t.p; }),
                      mTracosNoAr.end());
}

void Luizinho::DesmontarFormas() {
    // O programa acabou: os objetos saem de escopo, e o que eles desenharam some junto.
    for (Objeto& o : mObjetos) {
        if (o.rotulo) o.rotulo->Mostrar(false);
        for (BossProjectile* p : o.pontos) if (Vivo(p)) p->Kill();
    }
    mObjetos.clear();
    for (const TracoNoAr& t : mTracosNoAr) if (Vivo(t.p)) t.p->Kill();
    mTracosNoAr.clear();
    mChamadaEmCurso = false;
    RestaurarAnuncio(Polimorfismo::kFaseDasFormas);
    mAlvo.reset();
    mNasFormas = false;
}

void Luizinho::ComecarChamada(const float agora) {
    mChamada = Polimorfismo::Proxima(Sorteio(), mDegrauDasFormas.noLaco, mPlanoDasFormas);
    mInicioDaChamada = agora;
    mChamadaEmCurso = true;
    TrocarAnuncio(Polimorfismo::kFaseDasFormas, Polimorfismo::Placar(mChamada, false));

    const SDL_FRect campo = Campo();
    const auto lugares = Polimorfismo::Lugares(static_cast<int>(mChamada.objetos.size()), Sorteio(), campo.w,
                                               campo.y + Polimorfismo::kTopoDaFaixa, campo.y + Polimorfismo::kFundoDaFaixa);
    // Os rotulos sao criados na primeira vez e reaproveitados nas outras.
    while (mRotulos.size() < mChamada.objetos.size() && mScene) {
        auto r = std::make_unique<EtiquetaDeCodigo>(mScene, " ", kLarguraDoRotulo, static_cast<int>(kAlturaDoCanto),
                                                    kOrdemDosRotulos);
        mRotulos.push_back(r.get());
        mScene->AddActor(std::move(r));
    }

    const bool dinamico = mChamada.comando == Polimorfismo::Comando::TipoDinamico;
    mObjetos.clear();
    for (size_t k = 0; k < mChamada.objetos.size(); ++k) {
        Objeto o;
        o.rotulo = k < mRotulos.size() ? mRotulos[k] : nullptr;
        o.classe = mChamada.objetos[k];
        o.lugar = Vector2(campo.x + lugares[k].x, lugares[k].y);
        o.invoca = agora + Polimorfismo::kVooDoNew + Polimorfismo::kAntesDaChamada +
                   mDegrauDasFormas.entreChamadas * static_cast<float>(k);
        o.traca = o.invoca + (dinamico ? Polimorfismo::kRevelacao : 0.0f);
        o.contorno = Polimorfismo::Contorno(o.classe, mDegrauDasFormas.pontos);
        o.giro = Sorteio() % 2u == 0u ? mDegrauDasFormas.giro : -mDegrauDasFormas.giro;
        o.revelado = !dinamico;
        if (o.rotulo) {
            o.rotulo->Escrever(Polimorfismo::Rotulo(mChamada, static_cast<int>(k), o.revelado));
            o.rotulo->Pintar(o.revelado ? CorDaClasse(o.classe) : kMolduraDaForma, kFundoDoRotulo, kTintaDoRotulo);
            o.rotulo->SetPosition(GetPosition());
            o.rotulo->Mostrar(true);
        }
        mObjetos.push_back(std::move(o));
    }
}

void Luizinho::DesenharFormas(const float agora) {
    if (!mLinhas) return;
    for (const Objeto& o : mObjetos) {
        if (!o.rotulo || (o.partiu && agora > o.partiuEm + 0.25f)) continue;
        const Vector2 em = o.rotulo->GetPosition();
        // f -> objeto: a referencia da chamada, do professor ate quem responde a ela.
        if (agora >= o.invoca && !o.partiu) mLinhas->Acrescentar(GetPosition(), em, kLinhaDaReferencia);
        if (!o.revelado) continue;
        // A forma da classe, desenhada em cima do rotulo.
        const SDL_Color cor = CorDaClasse(o.classe);
        const Vector2 centro = em + Vector2(0.0f, -32.0f);
        constexpr float kIcone = 14.0f;
        auto v = Polimorfismo::Vertices(o.classe);
        if (v.empty()) v = Polimorfismo::Contorno(o.classe, 12);
        for (size_t i = 0; i < v.size(); ++i) {
            mLinhas->Acrescentar(centro + v[i] * kIcone, centro + v[(i + 1) % v.size()] * kIcone, cor);
        }
    }
}

void Luizinho::AtualizarFormas() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Polimorfismo::kFaseDasFormas && !EhClone();
    if (!aqui) {
        if (mNasFormas) DesmontarFormas();
        return;
    }
    const SDL_FRect campo = Campo();
    if (!mNasFormas) {
        mNasFormas = true;
        mChamadaEmCurso = false;
        mProximaChamada = Polimorfismo::kPrimeiro;
        mAlvo = Vector2(campo.x + campo.w / 2.0f, campo.y + kYDoProfessorNasFormas);
    }
    const float agora = fsm->GetStateTime();
    if (!mChamadaEmCurso && agora >= mProximaChamada) ComecarChamada(agora);
    if (!mChamadaEmCurso) return;

    bool acabou = true;
    const float voo = Suave(std::clamp((agora - mInicioDaChamada) / Polimorfismo::kVooDoNew, 0.0f, 1.0f));
    for (size_t k = 0; k < mObjetos.size(); ++k) {
        Objeto& o = mObjetos[k];
        // O new: o objeto sai do professor e vai ao lugar dele.
        if (o.rotulo && !o.partiu) o.rotulo->SetPosition(GetPosition() + (o.lugar - GetPosition()) * voo);

        // f.desenhar(): o professor vai ate o objeto chamado.
        if (agora >= o.invoca && !o.partiu) mAlvo = Vector2(o.lugar.x, campo.y + kYDoProfessorNasFormas);

        // O tipo dinamico so se sabe na chamada.
        if (!o.revelado && agora >= o.traca) {
            o.revelado = true;
            if (o.rotulo) {
                o.rotulo->Escrever(Polimorfismo::Rotulo(mChamada, static_cast<int>(k), true));
                o.rotulo->Pintar(CorDaClasse(o.classe), kFundoDoRotulo, kTintaDoRotulo);
            }
            TrocarAnuncio(Polimorfismo::kFaseDasFormas, Polimorfismo::Placar(mChamada, true));
        }

        // O contorno, ponto a ponto - e a forma parte, crescendo sem deixar de ser ela.
        if (agora >= o.traca && !o.partiu) {
            const float u = std::clamp((agora - o.traca) / Polimorfismo::kTraco, 0.0f, 1.0f);
            const int devem = std::min(static_cast<int>(o.contorno.size()),
                                       static_cast<int>(std::ceil(u * static_cast<float>(o.contorno.size()))));
            while (o.tracados < devem) {
                const Vector2 onde = o.lugar + o.contorno[static_cast<size_t>(o.tracados)] * Polimorfismo::kRaio;
                for (BossProjectile* p : DispararEm(Polimorfismo::kGatilhoDoTraco, onde, 1)) {
                    PorCara(p, Polimorfismo::Nome(o.classe));
                    o.pontos.push_back(p);
                }
                ++o.tracados;
            }
            if (agora >= o.traca + Polimorfismo::kTraco + 0.06f) {
                for (size_t i = 0; i < o.pontos.size() && i < o.contorno.size(); ++i) {
                    if (!Vivo(o.pontos[i])) continue;
                    if (auto* rb = o.pontos[i]->GetComponent<RigidBodyComponent>()) {
                        rb->SetVelocity(o.contorno[i] * mDegrauDasFormas.velocidade);
                    }
                }
                o.partiu = true;
                o.partiuEm = agora;
                for (BossProjectile* p : o.pontos) {
                    if (Vivo(p)) mTracosNoAr.push_back(TracoNoAr{p, o.lugar, p->TempoDeVida(), o.giro});
                }
            }
        }
        if (o.partiu && agora > o.partiuEm + 0.25f && o.rotulo) o.rotulo->Mostrar(false);
        if (!o.partiu || agora <= o.partiuEm + 0.25f) acabou = false;
    }
    DesenharFormas(agora);

    if (acabou) {
        mChamadaEmCurso = false;
        mObjetos.clear();
        const float passo = Random::GetFloatRange(mDegrauDasFormas.menorPasso, mDegrauDasFormas.maiorPasso);
        mProximaChamada = std::max(mInicioDaChamada + passo, agora + 0.2f);
    }
}

// A fase 2: os sistemas de tipos.

void Luizinho::LimparConta() {
    for (Operando& o : mOperandos) {
        if (Vivo(o.p)) o.p->Kill();
        o = Operando{};
    }
    if (Vivo(mOperador)) mOperador->Kill();
    mOperador = nullptr;
}

void Luizinho::DesmontarTipos() {
    LimparConta();
    RestaurarAnuncio(Coercao::kFaseDosTipos);
    mNosTipos = false;
    mContaDaVez = -1;
    mEtapa = EtapaDaConta::Esperando;
    mAlvo.reset();
}

void Luizinho::ComecarConta(const int k) {
    LimparConta();
    mContaDaVez = k;
    mConta = Coercao::Proximo(Variacao::NaOrdem(mOrdemDosTipos, k), Sorteio());
    mInicioDaConta = Coercao::kPrimeiro + static_cast<float>(k) * Coercao::kPorComando;
    mEtapa = EtapaDaConta::Esperando;
    TrocarAnuncio(Coercao::kFaseDosTipos, Coercao::Placar(mConta));

    // O professor muda de lugar a cada conta, no alto - e balanca em volta dele (AtualizarTipos).
    const SDL_FRect campo = Campo();
    const float fracao = static_cast<float>(Sorteio() % 1001u) / 1000.0f;
    mBaseDoProfessor = Vector2(campo.x + Coercao::kXMinimo + fracao * (Coercao::kXMaximo - Coercao::kXMinimo),
                               campo.y + Coercao::kYDoProfessor);
    mAlvo = mBaseDoProfessor;

    // Enfeite - o tiro passa por ele, e ele nao acerta.
    const Coercao::Ponto onde = Coercao::OndeFicaOOperador(PosicaoDoAluno().x - campo.x, campo.w);
    mOndeDaConta = Vector2(campo.x + onde.x, campo.y + onde.y);
    const auto criados = DispararEm(Coercao::kGatilhoDoOperador, mOndeDaConta, 1);
    if (!criados.empty()) {
        mOperador = criados.front();
        mOperador->DefinirEnfeite(true);
        mOperador->DefinirPosto("aviso");
        PorCara(mOperador, Coercao::CaraDoOperador(Coercao::OperacaoDe(mConta.forma), false));
    }
}

void Luizinho::LancarOperandos() {
    const Coercao::Resultado r = Coercao::Avaliar(mConta);
    const Coercao::Valor valores[2] = {Coercao::Esquerdo(mConta), Coercao::Direito(mConta)};
    const bool coagidos[2] = {r.coagidoEsquerdo, r.coagidoDireito};
    const Coercao::Valor depois[2] = {r.esquerdoDepois, r.direitoDepois};

    const Vector2 professor = GetPosition();
    const auto criados = DispararEm(Coercao::kGatilhoDoOperando, professor, 2);
    for (size_t i = 0; i < mOperandos.size() && i < criados.size(); ++i) {
        Operando& o = mOperandos[i];
        const float lado = (i == 0) ? -1.0f : 1.0f;
        o.p = criados[i];
        o.lado = lado;
        o.de = professor + Vector2(lado * 20.0f, 0.0f);
        o.para = mOndeDaConta + Vector2(lado * Coercao::kLadoDoOperando, 0.0f);
        o.depois = coagidos[i] ? Coercao::CaraDoOperando(depois[i]) : "";
        o.trocou = false;
        o.p->SetPosition(o.de);
        PorCara(o.p, Coercao::CaraDoOperando(valores[i]));
    }
}

void Luizinho::Compilar() {
    // O compilador recusa antes de rodar.
    const Vector2 professor = GetPosition();
    for (Operando& o : mOperandos) {
        if (Vivo(o.p)) o.p->Kill();
        o = Operando{};
    }
    if (Vivo(mOperador)) mOperador->Kill();
    mOperador = nullptr;

    const int n = Coercao::kEstilhacosDaCompilacao;
    const auto cacos = DispararEm(Coercao::kGatilhoDaCompilacao, professor, n,
                                  Coercao::CentroDoAnel(n, AnguloAteOAluno(professor)), Coercao::AberturaDoAnel(n));
    for (BossProjectile* c : cacos) PorCara(c, Coercao::kCaraDaCompilacao);
    TrocarAnuncio(Coercao::kFaseDosTipos, Coercao::PlacarComResultado(mConta));
}

void Luizinho::FazerAConta() {
    // Os dois tem de chegar: o Ponto Extra que leva um operando leva a conta.
    for (const Operando& o : mOperandos) if (!Vivo(o.p)) { LimparConta(); return; }
    for (Operando& o : mOperandos) {
        o.p->Kill();
        o = Operando{};
    }

    const Coercao::Resultado r = Coercao::Avaliar(mConta);
    const Vector2 onde = mOndeDaConta;
    const float alvo = AnguloAteOAluno(onde);

    if (r.desfecho == Coercao::Desfecho::ErroDeExecucao) {
        // O TypeError estoura onde a conta rodou.
        const int n = Coercao::kEstilhacosDoErro;
        const auto cacos = DispararEm(Coercao::kGatilhoDoErro, onde, n, Coercao::CentroDoAnel(n, alvo),
                                      Coercao::AberturaDoAnel(n), mDegrauDosTipos.rapidez);
        for (BossProjectile* c : cacos) PorCara(c, Coercao::kCaraDoTypeError);
        if (Vivo(mOperador)) PorCara(mOperador, Coercao::CaraDoOperador(Coercao::OperacaoDe(mConta.forma), true));
    }
    else if (r.valor.tipo == Coercao::Tipo::Numero) {
        // Um numero n e um anel de n, cada chip com o n escrito.
        const int n = r.valor.numero;
        const auto chips = DispararEm(Coercao::kGatilhoDoAnel, onde, n, Coercao::CentroDoAnel(n, alvo),
                                      Coercao::AberturaDoAnel(n), mDegrauDosTipos.rapidez);
        for (BossProjectile* c : chips) PorCara(c, Coercao::CaraDoNumero(n));
    }
    else {
        // Um texto e a palavra, letra por letra entre aspas, de frente para o aluno.
        const auto letras = Coercao::Letras(r.valor);
        const auto desvios = Coercao::DesviosDaPalavra(static_cast<int>(letras.size()), alvo);
        const auto chips = DispararEm(Coercao::kGatilhoDoTexto, onde, static_cast<int>(letras.size()), alvo, 0.0f,
                                      mDegrauDosTipos.rapidez);
        for (size_t i = 0; i < chips.size() && i < letras.size(); ++i) {
            chips[i]->SetPosition(onde + Vector2(desvios[i].x, desvios[i].y));
            PorCara(chips[i], Coercao::CaraDaLetra(letras[i]));
        }
    }
    TrocarAnuncio(Coercao::kFaseDosTipos, Coercao::PlacarComResultado(mConta));
}

void Luizinho::AtualizarTipos() {
    for (Operando& o : mOperandos) if (o.p && !Vivo(o.p)) o.p = nullptr;
    if (mOperador && !Vivo(mOperador)) mOperador = nullptr;

    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Coercao::kFaseDosTipos && !EhClone();
    if (!aqui) {
        if (mNosTipos) DesmontarTipos();
        return;
    }
    if (!mNosTipos) {
        mNosTipos = true;
        mContaDaVez = -1;
        const SDL_FRect campo = Campo();
        mBaseDoProfessor = Vector2(campo.x + campo.w / 2.0f, campo.y + Coercao::kYDoProfessor);
        mAlvo = mBaseDoProfessor;
        mHorariosDosTipos = Coercao::Horarios(Sorteio());
    }

    const float t = fsm->GetStateTime();
    if (t < Coercao::kPrimeiro) return;
    // A conta da vez: a ultima cujo horario ja chegou.
    int k = 0;
    while (k + 1 < static_cast<int>(mHorariosDosTipos.size()) && t >= mHorariosDosTipos[static_cast<size_t>(k + 1)]) ++k;
    if (k != mContaDaVez) ComecarConta(k);
    mAlvo = mBaseDoProfessor + Vector2(Coercao::kBalanco * std::sin(2.0f * Math::Pi * t / Coercao::kPeriodoDoBalanco), 0.0f);
    const float u = t - mInicioDaConta;
    const bool estatica = Coercao::TipagemDe(Coercao::LinguagemDe(mConta.forma)) == Coercao::Tipagem::ForteEstatica;

    switch (mEtapa) {
        case EtapaDaConta::Esperando:
            if (u >= Coercao::kSaida) {
                LancarOperandos();
                mEtapa = estatica ? EtapaDaConta::Checando : EtapaDaConta::Voando;
            }
            break;

        case EtapaDaConta::Checando:
            // Os operandos esperam o compilador na mao do professor.
            for (Operando& o : mOperandos) {
                if (Vivo(o.p)) o.p->SetPosition(GetPosition() + Vector2(o.lado * 20.0f, 0.0f));
            }
            if (u >= Coercao::kSaida + Coercao::kChecagem) {
                Compilar();
                mEtapa = EtapaDaConta::Feita;
            }
            break;

        case EtapaDaConta::Voando: {
            const float s = std::clamp((u - Coercao::kSaida) / mDegrauDosTipos.voo, 0.0f, 1.0f);
            for (Operando& o : mOperandos) {
                if (!Vivo(o.p)) continue;
                // Em arco, cada um para o seu lado: os dois se leem no caminho.
                const Vector2 reta = o.de + (o.para - o.de) * s;
                const float arco = std::sin(s * Math::Pi) * kArcoDoVoo * o.lado;
                o.p->SetPosition(reta + Vector2(arco, 0.0f));
                // A coercao se ve: no meio do voo o convertido troca de chip.
                if (!o.trocou && !o.depois.empty() && s >= Coercao::kCoercao) {
                    PorCara(o.p, o.depois);
                    o.trocou = true;
                }
            }
            if (u >= Coercao::kSaida + mDegrauDosTipos.voo) {
                FazerAConta();
                mEtapa = EtapaDaConta::Feita;
            }
            break;
        }

        case EtapaDaConta::Feita:
            // O operador fica um pouco depois da conta, aceso, e sai.
            if (u >= Coercao::kSaida + mDegrauDosTipos.voo + 0.8f && Vivo(mOperador)) {
                mOperador->Kill();
                mOperador = nullptr;
            }
            break;
    }

    // A conta que vai rodar: uma linha do professor ao operador.
    if (mEtapa != EtapaDaConta::Feita && Vivo(mOperador) && mLinhas) {
        mLinhas->Acrescentar(GetPosition(), mOperador->GetPosition(), kLinhaDaChamada);
    }
}

// A fase 3: a complexidade.

namespace {
    /// A cor de cada ordem, pelo custo: verde as baratas, amarelo as medias, vermelho as caras.
    SDL_Color CorDaOrdem(const Complexidade::Ordem o, const Uint8 alfa = 255) {
        switch (o) {
            case Complexidade::Ordem::Constante:
            case Complexidade::Ordem::Logaritmica: return {110, 230, 130, alfa};
            case Complexidade::Ordem::Linear:
            case Complexidade::Ordem::Linearitmica: return {255, 214, 72, alfa};
            case Complexidade::Ordem::Quadratica:
            case Complexidade::Ordem::Exponencial: return {255, 110, 90, alfa};
        }
        return {220, 220, 230, alfa};
    }
    constexpr SDL_Color kCasaApagada{120, 130, 160, 120};
    constexpr SDL_Color kCasaNaBusca{200, 210, 235, 220};
    constexpr SDL_Color kEixoDoGrafico{160, 170, 200, 210};
    constexpr SDL_Color kMolduraDoGrafico{100, 120, 165, 190};
    constexpr SDL_Color kFundoDoGrafico{20, 26, 42, 200};

    /// O grafico: o quadro no canto de cima a direita, e a area das curvas dentro dele.
    constexpr float kLarguraDoQuadro = 300.0f;
    constexpr float kAlturaDoQuadro = 180.0f;
    constexpr float kLarguraDasCurvas = 250.0f;
    constexpr float kAlturaDasCurvas = 120.0f;
    constexpr float kMargemDoQuadro = 18.0f;
}

void Luizinho::DesmontarComplexidade() {
    for (Galho& g : mGalhos) g.p = nullptr;
    mGalhos.clear();
    mLeituras.clear();
    mRodadaEmCurso = false;
    if (mQuadroDoGrafico) mQuadroDoGrafico->Mostrar(false);
    if (mNomeDoGrafico) mNomeDoGrafico->Mostrar(false);
    RestaurarAnuncio(Complexidade::kFaseDaComplexidade);
    mAlvo.reset();
    mNaComplexidade = false;
}

void Luizinho::ComecarRodadaDaComplexidade(const float agora) {
    mRodadaDaComplexidade = Complexidade::Proxima(mVezDaComplexidade++, Sorteio(), mPlanoDaComplexidade, mOrdemAnterior,
                                                  mDegrauDaComplexidade.nAMais);
    const Complexidade::Rodada& r = mRodadaDaComplexidade;
    mOrdemAnterior = static_cast<int>(r.ordem);
    TrocarAnuncio(Complexidade::kFaseDaComplexidade, Complexidade::Placar(r));
    // O processador muda de lugar a cada rodada: o leque sai de outro ponto.
    const SDL_FRect campo = Campo();
    mAlvo = Vector2(campo.x + Random::GetFloatRange(Complexidade::kMenorX, Complexidade::kMaiorX),
                    campo.y + kYDoProfessorNaComplexidade);
    if (mNomeDoGrafico) mNomeDoGrafico->Escrever(Complexidade::Notacao(r.ordem) + ", n = " + std::to_string(r.n));

    mLidaEm.assign(static_cast<size_t>(std::max(1, r.n)), -10.0f);
    mLeituras.clear();
    mProximaLeitura = 0;
    mGalhos.clear();
    mDobras = 0;
    mMeiosLidos = 0;
    mBusca = Complexidade::Intervalo{0, r.n - 1};
    const float t0 = agora + Complexidade::kAntes;
    mProximoMeio = t0;
    mProximaDobra = t0;

    switch (r.ordem) {
        case Complexidade::Ordem::Constante:
            // v[i]: uma leitura, na casa em que o aluno estiver.
            mLeituras.push_back(Leitura{t0, -1});
            break;
        case Complexidade::Ordem::Linear:
        case Complexidade::Ordem::Linearitmica:
        case Complexidade::Ordem::Quadratica: {
            // As passadas pelo leque, cada uma no sentido contrario da anterior.
            const bool lacos = r.ordem == Complexidade::Ordem::Quadratica;
            const float ritmo = mDegrauDaComplexidade.ritmo;
            const float naPassada = (lacos ? Complexidade::kNaPassadaDosLacos : Complexidade::kNaPassada) * ritmo;
            const float entre = (lacos ? Complexidade::kEntrePassadas * 0.6f : Complexidade::kEntrePassadas) * ritmo;
            const int passadas = Complexidade::Passadas(r.ordem, r.n);
            const bool primeiraDaEsquerda = Sorteio() % 2u == 0u;
            float quando = t0;
            for (int p = 0; p < passadas; ++p) {
                const bool daEsquerda = (p % 2 == 0) == primeiraDaEsquerda;
                for (int j = 0; j < r.n; ++j) {
                    mLeituras.push_back(Leitura{quando, daEsquerda ? j : r.n - 1 - j});
                    quando += naPassada;
                }
                quando += entre - naPassada;
            }
            break;
        }
        case Complexidade::Ordem::Logaritmica:
        case Complexidade::Ordem::Exponencial:
            break;
    }
    mRodadaEmCurso = true;
}

void Luizinho::VestirOperacao(BossProjectile* p) const {
    PorCara(p, Complexidade::CaraDaOperacao(mRodadaDaComplexidade.ordem));
    if (mDegrauDaComplexidade.tamanho != 1.0f) p->SetScale(p->GetScale() * mDegrauDaComplexidade.tamanho);
}

void Luizinho::Operar(const int casa, const bool acha) {
    const Complexidade::Rodada& r = mRodadaDaComplexidade;
    // A leitura que acha o aluno mira nele e sai grande; as outras caem em qualquer ponto da casa.
    const float angulo = acha ? Complexidade::PresoNaCasa(AnguloAteOAluno(GetPosition()), casa, r.n)
                              : Complexidade::AnguloNaCasa(casa, r.n, Random::GetFloatRange(0.0f, 1.0f));
    const float rad = Math::ToRadians(angulo);
    const Vector2 de = GetPosition() + Vector2(Math::Cos(rad), Math::Sin(rad)) * kRaioDoLeque;
    const char* gatilho = acha ? Complexidade::kGatilhoDoAcesso : Complexidade::kGatilhoDaOperacao;
    for (BossProjectile* p : DispararEm(gatilho, de, 1, angulo, 0.0f, mDegrauDaComplexidade.rapidez)) {
        VestirOperacao(p);
    }
    if (const auto* fsm = GetComponent<FSMComponent>(); fsm && casa >= 0 && static_cast<size_t>(casa) < mLidaEm.size()) {
        mLidaEm[static_cast<size_t>(casa)] = fsm->GetStateTime();
    }
}

void Luizinho::Dobrar() {
    // Cada galho vira dois, um para cada lado: 2, 4, 8... - o exponencial.
    std::vector<Galho> novos;
    for (const Galho& g : mGalhos) {
        if (!Vivo(g.p)) continue;
        const auto filhos = DispararEm(Complexidade::kGatilhoDaOperacao, g.p->GetPosition(), 2, g.angulo,
                                       2.0f * Complexidade::kDesvioDaDobra, mDegrauDaComplexidade.rapidez);
        for (size_t k = 0; k < filhos.size(); ++k) {
            VestirOperacao(filhos[k]);
            novos.push_back(Galho{filhos[k], g.angulo + (k == 0 ? -1.0f : 1.0f) * Complexidade::kDesvioDaDobra});
        }
        g.p->Kill();
    }
    mGalhos = std::move(novos);
}

void Luizinho::DesenharLeque(const float agora) {
    if (!mLinhas) return;
    const Complexidade::Rodada& r = mRodadaDaComplexidade;
    const bool busca = mRodadaEmCurso && r.ordem == Complexidade::Ordem::Logaritmica;
    for (int i = 0; i < r.n; ++i) {
        const float rad = Math::ToRadians(Complexidade::AnguloDaCasa(i, r.n));
        const Vector2 d(Math::Cos(rad), Math::Sin(rad));
        const bool acesa = static_cast<size_t>(i) < mLidaEm.size() && agora - mLidaEm[static_cast<size_t>(i)] < 0.3f;
        const bool naBusca = busca && !mBusca.Vazio() && i >= mBusca.lo && i <= mBusca.hi;
        const SDL_Color cor = acesa ? CorDaOrdem(r.ordem) : naBusca ? kCasaNaBusca : kCasaApagada;
        mLinhas->Acrescentar(GetPosition() + d * (kRaioDoLeque - 14.0f), GetPosition() + d * (kRaioDoLeque + 4.0f), cor);
    }
}

void Luizinho::DesenharGrafico() {
    if (!mLinhas || !mQuadroDoGrafico) return;
    const Vector2 quadro = mQuadroDoGrafico->GetPosition();
    const Vector2 origem = quadro + Vector2(-kLarguraDoQuadro / 2.0f + kMargemDoQuadro + 10.0f, kAlturaDoQuadro / 2.0f - kMargemDoQuadro);
    auto ponto = [&origem](const float n, const float y) {
        return Vector2(origem.x + (n - 1.0f) / (Complexidade::kNoGrafico - 1.0f) * kLarguraDasCurvas,
                       origem.y - y / Complexidade::kTetoDoGrafico * kAlturaDasCurvas);
    };
    mLinhas->Acrescentar(ponto(1.0f, 0.0f), ponto(Complexidade::kNoGrafico, 0.0f), kEixoDoGrafico);
    mLinhas->Acrescentar(ponto(1.0f, 0.0f), ponto(1.0f, Complexidade::kTetoDoGrafico), kEixoDoGrafico);
    const auto atual = mRodadaDaComplexidade.ordem;
    for (int k = 0; k < Complexidade::kOrdens; ++k) {
        const auto o = static_cast<Complexidade::Ordem>(k);
        const bool esta = o == atual;
        const SDL_Color cor = CorDaOrdem(o, esta ? 255 : 90);
        for (float n = 1.0f; n < Complexidade::kNoGrafico; n += 0.25f) {
            const Vector2 a = ponto(n, Complexidade::NaCurva(o, n));
            const Vector2 b = ponto(n + 0.25f, Complexidade::NaCurva(o, n + 0.25f));
            mLinhas->Acrescentar(a, b, cor);
            if (esta) mLinhas->Acrescentar(a + Vector2(0.0f, -1.0f), b + Vector2(0.0f, -1.0f), cor);
        }
    }
    // Onde a rodada cai na curva: o n dela (no canto, quando passa do grafico).
    const float n = std::min(static_cast<float>(mRodadaDaComplexidade.n), Complexidade::kNoGrafico);
    const Vector2 x = ponto(n, Complexidade::NaCurva(atual, n));
    const SDL_Color branco{255, 255, 255, 255};
    mLinhas->Acrescentar(x + Vector2(-5.0f, -5.0f), x + Vector2(5.0f, 5.0f), branco);
    mLinhas->Acrescentar(x + Vector2(-5.0f, 5.0f), x + Vector2(5.0f, -5.0f), branco);
}

void Luizinho::AtualizarComplexidade() {
    for (Galho& g : mGalhos) if (!Vivo(g.p)) g.p = nullptr;
    mGalhos.erase(std::remove_if(mGalhos.begin(), mGalhos.end(), [](const Galho& g) { return !g.p; }), mGalhos.end());

    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Complexidade::kFaseDaComplexidade && !EhClone();
    if (!aqui) {
        if (mNaComplexidade) DesmontarComplexidade();
        return;
    }
    const SDL_FRect campo = Campo();
    if (!mNaComplexidade) {
        mNaComplexidade = true;
        mVezDaComplexidade = 0;
        mOrdemAnterior = -1;
        mRodadaEmCurso = false;
        mProximaRodadaDaComplexidade = Complexidade::kPrimeira;
        mRodadaDaComplexidade = Complexidade::Rodada{};
        mAlvo = Vector2(campo.x + campo.w / 2.0f, campo.y + kYDoProfessorNaComplexidade);
        // O grafico, no canto de cima a direita: criado uma vez.
        const Vector2 quadro(campo.x + campo.w - kLarguraDoQuadro / 2.0f - 14.0f, campo.y + kAlturaDoQuadro / 2.0f + 40.0f);
        if (!mQuadroDoGrafico && mScene) {
            auto q = std::make_unique<PainelDeCodigo>(mScene, static_cast<int>(kLarguraDoQuadro), static_cast<int>(kAlturaDoQuadro), 52);
            mQuadroDoGrafico = q.get();
            mScene->AddActor(std::move(q));
            auto nome = std::make_unique<EtiquetaDeCodigo>(mScene, " ", 200, static_cast<int>(kAlturaDoCanto), kOrdemDosCantos);
            mNomeDoGrafico = nome.get();
            mScene->AddActor(std::move(nome));
        }
        if (mQuadroDoGrafico) {
            mQuadroDoGrafico->SetPosition(quadro);
            mQuadroDoGrafico->Pintar(kMolduraDoGrafico, kFundoDoGrafico);
            mQuadroDoGrafico->Mostrar(true);
        }
        if (mNomeDoGrafico) {
            mNomeDoGrafico->SetPosition(quadro + Vector2(0.0f, -kAlturaDoQuadro / 2.0f + 14.0f));
            mNomeDoGrafico->Pintar(kMolduraDoCanto, kFundoDoCanto, kTintaDoCanto);
            mNomeDoGrafico->Escrever(" ");
            mNomeDoGrafico->Mostrar(true);
        }
    }
    const float agora = fsm->GetStateTime();
    if (!mRodadaEmCurso && agora >= mProximaRodadaDaComplexidade) ComecarRodadaDaComplexidade(agora);

    if (mRodadaEmCurso) {
        const Complexidade::Rodada& r = mRodadaDaComplexidade;
        bool acabou = false;
        switch (r.ordem) {
            case Complexidade::Ordem::Logaritmica:
                // A busca binaria: le o meio, descarta a metade em que o aluno nao esta.
                if (!mBusca.Vazio() && agora >= mProximoMeio) {
                    const int alvo = Complexidade::CasaDoAngulo(AnguloAteOAluno(GetPosition()), r.n);
                    const int meio = Complexidade::Meio(mBusca);
                    Operar(meio, meio == alvo);
                    mBusca = Complexidade::Descartar(mBusca, alvo);
                    ++mMeiosLidos;
                    mProximoMeio = agora + Complexidade::kEntreLeituras * mDegrauDaComplexidade.ritmo;
                }
                acabou = mBusca.Vazio() && agora >= mProximoMeio;
                break;
            case Complexidade::Ordem::Exponencial:
                if (agora >= mProximaDobra) {
                    if (mDobras == 0) {
                        // A raiz: um tiro no aluno, que vai dobrar n vezes.
                        const float angulo = AnguloAteOAluno(GetPosition());
                        const float rad = Math::ToRadians(angulo);
                        const Vector2 de = GetPosition() + Vector2(Math::Cos(rad), Math::Sin(rad)) * kRaioDoLeque;
                        for (BossProjectile* p : DispararEm(Complexidade::kGatilhoDaOperacao, de, 1, angulo, 0.0f, mDegrauDaComplexidade.rapidez)) {
                            VestirOperacao(p);
                            mGalhos.push_back(Galho{p, angulo});
                        }
                    } else {
                        Dobrar();
                    }
                    ++mDobras;
                    mProximaDobra = agora + Complexidade::kEntreDobras * mDegrauDaComplexidade.ritmo;
                }
                acabou = mDobras > r.n;
                break;
            default:
                while (mProximaLeitura < mLeituras.size() && agora >= mLeituras[mProximaLeitura].quando) {
                    // A leitura sem casa marcada e o v[i]: a casa em que o aluno estiver.
                    const int marcada = mLeituras[mProximaLeitura].casa;
                    const int casa = marcada >= 0 ? marcada : Complexidade::CasaDoAngulo(AnguloAteOAluno(GetPosition()), r.n);
                    Operar(casa, marcada < 0);
                    ++mProximaLeitura;
                }
                acabou = mProximaLeitura >= mLeituras.size();
                break;
        }
        if (acabou) {
            mRodadaEmCurso = false;
            mProximaRodadaDaComplexidade = agora + mDegrauDaComplexidade.depois + Random::GetFloatRange(0.0f, 0.4f);
        }
    }
    if (mVezDaComplexidade > 0) {
        DesenharLeque(agora);
        DesenharGrafico();
    }
}

// A final: as excecoes.

namespace {
    constexpr SDL_Color kMolduraDoFinally{255, 206, 92, 255};
    constexpr SDL_Color kFundoDoFinally{90, 64, 16, 140};
    constexpr SDL_Color kMolduraDoCatch{96, 222, 112, 255};
    constexpr SDL_Color kFundoDoCatch{20, 60, 36, 150};
    constexpr int kOrdemDasFaixas = 52;
}

Vector2 Luizinho::MeioDaFaixa(const int i) const {
    const SDL_FRect campo = Campo();
    const Excecao::Retangulo f = Excecao::Faixa(i, campo.w);
    return Vector2(campo.x + f.x + f.w / 2.0f, campo.y + f.y + f.h / 2.0f);
}

void Luizinho::PintarFaixa(const int i, const SDL_Color moldura, const SDL_Color fundo) {
    if (i < 0 || i >= Excecao::kMaiorPilha) return;
    const FaixaEmCampo& f = mFaixas[static_cast<size_t>(i)];
    if (f.painel) f.painel->Pintar(moldura, fundo);
    if (f.canto) f.canto->Pintar(moldura, kFundoDoCanto, kTintaDoCanto);
}

void Luizinho::Empilhar(const int i) {
    if (i < 0 || i >= Excecao::kMaiorPilha || i >= static_cast<int>(mRodada.pilha.size())) return;
    const SDL_FRect campo = Campo();
    const Excecao::Retangulo r = Excecao::Faixa(i, campo.w);
    FaixaEmCampo& f = mFaixas[static_cast<size_t>(i)];

    auto painel = std::make_unique<PainelDeCodigo>(mScene, static_cast<int>(r.w), static_cast<int>(r.h), kOrdemDasFaixas);
    painel->SetPosition(MeioDaFaixa(i));
    painel->Pintar(kMolduraDoPainel, kFundoDoPainel);
    f.painel = painel.get();
    mScene->AddActor(std::move(painel));

    const std::string texto = Excecao::Rotulo(mRodada.pilha[static_cast<size_t>(i)]);
    const float largura = LarguraDoCanto(texto);
    auto canto = std::make_unique<EtiquetaDeCodigo>(mScene, texto, static_cast<int>(largura),
                                                    static_cast<int>(kAlturaDoCanto), kOrdemDosCantos);
    canto->SetPosition(Vector2(campo.x + r.x + 10.0f + largura / 2.0f, campo.y + r.y + r.h / 2.0f));
    canto->Pintar(kMolduraDoCanto, kFundoDoCanto, kTintaDoCanto);
    f.canto = canto.get();
    mScene->AddActor(std::move(canto));
}

void Luizinho::Desempilhar(const int i, const bool quebrando) {
    if (i < 0 || i >= Excecao::kMaiorPilha) return;
    FaixaEmCampo& f = mFaixas[static_cast<size_t>(i)];
    if (!f.painel && !f.canto) return;

    if (quebrando) {
        // A faixa se quebra, e os destrocos caem numa cortina.
        const SDL_FRect campo = Campo();
        const auto xs = Excecao::Cortina(PosicaoDoAluno().x - campo.x, campo.w, mDegrauDasExcecoes.entreDestrocos);
        const float y = MeioDaFaixa(i).y;
        const auto cacos = DispararEm(Excecao::kGatilhoDoDestroco, Vector2(campo.x + campo.w / 2.0f, y),
                                      static_cast<int>(xs.size()), 90.0f, 0.0f, mDegrauDasExcecoes.rapidez);
        for (size_t k = 0; k < cacos.size() && k < xs.size(); ++k) {
            cacos[k]->SetPosition(Vector2(campo.x + xs[k], y));
            PorCara(cacos[k], Excecao::kCaraDoDestroco);
        }
    }
    if (f.painel) f.painel->SetState(ActorState::Destroy);
    if (f.canto) f.canto->SetState(ActorState::Destroy);
    f = FaixaEmCampo{};
}

void Luizinho::DesmontarExcecoes() {
    for (int i = 0; i < Excecao::kMaiorPilha; ++i) Desempilhar(i, false);
    if (Vivo(mExcecao)) mExcecao->Kill();
    mExcecao = nullptr;
    RestaurarAnuncio(Excecao::kFaseDasExcecoes);
    mNasExcecoes = false;
    mRodadaDaVez = -1;
    mPassos.clear();
    mAlvo.reset();
}

void Luizinho::ComecarRodada(const int k) {
    // O que sobrou da rodada anterior retorna, sem quebrar.
    for (int i = 0; i < Excecao::kMaiorPilha; ++i) Desempilhar(i, false);
    if (Vivo(mExcecao)) mExcecao->Kill();
    mExcecao = nullptr;

    mRodadaDaVez = k;
    mRodada = Excecao::RodadaDaLuta(mSorteioDasRodadas, k);

    mPassos = Excecao::Propagar(mRodada);
    mProximoPasso = 0;
    mEmpilhados = 0;
    mLancou = false;
    mRetornou = false;
    mCapturouEm = -1;
    const float fracao = static_cast<float>(Sorteio() % 1001u) / 1000.0f;
    mXDaRodada = Campo().x + Excecao::kXMinimo + fracao * (Excecao::kXMaximo - Excecao::kXMinimo);
    TrocarAnuncio(Excecao::kFaseDasExcecoes, Excecao::PlacarDasChamadas(mRodada));
}

void Luizinho::ExecutarPasso(const Excecao::Passo& p) {
    switch (p.o) {
        case Excecao::Acontece::Finally: {
            // O finally roda de qualquer jeito: um anel de onde a excecao passa.
            PintarFaixa(p.quadro, kMolduraDoFinally, kFundoDoFinally);
            const Vector2 onde(mXDaRodada - Excecao::kAEsquerda, MeioDaFaixa(p.quadro).y);
            const int n = Excecao::kAnelDoFinally;
            const auto anel = DispararEm(Excecao::kGatilhoDoFinally, onde, n,
                                         Coercao::CentroDoAnel(n, AnguloAteOAluno(onde)), Coercao::AberturaDoAnel(n),
                                         mDegrauDasExcecoes.rapidez);
            for (BossProjectile* a : anel) PorCara(a, Excecao::kCaraDoFinally);
            break;
        }
        case Excecao::Acontece::Desempilha:
            Desempilhar(p.quadro, true);
            // O ponto de execucao sobe para quem chamou.
            if (p.quadro > 0) mAlvo = Vector2(mXDaRodada, MeioDaFaixa(p.quadro - 1).y);
            break;
        case Excecao::Acontece::Captura:
            mCapturouEm = p.quadro;
            PintarFaixa(p.quadro, kMolduraDoCatch, kFundoDoCatch);
            if (Vivo(mExcecao)) mExcecao->Kill();
            mExcecao = nullptr;
            mAlvo = Vector2(mXDaRodada, MeioDaFaixa(p.quadro).y);
            TrocarAnuncio(Excecao::kFaseDasExcecoes, Excecao::PlacarDoDesfecho(mRodada));
            break;
        case Excecao::Acontece::Escapa:
            // Ninguem tratou: o programa capota.
            if (Vivo(mExcecao)) mExcecao->Kill();
            mExcecao = nullptr;
            TrocarAnuncio(Excecao::kFaseDasExcecoes, Excecao::PlacarDoDesfecho(mRodada));
            break;
    }
}

void Luizinho::AtualizarExcecoes() {
    if (mExcecao && !Vivo(mExcecao)) mExcecao = nullptr;

    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Excecao::kFaseDasExcecoes && !EhClone();
    if (!aqui) {
        if (mNasExcecoes) DesmontarExcecoes();
        return;
    }
    if (!mNasExcecoes) {
        mNasExcecoes = true;
        mRodadaDaVez = -1;
        mAlvo = Vector2(Campo().x + Campo().w / 2.0f, MeioDaFaixa(0).y);
    }

    const float t = fsm->GetStateTime();
    if (t < Excecao::kPrimeiro) return;
    const int k = std::min(Excecao::kRodadas - 1, static_cast<int>((t - Excecao::kPrimeiro) / Excecao::kPorRodada));
    if (k != mRodadaDaVez) ComecarRodada(k);
    const float u = t - (Excecao::kPrimeiro + static_cast<float>(k) * Excecao::kPorRodada);

    // As chamadas empilham, e o professor desce ate a mais funda.
    while (mEmpilhados < mRodada.pilha.size() && u >= static_cast<float>(mEmpilhados) * Excecao::kEntreChamadas) {
        Empilhar(static_cast<int>(mEmpilhados));
        mAlvo = Vector2(mXDaRodada, MeioDaFaixa(static_cast<int>(mEmpilhados)).y);
        ++mEmpilhados;
    }

    // O lancamento: a excecao nasce no professor, na faixa de quem lanca.
    const int ultimo = static_cast<int>(mRodada.pilha.size()) - 1;
    if (!mLancou && u >= Excecao::kLancamento) {
        mLancou = true;
        TrocarAnuncio(Excecao::kFaseDasExcecoes, Excecao::PlacarDoLancamento(mRodada));
        const auto criada = DispararEm(Excecao::kGatilhoDaExcecao,
                                       Vector2(mXDaRodada - Excecao::kAEsquerda, MeioDaFaixa(ultimo).y), 1);
        if (!criada.empty()) {
            mExcecao = criada.front();
            mExcecao->DefinirEnfeite(true);
            PorCara(mExcecao, Excecao::CaraDaExcecao(mRodada.lancada));
        }
    }
    if (!mLancou) return;

    // Ela sobe uma faixa a cada kPorQuadro.
    const float subindo = u - Excecao::kLancamento;
    if (Vivo(mExcecao)) {
        const float passo = Excecao::kAlturaDaFaixa + Excecao::kEntreFaixas;
        const float y = MeioDaFaixa(ultimo).y - passo * subindo / Excecao::kPorQuadro;
        mExcecao->SetPosition(Vector2(mXDaRodada - Excecao::kAEsquerda, y));
    }
    while (mProximoPasso < mPassos.size() && subindo >= mPassos[mProximoPasso].quando) {
        ExecutarPasso(mPassos[mProximoPasso++]);
    }

    // Quem sobrou retorna, depois do desfecho.
    if (!mRetornou && !mPassos.empty() && subindo >= mPassos.back().quando + Excecao::kRetorno) {
        mRetornou = true;
        for (int i = 0; i < Excecao::kMaiorPilha; ++i) Desempilhar(i, false);
    }
}
