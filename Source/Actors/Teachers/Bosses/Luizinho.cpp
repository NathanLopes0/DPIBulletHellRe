//
// O chefe da INF 340 - Linguagens de Programacao.
//

#include "Luizinho.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "../../../Random.h"
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

    // A fase 1: as cores do texto do programa.
    constexpr SDL_Color kMolduraDoPainel{100, 120, 165, 190};
    constexpr SDL_Color kFundoDoPainel{24, 30, 48, 110};
    constexpr SDL_Color kMolduraDoBuraco{96, 222, 112, 255};      // outra variavel: aqui nao chove
    constexpr SDL_Color kFundoDoBuraco{20, 60, 36, 130};

    constexpr SDL_Color kMolduraDoCanto{120, 140, 180, 230};
    constexpr SDL_Color kFundoDoCanto{28, 32, 46, 225};
    constexpr SDL_Color kTintaDoCanto{214, 226, 250, 255};
    constexpr SDL_Color kMolduraOlhada{150, 150, 160, 230};       // a busca passou e nao achou
    constexpr SDL_Color kTintaOlhada{160, 164, 176, 255};
    constexpr SDL_Color kMolduraAchada{255, 214, 72, 255};        // a declaracao achada
    constexpr SDL_Color kFundoAchado{70, 62, 24, 235};
    constexpr SDL_Color kTintaAchada{255, 240, 190, 255};
    constexpr SDL_Color kTintaDoBuraco{200, 255, 210, 255};

    constexpr SDL_Color kLinhaDaBusca{150, 160, 190, 220};
    constexpr SDL_Color kLinhaAchada{255, 214, 72, 255};
    constexpr SDL_Color kLinhaDaChamada{90, 200, 240, 220};

    constexpr int kTotalDeBlocos = Escopo::kBlocos;
    constexpr Escopo::Bloco kBlocosDoPrograma[kTotalDeBlocos] = {
        Escopo::Bloco::Global, Escopo::Bloco::F, Escopo::Bloco::G, Escopo::Bloco::Interno, Escopo::Bloco::H};
}

Luizinho::Luizinho(Scene* scene) : Boss(scene) {
}

std::optional<Vector2> Luizinho::AlvoDoMovimento() const {
    return mAlvo;
}

void Luizinho::SortearALuta() {
    mLutaSorteada = true;
    const Dificuldade::Nivel nivel = NivelDeDificuldade();
    mDegrauDoEscopo = Escopo::DegrauDo(nivel);
    mDegrauDosTipos = Coercao::DegrauDo(nivel);
    mDegrauDaMemoria = Memoria::DegrauDo(nivel);
    mDegrauDasExcecoes = Excecao::DegrauDo(nivel);
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Escopo::kFaseDoEscopo);
        mOrdemDoEscopo = Escopo::OrdemDosComandos(sorteio);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Coercao::kFaseDosTipos);
        mOrdemDosTipos = Coercao::OrdemDosComandos(sorteio);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Memoria::kFaseDaMemoria);
        mRoteiro = Memoria::Sortear(sorteio);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Excecao::kFaseDasExcecoes);
        mOrdemDasRodadas = Excecao::OrdemDasRodadas(sorteio);
    }
}

void Luizinho::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();

    // STOP THE WORLD: com o mundo parado, o professor para junto - nem anda nem
    // dispara. A maquina de estados continua (ela e componente, e roda antes
    // daqui), entao a pausa NAO alonga a fase.
    if (mMundoParado) {
        if (auto* rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    } else {
        Boss::OnUpdate(deltaTime);
    }

    if (!mLinhas && mScene) {
        auto ator = std::make_unique<Actor>(mScene);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();

    AtualizarEscopo();
    AtualizarTipos();
    AtualizarMemoria();
    AtualizarExcecoes();
}

void Luizinho::AoTerminarABatalha() {
    // A TELA DA NOTA FICA LIMPA: os projeteis a batalha ja levou; o texto do
    // programa e as linhas sao do professor.
    for (PainelDeCodigo* p : mPaineis) if (p) p->Mostrar(false);
    for (EtiquetaDeCodigo* c : mCantos) if (c) c->Mostrar(false);
    LimparConta();
    if (mLinhas) mLinhas->Limpar();
    for (auto& [nome, etiqueta] : mRaizes) if (etiqueta) etiqueta->Mostrar(false);
    for (const FaixaEmCampo& f : mFaixas) {
        if (f.painel) f.painel->Mostrar(false);
        if (f.canto) f.canto->Mostrar(false);
    }
}

void Luizinho::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O ATAQUE DE GATILHO nasce onde o professor disse, no numero que ele
    // pediu - e so ele: os de relogio das outras fases seguem abaixo.
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

SDL_FRect Luizinho::Campo() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene)) return battle->GetPlayfieldBounds();
    return SDL_FRect{0.0f, 0.0f, Escopo::kLargura, Escopo::kAltura};
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

namespace {
    // AS LEGENDAS DE ABERTURA: o assunto da fase, em portugues, enquanto o
    // placar diz a palavra latina. \xc3\xa7 c cedilha, \xc3\xa3 a til, \xc3\xa1
    // a agudo, \xc3\xa9 e agudo, \xc3\xb3 o agudo, \xc3\xaa e circunflexo.
    const char* const kAberturaDoEscopo =
        "Amarra\xc3\xa7\xc3\xb5" "es: ligar cada nome \xc3\xa0 declara\xc3\xa7\xc3\xa3o dele. Cada comando "
        "procura um nome nos blocos do programa.";
    const char* const kAberturaDosTipos =
        "Sistemas de tipos: quando os tipos n\xc3\xa3o batem, cada linguagem decide se converte, se "
        "recusa ao rodar ou se nem compila.";
    const char* const kAberturaDaMemoria =
        "Ger\xc3\xaa" "ncia de mem\xc3\xb3ria: quem libera o que n\xc3\xa3o se usa mais - o coletor "
        "de lixo, no Java, ou o programador, no C++.";
    const char* const kAberturaDasExcecoes =
        "Exce\xc3\xa7\xc3\xb5" "es: o erro sai de onde aconteceu e sobe a pilha de chamadas at\xc3\xa9 "
        "quem saiba trat\xc3\xa1-lo.";
}

Vector2 Luizinho::PosicaoDoAluno() const {

    if (const auto* battle = dynamic_cast<Battle*>(mScene); battle && battle->Alvo()) {
        return battle->Alvo()->GetPosition();
    }
    const SDL_FRect campo = Campo();
    return Vector2(campo.x + campo.w / 2.0f, campo.y + campo.h - 60.0f);
}

float Luizinho::AnguloAteOAluno(const Vector2 de) const {
    const Vector2 d = PosicaoDoAluno() - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

void Luizinho::Anunciar(const std::string& fase, const std::string& texto) {
    if (mAnunciosDoArquivo.count(fase) == 0) mAnunciosDoArquivo[fase] = AnuncioDaFase(fase);
    if (AnuncioDaFase(fase) != texto) RegistrarAnuncio(fase, texto);
}

void Luizinho::Restaurar(const std::string& fase) {
    const auto it = mAnunciosDoArquivo.find(fase);
    if (it != mAnunciosDoArquivo.end()) RegistrarAnuncio(fase, it->second);
    RegistrarLegenda(fase, "");
}

// ---------------------------------------------------------------------------
// A fase 1: o escopo
// ---------------------------------------------------------------------------

void Luizinho::MontarEscopo() {
    const SDL_FRect campo = Campo();
    for (int i = 0; i < kTotalDeBlocos; ++i) {
        const Escopo::Bloco b = kBlocosDoPrograma[i];
        const Escopo::Retangulo r = Escopo::Area(b, campo.w, campo.h);

        // O PAINEL: o global e o campo inteiro, e o cartao de f e so o canto.
        if (b != Escopo::Bloco::Global && b != Escopo::Bloco::F) {
            const int ordem = (b == Escopo::Bloco::Interno) ? kOrdemDoInterno : kOrdemDosPaineis;
            auto painel = std::make_unique<PainelDeCodigo>(mScene, static_cast<int>(r.w), static_cast<int>(r.h), ordem);
            painel->SetPosition(Vector2(campo.x + r.x + r.w / 2.0f, campo.y + r.y + r.h / 2.0f));
            painel->Pintar(kMolduraDoPainel, kFundoDoPainel);
            mPaineis[static_cast<size_t>(i)] = painel.get();
            mScene->AddActor(std::move(painel));
        }

        const std::string texto = Escopo::Declaracoes(b);
        const bool cartao = (b == Escopo::Bloco::F);
        const int largura = cartao ? static_cast<int>(r.w) : static_cast<int>(Escopo::LarguraDoCanto(texto));
        const int altura = cartao ? static_cast<int>(r.h) : static_cast<int>(Escopo::kAlturaDoCanto);
        auto canto = std::make_unique<EtiquetaDeCodigo>(mScene, texto, largura, altura, kOrdemDosCantos);
        const Escopo::Ponto c = Escopo::Canto(b, campo.w, campo.h);
        canto->SetPosition(Vector2(campo.x + c.x, campo.y + c.y));
        canto->Pintar(kMolduraDoCanto, kFundoDoCanto, kTintaDoCanto);
        mCantos[static_cast<size_t>(i)] = canto.get();
        mScene->AddActor(std::move(canto));
    }
}

void Luizinho::DesmontarEscopo() {
    SecarGotas();
    for (PainelDeCodigo*& p : mPaineis) {
        if (p) p->SetState(ActorState::Destroy);
        p = nullptr;
    }
    for (EtiquetaDeCodigo*& c : mCantos) {
        if (c) c->SetState(ActorState::Destroy);
        c = nullptr;
    }
    Restaurar(Escopo::kFaseDoEscopo);
    mNoEscopo = false;
    mComandoDoEscopo = -1;
    mChuva = Chuva::Esperando;
    mAlvo.reset();
}

void Luizinho::SecarGotas() {
    for (const GotaEmCampo& g : mGotas) if (Vivo(g.p)) g.p->Kill();
    mGotas.clear();
}

void Luizinho::ComecarComando(const int k) {
    SecarGotas();
    mComandoDoEscopo = k;
    mForma = Escopo::Proxima(Variacao::NaOrdem(mOrdemDoEscopo, k));
    mInicioDoComando = Escopo::kPrimeiro + static_cast<float>(k) * Escopo::kPorComando;
    mChuva = Chuva::Esperando;
    Anunciar(Escopo::kFaseDoEscopo, Escopo::Placar(mForma));
    RegistrarLegenda(Escopo::kFaseDoEscopo, Escopo::Legenda(mForma));

    const SDL_FRect campo = Campo();
    const Escopo::Ponto onde = Escopo::OndeFicaOProfessor(mForma, campo.w, campo.h);
    mAlvo = Vector2(campo.x + onde.x, campo.y + onde.y);
}

void Luizinho::ChoverAvisos() {
    // A CHUVA NASCE APAGADA, de uma vez, na regiao inteira: e o aviso de onde
    // a variavel e visivel. Sem colisor ate acender - e sem ser enfeite: o
    // Ponto Extra a leva junto, como leva qualquer ataque.
    const SDL_FRect campo = Campo();
    const auto gotas = Escopo::Gotas(mForma, campo.w, campo.h);
    if (gotas.empty()) return;
    const auto criadas = DispararEm(Escopo::kGatilhoDaGota, Vector2(campo.x + campo.w / 2.0f, campo.y),
                                    static_cast<int>(gotas.size()));
    const std::string cara = Escopo::CaraDoAviso(Escopo::Nome(mForma));
    const size_t n = std::min(criadas.size(), gotas.size());
    for (size_t i = 0; i < n; ++i) {
        BossProjectile* p = criadas[i];
        p->SetPosition(Vector2(campo.x + gotas[i].x, campo.y + gotas[i].y));
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        PorCara(p, cara);
        Colidir(p, false);
        p->DefinirPosto("aviso");
        mGotas.push_back({p, campo.y + gotas[i].fim});
    }
}

void Luizinho::AcenderGotas() {
    const std::string cara = Escopo::CaraDaGota(Escopo::Nome(mForma));
    for (const GotaEmCampo& g : mGotas) {
        if (!Vivo(g.p)) continue;
        PorCara(g.p, cara);
        Colidir(g.p, true);
        g.p->DefinirPosto("");
        if (auto* rb = g.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, mDegrauDoEscopo.queda));
    }
}

void Luizinho::AtualizarEscopo() {
    // O que o campo levou (o Ponto Extra, o fim da batalha) sai da conta
    // antes de qualquer disparo deste quadro: o projetil pode voltar do pool
    // como outro.
    mGotas.erase(std::remove_if(mGotas.begin(), mGotas.end(),
                                [](const GotaEmCampo& g) { return !Vivo(g.p); }), mGotas.end());

    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Escopo::kFaseDoEscopo && !EhClone();
    if (!aqui) {
        if (mNoEscopo) DesmontarEscopo();
        return;
    }
    if (!mNoEscopo) {
        MontarEscopo();
        mNoEscopo = true;
        RegistrarLegenda(Escopo::kFaseDoEscopo, kAberturaDoEscopo);
        mComandoDoEscopo = -1;
        // O professor ja vai para o primeiro comando enquanto o placar diz o latim.
        const SDL_FRect campo = Campo();
        const Escopo::Ponto onde = Escopo::OndeFicaOProfessor(Escopo::Proxima(Variacao::NaOrdem(mOrdemDoEscopo, 0)), campo.w, campo.h);
        mAlvo = Vector2(campo.x + onde.x, campo.y + onde.y);
    }

    const float t = fsm->GetStateTime();
    if (t < Escopo::kPrimeiro) return;
    const int k = std::min(Escopo::kComandos - 1, static_cast<int>((t - Escopo::kPrimeiro) / Escopo::kPorComando));
    if (k != mComandoDoEscopo) ComecarComando(k);

    const float u = t - mInicioDoComando;
    const float kAcende = Escopo::kBusca + mDegrauDoEscopo.aviso;
    const float kSeca = kAcende + mDegrauDoEscopo.vivas;

    if (mChuva == Chuva::Esperando && u >= Escopo::kBusca) {
        ChoverAvisos();
        mChuva = Chuva::Aviso;
    }
    if (mChuva == Chuva::Aviso && u >= kAcende) {
        AcenderGotas();
        mChuva = Chuva::Viva;
    }
    if (mChuva == Chuva::Viva) {
        // A GOTA MORRE NO FIM DELA - a borda de baixo do bloco, ou o buraco
        // embaixo: la a variavel ja e outra.
        for (const GotaEmCampo& g : mGotas) {
            if (Vivo(g.p) && g.p->GetPosition().y >= g.fim) g.p->Kill();
        }
        if (u >= kSeca) {
            SecarGotas();
            mChuva = Chuva::Seca;
        }
    }

    DesenharEscopo(u);
}

void Luizinho::DesenharEscopo(const float u) {
    const SDL_FRect campo = Campo();
    auto canto = [&](const Escopo::Bloco b) {
        const Escopo::Ponto c = Escopo::Canto(b, campo.w, campo.h);
        return Vector2(campo.x + c.x, campo.y + c.y);
    };
    auto indice = [](const Escopo::Bloco b) { return static_cast<size_t>(b); };

    const bool chovendo = (mChuva == Chuva::Aviso || mChuva == Chuva::Viva);
    const bool mostrar = u < Escopo::kBusca + mDegrauDoEscopo.aviso + mDegrauDoEscopo.vivas;

    // TUDO VOLTA AO NORMAL, e o comando acende o que e dele.
    for (PainelDeCodigo* p : mPaineis) if (p) p->Pintar(kMolduraDoPainel, kFundoDoPainel);
    for (EtiquetaDeCodigo* c : mCantos) if (c) c->Pintar(kMolduraDoCanto, kFundoDoCanto, kTintaDoCanto);

    // A CHAMADA: de h ate o cartao de f, o comando inteiro.
    if (const auto quem = Escopo::QuemChamou(mForma)) {
        mLinhas->Acrescentar(canto(*quem), canto(Escopo::Bloco::F), kLinhaDaChamada);
    }
    if (!mostrar) return;

    // A BUSCA: um salto por bloco olhado, do professor ate a declaracao.
    const auto busca = Escopo::Busca(mForma);
    Vector2 de = GetPosition();
    for (size_t i = 0; i < busca.size(); ++i) {
        if (u < static_cast<float>(i) * Escopo::kPorSalto) break;
        const bool achou = (i + 1 == busca.size());
        const Vector2 ate = canto(busca[i]);
        mLinhas->Acrescentar(de, ate, achou ? kLinhaAchada : kLinhaDaBusca);
        if (u >= static_cast<float>(i + 1) * Escopo::kPorSalto) {
            if (EtiquetaDeCodigo* c = mCantos[indice(busca[i])]) {
                if (achou) c->Pintar(kMolduraAchada, kFundoAchado, kTintaAchada);
                else c->Pintar(kMolduraOlhada, kFundoDoCanto, kTintaOlhada);
            }
        }
        de = ate;
    }

    // OS BURACOS: os blocos que tem outra variavel com o nome. Aqui nao chove.
    if (chovendo) {
        for (const Escopo::Bloco b : Escopo::Ocultam(Escopo::Dono(mForma), Escopo::Nome(mForma))) {
            if (PainelDeCodigo* p = mPaineis[indice(b)]) p->Pintar(kMolduraDoBuraco, kFundoDoBuraco);
            if (EtiquetaDeCodigo* c = mCantos[indice(b)]) c->Pintar(kMolduraDoBuraco, kFundoDoCanto, kTintaDoBuraco);
        }
    }
}

// ---------------------------------------------------------------------------
// A fase 2: os sistemas de tipos
// ---------------------------------------------------------------------------

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
    Restaurar(Coercao::kFaseDosTipos);
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
    Anunciar(Coercao::kFaseDosTipos, Coercao::Placar(mConta));
    RegistrarLegenda(Coercao::kFaseDosTipos, Coercao::Legenda(mConta));

    // O PROFESSOR MUDA DE LUGAR a cada conta, no alto - e balanca em volta
    // dele (AtualizarTipos).
    const SDL_FRect campo = Campo();
    const float fracao = static_cast<float>(Sorteio() % 1001u) / 1000.0f;
    mBaseDoProfessor = Vector2(campo.x + Coercao::kXMinimo + fracao * (Coercao::kXMaximo - Coercao::kXMinimo),
                               campo.y + Coercao::kYDoProfessor);
    mAlvo = mBaseDoProfessor;

    // O OPERADOR APARECE EM CIMA DO ALUNO, decidido agora: e o aviso de onde a
    // conta vai rodar. Enfeite - o tiro passa por ele, e ele nao acerta.
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
    // O COMPILADOR RECUSA ANTES DE RODAR: os operandos estouram no professor,
    // e a conta nunca chega ao operador.
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
    Anunciar(Coercao::kFaseDosTipos, Coercao::PlacarComResultado(mConta));
}

void Luizinho::FazerAConta() {
    // OS DOIS TEM DE CHEGAR: o Ponto Extra que leva um operando leva a conta.
    for (const Operando& o : mOperandos) if (!Vivo(o.p)) { LimparConta(); return; }
    for (Operando& o : mOperandos) {
        o.p->Kill();
        o = Operando{};
    }

    const Coercao::Resultado r = Coercao::Avaliar(mConta);
    const Vector2 onde = mOndeDaConta;
    const float alvo = AnguloAteOAluno(onde);

    if (r.desfecho == Coercao::Desfecho::ErroDeExecucao) {
        // O TypeError ESTOURA ONDE A CONTA RODOU.
        const int n = Coercao::kEstilhacosDoErro;
        const auto cacos = DispararEm(Coercao::kGatilhoDoErro, onde, n, Coercao::CentroDoAnel(n, alvo),
                                      Coercao::AberturaDoAnel(n), mDegrauDosTipos.rapidez);
        for (BossProjectile* c : cacos) PorCara(c, Coercao::kCaraDoTypeError);
        if (Vivo(mOperador)) PorCara(mOperador, Coercao::CaraDoOperador(Coercao::OperacaoDe(mConta.forma), true));
    }
    else if (r.valor.tipo == Coercao::Tipo::Numero) {
        // UM NUMERO n E UM ANEL DE n, cada chip com o n escrito.
        const int n = r.valor.numero;
        const auto chips = DispararEm(Coercao::kGatilhoDoAnel, onde, n, Coercao::CentroDoAnel(n, alvo),
                                      Coercao::AberturaDoAnel(n), mDegrauDosTipos.rapidez);
        for (BossProjectile* c : chips) PorCara(c, Coercao::CaraDoNumero(n));
    }
    else {
        // UM TEXTO E A PALAVRA, letra por letra entre aspas, de frente para o aluno.
        const auto letras = Coercao::Letras(r.valor);
        const auto desvios = Coercao::DesviosDaPalavra(static_cast<int>(letras.size()), alvo);
        const auto chips = DispararEm(Coercao::kGatilhoDoTexto, onde, static_cast<int>(letras.size()), alvo, 0.0f,
                                      mDegrauDosTipos.rapidez);
        for (size_t i = 0; i < chips.size() && i < letras.size(); ++i) {
            chips[i]->SetPosition(onde + Vector2(desvios[i].x, desvios[i].y));
            PorCara(chips[i], Coercao::CaraDaLetra(letras[i]));
        }
    }
    Anunciar(Coercao::kFaseDosTipos, Coercao::PlacarComResultado(mConta));
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
        RegistrarLegenda(Coercao::kFaseDosTipos, kAberturaDosTipos);
        mContaDaVez = -1;
        const SDL_FRect campo = Campo();
        mBaseDoProfessor = Vector2(campo.x + campo.w / 2.0f, campo.y + Coercao::kYDoProfessor);
        mAlvo = mBaseDoProfessor;
    }

    const float t = fsm->GetStateTime();
    if (t < Coercao::kPrimeiro) return;
    const int k = std::min(Coercao::kComandos - 1,
                           static_cast<int>((t - Coercao::kPrimeiro) / Coercao::kPorComando));
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
            // OS OPERANDOS ESPERAM O COMPILADOR NA MAO DO PROFESSOR.
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
                // EM ARCO, cada um para o seu lado: os dois se leem no caminho.
                const Vector2 reta = o.de + (o.para - o.de) * s;
                const float arco = std::sin(s * Math::Pi) * kArcoDoVoo * o.lado;
                o.p->SetPosition(reta + Vector2(arco, 0.0f));
                // A COERCAO SE VE: no meio do voo o convertido troca de chip.
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

// ---------------------------------------------------------------------------
// A fase 3: a gerencia de memoria
// ---------------------------------------------------------------------------

namespace {
    // As linhas do heap: a raiz que aponta, o prox de um no, e o ponteiro
    // pendente para onde o bloco estava.
    constexpr SDL_Color kLinhaDaRaiz{255, 214, 72, 230};
    constexpr SDL_Color kLinhaDoProx{90, 200, 240, 220};
    constexpr SDL_Color kLinhaPendente{236, 70, 70, 240};
}

void Luizinho::PararOMundo(const bool parar) const {
    auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return;

    // PARADO, O PROJETIL NAO FAZ NADA: Actor::Update so roda o que esta ativo,
    // entao o corpo, os behaviors e a animacao esperam juntos. E parado ele
    // nao acerta ninguem (a colisao da batalha so olha o que esta ativo):
    // durante a marcacao o aluno pode passar por entre os blocos.
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (!p) continue;
        if (parar && p->GetState() == ActorState::Active) p->SetState(ActorState::Paused);
        else if (!parar && p->GetState() == ActorState::Paused) p->SetState(ActorState::Active);
    }
}

void Luizinho::LimparHeap() {
    for (BlocoEmCampo& b : mBlocos) for (BossProjectile* c : b.p) if (Vivo(c)) c->Kill();
    mBlocos.clear();
    mHeap = Memoria::Heap{};
    mLiberado.reset();
    mCelulasDoLiberado = 0;
}

void Luizinho::DesmontarMemoria() {
    if (mMundoParado) PararOMundo(false);
    mMundoParado = false;
    LimparHeap();
    for (auto& [nome, etiqueta] : mRaizes) if (etiqueta) etiqueta->SetState(ActorState::Destroy);
    mRaizes.clear();
    Restaurar(Memoria::kFaseDaMemoria);
    mNaMemoria = false;
    mPassoDaVez = -1;
    mEventos.clear();
    mProximoEvento = 0;
    mAlvo.reset();
}

void Luizinho::ComecarPasso(const int i) {
    mPassoDaVez = i;
    const Memoria::Passo passo = PassoDaVez();
    mEventos = Memoria::Eventos(passo, mDegrauDaMemoria);
    mProximoEvento = 0;
    Anunciar(Memoria::kFaseDaMemoria, Memoria::Placar(passo, false));
    RegistrarLegenda(Memoria::kFaseDaMemoria, Memoria::Legenda(passo));

    // O PRIMEIRO PROGRAMA ACABOU: o sistema devolve a memoria dele, e o
    // segundo comeca com o heap vazio.
    if (Memoria::TrocaDeLinguagem(i, mRoteiro)) LimparHeap();

    const SDL_FRect campo = Campo();
    const float fracao = static_cast<float>(Sorteio() % 1001u) / 1000.0f;
    mBaseDaMemoria = Vector2(campo.x + Memoria::kXMinimo + fracao * (Memoria::kXMaximo - Memoria::kXMinimo),
                             campo.y + Memoria::kYDoProfessor);
}

void Luizinho::Alocar(const Memoria::Evento& e) {
    const SDL_FRect campo = Campo();
    const auto* fsm = GetComponent<FSMComponent>();
    const float agora = fsm ? fsm->GetStateTime() : 0.0f;

    // O ALOCADOR ESCOLHE O ALUNO: o lugar livre mais perto dele.
    std::vector<Memoria::Retangulo> ocupados;
    for (const BlocoEmCampo& b : mBlocos) {
        ocupados.push_back(Memoria::Area({b.centro.x - campo.x, b.centro.y - campo.y}, b.celulas));
    }
    const Vector2 aluno = PosicaoDoAluno();
    const Memoria::Ponto onde = Memoria::OndeAlocar(e.celulas, {aluno.x - campo.x, aluno.y - campo.y},
                                                    ocupados, campo.w, campo.h);

    const int id = Memoria::Novo(mHeap, e.celulas);
    if (!e.raiz.empty()) {
        if (e.noProx) {
            const auto it = mHeap.raizes.find(e.raiz);
            if (it != mHeap.raizes.end()) Memoria::ApontarProx(mHeap, it->second, id);
        } else {
            Memoria::Apontar(mHeap, e.raiz, id);
        }
    }

    BlocoEmCampo bloco;
    bloco.id = id;
    bloco.centro = Vector2(campo.x + onde.x, campo.y + onde.y);
    bloco.celulas = e.celulas;
    bloco.java = Memoria::LinguagemDe(PassoDaVez()) == Memoria::Linguagem::Java;
    bloco.no = bloco.java && e.celulas == 2;
    bloco.acendeEm = agora + mDegrauDaMemoria.avisoDaAlocacao;

    // NASCE APAGADO, sem colisor: o aviso de onde o bloco vai ficar.
    const auto criadas = DispararEm(Memoria::kGatilhoDaCelula, bloco.centro, e.celulas);
    const auto centros = Memoria::Celulas(onde, e.celulas);
    for (size_t k = 0; k < criadas.size() && k < centros.size(); ++k) {
        BossProjectile* c = criadas[k];
        c->SetPosition(Vector2(campo.x + centros[k].x, campo.y + centros[k].y));
        if (auto* rb = c->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        PorCara(c, Memoria::kCaraDoAviso);
        Colidir(c, false);
        c->DefinirPosto("aviso");
        bloco.p.push_back(c);
    }
    mBlocos.push_back(bloco);
}

void Luizinho::Coletar() {
    // A VARREDURA: o que a marcacao nao alcancou e jogado fora - cai reto para
    // fora da tela, e na queda ainda acerta quem estiver embaixo.
    const auto varridos = Memoria::Varrer(mHeap);
    PararOMundo(false);
    mMundoParado = false;
    for (auto it = mBlocos.begin(); it != mBlocos.end();) {
        if (std::find(varridos.begin(), varridos.end(), it->id) == varridos.end()) { ++it; continue; }
        for (BossProjectile* c : it->p) {
            if (!Vivo(c)) continue;
            Colidir(c, true);
            c->DefinirPosto("");
            PorCara(c, Memoria::kCaraMarcada);
            if (auto* rb = c->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, Memoria::kQuedaDoLixo));
        }
        it = mBlocos.erase(it);
    }
}

void Luizinho::LiberarBloco(const std::string& raiz) {
    const auto r = mHeap.raizes.find(raiz);
    if (r == mHeap.raizes.end()) return;
    const int id = r->second;
    Memoria::Liberar(mHeap, raiz);
    const SDL_FRect campo = Campo();
    for (auto it = mBlocos.begin(); it != mBlocos.end(); ++it) {
        if (it->id != id) continue;
        // O BLOCO SAI, e o lugar dele fica marcado: e para la que p aponta.
        mLiberado = Memoria::Area({it->centro.x - campo.x, it->centro.y - campo.y}, it->celulas);
        mCelulasDoLiberado = it->celulas;
        for (BossProjectile* c : it->p) if (Vivo(c)) c->Kill();
        mBlocos.erase(it);
        break;
    }
}

void Luizinho::EscreverNoPendente(const Memoria::Evento& e) {
    if (!Memoria::Pendente(mHeap, e.raiz) || !mLiberado) return;
    // ESCREVER ONDE O BLOCO ESTAVA E COMPORTAMENTO INDEFINIDO: aquela memoria
    // nao e de ninguem, e o 7 estoura em cacos.
    const SDL_FRect campo = Campo();
    const auto centros = Memoria::Celulas(mLiberado->Centro(), mCelulasDoLiberado);
    const size_t k = std::min(static_cast<size_t>(std::max(0, e.indice)), centros.size() - 1);
    const Vector2 onde(campo.x + centros[k].x, campo.y + centros[k].y);
    const int n = mDegrauDaMemoria.estilhacosDoPendente;
    const auto cacos = DispararEm(Memoria::kGatilhoDoPendente, onde, n,
                                  Coercao::CentroDoAnel(n, AnguloAteOAluno(onde)), Coercao::AberturaDoAnel(n),
                                  mDegrauDaMemoria.rapidez);
    for (BossProjectile* c : cacos) PorCara(c, Memoria::kCaraDoEstilhaco);
    Anunciar(Memoria::kFaseDaMemoria, Memoria::Placar(PassoDaVez(), true));
}

void Luizinho::EstourarOsVazados() {
    // O std::bad_alloc: o heap encheu de blocos que ninguem libera, e todos
    // estouram de uma vez - cada celula em quatro cacos, em X.
    for (auto it = mBlocos.begin(); it != mBlocos.end();) {
        const Memoria::Bloco* b = Memoria::Achar(mHeap, it->id);
        if (!b || b->estado != Memoria::Estado::Vazou) { ++it; continue; }
        for (BossProjectile* c : it->p) {
            if (!Vivo(c)) continue;
            const Vector2 onde = c->GetPosition();
            c->Kill();
            const auto cacos = DispararEm(Memoria::kGatilhoDoEstouro, onde, Memoria::kEstilhacosPorCelula,
                                          90.0f, 270.0f, mDegrauDaMemoria.rapidez);
            for (BossProjectile* k : cacos) PorCara(k, Memoria::kCaraDoEstilhaco);
        }
        it = mBlocos.erase(it);
    }
    Anunciar(Memoria::kFaseDaMemoria, Memoria::Placar(PassoDaVez(), true));
}

void Luizinho::Executar(const Memoria::Evento& e) {
    switch (e.acao) {
        case Memoria::Acao::Alocar:
            Alocar(e);
            break;
        case Memoria::Acao::Parar:
            // STOP THE WORLD: o professor e os projeteis param, e o coletor marca.
            mMundoParado = true;
            PararOMundo(true);
            Anunciar(Memoria::kFaseDaMemoria, Memoria::Placar(PassoDaVez(), true));
            break;
        case Memoria::Acao::Varrer:
            Coletar();
            break;
        case Memoria::Acao::Liberar:
            LiberarBloco(e.raiz);
            Anunciar(Memoria::kFaseDaMemoria, Memoria::Placar(PassoDaVez(), true));
            break;
        case Memoria::Acao::Escrever:
            EscreverNoPendente(e);
            break;
        case Memoria::Acao::Estourar:
            EstourarOsVazados();
            break;
    }
    // SEM COLETOR, O LIXO VAZA na hora em que fica sem ninguem.
    if (Memoria::LinguagemDe(PassoDaVez()) == Memoria::Linguagem::Cpp) {
        Memoria::Vazar(mHeap);
    }
}

void Luizinho::PintarHeap() {
    const auto lixo = Memoria::Lixo(mHeap);
    for (BlocoEmCampo& b : mBlocos) {
        if (!b.aceso) continue;
        const Memoria::Bloco* h = Memoria::Achar(mHeap, b.id);
        const bool ehLixo = std::find(lixo.begin(), lixo.end(), b.id) != lixo.end();
        for (size_t k = 0; k < b.p.size(); ++k) {
            BossProjectile* c = b.p[k];
            if (!Vivo(c) && !(c && c->GetState() == ActorState::Paused)) continue;
            std::string cara;
            if (h && h->estado == Memoria::Estado::Vazou) cara = Memoria::kCaraQueVazou;
            else if (ehLixo) cara = mMundoParado ? Memoria::kCaraMarcada : Memoria::kCaraDoLixo;
            else if (b.no && k + 1 == b.p.size()) cara = Memoria::kCaraDoProx;
            else cara = b.java ? Memoria::kCaraViva : Memoria::kCaraCrua;
            PorCara(c, cara);
        }
    }
}

void Luizinho::DesenharHeap() {
    if (!mLinhas) return;
    auto primeira = [](const BlocoEmCampo& b) {
        for (BossProjectile* c : b.p) if (c && c->GetState() != ActorState::Inactive) return c->GetPosition();
        return b.centro;
    };
    auto achar = [this](const int id) -> const BlocoEmCampo* {
        for (const BlocoEmCampo& b : mBlocos) if (b.id == id) return &b;
        return nullptr;
    };
    const SDL_FRect campo = Campo();

    // AS RAIZES: a etiqueta embaixo do professor, e a linha ate o bloco. A
    // variavel de um programa que ja acabou (o "a" do Java, no C++) some.
    for (auto& [nome, etiqueta] : mRaizes) if (etiqueta) etiqueta->Mostrar(false);
    for (const auto& [nome, id] : mHeap.raizes) {
        auto it = mRaizes.find(nome);
        if (it == mRaizes.end()) {
            auto etiqueta = std::make_unique<EtiquetaDeCodigo>(mScene, nome, 30, 24, kOrdemDosCantos);
            etiqueta->Pintar(kMolduraAchada, kFundoAchado, kTintaAchada);
            it = mRaizes.emplace(nome, etiqueta.get()).first;
            mScene->AddActor(std::move(etiqueta));
        }
        EtiquetaDeCodigo* etiqueta = it->second;
        const Vector2 de = GetPosition() + Vector2(0.0f, kAbaixoDoProfessor);
        etiqueta->SetPosition(de);
        etiqueta->Mostrar(true);
        if (Memoria::Pendente(mHeap, nome) && mLiberado) {
            const Memoria::Ponto c = mLiberado->Centro();
            const Vector2 ate(campo.x + c.x, campo.y + c.y);
            mLinhas->Acrescentar(de, ate, kLinhaPendente);
            // O contorno de onde o bloco estava.
            const float x0 = campo.x + mLiberado->x, y0 = campo.y + mLiberado->y;
            const float x1 = x0 + mLiberado->w, y1 = y0 + mLiberado->h;
            mLinhas->Acrescentar(Vector2(x0, y0), Vector2(x1, y0), kLinhaPendente);
            mLinhas->Acrescentar(Vector2(x1, y0), Vector2(x1, y1), kLinhaPendente);
            mLinhas->Acrescentar(Vector2(x1, y1), Vector2(x0, y1), kLinhaPendente);
            mLinhas->Acrescentar(Vector2(x0, y1), Vector2(x0, y0), kLinhaPendente);
        } else if (const BlocoEmCampo* b = achar(id)) {
            mLinhas->Acrescentar(de, primeira(*b), kLinhaDaRaiz);
        }
    }

    // OS PROX: do campo prox de um no ate o no para onde ele aponta.
    for (const BlocoEmCampo& b : mBlocos) {
        const Memoria::Bloco* h = Memoria::Achar(mHeap, b.id);
        if (!h || h->prox < 0 || b.p.empty()) continue;
        if (const BlocoEmCampo* alvo = achar(h->prox)) {
            const BossProjectile* campoProx = b.p.back();
            const Vector2 de = campoProx ? campoProx->GetPosition() : b.centro;
            mLinhas->Acrescentar(de, primeira(*alvo), kLinhaDoProx);
        }
    }
}

void Luizinho::AtualizarMemoria() {
    // O que o campo levou (o Ponto Extra) sai das celulas antes de qualquer
    // disparo deste quadro. Uma celula PARADA esta viva: e o mundo parado.
    for (BlocoEmCampo& b : mBlocos) {
        for (BossProjectile*& c : b.p) {
            if (c && c->GetState() != ActorState::Active && c->GetState() != ActorState::Paused) c = nullptr;
        }
    }

    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Memoria::kFaseDaMemoria && !EhClone();
    if (!aqui) {
        if (mNaMemoria) DesmontarMemoria();
        return;
    }
    if (!mNaMemoria) {
        mNaMemoria = true;
        RegistrarLegenda(Memoria::kFaseDaMemoria, kAberturaDaMemoria);
        mPassoDaVez = -1;
        const SDL_FRect campo = Campo();
        mBaseDaMemoria = Vector2(campo.x + campo.w / 2.0f, campo.y + Memoria::kYDoProfessor);
    }

    const float t = fsm->GetStateTime();
    mAlvo = mBaseDaMemoria;
    const int i = Memoria::PassoEm(t, mRoteiro);
    if (i < 0) return;
    if (i != mPassoDaVez) ComecarPasso(i);

    const float u = t - Memoria::InicioDaVez(mPassoDaVez, mRoteiro);
    while (mProximoEvento < mEventos.size() && u >= mEventos[mProximoEvento].quando) {
        Executar(mEventos[mProximoEvento++]);
    }

    // O BLOCO ACENDE: ganha colisor, sai do posto de aviso, e e memoria.
    for (BlocoEmCampo& b : mBlocos) {
        if (b.aceso || t < b.acendeEm) continue;
        b.aceso = true;
        for (BossProjectile* c : b.p) {
            if (!c) continue;
            Colidir(c, true);
            c->DefinirPosto("");
        }
    }

    PintarHeap();
    DesenharHeap();
}

// ---------------------------------------------------------------------------
// A final: as excecoes
// ---------------------------------------------------------------------------

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
    const float largura = Escopo::LarguraDoCanto(texto);
    auto canto = std::make_unique<EtiquetaDeCodigo>(mScene, texto, static_cast<int>(largura),
                                                    static_cast<int>(Escopo::kAlturaDoCanto), kOrdemDosCantos);
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
        // A FAIXA SE QUEBRA, e os destrocos caem numa cortina - um em cima
        // do aluno, para quem ficar parado nao ficar na fresta.
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
    Restaurar(Excecao::kFaseDasExcecoes);
    mNasExcecoes = false;
    mRodadaDaVez = -1;
    mPassos.clear();
    mAlvo.reset();
}

void Luizinho::ComecarRodada(const int k) {
    // O QUE SOBROU DA RODADA ANTERIOR retorna, sem quebrar.
    for (int i = 0; i < Excecao::kMaiorPilha; ++i) Desempilhar(i, false);
    if (Vivo(mExcecao)) mExcecao->Kill();
    mExcecao = nullptr;

    mRodadaDaVez = k;
    mRodada = Excecao::RodadaDaVez(Variacao::NaOrdem(mOrdemDasRodadas, k));
    mPassos = Excecao::Propagar(mRodada);
    mProximoPasso = 0;
    mEmpilhados = 0;
    mLancou = false;
    mRetornou = false;
    mCapturouEm = -1;
    const float fracao = static_cast<float>(Sorteio() % 1001u) / 1000.0f;
    mXDaRodada = Campo().x + Excecao::kXMinimo + fracao * (Excecao::kXMaximo - Excecao::kXMinimo);
    Anunciar(Excecao::kFaseDasExcecoes, Excecao::PlacarDasChamadas(mRodada));
    RegistrarLegenda(Excecao::kFaseDasExcecoes, Excecao::Legenda(mRodada, false));
}

void Luizinho::ExecutarPasso(const Excecao::Passo& p) {
    switch (p.o) {
        case Excecao::Acontece::Finally: {
            // O FINALLY RODA DE QUALQUER JEITO: um anel de onde a excecao passa.
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
            // O PONTO DE EXECUCAO SOBE para quem chamou.
            if (p.quadro > 0) mAlvo = Vector2(mXDaRodada, MeioDaFaixa(p.quadro - 1).y);
            break;
        case Excecao::Acontece::Captura:
            mCapturouEm = p.quadro;
            PintarFaixa(p.quadro, kMolduraDoCatch, kFundoDoCatch);
            if (Vivo(mExcecao)) mExcecao->Kill();
            mExcecao = nullptr;
            mAlvo = Vector2(mXDaRodada, MeioDaFaixa(p.quadro).y);
            Anunciar(Excecao::kFaseDasExcecoes, Excecao::PlacarDoDesfecho(mRodada));
            RegistrarLegenda(Excecao::kFaseDasExcecoes, Excecao::Legenda(mRodada, true));
            break;
        case Excecao::Acontece::Escapa:
            // NINGUEM TRATOU: o programa capota.
            if (Vivo(mExcecao)) mExcecao->Kill();
            mExcecao = nullptr;
            Anunciar(Excecao::kFaseDasExcecoes, Excecao::PlacarDoDesfecho(mRodada));
            RegistrarLegenda(Excecao::kFaseDasExcecoes, Excecao::Legenda(mRodada, true));
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
        RegistrarLegenda(Excecao::kFaseDasExcecoes, kAberturaDasExcecoes);
        mRodadaDaVez = -1;
        mAlvo = Vector2(Campo().x + Campo().w / 2.0f, MeioDaFaixa(0).y);
    }

    const float t = fsm->GetStateTime();
    if (t < Excecao::kPrimeiro) return;
    const int k = std::min(Excecao::kRodadas - 1, static_cast<int>((t - Excecao::kPrimeiro) / Excecao::kPorRodada));
    if (k != mRodadaDaVez) ComecarRodada(k);
    const float u = t - (Excecao::kPrimeiro + static_cast<float>(k) * Excecao::kPorRodada);

    // AS CHAMADAS EMPILHAM, e o professor desce ate a mais funda.
    while (mEmpilhados < mRodada.pilha.size() && u >= static_cast<float>(mEmpilhados) * Excecao::kEntreChamadas) {
        Empilhar(static_cast<int>(mEmpilhados));
        mAlvo = Vector2(mXDaRodada, MeioDaFaixa(static_cast<int>(mEmpilhados)).y);
        ++mEmpilhados;
    }

    // O LANCAMENTO: a excecao nasce no professor, na faixa de quem lanca.
    const int ultimo = static_cast<int>(mRodada.pilha.size()) - 1;
    if (!mLancou && u >= Excecao::kLancamento) {
        mLancou = true;
        Anunciar(Excecao::kFaseDasExcecoes, Excecao::PlacarDoLancamento(mRodada));
        const auto criada = DispararEm(Excecao::kGatilhoDaExcecao,
                                       Vector2(mXDaRodada - Excecao::kAEsquerda, MeioDaFaixa(ultimo).y), 1);
        if (!criada.empty()) {
            mExcecao = criada.front();
            mExcecao->DefinirEnfeite(true);
            PorCara(mExcecao, Excecao::CaraDaExcecao(mRodada.lancada));
        }
    }
    if (!mLancou) return;

    // ELA SOBE uma faixa a cada kPorQuadro.
    const float subindo = u - Excecao::kLancamento;
    if (Vivo(mExcecao)) {
        const float passo = Excecao::kAlturaDaFaixa + Excecao::kEntreFaixas;
        const float y = MeioDaFaixa(ultimo).y - passo * subindo / Excecao::kPorQuadro;
        mExcecao->SetPosition(Vector2(mXDaRodada - Excecao::kAEsquerda, y));
    }
    while (mProximoPasso < mPassos.size() && subindo >= mPassos[mProximoPasso].quando) {
        ExecutarPasso(mPassos[mProximoPasso++]);
    }

    // QUEM SOBROU RETORNA, depois do desfecho.
    if (!mRetornou && !mPassos.empty() && subindo >= mPassos.back().quando + Excecao::kRetorno) {
        mRetornou = true;
        for (int i = 0; i < Excecao::kMaiorPilha; ++i) Desempilhar(i, false);
    }
}
