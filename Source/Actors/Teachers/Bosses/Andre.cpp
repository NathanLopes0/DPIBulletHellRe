//
// Andre - professor de Programacao 1 (INF 110)
//

#include "Andre.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "../../../Random.h"
#include "../../../Attacks/Prova.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../Player/Player.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {
    bool Vivo(const BossProjectile* p) { return p && p->GetState() == ActorState::Active; }

    std::uint32_t Sorteio() {
        return static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
    }

    void Parar(BossProjectile* p) {
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }

    void PorCara(BossProjectile* p, const std::string& cara) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }

    constexpr SDL_Color kApagado{110, 130, 170, 200};
    constexpr SDL_Color kAceso{255, 214, 72, 255};
    constexpr SDL_Color kVerdadeiro{96, 222, 112, 255};
    constexpr SDL_Color kFalso{236, 84, 72, 255};

    // A fase 1.

    /// O losango do fluxograma: meia largura e meia altura.
    constexpr float kMeioLosango = 44.0f;
    constexpr float kMeiaAlturaDoLosango = 32.0f;

    /// O balao estoura na borda de baixo do cabecalho.
    constexpr float kMeiaAlturaDoRotulo = 15.0f;

    /// O cabecalho fica aceso enquanto o confete dele comeca a cair.
    constexpr float kRotuloAceso = 0.45f;

    /// O confete nao nasce em fileira certinha: cada um ate kTremor px acima
    /// do lugar dele.
    constexpr float kTremor = 26.0f;

    /// O balao sem case sobe e sai do campo.
    constexpr float kVelocidadeDeSaida = 260.0f;

    // A fase 2.

    /// O cacho: o lugar de cada balao em relacao ao meio da casa - um cacho
    /// de verdade nao sobe em fila.
    constexpr float kDesvioDoCacho[Laco::kBaloesPorCacho] = {0.0f, 14.0f, 6.0f};
    const char* const kCoresDoBalaoOriginal[] = {"Red", "Blue", "Yellow"};

    /// O continue acende o indice de azul por este tempo.
    constexpr float kPuladoAceso = 0.45f;

    // A fase 3.

    /// Dois valores voando juntos vao lado a lado, para os dois numeros se
    /// lerem.
    constexpr float kEntreValores = 20.0f;

    /// O cartao de fora fica aceso este tanto depois de a funcao rodar.
    constexpr float kCartaoAceso = 0.45f;
}

Andre::Andre(Scene* scene) : Boss(scene)
{
}

std::optional<Vector2> Andre::AlvoDoMovimento() const {
    return mAlvo;
}

void Andre::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
    if (EhClone()) return;
    mRelogio += deltaTime;

    if (!mLinhas && mScene) {
        auto ator = std::make_unique<Actor>(mScene);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();

    // UMA MECANICA POR VEZ: quem fecha desmonta antes de quem abre montar, e
    // a prova escreve a abertura de cada problema por cima de tudo.
    const Sessao s = SessaoAtual();
    AtualizarCondicional(deltaTime, s);
    AtualizarLaco(s);
    AtualizarFuncao(deltaTime, s);
    AtualizarVetor(s.vetor);
    AtualizarProva();
}

Andre::Sessao Andre::SessaoAtual() {
    Sessao s;
    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm) return s;
    s.fase = fsm->GetStateName();
    const float t = fsm->GetStateTime();
    std::string chave = s.fase;

    s.agora = t;
    if (s.fase == Condicional::kFaseDoSe) {
        s.qual = Mecanica::Condicional;
    } else if (s.fase == Laco::kFaseDoLaco) {
        s.qual = Mecanica::Laco;
    } else if (s.fase == Funcao::kFaseDaFuncao) {
        s.qual = Mecanica::Funcao;
    } else if (s.fase == Prova::kFaseDaProva) {
        // A PROVA: cada problema e uma sessao da mecanica da fase de onde ele
        // vem, depois da abertura dele.
        const int k = Prova::ProblemaEm(t);
        chave += "/" + std::to_string(k);
        s.agora = 0.0f;
        auto usaOVetor = [](const int j) {
            return j >= 0 && j < Prova::kProblemas && Prova::kProva[static_cast<size_t>(j)].tema != Prova::Tema::Condicional;
        };
        if (k >= 0) {
            const auto& p = Prova::kProva[static_cast<size_t>(k)];
            const float local = t - p.inicio - Prova::kAbertura;
            // O VETOR FICA durante o problema que o usa e na abertura do
            // seguinte: os ultimos cachos ainda estao subindo.
            s.vetor = usaOVetor(k) || (local < 0.0f && usaOVetor(k - 1));
            if (local >= 0.0f) {
                s.qual = p.tema == Prova::Tema::Condicional ? Mecanica::Condicional
                       : p.tema == Prova::Tema::Laco ? Mecanica::Laco : Mecanica::Funcao;
                s.primeiraVez = p.vez;
                s.quantos = p.quantos;
                s.agora = local;
                chave += "/roda";
            }
        }
    }
    if (s.qual == Mecanica::Laco || s.qual == Mecanica::Funcao) s.vetor = true;

    if (chave != mChaveDaSessao) {
        mChaveDaSessao = chave;
        ++mNumeroDaSessao;
    }
    s.numero = mNumeroDaSessao;
    return s;
}

SDL_FRect Andre::Campo() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene)) return battle->GetPlayfieldBounds();
    return SDL_FRect{0.0f, 0.0f, 1200.0f, 746.0f};
}

Vector2 Andre::PosicaoDoAluno() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene); battle && battle->Alvo()) {
        return battle->Alvo()->GetPosition();
    }
    const SDL_FRect campo = Campo();
    return Vector2(campo.x + campo.w / 2.0f, campo.y + campo.h - 60.0f);
}

int Andre::CasaDoAluno() const {
    const SDL_FRect campo = Campo();
    return Funcao::CasaDoX(PosicaoDoAluno().x - campo.x, campo.w);
}

std::vector<BossProjectile*> Andre::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
                                               const std::optional<float> angulo) {
    mDisparandoEm = true;
    mOndeDisparar = onde;
    mQuantos = quantos;
    mAnguloEm = angulo;
    mCriados.clear();
    Disparar(gatilho);
    mDisparandoEm = false;
    mQuantos = 0;
    mAnguloEm.reset();
    return mCriados;
}

void Andre::MudarDeLugar(const float de, const float ate, const float y) {
    const SDL_FRect campo = Campo();
    const float fracao = static_cast<float>(Sorteio() % 1001u) / 1000.0f;
    mAlvo = Vector2(campo.x + de + fracao * (ate - de), campo.y + y);
}

void Andre::Anunciar(const std::string& fase, const std::string& texto) {
    if (mAnunciosDoArquivo.count(fase) == 0) mAnunciosDoArquivo[fase] = AnuncioDaFase(fase);
    RegistrarAnuncio(fase, texto);
}

void Andre::Restaurar(const std::string& fase) {
    const auto it = mAnunciosDoArquivo.find(fase);
    if (it != mAnunciosDoArquivo.end()) RegistrarAnuncio(fase, it->second);
}

// ---------------------------------------------------------------------------
// O VETOR DO CHAO
// ---------------------------------------------------------------------------

void Andre::CriarVetor() {
    const SDL_FRect campo = Campo();
    mIndices.clear();
    mAcesoDoIndice.assign(Laco::kCasas, -1.0f);
    mPuladoDoIndice.assign(Laco::kCasas, -1.0f);
    mErroAte = -1.0f;
    for (int i = 0; i < Laco::kCasas; ++i) {
        const Vector2 onde(campo.x + Laco::XDaCasa(i, campo.w), campo.y + campo.h - Laco::kAcimaDaBorda);
        const auto criados = DispararEm(Laco::kGatilhoDoIndice, onde);
        BossProjectile* p = criados.empty() ? nullptr : criados.front();
        mIndices.push_back(p);
        if (!p) continue;
        p->DefinirEnfeite(true);
        Parar(p);
        PorCara(p, Laco::CaraDoIndice(i));
    }
}

void Andre::ApagarVetor() {
    for (BossProjectile* p : mIndices) if (Vivo(p)) p->Kill();
    mIndices.clear();
    mAcesoDoIndice.clear();
    mPuladoDoIndice.clear();
    mCachos.clear();
    mSubindo.clear();
}

void Andre::AgendarCacho(const int casa, const float quando, const int dono) {
    mCachos.push_back(Cacho{casa, quando, dono, false});
}

void Andre::AtualizarVetor(const bool preciso) {
    // O que o campo ja levou sai da conta antes de qualquer disparo deste
    // quadro: um projetil morto pode voltar do pool como outro.
    mSubindo.erase(std::remove_if(mSubindo.begin(), mSubindo.end(),
                                  [](const Subindo& s) { return !Vivo(s.p); }), mSubindo.end());
    for (BossProjectile*& p : mIndices) if (!Vivo(p)) p = nullptr;

    if (!preciso) {
        if (!mIndices.empty()) ApagarVetor();
        return;
    }
    // Um indice que o campo levou (nao deveria: e enfeite) volta com o vetor.
    if (mIndices.empty() || std::find(mIndices.begin(), mIndices.end(), nullptr) != mIndices.end()) {
        for (BossProjectile* p : mIndices) if (Vivo(p)) p->Kill();
        CriarVetor();
    }

    for (auto it = mCachos.begin(); it != mCachos.end();) {
        if (mRelogio < it->quando) { ++it; continue; }
        if (!it->aceso) {
            // A CASA ACENDE quando o i passa por ela - e o cacho sobe kAviso
            // depois.
            it->aceso = true;
            if (it->casa >= 0 && it->casa < Laco::kCasas) {
                auto& ate = mAcesoDoIndice[static_cast<size_t>(it->casa)];
                ate = std::max(ate, it->quando + Laco::kAviso);
            }
        }
        if (mRelogio < it->quando + Laco::kAviso) { ++it; continue; }
        const Cacho c = *it;
        it = mCachos.erase(it);
        SoltarCacho(c.casa, c.dono);
    }

    for (int i = 0; i < static_cast<int>(mIndices.size()); ++i) {
        BossProjectile* p = mIndices[static_cast<size_t>(i)];
        if (!p) continue;
        Parar(p);
        const auto k = static_cast<size_t>(i);
        if (mRelogio < mErroAte) PorCara(p, Laco::CaraDoIndiceErro(i));
        else if (mRelogio < mAcesoDoIndice[k]) PorCara(p, Laco::CaraDoIndiceAceso(i));
        else if (mRelogio < mPuladoDoIndice[k]) PorCara(p, Laco::CaraDoIndicePulado(i));
        else PorCara(p, Laco::CaraDoIndice(i));
    }
}

void Andre::SoltarCacho(const int casa, const int dono) {
    const SDL_FRect campo = Campo();
    const Vector2 onde(campo.x + Laco::XDaCasa(casa, campo.w), campo.y + campo.h + Laco::kAbaixoDoChao);
    const auto baloes = DispararEm(Laco::kGatilhoDoCacho, onde);
    const char* cor = kCoresDoBalaoOriginal[Sorteio() % 3u];
    for (size_t k = 0; k < baloes.size() && k < static_cast<size_t>(Laco::kBaloesPorCacho); ++k) {
        const float lado = (static_cast<float>(k) - 1.0f) * Laco::kLadoDoCacho;
        // A VELOCIDADE E A DO ATAQUE em fases.json: a WaveAttack a escreve no
        // primeiro quadro do projetil, por cima de qualquer outra.
        baloes[k]->SetPosition(onde + Vector2(lado, kDesvioDoCacho[k]));
        PorCara(baloes[k], cor);
        if (dono == mDonoQueOJuizMata) mSubindo.push_back(Subindo{baloes[k], dono});
    }
}

// ---------------------------------------------------------------------------
// A FASE 1: O DESVIO CONDICIONAL
// ---------------------------------------------------------------------------

Vector2 Andre::Decisao() const {
    return GetPosition() + Vector2(0.0f, Condicional::kAbaixoDoProfessor);
}

Vector2 Andre::CabecalhoDoRamo(const Condicional::Comando& c, const int ramo) const {
    const SDL_FRect campo = Campo();
    const int ramos = Condicional::Ramos(c);
    const float meio = (Condicional::InicioDaFaixa(ramo, ramos, campo.w) + Condicional::FimDaFaixa(ramo, ramos, campo.w)) / 2.0f;
    return Vector2(campo.x + meio, campo.y + Condicional::kYDosRotulos);
}

std::vector<Vector2> Andre::Caminho(const Condicional::Comando& c, const int ramo) const {
    // EM ANGULO RETO, como num fluxograma: sai pelo lado do losango que da
    // para a faixa (ou por cima, se ela esta logo acima), vai de lado ate ela e
    // sobe ate o cabecalho.
    const Vector2 d = Decisao();
    const Vector2 alvo = CabecalhoDoRamo(c, ramo) + Vector2(0.0f, kMeiaAlturaDoRotulo);
    Vector2 saida(d.x, d.y - kMeiaAlturaDoLosango);
    if (alvo.x < d.x - kMeioLosango) saida = Vector2(d.x - kMeioLosango, d.y);
    else if (alvo.x > d.x + kMeioLosango) saida = Vector2(d.x + kMeioLosango, d.y);
    return {saida, Vector2(alvo.x, saida.y), alvo};
}

void Andre::AtualizarCondicional(const float deltaTime, const Sessao& s) {
    const bool ativo = s.qual == Mecanica::Condicional;
    if (!ativo || s.numero != mSessaoDoSe) {
        // A SESSAO FECHOU: o fluxograma sai, e o placar volta ao do arquivo.
        if (mSessaoDoSe >= 0) {
            DesmontarCondicional();
            Restaurar(mFaseDoSe);
            mSessaoDoSe = -1;
        }
        if (!ativo) return;
        mSessaoDoSe = s.numero;
        mFaseDoSe = s.fase;
        mVez = s.primeiraVez;
        mFeitosDoSe = 0;
        mLimiteDoSe = s.quantos;
        mProximoComando = 0.0f;
        mBaloesDoComando = Condicional::kBaloesPorComando;
        mBaloes.clear();
        mQuedas.clear();
    }
    const float agora = s.agora;

    // O que o campo ja levou sai da conta antes de qualquer disparo.
    mBaloes.erase(std::remove_if(mBaloes.begin(), mBaloes.end(),
                                 [](const Balao& b) { return !Vivo(b.p); }), mBaloes.end());
    for (BossProjectile*& p : mRotulos) if (!Vivo(p)) p = nullptr;

    if (agora >= mProximoComando && (mLimiteDoSe == 0 || mFeitosDoSe < mLimiteDoSe)) NovoComando(agora);
    if (mBaloesDoComando < Condicional::kBaloesPorComando && agora >= mProximoBalao) {
        SoltarBalao();
        ++mBaloesDoComando;
        mProximoBalao += Condicional::kEntreBaloes;
    }

    // O SWITCH SEM BREAK: os cases de baixo, cada um na sua vez.
    for (auto it = mQuedas.begin(); it != mQuedas.end();) {
        if (agora < it->quando) { ++it; continue; }
        const Queda q = *it;
        it = mQuedas.erase(it);
        Estourar(q.c, q.ramo, q.cor, agora);
    }

    AtualizarBaloes(deltaTime, agora);

    for (size_t k = 0; k < mRotulos.size(); ++k) {
        BossProjectile* p = mRotulos[k];
        if (!p) continue;
        Parar(p);
        const bool aceso = k < mAcesoAte.size() && agora < mAcesoAte[k];
        PorCara(p, aceso ? Condicional::CaraDoRotuloAceso(mComando, static_cast<int>(k))
                         : Condicional::CaraDoRotulo(mComando, static_cast<int>(k)));
    }

    DesenharFluxograma(agora);
}

void Andre::NovoComando(const float agora) {
    mComando = Condicional::Proximo(mVez++, Sorteio());
    ++mFeitosDoSe;
    mProximoComando = static_cast<float>(mFeitosDoSe) * Condicional::kPorComando;
    mBaloesDoComando = 0;
    mProximoBalao = agora + Condicional::kPrimeiroBalao;
    Anunciar(mFaseDoSe, Condicional::Placar(mComando));

    // O PROFESSOR MUDA DE LUGAR a cada comando: o losango vai com ele, e a
    // faixa embaixo dele nao fica sempre a mesma.
    MudarDeLugar(Condicional::kXMinimo, Condicional::kXMaximo, Condicional::kYDoProfessor);

    // OS CABECALHOS DOS RAMOS, um por faixa.
    for (BossProjectile* p : mRotulos) if (Vivo(p)) p->Kill();
    mRotulos.clear();
    const int ramos = Condicional::Ramos(mComando);
    mAcesoAte.assign(static_cast<size_t>(ramos), -1.0f);
    for (int k = 0; k < ramos; ++k) {
        const auto criados = DispararEm(Condicional::kGatilhoDoRotulo, CabecalhoDoRamo(mComando, k));
        BossProjectile* p = criados.empty() ? nullptr : criados.front();
        mRotulos.push_back(p);
        if (!p) continue;
        p->DefinirEnfeite(true);
        Parar(p);
        PorCara(p, Condicional::CaraDoRotulo(mComando, k));
    }
}

void Andre::SoltarBalao() {
    Balao b;
    b.c = mComando;
    b.n = Condicional::Sortear(mComando, Sorteio());
    b.cor = static_cast<int>(Sorteio() % static_cast<std::uint32_t>(Condicional::kCores));
    const auto criados = DispararEm(Condicional::kGatilhoDoBalao, GetPosition());
    if (criados.empty()) return;
    b.p = criados.front();
    Parar(b.p);
    PorCara(b.p, Condicional::CaraDoBalao(b.n, b.cor));
    mBaloes.push_back(b);
}

void Andre::AtualizarBaloes(const float deltaTime, const float agora) {
    const Vector2 decisao = Decisao();
    const float passo = Condicional::kVelocidadeDoBalao * deltaTime;
    const SDL_FRect campo = Campo();

    for (auto it = mBaloes.begin(); it != mBaloes.end();) {
        Balao& b = *it;
        BossProjectile* p = b.p;
        Parar(p);
        switch (b.etapa) {
            case Etapa::Descendo: {
                const Vector2 falta = decisao - p->GetPosition();
                if (falta.Length() > passo) {
                    p->SetPosition(p->GetPosition() + Vector2::Normalize(falta) * passo);
                    break;
                }
                // A CONDICAO E AVALIADA AQUI, com o valor deste balao - e a
                // atribuicao troca o numero dele na frente do aluno.
                p->SetPosition(decisao);
                const int depois = Condicional::Depois(b.c, b.n);
                if (depois != b.n) {
                    b.n = depois;
                    PorCara(p, Condicional::CaraDoBalao(b.n, b.cor));
                }
                b.ramos = Condicional::Executados(b.c, b.n);
                b.etapa = Etapa::Decidindo;
                b.ate = agora + Condicional::kNaDecisao;
                mDecisaoAte = b.ate;
                mDecisaoRamo = b.ramos.empty() ? -1 : b.ramos.front();
                // O PLACAR EXPLICA a regra que este balao mostrou - e volta ao
                // comando no balao seguinte que nao tem o que explicar.
                if (b.c == mComando) {
                    const std::string porque = Condicional::Explicacao(b.c, b.n);
                    Anunciar(mFaseDoSe, porque.empty() ? Condicional::Placar(b.c) : porque);
                }
                break;
            }
            case Etapa::Decidindo:
                p->SetPosition(decisao);
                if (agora < b.ate) break;
                if (b.ramos.empty()) {
                    b.etapa = Etapa::Saindo;     // nenhum case: nada executa
                } else {
                    b.caminho = Caminho(b.c, b.ramos.front());
                    b.etapa = Etapa::Voando;
                }
                break;
            case Etapa::Voando: {
                float resta = passo;
                while (resta > 0.0f && !b.caminho.empty()) {
                    const Vector2 falta = b.caminho.front() - p->GetPosition();
                    const float d = falta.Length();
                    if (d <= resta) {
                        p->SetPosition(b.caminho.front());
                        b.caminho.erase(b.caminho.begin());
                        resta -= d;
                    } else {
                        p->SetPosition(p->GetPosition() + falta * (resta / d));
                        resta = 0.0f;
                    }
                }
                if (!b.caminho.empty()) break;
                // CHEGOU AO CABECALHO DO RAMO: estoura, e o confete cai pela
                // faixa. No switch sem break, os cases de baixo vem depois.
                p->Kill();
                Estourar(b.c, b.ramos.front(), b.cor, agora);
                for (size_t k = 1; k < b.ramos.size(); ++k) {
                    mQuedas.push_back(Queda{b.c, b.ramos[k], b.cor, agora + Condicional::kCascata * static_cast<float>(k)});
                }
                it = mBaloes.erase(it);
                continue;
            }
            case Etapa::Saindo:
                p->SetPosition(p->GetPosition() + Vector2(0.0f, -kVelocidadeDeSaida * deltaTime));
                if (p->GetPosition().y < campo.y - 60.0f) {
                    p->Kill();
                    it = mBaloes.erase(it);
                    continue;
                }
                break;
        }
        ++it;
    }
}

void Andre::Estourar(const Condicional::Comando& c, const int ramo, const int cor, const float agora) {
    const SDL_FRect campo = Campo();
    const int ramos = Condicional::Ramos(c);
    const float inicio = campo.x + Condicional::InicioDaFaixa(ramo, ramos, campo.w);
    const float fim = campo.x + Condicional::FimDaFaixa(ramo, ramos, campo.w);
    const float y = campo.y + Condicional::kYDosRotulos;
    const auto confetes = Condicional::Confetes(inicio, fim);
    const auto pedacos = DispararEm(Condicional::kGatilhoDoConfete, Vector2((inicio + fim) / 2.0f, y),
                                    static_cast<int>(confetes.size()));
    for (size_t k = 0; k < pedacos.size() && k < confetes.size(); ++k) {
        const float tremor = Random::GetFloatRange(0.0f, kTremor);
        pedacos[k]->SetPosition(Vector2(confetes[k].x, y - confetes[k].acima - tremor));
        PorCara(pedacos[k], Condicional::CaraDoConfete(cor));
    }
    if (c == mComando && ramo >= 0 && static_cast<size_t>(ramo) < mAcesoAte.size()) {
        mAcesoAte[static_cast<size_t>(ramo)] = agora + kRotuloAceso;
    }
}

void Andre::DesenharFluxograma(const float agora) {
    if (!mLinhas || mRotulos.empty()) return;
    const Vector2 d = Decisao();

    // A SETA QUE ENTRA NO LOSANGO, do professor.
    mLinhas->Acrescentar(GetPosition() + Vector2(0.0f, 40.0f), Vector2(d.x, d.y - kMeiaAlturaDoLosango), kApagado);

    // O LOSANGO: verde quando a condicao deu verdadeiro, vermelho no else, e
    // amarelo no case do switch.
    SDL_Color cor = kApagado;
    if (agora < mDecisaoAte) {
        if (mComando.forma == Condicional::Forma::Escolha) cor = mDecisaoRamo < 0 ? kFalso : kAceso;
        else cor = mDecisaoRamo == 0 ? kVerdadeiro : kFalso;
    }
    const Vector2 cima(d.x, d.y - kMeiaAlturaDoLosango), baixo(d.x, d.y + kMeiaAlturaDoLosango);
    const Vector2 esquerda(d.x - kMeioLosango, d.y), direita(d.x + kMeioLosango, d.y);
    mLinhas->Acrescentar(cima, direita, cor);
    mLinhas->Acrescentar(direita, baixo, cor);
    mLinhas->Acrescentar(baixo, esquerda, cor);
    mLinhas->Acrescentar(esquerda, cima, cor);

    // AS SETAS DOS RAMOS: a que tem um balao andando nela, ou cujo confete
    // acabou de cair, acesa.
    const int ramos = Condicional::Ramos(mComando);
    for (int k = 0; k < ramos; ++k) {
        bool aceso = static_cast<size_t>(k) < mAcesoAte.size() && agora < mAcesoAte[static_cast<size_t>(k)];
        for (const Balao& b : mBaloes) {
            aceso = aceso || (b.etapa == Etapa::Voando && b.c == mComando && !b.ramos.empty() && b.ramos.front() == k);
        }
        const auto caminho = Caminho(mComando, k);
        for (size_t i = 1; i < caminho.size(); ++i) {
            mLinhas->Acrescentar(caminho[i - 1], caminho[i], aceso ? kAceso : kApagado);
        }
    }
}

void Andre::DesmontarCondicional() {
    for (BossProjectile* p : mRotulos) if (Vivo(p)) p->Kill();
    for (const Balao& b : mBaloes) if (Vivo(b.p)) b.p->Kill();
    mRotulos.clear();
    mAcesoAte.clear();
    mBaloes.clear();
    mQuedas.clear();
}

// ---------------------------------------------------------------------------
// A FASE 2: A REPETICAO
// ---------------------------------------------------------------------------

void Andre::AtualizarLaco(const Sessao& s) {
    const bool ativo = s.qual == Mecanica::Laco;
    if (!ativo || s.numero != mSessaoDoLaco) {
        if (mSessaoDoLaco >= 0) {
            mRodando = false;
            mDonoQueOJuizMata = -1;
            Restaurar(mFaseDoLaco);
            mSessaoDoLaco = -1;
        }
        if (!ativo) return;
        mSessaoDoLaco = s.numero;
        mFaseDoLaco = s.fase;
        mVezDoLaco = s.primeiraVez;
        mFeitosDoLaco = 0;
        mLimiteDoLaco = s.quantos;
        mRodando = false;
        mProximoLaco = Laco::kPrimeiro;
    }
    const float agora = s.agora;

    if (!mRodando && agora >= mProximoLaco && (mLimiteDoLaco == 0 || mFeitosDoLaco < mLimiteDoLaco)) NovoLaco(agora);
    if (!mRodando) return;

    const float t = agora - mInicioDoLaco;
    while (mProximaVisita < mVisitas.size() && mVisitas[mProximaVisita].quando <= t) {
        Visitar(mVisitas[mProximaVisita++]);
    }
    // O JUIZ DO BOCA MATA O while (1): o placar da o veredito, e o que o
    // programa soltou some com ele.
    if (mPrograma.forma == Laco::Forma::Infinito && !mTempoEsgotado && t >= Laco::kLimiteDeTempo) {
        mTempoEsgotado = true;
        Anunciar(mFaseDoLaco, Laco::kTempoEsgotado);
        for (const Subindo& b : mSubindo) if (b.dono == mDonoDoLaco && Vivo(b.p)) b.p->Kill();
        mSubindo.erase(std::remove_if(mSubindo.begin(), mSubindo.end(),
                                      [this](const Subindo& b) { return b.dono == mDonoDoLaco; }), mSubindo.end());
        mCachos.erase(std::remove_if(mCachos.begin(), mCachos.end(),
                                     [this](const Cacho& c) { return c.dono == mDonoDoLaco; }), mCachos.end());
        for (float& ate : mAcesoDoIndice) ate = -1.0f;
        mDonoQueOJuizMata = -1;
    }
    if (t >= mDuracaoDoLaco) {
        mRodando = false;
        mProximoLaco = agora + Laco::kEntreLacos;
    }
}

void Andre::NovoLaco(const float agora) {
    mPrograma = Laco::Proximo(mVezDoLaco++, Sorteio());
    ++mFeitosDoLaco;
    ++mDonoDoLaco;
    mVisitas = Laco::Execucao(mPrograma, Sorteio());
    mDuracaoDoLaco = Laco::Duracao(mPrograma, mVisitas);
    mProximaVisita = 0;
    mInicioDoLaco = agora;
    mRodando = true;
    mTempoEsgotado = false;
    mDonoQueOJuizMata = mPrograma.forma == Laco::Forma::Infinito ? mDonoDoLaco : -1;
    Anunciar(mFaseDoLaco, Laco::Placar(mPrograma));
    MudarDeLugar(Laco::kXMinimo, Laco::kXMaximo, Laco::kYDoProfessor);
}

void Andre::Visitar(const Laco::Visita& v) {
    const SDL_FRect campo = Campo();
    switch (v.tipo) {
        case Laco::Tipo::Balao:
            AgendarCacho(v.i, mRelogio, mDonoDoLaco);
            break;
        case Laco::Tipo::Pulado:
            if (v.i >= 0 && v.i < Laco::kCasas && static_cast<size_t>(v.i) < mPuladoDoIndice.size()) {
                mPuladoDoIndice[static_cast<size_t>(v.i)] = mRelogio + kPuladoAceso;
            }
            break;
        case Laco::Tipo::Fora: {
            // O i SAIU DO VETOR: o programa capota, e a memoria despejada voa
            // da borda por onde ele saiu, para dentro do campo.
            Anunciar(mFaseDoLaco, Laco::kSegfault);
            mErroAte = mRelogio + Laco::kCapotando;
            const bool direita = v.i >= Laco::kCasas;
            const Vector2 borda(direita ? campo.x + campo.w : campo.x, campo.y + campo.h - Laco::kAcimaDoChao);
            const float angulo = direita ? 180.0f + Laco::kInclinacaoDoDespejo : 360.0f - Laco::kInclinacaoDoDespejo;
            const auto bytes = DispararEm(Laco::kGatilhoDoDespejo, borda, 0, angulo);
            for (size_t k = 0; k < bytes.size(); ++k) PorCara(bytes[k], Laco::CaraDoByte(static_cast<int>(k)));
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// A FASE 3: AS FUNCOES
// ---------------------------------------------------------------------------

BossProjectile* Andre::PorCartao(const Funcao::Nome f, const Vector2 onde) {
    const auto criados = DispararEm(Funcao::kGatilhoDoCartao, onde);
    BossProjectile* p = criados.empty() ? nullptr : criados.front();
    if (!p) return nullptr;
    p->DefinirEnfeite(true);
    Parar(p);
    PorCara(p, Funcao::CaraDoCartao(f));
    return p;
}

void Andre::ApagarCartoes() {
    if (Vivo(mCartaoDeFora)) mCartaoDeFora->Kill();
    if (Vivo(mCartaoDeDentro)) mCartaoDeDentro->Kill();
    mCartaoDeFora = nullptr;
    mCartaoDeDentro = nullptr;
}

void Andre::Levar(const std::vector<int>& valores, const Vector2 de, const std::optional<Vector2> para) {
    for (const Valor& v : mValores) if (Vivo(v.p)) v.p->Kill();
    mValores.clear();
    const float meio = (static_cast<float>(valores.size()) - 1.0f) / 2.0f;
    for (size_t k = 0; k < valores.size(); ++k) {
        const Vector2 desvio((static_cast<float>(k) - meio) * kEntreValores * 2.0f, 0.0f);
        const auto criados = DispararEm(Funcao::kGatilhoDoValor, de + desvio);
        if (criados.empty()) continue;
        BossProjectile* p = criados.front();
        p->DefinirEnfeite(true);
        Parar(p);
        PorCara(p, Funcao::CaraDoValor(valores[k]));
        mValores.push_back(Valor{p, desvio, para});
    }
}

bool Andre::MoverValores(const float deltaTime) {
    const float passo = Funcao::kVelocidadeDoValor * deltaTime;
    bool todos = true;
    for (auto it = mValores.begin(); it != mValores.end();) {
        BossProjectile* p = it->p;
        if (!Vivo(p)) { it = mValores.erase(it); continue; }
        Parar(p);
        const Vector2 alvo = (it->para ? *it->para : GetPosition()) + it->desvio;
        const Vector2 falta = alvo - p->GetPosition();
        if (falta.Length() <= passo) {
            // CHEGOU: o valor entra na funcao (ou volta para quem chamou).
            p->Kill();
            it = mValores.erase(it);
            continue;
        }
        p->SetPosition(p->GetPosition() + Vector2::Normalize(falta) * passo);
        todos = false;
        ++it;
    }
    return todos;
}

void Andre::AtualizarFuncao(const float deltaTime, const Sessao& s) {
    const bool ativo = s.qual == Mecanica::Funcao;
    if (!ativo || s.numero != mSessaoDaFuncao) {
        if (mSessaoDaFuncao >= 0) {
            DesmontarFuncao();
            Restaurar(mFaseDaFuncao);
            mSessaoDaFuncao = -1;
        }
        if (!ativo) return;
        mSessaoDaFuncao = s.numero;
        mFaseDaFuncao = s.fase;
        mVezDaFuncao = s.primeiraVez;
        mFeitasDaFuncao = 0;
        mLimiteDaFuncao = s.quantos;
        mPasso = Passo::Parado;
        mProximaChamada = Funcao::kPrimeiro;
    }
    const float agora = s.agora;

    if (mPasso == Passo::Parado) {
        if (agora >= mProximaChamada && (mLimiteDaFuncao == 0 || mFeitasDaFuncao < mLimiteDaFuncao)) NovaChamada(agora);
    }

    const bool chegaram = MoverValores(deltaTime);
    switch (mPasso) {
        case Passo::Parado:
            break;
        case Passo::IdaDeDentro:
            if (chegaram) {
                mPasso = Passo::NaDeDentro;
                mAteDoPasso = agora + Funcao::kExecutando;
            }
            break;
        case Passo::NaDeDentro:
            if (agora >= mAteDoPasso) {
                // O RETORNO VOLTA PARA QUEM CHAMOU - o professor, onde ele
                // estiver.
                Levar({Funcao::Retorno(mChamada)}, mOndeDeDentro, std::nullopt);
                mPasso = Passo::Volta;
            }
            break;
        case Passo::Volta:
            if (chegaram) {
                // E O PLACAR TROCA A CHAMADA PELO VALOR DELA.
                Anunciar(mFaseDaFuncao, Funcao::PlacarComValores(mChamada, 0));
                mArgumentos = Funcao::ArgumentosDaExterna(mChamada, 0);
                Levar(mArgumentos, GetPosition(), mOndeDeFora);
                mPasso = Passo::IdaDeFora;
            }
            break;
        case Passo::IdaDeFora:
            if (chegaram) {
                mPasso = Passo::NaDeFora;
                mAteDoPasso = agora + Funcao::kExecutando;
            }
            break;
        case Passo::NaDeFora:
            if (agora >= mAteDoPasso) Executar(agora);
            break;
        case Passo::Seguindo:
            // O PONTEIRO: o cartao ligado ao aluno, e o cacho sobe onde ele
            // esta agora.
            if (mLinhas && Vivo(mCartaoDeFora)) mLinhas->Acrescentar(mOndeDeFora, PosicaoDoAluno(), kAceso);
            if (agora >= mProximoSegue) {
                mProximoSegue += Funcao::kEntreSegue;
                AgendarCacho(CasaDoAluno(), mRelogio, 0);
            }
            if (agora >= mSeguindoAte) TerminarChamada(agora);
            break;
    }

    // OS CARTOES: o de dentro aceso enquanto executa; o de fora enquanto
    // executa e um pouco depois. E as chamadas, do professor a cada um.
    if (Vivo(mCartaoDeDentro)) {
        const bool aceso = mPasso == Passo::NaDeDentro;
        PorCara(mCartaoDeDentro, aceso ? Funcao::CaraDoCartaoAceso(Funcao::Interna(mChamada))
                                       : Funcao::CaraDoCartao(Funcao::Interna(mChamada)));
        Parar(mCartaoDeDentro);
        if (mLinhas) {
            const bool indo = mPasso == Passo::IdaDeDentro || mPasso == Passo::NaDeDentro || mPasso == Passo::Volta;
            mLinhas->Acrescentar(GetPosition(), mOndeDeDentro, indo ? kAceso : kApagado);
        }
    }
    if (Vivo(mCartaoDeFora)) {
        const bool aceso = mPasso == Passo::NaDeFora || mPasso == Passo::Seguindo || agora < mForaAcesoAte;
        PorCara(mCartaoDeFora, aceso ? Funcao::CaraDoCartaoAceso(Funcao::Externa(mChamada))
                                     : Funcao::CaraDoCartao(Funcao::Externa(mChamada)));
        Parar(mCartaoDeFora);
        if (mLinhas) {
            const bool indo = mPasso == Passo::IdaDeFora || mPasso == Passo::NaDeFora;
            mLinhas->Acrescentar(GetPosition(), mOndeDeFora, indo ? kAceso : kApagado);
        }
    }
}

void Andre::NovaChamada(const float agora) {
    mChamada = Funcao::Proximo(mVezDaFuncao++, Sorteio());
    ++mFeitasDaFuncao;
    Anunciar(mFaseDaFuncao, Funcao::Placar(mChamada));
    MudarDeLugar(Funcao::kXMinimo, Funcao::kXMaximo, Funcao::kYDoProfessor);

    // OS CARTOES DESTA CHAMADA: o da funcao de fora no meio, o da de dentro
    // de um lado.
    ApagarCartoes();
    const SDL_FRect campo = Campo();
    mOndeDeFora = Vector2(campo.x + campo.w / 2.0f, campo.y + Funcao::kYDoCartao);
    mCartaoDeFora = PorCartao(Funcao::Externa(mChamada), mOndeDeFora);
    if (Funcao::TemInterna(mChamada)) {
        const bool esquerda = (Sorteio() % 2u) == 0u;
        mOndeDeDentro = Vector2(campo.x + campo.w * (esquerda ? Funcao::kXDoLado : 1.0f - Funcao::kXDoLado),
                                campo.y + Funcao::kYDoCartaoDeDentro);
        mCartaoDeDentro = PorCartao(Funcao::Interna(mChamada), mOndeDeDentro);
        Levar(Funcao::ArgumentosDaInterna(mChamada), GetPosition(), mOndeDeDentro);
        mPasso = Passo::IdaDeDentro;
        return;
    }
    switch (mChamada.forma) {
        case Funcao::Forma::PorValor: {
            // A COPIA: a casa do aluno agora, e o valor sai DELE para a funcao.
            const int casa = CasaDoAluno();
            Anunciar(mFaseDaFuncao, Funcao::PlacarComValores(mChamada, casa));
            mArgumentos = Funcao::ArgumentosDaExterna(mChamada, casa);
            Levar(mArgumentos, PosicaoDoAluno(), mOndeDeFora);
            mPasso = Passo::IdaDeFora;
            break;
        }
        case Funcao::Forma::PorReferencia:
            // O ENDERECO nao voa: a funcao passa a enxergar o aluno.
            mArgumentos.clear();
            mPasso = Passo::NaDeFora;
            mAteDoPasso = agora + Funcao::kExecutando;
            break;
        default:
            mArgumentos = Funcao::ArgumentosDaExterna(mChamada, 0);
            Levar(mArgumentos, GetPosition(), mOndeDeFora);
            mPasso = Passo::IdaDeFora;
            break;
    }
}

void Andre::Executar(const float agora) {
    switch (Funcao::Externa(mChamada)) {
        case Funcao::Nome::Baloes: {
            // UM LACO DENTRO DA FUNCAO: de 'de' a 'ate', um cacho por casa.
            const auto casas = mArgumentos.size() >= 2 ? Funcao::CasasDosBaloes(mArgumentos[0], mArgumentos[1])
                                                       : std::vector<int>{};
            for (size_t k = 0; k < casas.size(); ++k) {
                AgendarCacho(casas[k], mRelogio + Funcao::kEntreCachos * static_cast<float>(k), 0);
            }
            TerminarChamada(agora);
            break;
        }
        case Funcao::Nome::Leque: {
            const int n = mArgumentos.empty() ? 0 : mArgumentos.front();
            const Vector2 d = PosicaoDoAluno() - mOndeDeFora;
            const float angulo = Math::ToDegrees(Math::Atan2(d.y, d.x));
            if (n > 0) DispararEm(Funcao::kGatilhoDoLeque, mOndeDeFora, n, angulo);
            TerminarChamada(agora);
            break;
        }
        case Funcao::Nome::Segue:
            mPasso = Passo::Seguindo;
            mSeguindoAte = agora + Funcao::kDuracaoDoSegue;
            mProximoSegue = agora;
            break;
        default:
            TerminarChamada(agora);
            break;
    }
}

void Andre::TerminarChamada(const float agora) {
    mPasso = Passo::Parado;
    mProximaChamada = agora + Funcao::kEntreComandos;
    mForaAcesoAte = agora + kCartaoAceso;
}

void Andre::DesmontarFuncao() {
    ApagarCartoes();
    for (const Valor& v : mValores) if (Vivo(v.p)) v.p->Kill();
    mValores.clear();
    mPasso = Passo::Parado;
}

// ---------------------------------------------------------------------------
// A REPESCAGEM: A PROVA NO BOCA
// ---------------------------------------------------------------------------

void Andre::AtualizarProva() {
    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm || fsm->GetStateName() != Prova::kFaseDaProva) {
        if (mProblema != -2) Restaurar(Prova::kFaseDaProva);
        mProblema = -2;
        return;
    }
    const int k = Prova::ProblemaEm(fsm->GetStateTime());
    if (k == mProblema) return;

    // O PROBLEMA QUE FECHOU GANHA O BALAO DELE - o YES do BOCA -, que sobe do
    // professor com a letra do problema. O que nao foi aceito nao ganha nada.
    if (Prova::Aceito(mProblema)) {
        for (BossProjectile* p : DispararEm(Prova::kGatilhoDoTrofeu, GetPosition())) {
            p->DefinirEnfeite(true);
            PorCara(p, Prova::CaraDoTrofeu(mProblema));
        }
    }
    Anunciar(Prova::kFaseDaProva, k >= 0 ? Prova::Abertura(k) : Prova::Veredito(mProblema));
    mProblema = k;
}

// ---------------------------------------------------------------------------
// OS GANCHOS DO DISPARO
// ---------------------------------------------------------------------------

void Andre::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // TODO ataque do Andre e de gatilho: nasce onde ele disse, no numero e na
    // direcao que ele pediu.
    if (!mDisparandoEm) return;
    params.firePosition = mOndeDisparar;
    if (mQuantos > 0) params.numProjectiles = mQuantos;
    if (mAnguloEm) params.centralAngle = *mAnguloEm;
}

void Andre::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}
