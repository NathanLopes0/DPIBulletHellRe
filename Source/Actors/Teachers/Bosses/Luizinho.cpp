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
    mDegrauDasAmarracoes = Amarracao::DegrauDo(nivel);
    mDegrauDosTipos = Coercao::DegrauDo(nivel);
    mDegrauDoTermina = Termina::DegrauDo(nivel);
    mDegrauDasExcecoes = Excecao::DegrauDo(nivel);
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Amarracao::kFaseDasAmarracoes);
        mPrograma = Amarracao::SortearPrograma(sorteio);
        mFAEsquerda = sorteio.Chance(0.5f);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Coercao::kFaseDosTipos);
        mOrdemDosTipos = Coercao::OrdemDosComandos(sorteio);
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

    AtualizarAmarracoes();
    AtualizarTipos();
    AtualizarTermina();
    AtualizarExcecoes();
}

void Luizinho::AoTerminarABatalha() {
    // A tela da nota fica limpa: os projeteis a batalha ja levou.
    for (PainelDeCodigo* p : mPaineis) if (p) p->Mostrar(false);
    for (EtiquetaDeCodigo* c : mCantos) if (c) c->Mostrar(false);
    LimparConta();
    if (mLinhas) mLinhas->Limpar();
    if (mCaixaDoTermina) mCaixaDoTermina->Mostrar(false);
    for (EtiquetaDeCodigo* e : {mPerguntaDoTermina, mRespostaDoTermina, mContador}) if (e) e->Mostrar(false);
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

// A fase 1: as amarracoes.

namespace {
    constexpr SDL_Color kMolduraDoInterno{120, 140, 190, 200};
    constexpr SDL_Color kFundoDoInterno{30, 38, 62, 120};
    constexpr SDL_Color kCantoOlhado{96, 100, 116, 230};     // a busca passou e nao achou x
    constexpr SDL_Color kCantoDoDono{255, 206, 92, 255};     // quem declara o x achado
    constexpr SDL_Color kLinhaDaBusca{150, 200, 255, 220};
    constexpr SDL_Color kLinhaDaChamadaDeF{220, 140, 255, 220};   // g chama f
    constexpr float kYDoProfessorNoGlobal = 95.0f;

    size_t IndiceDoBloco(const Amarracao::Bloco b) { return static_cast<size_t>(b); }
}

Vector2 Luizinho::OndeFicaOCanto(const Amarracao::Bloco b) const {
    const SDL_FRect campo = Campo();
    const auto c = Amarracao::OndeFicaOCanto(b, mFAEsquerda, campo.w, campo.h, mLarguraDosCantos[IndiceDoBloco(b)]);
    return Vector2(campo.x + c.x, campo.y + c.y);
}

void Luizinho::PintarCanto(const Amarracao::Bloco b, const SDL_Color moldura) {
    if (EtiquetaDeCodigo* c = mCantos[IndiceDoBloco(b)]) c->Pintar(moldura, kFundoDoCanto, kTintaDoCanto);
}

void Luizinho::MontarAmarracoes() {
    const SDL_FRect campo = Campo();
    for (int i = 0; i < Amarracao::kBlocos; ++i) {
        const auto b = static_cast<Amarracao::Bloco>(i);
        const size_t k = IndiceDoBloco(b);
        const auto r = Amarracao::Area(b, mFAEsquerda, campo.w, campo.h);
        const bool interno = b == Amarracao::Bloco::Interno;
        // O global e o campo: nao tem painel, so o canto.
        if (b != Amarracao::Bloco::Global && !mPaineis[k] && mScene) {
            auto painel = std::make_unique<PainelDeCodigo>(mScene, static_cast<int>(r.w), static_cast<int>(r.h),
                                                           interno ? kOrdemDoInterno : kOrdemDosPaineis);
            mPaineis[k] = painel.get();
            mScene->AddActor(std::move(painel));
        }
        if (!mCantos[k] && mScene) {
            const std::string texto = Amarracao::Declaracao(mPrograma, b);
            mLarguraDosCantos[k] = LarguraDoCanto(texto);
            auto canto = std::make_unique<EtiquetaDeCodigo>(mScene, texto, static_cast<int>(mLarguraDosCantos[k]),
                                                            static_cast<int>(kAlturaDoCanto), kOrdemDosCantos);
            mCantos[k] = canto.get();
            mScene->AddActor(std::move(canto));
        }
        if (mPaineis[k]) {
            mPaineis[k]->SetPosition(Vector2(campo.x + r.x + r.w / 2.0f, campo.y + r.y + r.h / 2.0f));
            mPaineis[k]->Pintar(interno ? kMolduraDoInterno : kMolduraDoPainel, interno ? kFundoDoInterno : kFundoDoPainel);
            mPaineis[k]->Mostrar(true);
        }
        if (mCantos[k]) {
            mCantos[k]->SetPosition(OndeFicaOCanto(b));
            PintarCanto(b, kMolduraDoCanto);
            mCantos[k]->Mostrar(true);
        }
    }
}

void Luizinho::DesmontarAmarracoes() {
    for (PainelDeCodigo* p : mPaineis) if (p) p->Mostrar(false);
    for (EtiquetaDeCodigo* c : mCantos) if (c) c->Mostrar(false);
    mComandoEmCurso = false;
    mBuscaDoComando.clear();
    RestaurarAnuncio(Amarracao::kFaseDasAmarracoes);
    mAlvo.reset();
    mNasAmarracoes = false;
}

void Luizinho::ComecarComando(const float agora) {
    // Do meio da fase em diante, a pegadinha: g() chama f(), no escopo estatico e depois no dinamico.
    if (mChamadas < 2 && agora >= Amarracao::kInicioDaChamada) {
        mComando = Amarracao::Chamada(mChamadas == 0 ? Amarracao::Regra::Estatico : Amarracao::Regra::Dinamico);
        ++mChamadas;
    } else {
        mComando = Amarracao::Proximo(Sorteio(), mBlocoAnterior);
    }
    mBlocoAnterior = mComando.onde;
    mBuscaDoComando = Amarracao::Busca(mPrograma, mComando);
    mSaltos = 0;
    mRajadas = 0;
    mInicioDoComando = agora;
    mComandoEmCurso = true;
    for (int i = 0; i < Amarracao::kBlocos; ++i) PintarCanto(static_cast<Amarracao::Bloco>(i), kMolduraDoCanto);
    TrocarAnuncio(Amarracao::kFaseDasAmarracoes, Amarracao::Placar(mPrograma, mComando, false));

    const SDL_FRect campo = Campo();
    const auto onde = Amarracao::OndeFicaOProfessor(mComando.onde, mFAEsquerda, campo.w, campo.h);
    mAlvo = Vector2(campo.x + onde.x, campo.y + onde.y);
}

void Luizinho::Responder() {
    if (mBuscaDoComando.empty()) return;
    const Amarracao::Bloco dono = mBuscaDoComando.back();
    const int valor = mPrograma.Valor(dono);
    if (valor <= 0) return;
    const Vector2 de = OndeFicaOCanto(dono);
    const float rapidez = mDegrauDasAmarracoes.velocidade / Amarracao::kVelocidade;
    const float desvio = Amarracao::DesvioDaRajada(Random::GetFloatRange(0.0f, 1.0f));
    for (BossProjectile* p : DispararEm(Amarracao::kGatilhoDaResposta, de, valor, AnguloAteOAluno(de) + desvio,
                                        Amarracao::AberturaDoLeque(valor), rapidez)) {
        PorCara(p, "X" + std::to_string(valor));
    }
}

void Luizinho::DesenharBusca() {
    if (!mLinhas || mBuscaDoComando.empty()) return;
    // A chamada: g chama f, e o atira(x) roda em f.
    if (mComando.chamada) {
        mLinhas->Acrescentar(OndeFicaOCanto(Amarracao::Bloco::G), OndeFicaOCanto(Amarracao::Bloco::F), kLinhaDaChamadaDeF);
    }
    // A busca, do professor de canto em canto ate o dono.
    Vector2 de = GetPosition();
    for (size_t i = 0; i < mSaltos && i < mBuscaDoComando.size(); ++i) {
        const Vector2 ate = OndeFicaOCanto(mBuscaDoComando[i]);
        mLinhas->Acrescentar(de, ate, kLinhaDaBusca);
        de = ate;
    }
}

void Luizinho::AtualizarAmarracoes() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Amarracao::kFaseDasAmarracoes && !EhClone();
    if (!aqui) {
        if (mNasAmarracoes) DesmontarAmarracoes();
        return;
    }
    if (!mNasAmarracoes) {
        mNasAmarracoes = true;
        mComandoEmCurso = false;
        mProximoComando = Amarracao::kPrimeiro;
        mChamadas = 0;
        mBlocoAnterior = Amarracao::Bloco::F;
        MontarAmarracoes();
        const SDL_FRect campo = Campo();
        mAlvo = Vector2(campo.x + campo.w / 2.0f, campo.y + kYDoProfessorNoGlobal);
    }
    const float agora = fsm->GetStateTime();
    if (!mComandoEmCurso && agora >= mProximoComando) ComecarComando(agora);

    if (mComandoEmCurso) {
        const float u = agora - mInicioDoComando;
        const float porSalto = mDegrauDasAmarracoes.porSalto;
        // A busca olha um bloco por vez: quem nao tem x apaga, e o dono acende.
        while (mSaltos < mBuscaDoComando.size() &&
               u >= Amarracao::kAntesDaBusca + porSalto * static_cast<float>(mSaltos + 1)) {
            const Amarracao::Bloco b = mBuscaDoComando[mSaltos];
            ++mSaltos;
            const bool dono = mSaltos == mBuscaDoComando.size();
            PintarCanto(b, dono ? kCantoDoDono : kCantoOlhado);
            if (dono) TrocarAnuncio(Amarracao::kFaseDasAmarracoes, Amarracao::Placar(mPrograma, mComando, true));
        }
        const float resposta = Amarracao::kAntesDaBusca + porSalto * static_cast<float>(mBuscaDoComando.size()) +
                               Amarracao::kAntesDaResposta;
        while (mSaltos == mBuscaDoComando.size() && mRajadas < mDegrauDasAmarracoes.rajadas &&
               u >= resposta + Amarracao::kEntreRajadas * static_cast<float>(mRajadas)) {
            Responder();
            ++mRajadas;
        }
        if (mRajadas >= mDegrauDasAmarracoes.rajadas) {
            mComandoEmCurso = false;
            mProximoComando = agora + Random::GetFloatRange(Amarracao::kMenorPausa, Amarracao::kMaiorPausa);
        }
    }
    DesenharBusca();
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

// A fase 3: o programa Termina.

namespace {
    constexpr SDL_Color kMolduraDoTermina{150, 170, 220, 230};
    constexpr SDL_Color kFundoDoTermina{20, 26, 42, 220};
    constexpr SDL_Color kRespostaPensando{150, 156, 176, 255};
    constexpr SDL_Color kRespostaSim{110, 230, 130, 255};
    constexpr SDL_Color kRespostaNao{255, 110, 90, 255};
    constexpr float kLarguraDaCaixa = 280.0f;
    constexpr float kAlturaDaCaixa = 96.0f;
    constexpr float kMenorXNoTermina = 360.0f;
    constexpr float kMaiorXNoTermina = 840.0f;
}

void Luizinho::MontarTermina() {
    const SDL_FRect campo = Campo();
    const Vector2 caixa(campo.x + campo.w - kLarguraDaCaixa / 2.0f - 16.0f, campo.y + kAlturaDaCaixa / 2.0f + 40.0f);
    if (!mCaixaDoTermina && mScene) {
        auto painel = std::make_unique<PainelDeCodigo>(mScene, static_cast<int>(kLarguraDaCaixa),
                                                       static_cast<int>(kAlturaDaCaixa), kOrdemDosPaineis);
        mCaixaDoTermina = painel.get();
        mScene->AddActor(std::move(painel));
        auto pergunta = std::make_unique<EtiquetaDeCodigo>(mScene, " ", 230, static_cast<int>(kAlturaDoCanto), kOrdemDosCantos);
        mPerguntaDoTermina = pergunta.get();
        mScene->AddActor(std::move(pergunta));
        auto resposta = std::make_unique<EtiquetaDeCodigo>(mScene, " ", 96, 32, kOrdemDosCantos);
        mRespostaDoTermina = resposta.get();
        mScene->AddActor(std::move(resposta));
        auto contador = std::make_unique<EtiquetaDeCodigo>(mScene, " ", 120, static_cast<int>(kAlturaDoCanto), kOrdemDosCantos);
        mContador = contador.get();
        mScene->AddActor(std::move(contador));
    }
    if (mCaixaDoTermina) {
        mCaixaDoTermina->SetPosition(caixa);
        mCaixaDoTermina->Pintar(kMolduraDoTermina, kFundoDoTermina);
        mCaixaDoTermina->Mostrar(true);
    }
    if (mPerguntaDoTermina) {
        mPerguntaDoTermina->SetPosition(caixa + Vector2(0.0f, -20.0f));
        mPerguntaDoTermina->Pintar(kMolduraDoCanto, kFundoDoCanto, kTintaDoCanto);
        mPerguntaDoTermina->Escrever("Termina(P, n)");
        mPerguntaDoTermina->Mostrar(true);
    }
    if (mRespostaDoTermina) {
        mRespostaDoTermina->SetPosition(caixa + Vector2(0.0f, 20.0f));
        mRespostaDoTermina->Mostrar(true);
    }
    MostrarVeredito(std::nullopt);
    if (mContador) mContador->Mostrar(false);
}

void Luizinho::DesmontarTermina() {
    if (mCaixaDoTermina) mCaixaDoTermina->Mostrar(false);
    for (EtiquetaDeCodigo* e : {mPerguntaDoTermina, mRespostaDoTermina, mContador}) if (e) e->Mostrar(false);
    RestaurarAnuncio(Termina::kFaseDoTermina);
    mAlvo.reset();
    mNoTermina = false;
    mRodadaDoTerminaEmCurso = false;
    mNoParadoxo = false;
}

void Luizinho::MostrarVeredito(const std::optional<bool> veredito) {
    mVeredito = veredito;
    if (!mRespostaDoTermina) return;
    const SDL_Color cor = !veredito ? kRespostaPensando : *veredito ? kRespostaSim : kRespostaNao;
    mRespostaDoTermina->Escrever(Termina::Resposta(veredito));
    mRespostaDoTermina->Pintar(cor, kFundoDoCanto, cor);
}

void Luizinho::EscreverContador(const std::string& texto, const SDL_Color moldura) {
    if (!mContador) return;
    mContador->Escrever(texto);
    mContador->Pintar(moldura, kFundoDoCanto, kTintaDoCanto);
    mContador->Mostrar(true);
}

void Luizinho::ComecarRodadaDoTermina(const float agora) {
    mRodadaDoTermina = Termina::Proxima(Sorteio(), mProgramaAnterior);
    const Termina::Rodada& r = mRodadaDoTermina;
    mProgramaAnterior = static_cast<int>(r.programa);
    mValoresDoLaco = Termina::Valores(r.programa, r.n, Termina::kMaisVoltas);
    mVoltasDadas = 0;
    mCtrlC = Random::GetFloatRange(Termina::kMenorCtrlC, Termina::kMaiorCtrlC);
    mFimDaRodadaDoTermina = -1.0f;
    mInicioDaRodadaDoTermina = agora;
    mRodadaDoTerminaEmCurso = true;
    MostrarVeredito(std::nullopt);
    if (mPerguntaDoTermina) mPerguntaDoTermina->Escrever(Termina::Pergunta(r));
    TrocarAnuncio(Termina::kFaseDoTermina, Termina::Placar(r, std::nullopt));
    EscreverContador("n = " + std::to_string(r.n), kMolduraDoCanto);

    // O professor muda de lugar a cada rodada: a espiral sai de outro ponto.
    const SDL_FRect campo = Campo();
    mAlvo = Vector2(campo.x + Random::GetFloatRange(kMenorXNoTermina, kMaiorXNoTermina), campo.y + kYDoProfessorNoTermina);
}

void Luizinho::ComecarParadoxo(const float agora) {
    mParadoxoFeito = true;
    mNoParadoxo = true;
    mVirada = -1;
    mEstourou = false;
    mInicioDaRodadaDoTermina = agora;
    mProximaVolta = agora;
    MostrarVeredito(std::nullopt);
    if (mPerguntaDoTermina) mPerguntaDoTermina->Escrever("Termina(Paradoxo)");
    TrocarAnuncio(Termina::kFaseDoTermina, Termina::PlacarDoParadoxo(std::nullopt, false));
    EscreverContador("Paradoxo()", kMolduraDoCanto);
    const SDL_FRect campo = Campo();
    mAlvo = Vector2(campo.x + campo.w / 2.0f, campo.y + kYDoProfessorNoTermina);
}

void Luizinho::DarVolta() {
    const int n = mDegrauDoTermina.bracos;
    const float rapidez = mDegrauDoTermina.velocidade / Termina::kVelocidade;
    DispararEm(Termina::kGatilhoDaVolta, GetPosition(), n, mGiroDaEspiral, Coercao::AberturaDoAnel(n), rapidez);
    mGiroDaEspiral += Termina::kGiroPorVolta;
}

void Luizinho::EstourarOTermina() {
    mEstourou = true;
    // O Termina se contradiz e estoura: um anel de cacos, de onde ele estava.
    const Vector2 de = mCaixaDoTermina ? mCaixaDoTermina->GetPosition() : GetPosition();
    const int n = Termina::kCacosDaContradicao;
    DispararEm(Termina::kGatilhoDaContradicao, de, n, Coercao::CentroDoAnel(n, AnguloAteOAluno(de)),
               Coercao::AberturaDoAnel(n), mDegrauDoTermina.velocidade / Termina::kVelocidade);
    if (mCaixaDoTermina) mCaixaDoTermina->Mostrar(false);
    if (mPerguntaDoTermina) mPerguntaDoTermina->Mostrar(false);
    if (mRespostaDoTermina) mRespostaDoTermina->Mostrar(false);
    TrocarAnuncio(Termina::kFaseDoTermina, Termina::PlacarDoParadoxo(std::nullopt, true));
    EscreverContador("repete", kRespostaNao);
}

void Luizinho::AtualizarTermina() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Termina::kFaseDoTermina && !EhClone();
    if (!aqui) {
        if (mNoTermina) DesmontarTermina();
        return;
    }
    const SDL_FRect campo = Campo();
    if (!mNoTermina) {
        mNoTermina = true;
        mRodadaDoTerminaEmCurso = false;
        mProximaRodadaDoTermina = Termina::kPrimeira;
        mProgramaAnterior = -1;
        mParadoxoFeito = false;
        mNoParadoxo = false;
        mGiroDaEspiral = 90.0f;
        MontarTermina();
        mAlvo = Vector2(campo.x + campo.w / 2.0f, campo.y + kYDoProfessorNoTermina);
    }
    const float agora = fsm->GetStateTime();
    if (!mRodadaDoTerminaEmCurso && !mNoParadoxo && agora >= mProximaRodadaDoTermina) {
        if (!mParadoxoFeito && agora >= Termina::kInicioDoParadoxo) ComecarParadoxo(agora);
        else ComecarRodadaDoTermina(agora);
    }

    if (mNoParadoxo) {
        // O Termina responde, o Paradoxo faz o contrario, e ele muda de ideia - ate estourar.
        const float u = agora - mInicioDaRodadaDoTermina;
        if (!mEstourou) {
            int k = -1;
            float t = Termina::kPensando;
            if (u >= t) {
                k = 0;
                while (k < Termina::kViradas && u >= t + Termina::DuracaoDaVirada(k)) t += Termina::DuracaoDaVirada(k++);
            }
            if (k != mVirada) {
                mVirada = k;
                if (k >= Termina::kViradas) {
                    EstourarOTermina();
                } else if (k >= 0) {
                    const bool v = Termina::VereditoDaVirada(k);
                    MostrarVeredito(v);
                    TrocarAnuncio(Termina::kFaseDoTermina, Termina::PlacarDoParadoxo(v, false));
                    EscreverContador(v ? "repete" : "para", v ? kRespostaNao : kRespostaSim);
                }
            }
        }
        // Disse que para, e ele repete; depois do estouro, repete ate a fase acabar.
        const bool repete = mEstourou || (mVirada >= 0 && mVeredito && *mVeredito);
        if (repete && agora >= mProximaVolta) {
            DarVolta();
            mProximaVolta = agora + mDegrauDoTermina.porVolta;
        }
        if (!repete) mProximaVolta = agora;
    } else if (mRodadaDoTerminaEmCurso) {
        const Termina::Rodada& r = mRodadaDoTermina;
        const float u = agora - mInicioDaRodadaDoTermina;
        if (!mVeredito && u >= Termina::kPensando) {
            MostrarVeredito(Termina::Para(r.programa, r.n));
            TrocarAnuncio(Termina::kFaseDoTermina, Termina::Placar(r, mVeredito));
        }
        if (mVeredito && mFimDaRodadaDoTermina < 0.0f) {
            // O laco roda: uma volta a cada porVolta, o contador mostra o n.
            const float rodando = u - Termina::kPensando - 0.15f;
            const bool para = *mVeredito;
            while (rodando >= mDegrauDoTermina.porVolta * static_cast<float>(mVoltasDadas) &&
                   mVoltasDadas + 1 < static_cast<int>(mValoresDoLaco.size()) && (para || rodando < mCtrlC)) {
                ++mVoltasDadas;
                DarVolta();
                EscreverContador("n = " + std::to_string(mValoresDoLaco[static_cast<size_t>(mVoltasDadas)]), kMolduraDoCanto);
            }
            if (para && mVoltasDadas + 1 >= static_cast<int>(mValoresDoLaco.size())) {
                EscreverContador("fim", kRespostaSim);
                mFimDaRodadaDoTermina = agora;
            } else if (!para && rodando >= mCtrlC) {
                EscreverContador("^C", kRespostaNao);
                mFimDaRodadaDoTermina = agora;
            }
        }
        if (mFimDaRodadaDoTermina >= 0.0f && agora >= mFimDaRodadaDoTermina + Termina::kDepois) {
            mRodadaDoTerminaEmCurso = false;
            mProximaRodadaDoTermina = agora;
        }
    }
    // O contador acompanha o professor, embaixo do cartao.
    if (mContador) mContador->SetPosition(GetPosition() + Vector2(0.0f, 72.0f));
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
