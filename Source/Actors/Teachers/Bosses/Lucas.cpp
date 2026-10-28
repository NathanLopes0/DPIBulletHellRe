//
// O chefe da INF 321 - Programacao Web.
//

#include "Lucas.h"

#include <algorithm>
#include <cmath>

#include "../../../Attacks/Ancora.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {
    /// O tiro guardado ainda e o mesmo se a idade dele bate com a esperada: o
    /// pool devolve o projetil com o relogio zerado.
    constexpr float kFolgaDaIdade = 0.1f;

    /// O desenho da pagina: o layout esmaecido, o elemento renderizado aceso.
    constexpr SDL_Color kLayout{110, 120, 160, 90};
    constexpr SDL_Color kElemento{232, 120, 64, 200};
    constexpr SDL_Color kAceso{238, 206, 86, 230};
    constexpr int kOrdemDasLinhas = 84;

    /// O professor fica no header, em cima do elemento que renderiza.
    constexpr float kYDoProfessor = 105.0f;
    /// O cursor alcanca o botao em kVooDoCursor (Ancora::Aproximar).
    constexpr float kRapidezDoCursor = 7.0f;

    /// A ficha do event loop chega a pilha antes de o callback atacar.
    constexpr float kRapidezDasFichas = 12.0f;
    constexpr float kChegadaNaPilha = 0.2f;
}

Lucas::Lucas(Scene* scene) : Boss(scene) {
}

void Lucas::SortearALuta() {
    mLutaSorteada = true;
    mSorteioDaPagina = SorteioDaFase(Pagina::kFaseDaPagina);
    mSorteioDoEstilo = SorteioDaFase(Estilo::kFaseDoEstilo);
    mDegrauDoEstilo = Estilo::DegrauDo(NivelDeDificuldade());
    mDegrauDaPagina = Pagina::DegrauDo(NivelDeDificuldade());
    mDegrauDoElixir = Elixir::DegrauDo(NivelDeDificuldade());
    mSorteioDoElixir = SorteioDaFase(Elixir::kFaseDoElixir);
    mEstrategias = Elixir::Sortear(mSorteioDoElixir);
    mSorteioDoLoop = SorteioDaFase(EventLoop::kFaseDoLoop);
    mDegrauDoLoop = EventLoop::DegrauDo(NivelDeDificuldade());
}

void Lucas::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    AtualizarPagina(deltaTime);
    AtualizarEstilo(deltaTime);
    AtualizarSupervisao(deltaTime);
    AtualizarEventLoop(deltaTime);
}

Vector2 Lucas::PosicaoDoAluno() const {

    if (const auto* battle = dynamic_cast<Battle*>(mScene); battle && battle->Alvo()) {
        return battle->Alvo()->GetPosition();
    }
    return GetPosition() + GetDirectionToPlayer() * GetDistanceToPlayer();
}

bool Lucas::Vivo(const BossProjectile* p, const float idade) {
    return p && p->GetState() == ActorState::Active && std::fabs(p->TempoDeVida() - idade) <= kFolgaDaIdade;
}

void Lucas::CriarLinhas() {
    if (mLinhas || !mScene) return;
    auto ator = std::make_unique<Actor>(mScene);
    mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
    mScene->AddActor(std::move(ator));
}

std::vector<BossProjectile*> Lucas::DispararDe(const char* gatilho, const Vector2 onde,
                                               const std::optional<float> rumo, const bool enfeite) {
    mDisparandoDe = true;
    mOndeDisparar = onde;
    if (rumo) {
        mRumoDoDisparo = *rumo;
    } else {
        const Vector2 d = PosicaoDoAluno() - onde;
        mRumoDoDisparo = Math::ToDegrees(Math::Atan2(d.y, d.x));
    }
    mEnfeiteAoNascer = enfeite;
    mDisparados.clear();
    Disparar(gatilho);
    mDisparandoDe = false;
    mEnfeiteAoNascer = false;
    return mDisparados;
}

// ---------------------------------------------------------------------------
// FASE 1 - A PAGINA
// ---------------------------------------------------------------------------

void Lucas::RecarregarPagina() {
    // O F5: os elementos da pagina velha somem - o cursor e o spinner que
    // sobraram com eles -, o layout fica, e vem outra pagina.
    if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->GetProjectileManager()) {
        for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
            if (p && p->GetState() == ActorState::Active && p->Posto() == Pagina::kPostoDaPagina) p->Kill();
        }
    }
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    mPagina = Pagina::Gerar(mSorteioDaPagina, largura);
    mRenderizados.clear();
    mProximoElemento = 0;
    mRecarregando = false;
}

void Lucas::AtualizarPagina(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naPagina = fsm && fsm->GetStateName() == Pagina::kFaseDaPagina && !EhClone();

    if (!naPagina) {
        if (mNaPagina) {
            // A FASE ACABOU: a pagina fecha, e o placar volta ao nome dela.
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->GetProjectileManager()) {
                for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
                    if (p && p->GetState() == ActorState::Active && p->Posto() == Pagina::kPostoDaPagina) p->Kill();
                }
            }
            mRenderizados.clear();
            if (mLinhas) mLinhas->Limpar();
            if (!mAnuncioDaPagina.empty()) RegistrarAnuncio(Pagina::kFaseDaPagina, mAnuncioDaPagina);
            mAlvo.reset();
        }
        mNaPagina = false;
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());

    if (!mNaPagina) {
        mNaPagina = true;
        mProximoRender = Pagina::kPrimeiro;
        mXDoProfessor = largura / 2.0f;
        if (mAnuncioDaPagina.empty()) mAnuncioDaPagina = AnuncioDaFase(Pagina::kFaseDaPagina);
        RegistrarAnuncio(Pagina::kFaseDaPagina, mAnuncioDaPagina);
        CriarLinhas();
        RecarregarPagina();
    }

    const float agora = fsm->GetStateTime();
    while (agora >= mProximoRender) {
        if (mRecarregando) RecarregarPagina();
        if (mProximoElemento >= mPagina.size()) {
            // A PAGINA ACABOU: o F5 vem depois de uma pausa.
            mRecarregando = true;
            mProximoRender += Pagina::kRecarga;
            RegistrarAnuncio(Pagina::kFaseDaPagina, "F5\nrecarregando a p\xc3\xa1gina");
            continue;
        }
        // UM ELEMENTO NOVO: o placar o escreve, e ele comeca o ataque dele.
        Renderizado r;
        r.e = mPagina[mProximoElemento++];
        const Pagina::Retangulo& c = r.e.caixa;
        switch (r.e.tipo) {
            case Pagina::Tipo::Botao: {
                const auto cursor = DispararDe(Pagina::kGatilhoDoCursor, GetPosition(), 90.0f);
                if (!cursor.empty()) r.cursor = cursor.front();
                break;
            }
            case Pagina::Tipo::Imagem:
                for (int k = 0; k < Pagina::kPontosDoSpinner; ++k) {
                    const auto ponto = DispararDe(Pagina::kGatilhoDoSpinner, Vector2(c.Cx(), c.Cy()), 90.0f, true);
                    if (!ponto.empty()) r.spinner.push_back(ponto.front());
                }
                break;
            case Pagina::Tipo::Marquee: {
                // A LINHA ATRAVESSA A TELA de um lado sorteado, descendo.
                const bool paraDireita = mSorteioDaPagina.Chance(0.5f);
                const float rumo = Math::ToDegrees(Math::Atan2(Pagina::kDescidaDoMarquee, paraDireita ? 220.0f : -220.0f));
                for (int k = 0; k < Pagina::kTagsDoMarquee; ++k) {
                    const float x = paraDireita ? -20.0f - k * Pagina::kEntreTagsDoMarquee
                                                : largura + 20.0f + k * Pagina::kEntreTagsDoMarquee;
                    DispararDe(Pagina::kGatilhoDoMarquee, Vector2(x, c.Cy()), rumo);
                }
                r.feito = true;
                break;
            }
            default:
                break;
        }
        mRenderizados.push_back(r);
        RegistrarAnuncio(Pagina::kFaseDaPagina, Pagina::Placar(r.e));
        mProximoRender += mDegrauDaPagina.porElemento;
    }

    RenderizarPagina(deltaTime);

    // O PROFESSOR FICA NO HEADER, em cima da regiao que esta renderizando - ou
    // da ancora para onde o link navegou. Em cima de cada elemento, como era,
    // ele trocava de lugar a cada 1,2 s, e o tiro do aluno nao o achava: quem
    // desviava terminava a fase com 58.
    if (!mRenderizados.empty()) {
        const Pagina::Elemento& e = mRenderizados.back().e;
        const auto layout = Pagina::Layout(largura);
        mXDoProfessor = layout[static_cast<size_t>(e.tipo == Pagina::Tipo::Link ? e.ancora : e.regiao)].Cx();
    }

    mAlvo = Vector2(mXDoProfessor, kYDoProfessor);
    DesenharPagina();
}

void Lucas::RenderizarPagina(const float deltaTime) {
    for (Renderizado& r : mRenderizados) {
        r.idade += deltaTime;
        if (r.feito) continue;
        const Pagina::Retangulo& c = r.e.caixa;
        const Vector2 centro(c.Cx(), c.Cy());
        switch (r.e.tipo) {
            case Pagina::Tipo::Botao:
                // O CURSOR VOA ATE O BOTAO E CLICA - a nao ser que o aluno o
                // derrube no caminho: event.preventDefault().
                if (!Vivo(r.cursor, r.idade)) {
                    r.feito = r.cancelado = true;
                    RegistrarAnuncio(Pagina::kFaseDaPagina, "event.preventDefault()\nclique cancelado");
                    break;
                }
                r.cursor->SetPosition(Ancora::Aproximar(r.cursor->GetPosition(), centro, kRapidezDoCursor, deltaTime));
                if (auto* rb = r.cursor->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
                if (r.idade >= Pagina::kVooDoCursor) {
                    if (auto* anim = r.cursor->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation("Clique");
                    DispararDe(Pagina::kGatilhoDoClique, centro, std::nullopt);
                    r.cursor->Kill();
                    r.cursor = nullptr;
                    r.feito = true;
                }
                break;
            case Pagina::Tipo::Imagem: {
                // O SPINNER GIRA ENQUANTO CARREGA; carregada, a imagem estoura.
                const float giro = r.idade * Pagina::kVoltasDoSpinner * 2.0f * Math::Pi;
                for (size_t k = 0; k < r.spinner.size(); ++k) {
                    BossProjectile* ponto = r.spinner[k];
                    if (!Vivo(ponto, r.idade)) continue;
                    const float a = giro + static_cast<float>(k) * 2.0f * Math::Pi / static_cast<float>(r.spinner.size());
                    ponto->SetPosition(centro + Vector2(Math::Cos(a), Math::Sin(a)) * Pagina::kRaioDoSpinner);
                    if (auto* rb = ponto->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
                }
                if (r.idade >= mDegrauDaPagina.carregando) {
                    for (BossProjectile* ponto : r.spinner) if (Vivo(ponto, r.idade)) ponto->Kill();
                    r.spinner.clear();
                    DispararDe(Pagina::kGatilhoDaCarga, centro, 90.0f);
                    r.feito = true;
                }
                break;
            }
            case Pagina::Tipo::Tabela: {
                // AS LINHAS ACENDEM UMA A UMA, e cada uma cai.
                const auto celulas = Pagina::Celulas(c);
                while (r.passos < Pagina::kLinhasDaTabela &&
                       r.idade >= 0.25f + static_cast<float>(r.passos) * Pagina::kEntreLinhas) {
                    for (const auto& p : celulas[static_cast<size_t>(r.passos)]) {
                        DispararDe(Pagina::kGatilhoDaCelula, Vector2(p[0], p[1]), 90.0f);
                    }
                    ++r.passos;
                }
                r.feito = r.passos >= Pagina::kLinhasDaTabela;
                break;
            }
            case Pagina::Tipo::Link:
                // O PROFESSOR NAVEGA ATE A ANCORA e chega num anel.
                if (r.idade >= 0.45f) {
                    DispararDe(Pagina::kGatilhoDaNavegacao, GetPosition(), 90.0f);
                    r.feito = true;
                }
                break;
            case Pagina::Tipo::Input:
                // O TEXTO E DIGITADO letra por letra, e cada letra sai no aluno.
                while (r.passos < Pagina::kLetras &&
                       r.idade >= 0.2f + static_cast<float>(r.passos) * Pagina::kEntreLetras) {
                    DispararDe(Pagina::kGatilhoDaDigitacao,
                               Vector2(c.x + 14.0f + static_cast<float>(r.passos) * 16.0f, c.Cy()), std::nullopt);
                    ++r.passos;
                }
                r.feito = r.passos >= Pagina::kLetras;
                break;
            default:
                r.feito = true;
                break;
        }
    }
}

void Lucas::DesenharPagina() {
    if (!mLinhas || !mScene || !mScene->GetGame()) return;
    mLinhas->Limpar();
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const auto retangulo = [&](const Pagina::Retangulo& r, const SDL_Color cor) {
        const Vector2 a(r.x, r.y), b(r.x + r.w, r.y), c(r.x + r.w, r.y + r.h), d(r.x, r.y + r.h);
        mLinhas->Acrescentar(a, b, cor);
        mLinhas->Acrescentar(b, c, cor);
        mLinhas->Acrescentar(c, d, cor);
        mLinhas->Acrescentar(d, a, cor);
    };
    // O LAYOUT, a fase inteira.
    for (const Pagina::Retangulo& r : Pagina::Layout(largura)) retangulo(r, kLayout);

    // OS ELEMENTOS RENDERIZADOS, cada um com o desenho do tipo dele; o ultimo,
    // aceso.
    for (size_t k = 0; k < mRenderizados.size(); ++k) {
        const Renderizado& r = mRenderizados[k];
        const Pagina::Retangulo& c = r.e.caixa;
        const SDL_Color cor = (k + 1 == mRenderizados.size()) ? kAceso : kElemento;
        switch (r.e.tipo) {
            case Pagina::Tipo::Link:
                // O link e o texto sublinhado.
                mLinhas->Acrescentar(Vector2(c.x, c.y + c.h - 4.0f), Vector2(c.x + c.w, c.y + c.h - 4.0f), cor);
                break;
            case Pagina::Tipo::Tabela: {
                retangulo(c, cor);
                for (int l = 1; l < Pagina::kLinhasDaTabela; ++l) {
                    const float y = c.y + c.h * static_cast<float>(l) / Pagina::kLinhasDaTabela;
                    mLinhas->Acrescentar(Vector2(c.x, y), Vector2(c.x + c.w, y), cor);
                }
                for (int col = 1; col < Pagina::kColunasDaTabela; ++col) {
                    const float x = c.x + c.w * static_cast<float>(col) / Pagina::kColunasDaTabela;
                    mLinhas->Acrescentar(Vector2(x, c.y), Vector2(x, c.y + c.h), cor);
                }
                break;
            }
            case Pagina::Tipo::Imagem:
                retangulo(c, cor);
                if (r.feito) {   // carregada: o X de "imagem" dos wireframes
                    mLinhas->Acrescentar(Vector2(c.x, c.y), Vector2(c.x + c.w, c.y + c.h), cor);
                    mLinhas->Acrescentar(Vector2(c.x + c.w, c.y), Vector2(c.x, c.y + c.h), cor);
                }
                break;
            case Pagina::Tipo::Input: {
                retangulo(c, cor);
                if (!r.feito) {   // o cursor de texto, andando
                    const float x = c.x + 10.0f + static_cast<float>(r.passos) * 16.0f;
                    mLinhas->Acrescentar(Vector2(x, c.y + 8.0f), Vector2(x, c.y + c.h - 8.0f), kAceso);
                }
                break;
            }
            default:
                retangulo(c, cor);
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// FASE 2 - O DEVTOOLS
// ---------------------------------------------------------------------------

void Lucas::RepovoarGrade(const float agora) {
    // O CARTAO QUE FALTA NASCE DO PROFESSOR e voa ate o lugar dele - menos o da
    // linha com display: none, que nao e renderizada.
    for (int i = 0; i < Estilo::kCartoes; ++i) {
        auto& cartao = mCartoes[static_cast<size_t>(i)];
        if (cartao || Estilo::Oculto(mCss, i)) continue;
        const auto nascidos = DispararDe(Estilo::kGatilhoDoCartao, GetPosition(), 90.0f);
        if (nascidos.empty()) continue;
        cartao = nascidos.front();
        mNascimentoDosCartoes[static_cast<size_t>(i)] = agora;
    }
}

void Lucas::AtualizarEstilo(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noEstilo = fsm && fsm->GetStateName() == Estilo::kFaseDoEstilo && !EhClone();
    if (!noEstilo) {
        if (mNoEstilo) {
            // A FASE ACABOU: a grade some, e o placar volta ao nome dela.
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->GetProjectileManager()) {
                for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
                    if (p && p->GetState() == ActorState::Active && p->Posto() == Estilo::kPostoDaGrade) p->Kill();
                }
            }
            mCartoes.fill(nullptr);
            if (!mAnuncioDoEstilo.empty()) RegistrarAnuncio(Estilo::kFaseDoEstilo, mAnuncioDoEstilo);
            mAlvo.reset();
        }
        mNoEstilo = false;
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const float agora = fsm->GetStateTime();

    if (!mNoEstilo) {
        mNoEstilo = true;
        mCss = Estilo::Css{};
        mRegra = Estilo::Regra{};
        mRegra.tipo = Estilo::Tipo::Hover;   // nenhuma regra ainda: a primeira nao e um :hover
        mProximaRegra = Estilo::kPrimeira;
        mInicioDaRegra = 0.0f;
        mCaindo = false;
        mCartoes.fill(nullptr);
        mXDoComecoDaVarredura = GetPosition().x;
        mTempoVarrendo = 0.0f;
        if (mAnuncioDoEstilo.empty()) mAnuncioDoEstilo = AnuncioDaFase(Estilo::kFaseDoEstilo);
        RegistrarAnuncio(Estilo::kFaseDoEstilo, mAnuncioDoEstilo);
        RepovoarGrade(agora);
    }

    // O PROFESSOR VARRE O ALTO, devagar no Normal e mais depressa no Dificil -
    // e para enquanto digita a regra (Estilo::kDigitando).
    if (agora - mInicioDaRegra >= Estilo::kDigitando || agora < Estilo::kPrimeira) mTempoVarrendo += deltaTime;
    const float velocidade = NoNivel(Flex::kVarredura, Flex::kVarreduraNoDificil);
    mAlvo = Vector2(Flex::XDaVarredura(mTempoVarrendo, mXDoComecoDaVarredura, velocidade, largura), Flex::kYDaVarredura);

    // O CARTAO QUE NAO ESTA MAIS LA sai da conta - derrubado pelo escudo do
    // Ponto Extra, ou devolvido ao pool.
    for (int i = 0; i < Estilo::kCartoes; ++i) {
        auto& cartao = mCartoes[static_cast<size_t>(i)];
        if (cartao && (!Vivo(cartao, agora - mNascimentoDosCartoes[static_cast<size_t>(i)]) ||
                       cartao->Posto() != Estilo::kPostoDaGrade)) {
            cartao = nullptr;
        }
    }

    // UMA REGRA NOVA: o placar a escreve, e a grade obedece.
    if (agora >= mProximaRegra) {
        const Estilo::Tipo anterior = mRegra.tipo;
        const int ocultaAntes = mCss.linhaOculta;
        mRegra = Estilo::Proxima(mSorteioDoEstilo, mCss, anterior);
        mInicioDaRegra = agora;
        mProximaRegra += mDegrauDoEstilo.porRegra;
        RegistrarAnuncio(Estilo::kFaseDoEstilo, Estilo::Placar(mRegra));
        if (mRegra.tipo == Estilo::Tipo::Esconder && ocultaAntes < 0) {
            // display: none: a linha nao e mais renderizada.
            for (int i = 0; i < Estilo::kCartoes; ++i) {
                auto& cartao = mCartoes[static_cast<size_t>(i)];
                if (cartao && Estilo::Oculto(mCss, i)) { cartao->Kill(); cartao = nullptr; }
            }
        }
        mCaindo = mRegra.tipo == Estilo::Tipo::Fixar;
        mRepintou = !Estilo::Repinta(mRegra.tipo);
        mLetrasEscorridas = 0;
        if (!mCaindo) RepovoarGrade(agora);
    }

    const float t = agora - mInicioDaRegra;

    // A CASCATA: enquanto digita, cada letra escorre reta embaixo dele.
    while (agora >= Estilo::kPrimeira && mLetrasEscorridas < mDegrauDoEstilo.letras &&
           t >= static_cast<float>(mLetrasEscorridas) * Estilo::kEntreLetrasDaCascata) {
        DispararDe(Estilo::kGatilhoDaCascata, GetPosition() + Vector2(0.0f, 40.0f), 90.0f);
        ++mLetrasEscorridas;
    }

    // O REPAINT: a transicao acabou, e cada cartao solta um tiro reto para
    // baixo, do lugar em que esta.
    if (!mRepintou && t >= Estilo::kRepaint) {
        mRepintou = true;
        for (BossProjectile* cartao : mCartoes) {
            if (cartao) DispararDe(Estilo::kGatilhoDoRepaint, cartao->GetPosition(), 90.0f);
        }
    }

    // POSITION: FIXED; BOTTOM: 0 - a grade se espalha de borda a borda e cai
    // ate o chao; o que caiu sai da grade, e outra nasce do professor.
    if (mCaindo && t >= Estilo::kEspalhar) {
        for (auto& cartao : mCartoes) {
            if (!cartao) continue;
            cartao->DefinirPosto("");
            if (auto* rb = cartao->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, Estilo::kQuedaDoFixo));
            cartao = nullptr;
        }
        mCaindo = false;
        RepovoarGrade(agora);
    }

    // CADA CARTAO VAI ATE O LUGAR QUE O CSS DE AGORA LHE DA.
    const float giro = (mRegra.tipo == Estilo::Tipo::Animar && t < Estilo::kDuracaoDoGiro) ? t * Estilo::kGiroPorSegundo : 0.0f;
    const auto lugares = Estilo::Posicoes(mCss, {Estilo::XDaGrade(GetPosition().x, largura), Estilo::kYDaGrade}, giro);
    const float xDoAluno = PosicaoDoAluno().x;
    for (int i = 0; i < Estilo::kCartoes; ++i) {
        BossProjectile* cartao = mCartoes[static_cast<size_t>(i)];
        if (!cartao) continue;
        Vector2 alvo(lugares[static_cast<size_t>(i)].x, lugares[static_cast<size_t>(i)].y);
        if (mCaindo) {
            const Estilo::Ponto e = Estilo::Espalhado(i, largura, alvo.y);
            alvo = Vector2(e.x, e.y);
        }
        // :HOVER - o cartao em cima do aluno, onde o mouse esta, desce ate ele.
        if (mRegra.tipo == Estilo::Tipo::Hover && std::fabs(alvo.x - xDoAluno) < Estilo::kAlcanceDoHover) {
            alvo.y += Estilo::Hover(std::fmod(t, Estilo::kDuracaoDoHover));
        }
        cartao->SetPosition(Ancora::Aproximar(cartao->GetPosition(), alvo, Estilo::kRapidezDosCartoes, deltaTime));
        if (auto* rb = cartao->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
}

// ---------------------------------------------------------------------------
// FASE 3 - A ARVORE DE SUPERVISAO
// ---------------------------------------------------------------------------

void Lucas::IniciarWorker(const int k, const float agora) {
    Worker& w = mWorkers[static_cast<size_t>(k)];
    const auto nascidos = DispararDe(Elixir::kGatilhoDoWorker, GetPosition(), 90.0f);
    w.p = nascidos.empty() ? nullptr : nascidos.front();
    w.nasceu = agora;
    w.caixa = 0;
    w.reinicio = -1.0f;
}

void Lucas::AtualizarSupervisao(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noElixir = fsm && fsm->GetStateName() == Elixir::kFaseDoElixir && !EhClone();
    if (!noElixir) {
        if (mNoElixir) {
            // A FASE ACABOU: a arvore cai inteira, e o placar volta ao nome dela.
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->GetProjectileManager()) {
                for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
                    if (p && p->GetState() == ActorState::Active && p->Posto() == Elixir::kPostoDosWorkers) p->Kill();
                }
            }
            mWorkers.fill(Worker{});
            mCasts.clear();
            if (mLinhas) mLinhas->Limpar();
            if (!mAnuncioDoElixir.empty()) RegistrarAnuncio(Elixir::kFaseDoElixir, mAnuncioDoElixir);
            mAlvo.reset();
        }
        mNoElixir = false;
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const float agora = fsm->GetStateTime();

    if (!mNoElixir) {
        mNoElixir = true;
        mRodadaDoElixir = -1;
        mCasts.clear();
        mProximoCast = Elixir::kPrimeira;
        mUltimoCast = -1;
        mTempoVarrendoNoElixir = 0.0f;
        mParadaDoSupervisor = -1;
        mXDoComecoDaVarredura = GetPosition().x;
        if (mAnuncioDoElixir.empty()) mAnuncioDoElixir = AnuncioDaFase(Elixir::kFaseDoElixir);
        RegistrarAnuncio(Elixir::kFaseDoElixir, mAnuncioDoElixir);
        CriarLinhas();
        for (int k = 0; k < Elixir::kWorkers; ++k) IniciarWorker(k, agora);
    }

    // A RODADA: uma estrategia de supervisao por rodada.
    const int rodada = std::min(2, static_cast<int>(agora / Elixir::kRodada));
    const Elixir::Estrategia estrategia = mEstrategias[static_cast<size_t>(rodada)];
    mRodadaDoElixir = rodada;

    // O SUPERVISOR VARRE O ALTO devagar, um segundo andando e um parado.
    if (std::fmod(agora, 2.0f) < 1.0f) mTempoVarrendoNoElixir += deltaTime;
    mAlvo = Vector2(Flex::XDaVarredura(mTempoVarrendoNoElixir, mXDoComecoDaVarredura,
                                       NoNivel(Flex::kVarredura, Flex::kVarreduraNoDificil), largura),
                    Flex::kYDaVarredura);

    // PARADO, ELE MANDA AS DUVIDAS PELO PIPE: cada etapa escorre reta embaixo.
    if (const float parado = std::fmod(agora, 2.0f) - 1.0f; parado >= 0.0f) {
        const int parada = static_cast<int>(agora / 2.0f);
        if (parada != mParadaDoSupervisor) {
            mParadaDoSupervisor = parada;
            mEtapasNoPipe = 0;
            RegistrarAnuncio(Elixir::kFaseDoElixir, Elixir::Pipe());
        }
        while (mEtapasNoPipe < mDegrauDoElixir.etapas &&
               parado >= static_cast<float>(mEtapasNoPipe) * Elixir::kEntreEtapas) {
            DispararDe(Elixir::kGatilhoDoPipe, GetPosition() + Vector2(0.0f, 40.0f), 90.0f);
            ++mEtapasNoPipe;
        }
    }

    // O WORKER QUE CAIU: um crash. Termina quem a estrategia manda - cada
    // outro estoura num anel -, e todos voltam juntos depois de kReinicio.
    for (int k = 0; k < Elixir::kWorkers; ++k) {
        Worker& w = mWorkers[static_cast<size_t>(k)];
        if (w.reinicio >= 0.0f || !w.p) continue;
        if (Vivo(w.p, agora - w.nasceu) && w.p->Posto() == Elixir::kPostoDosWorkers) continue;
        w.p = nullptr;
        for (const int j : Elixir::Reiniciam(estrategia, k)) {
            Worker& outro = mWorkers[static_cast<size_t>(j)];
            if (j != k && outro.p && Vivo(outro.p, agora - outro.nasceu)) {
                DispararDe(Elixir::kGatilhoDoCrash, outro.p->GetPosition(), 90.0f);
                outro.p->Kill();
            }
            outro.p = nullptr;
            outro.caixa = 0;
            outro.reinicio = agora + Elixir::kReinicio;
        }
        RegistrarAnuncio(Elixir::kFaseDoElixir, Elixir::Crash(k, estrategia));
    }
    for (int k = 0; k < Elixir::kWorkers; ++k) {
        if (mWorkers[static_cast<size_t>(k)].reinicio >= 0.0f && agora >= mWorkers[static_cast<size_t>(k)].reinicio) {
            IniciarWorker(k, agora);
        }
    }

    // CADA WORKER VAI ATE O LUGAR DELE na arvore, embaixo do supervisor.
    const auto lugar = [&](const int k) {
        const auto p = Elixir::Lugar(k, GetPosition().x, GetPosition().y, largura);
        return Vector2(p[0], p[1]);
    };
    for (int k = 0; k < Elixir::kWorkers; ++k) {
        BossProjectile* p = mWorkers[static_cast<size_t>(k)].p;
        if (!p) continue;
        p->SetPosition(Ancora::Aproximar(p->GetPosition(), lugar(k), 6.0f, deltaTime));
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }

    // UM CAST NOVO, para um worker vivo, correndo pela linha.
    if (agora >= mProximoCast) {
        mProximoCast += mDegrauDoElixir.entreMensagens;
        std::vector<int> vivos;
        for (int k = 0; k < Elixir::kWorkers; ++k) if (mWorkers[static_cast<size_t>(k)].p) vivos.push_back(k);
        const int para = Elixir::Destino(mSorteioDoElixir, vivos, mUltimoCast);
        if (para >= 0) {
            const auto nascidos = DispararDe(Elixir::kGatilhoDoCast, GetPosition(), 90.0f, true);
            if (!nascidos.empty()) mCasts.push_back({nascidos.front(), agora, para});
            mUltimoCast = para;
            RegistrarAnuncio(Elixir::kFaseDoElixir, Elixir::Cast(para, estrategia));
        }
    }

    // A MENSAGEM CORRE PELA LINHA; chegando, entra na caixa - e a caixa cheia
    // vira o handle_cast.
    for (auto it = mCasts.begin(); it != mCasts.end();) {
        Worker& w = mWorkers[static_cast<size_t>(it->para)];
        const float u = (agora - it->nasceu) / Elixir::kVooDaMensagem;
        if (!Vivo(it->p, agora - it->nasceu) || !w.p) {
            if (Vivo(it->p, agora - it->nasceu)) it->p->Kill();
            it = mCasts.erase(it);
            continue;
        }
        if (u < 1.0f) {
            it->p->SetPosition(GetPosition() + (w.p->GetPosition() - GetPosition()) * u);
            if (auto* rb = it->p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
            ++it;
            continue;
        }
        it->p->Kill();
        if (++w.caixa >= Elixir::kCaixa) {
            w.caixa = 0;
            DispararDe(Elixir::kGatilhoDoHandle, w.p->GetPosition(), std::nullopt);
            RegistrarAnuncio(Elixir::kFaseDoElixir, Elixir::Handle(it->para, estrategia));
        }
        it = mCasts.erase(it);
    }
    DesenharSupervisao();
}

void Lucas::DesenharSupervisao() {
    if (!mLinhas) return;
    mLinhas->Limpar();
    // AS LINHAS DA ARVORE, do supervisor a cada worker vivo; e a caixa de
    // correio de cada um, uma marca por mensagem.
    constexpr SDL_Color kLigacao{206, 176, 246, 150};
    constexpr SDL_Color kCaixa{206, 176, 246, 230};
    for (const Worker& w : mWorkers) {
        if (!w.p) continue;
        const Vector2 o = w.p->GetPosition();
        mLinhas->Acrescentar(GetPosition(), o, kLigacao);
        for (int c = 0; c < w.caixa; ++c) {
            const Vector2 a(o.x - 16.0f + static_cast<float>(c) * 12.0f, o.y + 24.0f);
            mLinhas->Acrescentar(a, a + Vector2(8.0f, 0.0f), kCaixa);
            mLinhas->Acrescentar(a + Vector2(0.0f, 4.0f), a + Vector2(8.0f, 4.0f), kCaixa);
        }
    }
}

// ---------------------------------------------------------------------------
// FINAL - O EVENT LOOP
// ---------------------------------------------------------------------------

void Lucas::NovoScript(const float agora) {
    for (Ficha& f : mFichas) {
        if (Vivo(f.p, agora - f.nasceu)) f.p->Kill();
    }
    mScript = EventLoop::Gerar(mSorteioDoLoop);
    mAgenda = EventLoop::Simular(mScript, mDegrauDoLoop.porLinha, mDegrauDoLoop.porCallback);
    mInicioDoScript = agora;
    mProximaExecucao = 0;
    mFichas.assign(mScript.size(), Ficha{});
}

void Lucas::AtualizarEventLoop(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noLoop = fsm && fsm->GetStateName() == EventLoop::kFaseDoLoop && !EhClone();
    if (!noLoop) {
        if (mNoLoop) {
            // A FASE ACABOU: as filas esvaziam, e o placar volta ao nome dela.
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->GetProjectileManager()) {
                for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
                    if (p && p->GetState() == ActorState::Active && p->Posto() == EventLoop::kPostoDoLoop) p->Kill();
                }
            }
            mFichas.clear();
            if (mLinhas) mLinhas->Limpar();
            if (!mAnuncioDoLoop.empty()) RegistrarAnuncio(EventLoop::kFaseDoLoop, mAnuncioDoLoop);
        }
        mNoLoop = false;
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const float agora = fsm->GetStateTime();

    if (!mNoLoop) {
        mNoLoop = true;
        mFichas.clear();
        if (mAnuncioDoLoop.empty()) mAnuncioDoLoop = AnuncioDaFase(EventLoop::kFaseDoLoop);
        RegistrarAnuncio(EventLoop::kFaseDoLoop, mAnuncioDoLoop);
        CriarLinhas();
        NovoScript(agora + EventLoop::kPrimeiro);
    }

    // O SCRIPT ACABOU, a pilha e as filas vazias: outro.
    if (mProximaExecucao >= mAgenda.execucoes.size() &&
        agora >= mInicioDoScript + mAgenda.fim + EventLoop::kEntreScripts) {
        NovoScript(agora);
    }
    const float t = agora - mInicioDoScript;
    const auto ponto = [&](const EventLoop::Lugar lugar, const int posicao) {
        const EventLoop::Ponto p = EventLoop::Posicao(lugar, posicao, largura);
        return Vector2(p.x, p.y);
    };
    const Vector2 pilha = ponto(EventLoop::Lugar::Pilha, 0);

    // O QUE ENTRA NA PILHA: a linha do script, que nasce la - o console.log
    // atira na hora -, ou o callback que o loop trouxe de uma fila, que ataca
    // quando a ficha chega.
    while (mProximaExecucao < mAgenda.execucoes.size()) {
        const EventLoop::Execucao& ex = mAgenda.execucoes[mProximaExecucao];
        if (t < ex.inicio + (ex.callback ? kChegadaNaPilha : 0.0f)) break;
        ++mProximaExecucao;
        const EventLoop::Linha& l = mScript[static_cast<size_t>(ex.linha)];
        if (!ex.callback) {
            const auto nascidos = DispararDe(EventLoop::kGatilhoDaFicha, pilha, 90.0f, true);
            if (!nascidos.empty()) {
                mFichas[static_cast<size_t>(ex.linha)] = {nascidos.front(), agora};
                const char* animacao = l.tipo == EventLoop::Tipo::Log     ? "FichaLog"
                                     : l.tipo == EventLoop::Tipo::Timeout ? "FichaTimeout"
                                     : l.tipo == EventLoop::Tipo::Fetch   ? "FichaFetch"
                                                                          : "FichaThen";
                if (auto* anim = nascidos.front()->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(animacao);
            }
            RegistrarAnuncio(EventLoop::kFaseDoLoop, EventLoop::Placar(l));
            if (l.tipo == EventLoop::Tipo::Log) DispararDe(EventLoop::kGatilhoDoLog, pilha, std::nullopt);
            continue;
        }
        RegistrarAnuncio(EventLoop::kFaseDoLoop, EventLoop::PlacarDoCallback(l));
        switch (l.tipo) {
            case EventLoop::Tipo::Then:    DispararDe(EventLoop::kGatilhoDoThen, pilha, 90.0f); break;
            case EventLoop::Tipo::Fetch:   DispararDe(EventLoop::kGatilhoDoFetch, pilha, 90.0f); break;
            case EventLoop::Tipo::Timeout: DispararDe(EventLoop::kGatilhoDoTimeout, pilha, 90.0f); break;
            default: break;
        }
    }

    // CADA FICHA VAI PARA ONDE O LOOP A POE AGORA; a que saiu da pilha some.
    for (size_t i = 0; i < mFichas.size(); ++i) {
        Ficha& f = mFichas[i];
        if (!f.p) continue;
        if (!Vivo(f.p, agora - f.nasceu)) { f.p = nullptr; continue; }
        const EventLoop::Estado e = EventLoop::Onde(mScript, mAgenda, static_cast<int>(i), t,
                                                    mDegrauDoLoop.porLinha, mDegrauDoLoop.porCallback);
        if (e.lugar == EventLoop::Lugar::Nenhum) {
            f.p->Kill();
            f.p = nullptr;
            continue;
        }
        f.p->SetPosition(Ancora::Aproximar(f.p->GetPosition(), ponto(e.lugar, e.posicao), kRapidezDasFichas, deltaTime));
        if (auto* rb = f.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
    DesenharEventLoop(t);
}

void Lucas::DesenharEventLoop(const float t) {
    if (!mLinhas || !mScene || !mScene->GetGame()) return;
    mLinhas->Limpar();
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    constexpr SDL_Color kLoop{238, 206, 86, 70};
    constexpr SDL_Color kPilha{230, 230, 240, 170};
    constexpr SDL_Color kApis{150, 160, 190, 150};
    constexpr SDL_Color kMicro{238, 206, 86, 170};
    constexpr SDL_Color kTarefa{96, 160, 238, 170};
    constexpr SDL_Color kTimer{238, 206, 86, 230};

    // O LOOP: o anel por onde o professor orbita.
    constexpr int kSegmentos = 48;
    const Vector2 centro(largura / 2.0f, EventLoop::kYDoLoop);
    for (int k = 0; k < kSegmentos; ++k) {
        const float a0 = 2.0f * Math::Pi * static_cast<float>(k) / kSegmentos;
        const float a1 = 2.0f * Math::Pi * static_cast<float>(k + 1) / kSegmentos;
        mLinhas->Acrescentar(centro + Vector2(Math::Cos(a0), Math::Sin(a0)) * EventLoop::kRaioDoLoop,
                             centro + Vector2(Math::Cos(a1), Math::Sin(a1)) * EventLoop::kRaioDoLoop, kLoop);
    }

    // AS CAIXAS: a pilha, as Web APIs e as duas filas.
    const auto caixas = EventLoop::Caixas(largura);
    const SDL_Color cores[] = {kPilha, kApis, kMicro, kTarefa};
    for (size_t k = 0; k < caixas.size(); ++k) {
        const EventLoop::Retangulo& r = caixas[k];
        const Vector2 a(r.x, r.y), b(r.x + r.w, r.y), c(r.x + r.w, r.y + r.h), d(r.x, r.y + r.h);
        mLinhas->Acrescentar(a, b, cores[k]);
        mLinhas->Acrescentar(b, c, cores[k]);
        mLinhas->Acrescentar(c, d, cores[k]);
        mLinhas->Acrescentar(d, a, cores[k]);
    }

    // O TIMER DE CADA FICHA NAS WEB APIS, encolhendo ate zero.
    for (size_t i = 0; i < mFichas.size(); ++i) {
        if (!mFichas[i].p) continue;
        const EventLoop::Estado e = EventLoop::Onde(mScript, mAgenda, static_cast<int>(i), t,
                                                    mDegrauDoLoop.porLinha, mDegrauDoLoop.porCallback);
        if (e.lugar != EventLoop::Lugar::Apis) continue;
        const EventLoop::Ponto p = EventLoop::Posicao(e.lugar, e.posicao, largura);
        const Vector2 de(p.x - 24.0f, p.y + 28.0f);
        mLinhas->Acrescentar(de, de + Vector2(48.0f * e.falta, 0.0f), kTimer);
    }
}

// ---------------------------------------------------------------------------

void Lucas::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoDe) return;
    for (auto& p : projectiles) {
        auto* b = dynamic_cast<BossProjectile*>(p.get());
        if (!b) continue;
        if (mEnfeiteAoNascer) b->DefinirEnfeite(true);
        mDisparados.push_back(b);
    }
}

void Lucas::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O QUE SAI DE UM LUGAR (um elemento da pagina, a mensagem de um
    // processo) sai de onde o Lucas manda, no rumo dele.
    if (mDisparandoDe) {
        params.firePosition = mOndeDisparar;
        params.centralAngle = mRumoDoDisparo;
        return;
    }
}
