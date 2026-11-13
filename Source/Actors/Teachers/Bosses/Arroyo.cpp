//
// O chefe da INF 332 - Projeto e Analise de Algoritmos.
//

#include "Arroyo.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "BossesProjectiles/BossProjectile.h"

Arroyo::Arroyo(Scene* scene) : Boss(scene) {
}

void Arroyo::SortearALuta() {
    mLutaSorteada = true;
    Variacao::Sorteio arvore = SorteioDaFase(Recorrencia::kFase);
    mOrdemDasRecorrencias = Recorrencia::Sortear(arvore);
    mSorteio = SorteioDaFase(Intratavel::kFase);
    const Dificuldade::Nivel n = NivelDeDificuldade();
    mDegrauDaArvore = Recorrencia::DegrauDo(n);
    mDegrauDosParadigmas = Paradigmas::DegrauDo(n);
    mDegrauDoIntratavel = Intratavel::DegrauDo(n);
    mDegrauDaTempera = Tempera::DegrauDo(n);

    // O placar de cada rodada da arvore: a recorrencia sorteada.
    auto etapas = EtapasDa(Recorrencia::kFase);
    for (size_t r = 0; r < etapas.size() && r < mOrdemDasRecorrencias.size(); ++r) {
        etapas[r].second = Recorrencia::Placar(Recorrencia::Casos()[static_cast<size_t>(mOrdemDasRecorrencias[r])], false);
    }
    RegistrarEtapas(Recorrencia::kFase, etapas);
}

// ---- O que as fases dividem ----

std::vector<BossProjectile*> Arroyo::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
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

BossProjectile* Arroyo::Um(const char* gatilho, const Vector2 onde, const std::string& cara, const float escala,
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
    if (!cara.empty()) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(cara);
    }
    return p;
}

void Arroyo::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
    if (!mDisparandoEm) return;
    params.firePosition = mOndeDisparar;
    if (mQuantos > 0) params.numProjectiles = mQuantos;
    if (mCentroEm) params.centralAngle = *mCentroEm;
    if (mAberturaEm) params.angle = *mAberturaEm;
    if (mVelocidadeEm) params.projectileSpeed = *mVelocidadeEm;
}

void Arroyo::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                  const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

float Arroyo::AnguloAteOAluno(const Vector2 de) const {
    const Vector2 d = PosicaoDoAluno() - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

bool Arroyo::Vivo(const BossProjectile* p) {
    return p && p->GetState() == ActorState::Active;
}

void Arroyo::Matar(BossProjectile*& p) {
    if (Vivo(p)) p->Kill();
    p = nullptr;
}

void Arroyo::Linha(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Acrescentar(de, ate, cor);
}

void Arroyo::PlacarDaEtapa(const std::string& fase, const size_t etapa, const std::string& texto) {
    auto etapas = EtapasDa(fase);
    if (etapa >= etapas.size() || etapas[etapa].second == texto) return;
    etapas[etapa].second = texto;
    RegistrarEtapas(fase, etapas);
}

void Arroyo::PlacarDaFase(const std::string& fase, const std::string& texto) {
    if (AnuncioDaFase(fase) != texto) RegistrarAnuncio(fase, texto);
}

std::optional<Vector2> Arroyo::AlvoDoMovimento() const {
    return mAlvo;
}

void Arroyo::Desmontar() {
    for (No& n : mNos) Matar(n.p);
    mNos.clear();
    mRodada = -1;
    DesmontarParadigmas();
    DesmontarIntratavel();
    DesmontarTempera();
    if (mLinhas) mLinhas->Limpar();
    mAlvo.reset();
}

void Arroyo::AoTerminarABatalha() {
    Desmontar();
}

void Arroyo::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    if (EhClone()) return;

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

    if (fase == Recorrencia::kFase) AtualizarArvore(t);
    else if (fase == Paradigmas::kFase) AtualizarParadigmas(t);
    else if (fase == Intratavel::kFase) AtualizarIntratavel(t);
    else if (fase == Tempera::kFase) AtualizarTempera(t, deltaTime);
}

// ---- Fase 1: a arvore de recursao ----

void Arroyo::AtualizarArvore(const float t) {
    using namespace Recorrencia;
    if (mOrdemDasRecorrencias.empty()) return;
    const int rodada = std::min(kRodadas - 1, static_cast<int>(t / kDuracaoDaRodada));
    if (rodada != mRodada) {
        mRodada = rodada;
        mRaizSaiu = false;
        mArvorePronta = false;
    }
    const Caso& caso = Casos()[static_cast<size_t>(mOrdemDasRecorrencias[static_cast<size_t>(rodada)])];
    const float inicio = static_cast<float>(rodada) * kDuracaoDaRodada;

    // O que o aluno derrubou (ou o Ponto Extra limpou) sai antes de qualquer no nascer: o projetil
    // morto volta ao pool, e o que nasce agora pode ser ele.
    mNos.erase(std::remove_if(mNos.begin(), mNos.end(), [](const No& n) { return !Vivo(n.p); }), mNos.end());

    if (!mRaizSaiu && t - inicio >= 0.15f) {
        mRaizSaiu = true;
        const Vector2 onde = GetPosition() + Vector2(0.0f, 46.0f);
        Vector2 rumo(0.0f, 1.0f);
        ApontarParaOAluno(onde, rumo);
        rumo = rumo * mDegrauDaArvore.velocidade;
        if (BossProjectile* raiz = Um(kGatilhoDoNo, onde, Cara(caso, 0), 1.3f, rumo)) {
            raiz->DefinirResistencia(Resistencia(0));
            mNos.push_back(No{raiz, 0, t, rumo, onde});
        }
    }

    // Cada no, na hora dele, paga o trabalho e se divide - ou, folha, paga o Θ(1) e acaba.
    std::vector<No> seguem;
    for (No& n : mNos) {
        if (t - n.nasceu < mDegrauDaArvore.porNivel) {
            seguem.push_back(n);
            continue;
        }
        const Vector2 onde = n.p->GetPosition();
        Vector2 rumo = n.rumo;
        if (rumo.LengthSq() < 1.0f) rumo = Vector2(0.0f, mDegrauDaArvore.velocidade);
        n.p->Kill();
        if (n.nivel >= Niveis(caso)) {
            DispararEm(kGatilhoDaFolha, onde, 1, AnguloAteOAluno(onde));
            continue;
        }
        const int trabalho = Trabalho(caso, n.nivel);
        DispararEm(kGatilhoDoTrabalho, onde, trabalho, AnguloAteOAluno(onde),
                   std::min(110.0f, 14.0f * static_cast<float>(trabalho - 1)), mDegrauDaArvore.doTrabalho);
        const float base = Math::Atan2(rumo.y, rumo.x);
        const float rapidez = rumo.Length();
        for (const float desvio : Desvios(caso.a)) {
            const float a = base + Math::ToRadians(desvio);
            const float escala = n.nivel + 1 >= Niveis(caso) ? 0.85f : 1.05f;
            const Vector2 v(Math::Cos(a) * rapidez, Math::Sin(a) * rapidez);
            BossProjectile* filho = Um(kGatilhoDoNo, onde, Cara(caso, n.nivel + 1), escala, v);
            if (!filho) continue;
            filho->DefinirResistencia(Resistencia(n.nivel + 1));
            seguem.push_back(No{filho, n.nivel + 1, t, v, onde});
        }
    }
    mNos = std::move(seguem);

    // A arvore na tela: de onde cada no saiu ate onde ele esta.
    for (const No& n : mNos) Linha(n.origem, n.p->GetPosition(), kAresta);

    // Toda a arvore resolvida (ou podada): o Teorema Mestre entra no placar.
    if (mRaizSaiu && mNos.empty() && !mArvorePronta) {
        mArvorePronta = true;
        PlacarDaEtapa(kFase, static_cast<size_t>(rodada), Placar(caso, true));
    }
}

// ---- Fase 2: os paradigmas ----

Vector2 Arroyo::CentroDoTabuleiro() const {
    const SDL_FRect c = Campo();
    return Vector2(c.x + c.w / 2.0f, c.y + c.h * 0.56f);
}

float Arroyo::TopoDaTabela() const {
    const SDL_FRect c = Campo();
    return c.y + 122.0f;
}

void Arroyo::DesmontarParadigmas() {
    for (BossProjectile*& p : mCasas) Matar(p);
    mCasas.clear();
    for (BossProjectile*& p : mRainhas) Matar(p);
    mEtapaDosParadigmas = -1;
    mTrocoDaVez = -1;
}

void Arroyo::AtualizarParadigmas(const float t) {
    using namespace Paradigmas;
    const SDL_FRect campo = Campo();
    const int etapa = EtapaNoTempo(t);
    if (etapa != mEtapaDosParadigmas) {
        DesmontarParadigmas();
        mEtapaDosParadigmas = etapa;
        if (etapa == 1) {
            mTabela = Tabela(kPalavraA, kPalavraB);
            mOndas = Ondas(static_cast<int>(std::string(kPalavraA).size()), static_cast<int>(std::string(kPalavraB).size()));
            mOndaDaVez = 0;
            mProximaOnda = kInicio[1] + 0.35f;
            mTabelaPronta = false;
            mXDaOnda = campo.x + campo.w / 2.0f;
        } else if (etapa == 2) {
            mBusca = Busca(kRainhas);
            mPassoDaVez = 0;
            mProximoPasso = kInicio[2] + 0.35f;
            mSolucaoAtirou = false;
        }
    }

    if (etapa == 0) {
        // O guloso: tres trocos, cada um com as moedas da maior para a menor.
        const int troco = std::min(static_cast<int>(Trocos().size()) - 1, static_cast<int>(t / kDuracaoDoTroco));
        if (troco != mTrocoDaVez) {
            mTrocoDaVez = troco;
            mMoedas = Guloso(Trocos()[static_cast<size_t>(troco)]);
            mMoedaDaVez = 0;
            mProximaMoeda = static_cast<float>(troco) * kDuracaoDoTroco + 0.25f;
            PlacarDaEtapa(kFase, 0, PlacarDoTroco(Trocos()[static_cast<size_t>(troco)]));
        }
        const float lado = (mTrocoDaVez % 2 == 0) ? -1.0f : 1.0f;
        mAlvo = Vector2(campo.x + campo.w / 2.0f + lado * campo.w * 0.22f, campo.y + 70.0f);
        if (mMoedaDaVez < mMoedas.size() && t >= mProximaMoeda) {
            const int valor = mMoedas[mMoedaDaVez++];
            mProximaMoeda += kEntreMoedas;
            const Vector2 onde = GetPosition() + Vector2(0.0f, 40.0f);
            // A moeda grande e lenta; as pequenas, ligeiras.
            const float rapidez = mDegrauDosParadigmas.velocidadeDaMoeda * (valor >= 50 ? 0.8f : 1.0f);
            for (BossProjectile* m : DispararEm(kGatilhoDaMoeda, onde, 1, AnguloAteOAluno(onde), std::nullopt, rapidez)) {
                m->SetScale(EscalaDaMoeda(valor));
                if (auto* anim = m->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(CaraDaMoeda(valor));
            }
        }
        return;
    }

    if (etapa == 1) {
        // A programacao dinamica: uma onda da tabela por vez, cada casa chovendo.
        const int linhas = static_cast<int>(mTabela.size()) - 1;
        const int colunas = linhas >= 0 ? static_cast<int>(mTabela.front().size()) - 1 : 0;
        const float topo = TopoDaTabela();
        const float centroX = campo.x + campo.w / 2.0f;
        mCasas.erase(std::remove_if(mCasas.begin(), mCasas.end(), [](const BossProjectile* p) { return !Vivo(p); }), mCasas.end());
        if (mOndaDaVez < mOndas.size() && t >= mProximaOnda) {
            float soma = 0.0f;
            for (const auto& [i, j] : mOndas[mOndaDaVez]) {
                const Vector2 lugar = LugarDaCasa(i, j, linhas, colunas, centroX, topo);
                soma += lugar.x;
                const bool casou = Casou(kPalavraA, kPalavraB, i, j);
                const int valor = mTabela[static_cast<size_t>(i)][static_cast<size_t>(j)];
                if (BossProjectile* casa = Um(kGatilhoDaCelula, lugar, CaraDaCasa(valor, casou))) {
                    casa->DefinirEnfeite(true);
                    mCasas.push_back(casa);
                }
                DispararEm(casou ? kGatilhoDoLeque : kGatilhoDaChuva, lugar + Vector2(0.0f, 14.0f));
            }
            mXDaOnda = soma / static_cast<float>(std::max<size_t>(1, mOndas[mOndaDaVez].size()));
            ++mOndaDaVez;
            mProximaOnda += mDegrauDosParadigmas.porOnda;
        }
        if (mOndaDaVez >= mOndas.size() && !mTabelaPronta) {
            mTabelaPronta = true;
            PlacarDaEtapa(kFase, 1, "LCS(" + std::string(kPalavraA) + ", " + kPalavraB + ") = " +
                                     std::to_string(mTabela.back().back()));
        }
        // Ao lado da tabela, e nao em cima da chuva dela: quem atira na professora nao precisa
        // ficar embaixo da onda.
        const float metade = kLarguraDaCasa * static_cast<float>(colunas) / 2.0f;
        mAlvo = Vector2(centroX + (mXDaOnda < centroX ? 1.0f : -1.0f) * (metade + 120.0f), topo + 30.0f);
        return;
    }

    // O backtracking: um passo da busca por vez.
    const Vector2 centro = CentroDoTabuleiro();
    if (mPassoDaVez < mBusca.size() && t >= mProximoPasso) {
        const Passo& p = mBusca[mPassoDaVez++];
        mProximoPasso += mDegrauDosParadigmas.porPasso;
        BossProjectile*& rainha = mRainhas[static_cast<size_t>(p.linha)];
        const Vector2 lugar = LugarNoTabuleiro(p.linha, p.coluna, kRainhas, centro);
        if (p.coloca) {
            Matar(rainha);
            rainha = Um(kGatilhoDaRainha, lugar, "");
            if (rainha) rainha->DefinirEnfeite(true);
            mUltimaColuna = p.coluna;
            // A rainha atira pela coluna e pelas diagonais de baixo.
            DispararEm(kGatilhoDaColuna, lugar, 3, 90.0f, 90.0f);
        } else {
            Matar(rainha);
        }
    }
    const std::vector<int> deP = NoTabuleiro(mBusca, static_cast<int>(mPassoDaVez), kRainhas);
    if (mPassoDaVez >= mBusca.size() && !mSolucaoAtirou) {
        mSolucaoAtirou = true;
        for (int l = 0; l < kRainhas; ++l) {
            if (deP[static_cast<size_t>(l)] < 0) continue;
            DispararEm(kGatilhoDaSolucao, LugarNoTabuleiro(l, deP[static_cast<size_t>(l)], kRainhas, centro));
        }
        int voltas = 0;
        for (const Passo& p : mBusca) if (!p.coloca) ++voltas;
        PlacarDaEtapa(kFase, 2, "Solu\xc3\xa7\xc3\xa3o! " + std::to_string(mBusca.size()) + " passos, " +
                                 std::to_string(voltas) + " voltas atr\xc3\xa1s");
    }
    // O tabuleiro, e o que cada rainha de pe ataca (a linha, a coluna e as diagonais).
    const float meia = kCasaDoTabuleiro * static_cast<float>(kRainhas) / 2.0f;
    for (int k = 0; k <= kRainhas; ++k) {
        const float d = -meia + kCasaDoTabuleiro * static_cast<float>(k);
        Linha(centro + Vector2(-meia, d), centro + Vector2(meia, d), kTabuleiro);
        Linha(centro + Vector2(d, -meia), centro + Vector2(d, meia), kTabuleiro);
    }
    for (int l = 0; l < kRainhas; ++l) {
        const int c = deP[static_cast<size_t>(l)];
        if (c < 0) continue;
        const Vector2 q = LugarNoTabuleiro(l, c, kRainhas, centro);
        Linha(Vector2(centro.x - meia, q.y), Vector2(centro.x + meia, q.y), kAtaqueDaRainha);
        Linha(Vector2(q.x, centro.y - meia), Vector2(q.x, centro.y + meia), kAtaqueDaRainha);
        for (const float sx : {-1.0f, 1.0f}) {
            // A diagonal ate a borda do tabuleiro.
            const float ate = std::min(sx > 0 ? (centro.x + meia - q.x) : (q.x - (centro.x - meia)), centro.y + meia - q.y);
            const float antes = std::min(sx > 0 ? (q.x - (centro.x - meia)) : (centro.x + meia - q.x), q.y - (centro.y - meia));
            Linha(q + Vector2(-sx * antes, -antes), q + Vector2(sx * ate, ate), kAtaqueDaRainha);
        }
    }
    // Ao lado do tabuleiro, do lado oposto ao da ultima rainha: a coluna dela atira para baixo.
    const float lado = mUltimaColuna < kRainhas / 2 ? 1.0f : -1.0f;
    mAlvo = Vector2(centro.x + lado * (meia + 110.0f), centro.y - meia + 20.0f);
}

// ---- Fase 3: o limite inferior e o caixeiro ----

float Arroyo::XDoFio(const int i) const {
    const SDL_FRect c = Campo();
    return Intratavel::XDoFio(i, c.x + 70.0f, c.w - 140.0f);
}

float Arroyo::YDaCamada(const int k) const {
    const SDL_FRect c = Campo();
    return Intratavel::YDaCamada(k, c.y + 150.0f, c.y + c.h * 0.66f);
}

void Arroyo::DesmontarIntratavel() {
    for (Leva& l : mLevas) for (BossProjectile*& p : l.porFio) Matar(p);
    mLevas.clear();
    for (BossProjectile*& p : mCidades) Matar(p);
    mCidades.clear();
    Matar(mCaixeiro);
    mRodadaDoCaixeiro = -1;
    mProximaLeva = 0.0f;
}

void Arroyo::ComecarVolta(const float t) {
    mRota = Intratavel::Tentativa(mSorteio, static_cast<int>(mOndeCidades.size()));
    mInicioDaVolta = t;
    mProximaParada = 1;
    ++mTentativa;
}

void Arroyo::AtualizarIntratavel(const float t) {
    using namespace Intratavel;
    const SDL_FRect campo = Campo();

    if (t < kFimDaRede) {
        // A rede: os fios e os comparadores desenhados, e as levas descendo.
        const float topo = campo.y + 110.0f;
        const float fundo = campo.y + campo.h;
        for (int i = 0; i < kFios; ++i) Linha(Vector2(XDoFio(i), topo), Vector2(XDoFio(i), fundo), kFio);
        for (int k = 0; k < static_cast<int>(Camadas().size()); ++k) {
            for (const auto& [i, j] : Camadas()[static_cast<size_t>(k)]) {
                Linha(Vector2(XDoFio(i), YDaCamada(k)), Vector2(XDoFio(j), YDaCamada(k)), kComparador);
            }
        }
        PlacarDaEtapa(kFase, 0, PlacarDaRede());
        mAlvo = Vector2(campo.x + campo.w / 2.0f, campo.y + 55.0f);

        if (t >= mProximaLeva && t < kFimDaRede - 2.5f) {
            mProximaLeva = t + kEntreOndas;
            Leva leva;
            std::vector<int> valores = {1, 2, 3, 4, 5, 6};
            mSorteio.Embaralhar(valores);
            for (int i = 0; i < kFios; ++i) {
                leva.valores[static_cast<size_t>(i)] = valores[static_cast<size_t>(i)];
                leva.porFio[static_cast<size_t>(i)] = Um(kGatilhoDoValor, Vector2(XDoFio(i), topo),
                                                        "V" + std::to_string(valores[static_cast<size_t>(i)]), 1.0f,
                                                        Vector2(0.0f, mDegrauDoIntratavel.queda));
            }
            mLevas.push_back(leva);
        }
        for (Leva& leva : mLevas) {
            // A troca de fio terminou: cada valor assenta no fio novo, e volta a cair reto.
            if (leva.trocandoAte >= 0.0f && t >= leva.trocandoAte) {
                leva.trocandoAte = -1.0f;
                for (int i = 0; i < kFios; ++i) {
                    BossProjectile* v = leva.porFio[static_cast<size_t>(i)];
                    if (!Vivo(v)) continue;
                    v->SetPosition(Vector2(XDoFio(i), v->GetPosition().y));
                    if (auto* rb = v->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, mDegrauDoIntratavel.queda));
                }
            }
            if (leva.trocandoAte >= 0.0f || leva.camada >= static_cast<int>(Camadas().size())) continue;
            // A leva cai junta: a altura dela e a de qualquer valor vivo.
            float y = -1.0f;
            for (BossProjectile* v : leva.porFio) if (Vivo(v)) { y = v->GetPosition().y; break; }
            if (y < 0.0f || y < YDaCamada(leva.camada)) continue;
            // O comparador: o par fora de ordem troca de fio, na diagonal.
            bool trocou = false;
            for (const auto& [i, j] : Camadas()[static_cast<size_t>(leva.camada)]) {
                auto& vi = leva.valores[static_cast<size_t>(i)];
                auto& vj = leva.valores[static_cast<size_t>(j)];
                if (vi <= vj) continue;
                std::swap(vi, vj);
                std::swap(leva.porFio[static_cast<size_t>(i)], leva.porFio[static_cast<size_t>(j)]);
                for (const int fio : {i, j}) {
                    BossProjectile* v = leva.porFio[static_cast<size_t>(fio)];
                    if (!Vivo(v)) continue;
                    const float vx = (XDoFio(fio) - v->GetPosition().x) / kTroca;
                    if (auto* rb = v->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(vx, mDegrauDoIntratavel.queda));
                }
                trocou = true;
            }
            if (trocou) leva.trocandoAte = t + kTroca;
            ++leva.camada;
        }
        mLevas.erase(std::remove_if(mLevas.begin(), mLevas.end(), [](const Leva& l) {
            return std::none_of(l.porFio.begin(), l.porFio.end(), [](const BossProjectile* p) { return Vivo(p); });
        }), mLevas.end());
        return;
    }

    // O caixeiro: uma rodada por numero de cidades.
    const int rodada = std::min(static_cast<int>(kCidades.size()) - 1, static_cast<int>((t - kFimDaRede) / kDuracaoDaRodada));
    if (rodada != mRodadaDoCaixeiro) {
        for (Leva& l : mLevas) for (BossProjectile*& p : l.porFio) Matar(p);
        mLevas.clear();
        for (BossProjectile*& p : mCidades) Matar(p);
        mCidades.clear();
        mRodadaDoCaixeiro = rodada;
        const int n = kCidades[static_cast<size_t>(rodada)];
        mOndeCidades = Cidades(mSorteio, n, campo.x + 90.0f, campo.y + 115.0f, campo.w - 180.0f, campo.h * 0.48f);
        mMelhor.clear();
        mMelhorComprimento = std::numeric_limits<float>::max();
        mTentativa = 0;
        ComecarVolta(t);
    }
    // As cidades: o Ponto Extra pode leva-las, e elas voltam.
    mCidades.resize(mOndeCidades.size(), nullptr);
    for (size_t k = 0; k < mOndeCidades.size(); ++k) {
        if (Vivo(mCidades[k])) continue;
        mCidades[k] = Um(kGatilhoDaCidade, mOndeCidades[k], "Cidade");
        if (mCidades[k]) mCidades[k]->DefinirEnfeite(true);
    }
    if (!Vivo(mCaixeiro)) {
        mCaixeiro = Um(kGatilhoDoCaixeiro, mOndeCidades.front(), "");
        if (mCaixeiro) mCaixeiro->DefinirEnfeite(true);
    }

    const float u = (t - mInicioDaVolta) / mDegrauDoIntratavel.porTentativa;
    if (mCaixeiro) mCaixeiro->SetPosition(NaRota(mOndeCidades, mRota, std::min(u, 1.0f)));
    // A visita: cada cidade em que o caixeiro chega atira.
    const float total = Comprimento(mOndeCidades, mRota);
    float andado = 0.0f;
    for (size_t k = 1; k < mRota.size() && total > 0.0f; ++k) {
        andado += (mOndeCidades[static_cast<size_t>(mRota[k])] - mOndeCidades[static_cast<size_t>(mRota[k - 1])]).Length();
        if (k == mProximaParada && u * total >= andado) {
            const Vector2 cidade = mOndeCidades[static_cast<size_t>(mRota[k])];
            DispararEm(kGatilhoDaVisita, cidade, 0, AnguloAteOAluno(cidade));
            if (auto* c = mCidades[static_cast<size_t>(mRota[k])]) {
                if (auto* anim = c->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation("Acesa");
            }
            ++mProximaParada;
        }
    }
    if (u >= 1.0f) {
        const float comprimento = total;
        for (BossProjectile* c : mCidades) {
            if (!Vivo(c)) continue;
            if (auto* anim = c->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation("Cidade");
        }
        if (comprimento < mMelhorComprimento) {
            // A melhor rota ate agora: verificar e facil, e cada cidade comemora.
            mMelhorComprimento = comprimento;
            mMelhor = mRota;
            for (const Vector2& c : mOndeCidades) DispararEm(kGatilhoDaRotaNova, c);
        }
        ComecarVolta(t);
    }
    // A melhor rota em ouro, a da vez apagada.
    auto desenhar = [&](const std::vector<int>& rota, const SDL_Color cor) {
        for (size_t k = 0; k < rota.size(); ++k) {
            Linha(mOndeCidades[static_cast<size_t>(rota[k])], mOndeCidades[static_cast<size_t>(rota[(k + 1) % rota.size()])], cor);
        }
    };
    if (!mMelhor.empty()) desenhar(mMelhor, kMelhorRota);
    desenhar(mRota, kRotaDaVez);
    const int n = static_cast<int>(mOndeCidades.size());
    PlacarDaEtapa(kFase, 1, PlacarDoCaixeiro(n, mTentativa, mMelhor.empty() ? 0.0f : mMelhorComprimento));
    // O professor confere as rotas andando de um lado ao outro, por cima do mapa.
    const float onda = Math::Sin(Math::TwoPi * (t - kFimDaRede) / kVaiVemDoCaixeiro);
    mAlvo = Vector2(campo.x + campo.w * (0.5f + 0.25f * onda), campo.y + 55.0f);
}

// ---- A repescagem: a tempera simulada ----

void Arroyo::DesmontarTempera() {
    for (Solucao& s : mSolucoes) Matar(s.p);
    mSolucoes.clear();
    for (BossProjectile*& p : mVales) Matar(p);
    mVales.clear();
    mOndeVales.clear();
    mMetadeCongelada = -1;
    mSaltos.clear();
    mAvisoDaTemperaAte = -1.0f;
}

void Arroyo::Circulo(const Vector2 centro, const float raio, const SDL_Color cor, const int segmentos) {
    Vector2 antes(centro.x + raio, centro.y);
    for (int k = 1; k <= segmentos; ++k) {
        const float a = Math::TwoPi * static_cast<float>(k) / static_cast<float>(segmentos);
        const Vector2 agora(centro.x + raio * Math::Cos(a), centro.y + raio * Math::Sin(a));
        Linha(antes, agora, cor);
        antes = agora;
    }
}

void Arroyo::Termometro(const float temperatura) {
    // Na borda direita do campo: o tubo, o bulbo e o liquido, do azul do frio ao vermelho do quente.
    const SDL_FRect campo = Campo();
    const float x = campo.x + campo.w - 24.0f, topo = campo.y + 110.0f, fundo = campo.y + 300.0f;
    Linha(Vector2(x - 6.0f, topo), Vector2(x - 6.0f, fundo), kBordaDoTermometro);
    Linha(Vector2(x + 6.0f, topo), Vector2(x + 6.0f, fundo), kBordaDoTermometro);
    Linha(Vector2(x - 6.0f, topo), Vector2(x + 6.0f, topo), kBordaDoTermometro);
    Circulo(Vector2(x, fundo + 9.0f), 10.0f, kBordaDoTermometro, 20);
    const float nivel = Tempera::Nivel(temperatura);
    const auto cor = [](const float u) {
        return SDL_Color{static_cast<Uint8>(90.0f + 165.0f * u), static_cast<Uint8>(130.0f - 40.0f * u),
                         static_cast<Uint8>(255.0f - 205.0f * u), 220};
    };
    for (float y = fundo + 9.0f; y >= fundo - nivel * (fundo - topo); y -= 2.0f) {
        const float u = std::clamp((fundo - y) / (fundo - topo), 0.0f, 1.0f);
        const float meia = y > fundo ? 7.0f : 4.0f;
        Linha(Vector2(x - meia, y), Vector2(x + meia, y), cor(u));
    }
}

void Arroyo::AtualizarTempera(const float t, const float deltaTime) {
    using namespace Tempera;
    const SDL_FRect campo = Campo();
    if (mOndeVales.empty()) {
        // Os dois otimos locais, rasos, um de cada lado.
        mOndeVales = {Vector2(campo.x + campo.w * 0.2f, campo.y + campo.h * 0.42f),
                      Vector2(campo.x + campo.w * 0.8f, campo.y + campo.h * 0.42f)};
        mProximaSolucao = t;
    }
    mVales.resize(mOndeVales.size(), nullptr);
    for (size_t k = 0; k < mOndeVales.size(); ++k) {
        if (Vivo(mVales[k])) continue;
        mVales[k] = Um(kGatilhoDoVale, mOndeVales[k], "");
        if (mVales[k]) mVales[k]->DefinirEnfeite(true);
    }

    // O enxame: falta solucao (o Ponto Extra as leva), nasce outra do professor.
    mSolucoes.erase(std::remove_if(mSolucoes.begin(), mSolucoes.end(), [](const Solucao& s) { return !Vivo(s.p); }),
                    mSolucoes.end());
    if (static_cast<int>(mSolucoes.size()) < mDegrauDaTempera.solucoes && t >= mProximaSolucao) {
        mProximaSolucao = t + 0.25f;
        const Vector2 onde = GetPosition() + Vector2(0.0f, 40.0f);
        if (BossProjectile* p = Um(kGatilhoDaSolucao, onde, "Q4")) {
            mSolucoes.push_back(Solucao{p, onde, t + mSorteio.Real(0.0f, mDegrauDaTempera.porPasso)});
        }
    }

    const float temperatura = Temperatura(t);
    const Vector2 aluno = PosicaoDoAluno();
    const char* cara = temperatura > 600.0f ? "Q4" : temperatura > 250.0f ? "Q3" : temperatura > 60.0f ? "Q2" : "Q1";

    // A paisagem de energia: os aneis dos dois otimos locais, rasos, e os do aluno, mais fundos.
    for (const Vector2& v : mOndeVales) {
        for (int k = 1; k <= kNiveisRasos; ++k) Circulo(v, kRaioDoNivelRaso * static_cast<float>(k), kNivelRaso);
    }
    for (int k = 1; k <= kNiveisFundos; ++k) Circulo(aluno, kRaioDoNivelFundo * static_cast<float>(k), kNivelFundo, 44);
    Termometro(temperatura);

    for (Solucao& s : mSolucoes) {
        const Vector2 aqui = s.p->GetPosition();
        if (t >= s.proxima) {
            s.proxima = t + mDegrauDaTempera.porPasso;
            Vector2 proposta = Proposta(s.alvo, temperatura, mSorteio);
            proposta.x = std::clamp(proposta.x, campo.x + 16.0f, campo.x + campo.w - 16.0f);
            proposta.y = std::clamp(proposta.y, campo.y + 16.0f, campo.y + campo.h - 16.0f);
            const float delta = Energia(proposta, aluno, mOndeVales) - Energia(s.alvo, aluno, mOndeVales);
            if (Aceita(delta, temperatura, mSorteio.Real(0.0f, 1.0f))) {
                // O passo para pior aceito e o coracao da tempera: fica riscado.
                if (delta > 0.0f) mSaltos.push_back(Salto{s.alvo, proposta, t + kDuracaoDoSalto});
                s.alvo = proposta;
            }
        }
        // Vai ao ponto aceito sem passar da rapidez maxima.
        Vector2 falta = s.alvo - aqui;
        const float d = falta.Length();
        Vector2 v = Vector2::Zero;
        if (d > 1.0f) v = falta * (std::min(mDegrauDaTempera.rapidez, d / std::max(deltaTime, 1.0e-3f)) / d);
        if (auto* rb = s.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(v);
        if (auto* anim = s.p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) anim->SetAnimation(cara);
    }

    mSaltos.erase(std::remove_if(mSaltos.begin(), mSaltos.end(), [t](const Salto& s) { return t >= s.ate; }), mSaltos.end());
    for (const Salto& s : mSaltos) Linha(s.de, s.para, kSaltoPior);

    // No fim de cada esfriamento, quem ficou preso num otimo local estoura.
    const int metade = std::min(kEtapas - 1, static_cast<int>(t / kReaquecimento));
    const float inicioDaMetade = static_cast<float>(metade) * kReaquecimento;
    const float fimDaMetade = inicioDaMetade + kReaquecimento - kAntesDoReaquecimento;
    if (t >= fimDaMetade && mMetadeCongelada < metade) {
        mMetadeCongelada = metade;
        int presas = 0;
        for (const Solucao& s : mSolucoes) {
            for (const Vector2& v : mOndeVales) {
                if ((s.p->GetPosition() - v).Length() < kPresa) {
                    DispararEm(kGatilhoDoCongelado, s.p->GetPosition());
                    ++presas;
                    break;
                }
            }
        }
        if (presas > 0) {
            mAvisoDaTempera = PlacarDoOtimoLocal(presas);
            mAvisoDaTemperaAte = inicioDaMetade + kReaquecimento;
        }
    }
    // O placar: o anuncio da etapa no comeco dela, o aviso do otimo local no fim, e no resto o estagio.
    if (t < mAvisoDaTemperaAte) PlacarDaEtapa(kFase, static_cast<size_t>(metade), mAvisoDaTempera);
    else if (t - inicioDaMetade >= kTempoDoAnuncio) PlacarDaEtapa(kFase, static_cast<size_t>(metade), Placar(temperatura));
}
