//
// Thiago - professor de Banco de Dados (INF 220)
//

#include "Thiago.h"

#include <algorithm>
#include <limits>

#include "../../../Attacks/Ancora.h"
#include "../../../Attacks/AttackParameters/ConsultaAttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Core/Random.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Gameplay/Tabela.h"
#include "BossesProjectiles/BossProjectile.h"

Thiago::Thiago(Scene* scene) : Boss(scene)
{
}

void Thiago::SortearALuta() {
    mLutaSorteada = true;
    const Dificuldade::Nivel nivel = NivelDeDificuldade();
    mDegrau = Banco::DegrauDo(nivel);
    mDegrauDasTransacoes = Transacao::DegrauDo(nivel);
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Banco::kFaseDaSelecao);
        mOrdemDasConsultas = Variacao::Ordem(sorteio, Banco::kConsultasNoCiclo);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Transacao::kFaseDaTransacao);
        mPlanoDasTransacoes = Transacao::Sortear(sorteio);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Banco::kFaseDoProduto);
        mOrdemDosPlanos = Variacao::Ordem(sorteio, Banco::kPlanos);
    }
}

void Thiago::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    if (EhClone()) return;
    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm || fsm->GetStateName().empty()) return;
    if (!mTabelaCriada) CriarTabela();
    AtualizarSelecao(deltaTime);
    AtualizarEscrita();
    AtualizarTransacoes();
    AtualizarProduto();
    AtualizarTabela(deltaTime);
}

std::optional<Vector2> Thiago::AlvoDoMovimento() const {
    return mAlvo;
}

// ---------------------------------------------------------------------------
// A TABELA VISIVEL
// ---------------------------------------------------------------------------

SDL_FRect Thiago::Campo() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene)) return battle->GetPlayfieldBounds();
    return SDL_FRect{0.0f, 0.0f, 1200.0f, 746.0f};
}

Vector2 Thiago::CentroDaCasa(const int linha, const int coluna) const {
    // A MESMA CONTA da ConsultaAttack (Tabela.h): a casa que acende e a faixa
    // que e varrida sao, por construcao, o mesmo lugar.
    const SDL_FRect campo = Campo();
    return Vector2(Tabela::Centro(campo.x, campo.w, Banco::kColunas, coluna),
                   Tabela::Centro(campo.y, campo.h, Banco::kLinhas, linha));
}

Vector2 Thiago::LugarDoDado(const int linha, const int coluna) const {
    const SDL_FRect campo = Campo();
    const float topo = Tabela::Inicio(campo.y, campo.h, Banco::kLinhas, linha);
    return Vector2(Tabela::Centro(campo.x, campo.w, Banco::kColunas, coluna),
                   topo + Tabela::Espessura(campo.h, Banco::kLinhas) * Banco::kDadoNaLinha);
}

std::vector<BossProjectile*> Thiago::DispararEm(const char* gatilho, const Vector2 onde) {
    mDisparandoEm = true;
    mOndeDisparar = onde;
    mCriados.clear();
    Disparar(gatilho);
    mDisparandoEm = false;
    return mCriados;
}

void Thiago::Varrer(const char* gatilho, const int indice, const float aviso, const float velocidade) {
    mIndiceDaConsulta = indice;
    mAvisoDaConsulta = aviso;
    mVelocidadeDaConsulta = velocidade;
    DispararEm(gatilho, GetPosition());
}

void Thiago::CriarTabela() {
    const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
    mRegistros = Banco::Sortear(sorteio);
    mTabelaCriada = true;

    const SDL_FRect campo = Campo();
    for (int c = 0; c < Banco::kColunas; ++c) {
        const auto col = static_cast<Banco::Coluna>(c);
        const Vector2 lugar(Tabela::Centro(campo.x, campo.w, Banco::kColunas, c), Banco::kYDoCabecalho);
        const auto criados = DispararEm(Banco::kGatilhoDoCabecalho, lugar);
        mCabecalhos[static_cast<size_t>(c)] = criados.empty() ? nullptr : criados.front();
        if (auto* p = mCabecalhos[static_cast<size_t>(c)]) {
            p->DefinirEnfeite(true);
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Banco::CaraDoCabecalho(col));
        }
    }
    mCasas.assign(mRegistros.size(), {});
    for (size_t l = 0; l < mRegistros.size(); ++l) EscreverRegistro(l, Vector2::Zero);
    DesenharGrade();
}

void Thiago::DesenharGrade() {
    if (!mScene || EhClone()) return;
    if (!mGrade) {
        // ATRAS DE TUDO, ate do quadro (60): e a tabela riscada no chao da sala.
        auto ator = std::make_unique<Actor>(mScene);
        mGrade = ator->AddComponent<DrawLinhasComponent>(kOrdemDaGrade);
        mScene->AddActor(std::move(ator));
    }
    mGrade->Limpar();
    const SDL_FRect campo = Campo();
    // AS DIVISAS DAS LINHAS e das colunas: a mesma conta das casas (Tabela.h),
    // entao o que acende e o que e varrido cai sempre dentro de um quadrado.
    for (int l = 1; l < Banco::kLinhas; ++l) {
        const float y = Tabela::Inicio(campo.y, campo.h, Banco::kLinhas, l);
        mGrade->Acrescentar(Vector2(campo.x, y), Vector2(campo.x + campo.w, y), kCorDaGrade);
    }
    for (int c = 1; c < Banco::kColunas; ++c) {
        const float x = Tabela::Inicio(campo.x, campo.w, Banco::kColunas, c);
        mGrade->Acrescentar(Vector2(x, campo.y + Banco::kYDoCabecalho - 14.0f), Vector2(x, campo.y + campo.h), kCorDaGrade);
    }
    // O CABECALHO SUBLINHADO: e o que separa o nome da coluna dos valores.
    const float yDoTraco = campo.y + Banco::kYDoCabecalho + 16.0f;
    mGrade->Acrescentar(Vector2(campo.x, yDoTraco), Vector2(campo.x + campo.w, yDoTraco), kCorDoCabecalho);
}

void Thiago::AoTerminarABatalha() {
    if (mGrade) mGrade->Limpar();
}

void Thiago::EscreverRegistro(const size_t linha, const Vector2 de) {
    if (linha >= mCasas.size() || linha >= mRegistros.size()) return;
    for (int c = 0; c < Banco::kColunas; ++c) {
        const auto col = static_cast<Banco::Coluna>(c);
        const Vector2 onde = (de.x == 0.0f && de.y == 0.0f) ? LugarDoDado(static_cast<int>(linha), c) : de;
        const auto criados = DispararEm(Banco::kGatilhoDoDado, onde);
        BossProjectile* p = criados.empty() ? nullptr : criados.front();
        mCasas[linha][static_cast<size_t>(c)] = p;
        if (!p) continue;
        p->DefinirEnfeite(true);
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Banco::Cara(mRegistros[linha], col));
    }
}

void Thiago::AtualizarTabela(const float deltaTime) {
    for (size_t l = 0; l < mCasas.size(); ++l) {
        for (int c = 0; c < Banco::kColunas; ++c) {
            BossProjectile* p = mCasas[l][static_cast<size_t>(c)];
            if (!p || p->GetState() != ActorState::Active) continue;
            p->SetPosition(Ancora::Aproximar(p->GetPosition(), LugarDoDado(static_cast<int>(l), c), kRapidezDaCasa, deltaTime));
            if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
    }
    for (BossProjectile* p : mCabecalhos) {
        if (!p || p->GetState() != ActorState::Active) continue;
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
}

// ---------------------------------------------------------------------------
// A FASE 1: O SELECT
// ---------------------------------------------------------------------------

void Thiago::Marcar(const std::vector<Banco::Casa>& casas, const float ate) {
    for (const Banco::Casa& casa : casas) {
        for (BossProjectile* p : DispararEm(Banco::kGatilhoDoAviso, CentroDaCasa(casa.linha, casa.coluna))) {
            p->DefinirEnfeite(true);
            mMarcas.push_back(Marca{p, ate});
        }
    }
}

void Thiago::MarcarVarredura(const std::vector<Banco::Casa>& casas, const bool porLinha,
                             const float agora, const float aviso, const bool invertida) {
    // A CASA APAGA QUANDO A VARREDURA PASSA POR ELA: a parede nasce na borda
    // (kMargemDaVarredura) e anda na velocidade dela, e a casa deixa de ser
    // perigo quando a parede sai pelo outro lado dela.
    const SDL_FRect campo = Campo();
    for (const Banco::Casa& casa : casas) {
        float fim = porLinha
            ? Tabela::Inicio(campo.x, campo.w, Banco::kColunas, casa.coluna) + Tabela::Espessura(campo.w, Banco::kColunas)
            : Tabela::Inicio(campo.y, campo.h, Banco::kLinhas, casa.linha) + Tabela::Espessura(campo.h, Banco::kLinhas);
        float inicio = porLinha ? campo.x : campo.y;
        if (invertida) {
            // DO OUTRO LADO: a casa apaga quando a parede passa pela borda de
            // cima dela - contada a partir da outra ponta do campo.
            const float extensao = porLinha ? campo.w : campo.h;
            const float comeco = fim - Tabela::Espessura(extensao, porLinha ? Banco::kColunas : Banco::kLinhas);
            fim = (inicio + extensao) - comeco + inicio;
        }
        const float velocidade = porLinha ? mDegrau.velocidadeDaLinha : mDegrau.velocidadeDaColuna;
        Marcar({casa}, agora + aviso + (fim - inicio - Banco::kMargemDaVarredura) / velocidade);
    }
}

void Thiago::AtualizarSelecao(const float deltaTime) {
    (void)deltaTime;
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naSelecao = fsm && fsm->GetStateName() == Banco::kFaseDaSelecao;

    if (!naSelecao) {
        // A FASE ACABOU: o que estava aceso apaga, e o placar volta ao do arquivo.
        if (mNaSelecao) {
            ApagarCasas();
            if (!mAnuncioDaSelecao.empty()) RegistrarAnuncio(Banco::kFaseDaSelecao, mAnuncioDaSelecao);
            mAlvo.reset();
            mNaSelecao = false;
        }
        return;
    }
    if (!mNaSelecao) {
        mNaSelecao = true;
        mVez = 0;
        mProximaConsulta = Banco::kPrimeira;
        if (mAnuncioDaSelecao.empty()) mAnuncioDaSelecao = AnuncioDaFase(Banco::kFaseDaSelecao);
        mAlvo = Vector2(Banco::XDoProfessor(Banco::kColunas / 2, Campo().w), Banco::kYDoProfessor);
    }
    const float agora = fsm->GetStateTime();

    // A CONSULTA NOVA: o placar a escreve, o professor vai para cima da coluna
    // que ela le, e as casas que ela le acendem.
    if (agora >= mProximaConsulta) {
        const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
        const Banco::Consulta c = Banco::Proxima(mRegistros, sorteio, Variacao::NaOrdem(mOrdemDasConsultas, mVez++), mDegrau.linhasDaSelecao);
        RegistrarAnuncio(Banco::kFaseDaSelecao, Banco::Sql(c));
        const Banco::Coluna lida = (c.tipo == Banco::Tipo::Selecao) ? c.onde.coluna : c.coluna;
        mAlvo = Vector2(Banco::XDoProfessor(static_cast<int>(lida), Campo().w), Banco::kYDoProfessor);
        mProximaConsulta = agora + mDegrau.passo;

        const auto casas = Banco::Casas(mRegistros, c);
        switch (c.tipo) {
            case Banco::Tipo::Selecao:
                // AS LINHAS DO WHERE, varridas de ponta a ponta.
                MarcarVarredura(casas, true, agora, mDegrau.avisoDaLinha);
                for (const int l : Banco::Linhas(mRegistros, c.onde)) {
                    Varrer(Banco::kGatilhoDaSelecao, l, mDegrau.avisoDaLinha, mDegrau.velocidadeDaLinha);
                }
                break;
            case Banco::Tipo::Projecao:
                // A COLUNA, varrida de cima a baixo.
                MarcarVarredura(casas, false, agora, mDegrau.avisoDaColuna);
                Varrer(Banco::kGatilhoDaProjecao, static_cast<int>(c.coluna), mDegrau.avisoDaColuna,
                       mDegrau.velocidadeDaColuna);
                break;
            case Banco::Tipo::Ambas:
                // SO OS CRUZAMENTOS: as casas acendem por inteiro.
                Marcar(casas, agora + mDegrau.avisoDaCasa);
                mPendentes.push_back(Pendente{casas, agora + mDegrau.avisoDaCasa});
                break;
            case Banco::Tipo::Ordenacao: {
                // AS LINHAS TROCAM DE LUGAR: cada registro leva as casas dele
                // para a linha nova, devagar, na frente do aluno.
                const auto antes = Banco::Ordenar(mRegistros, c.coluna);
                std::vector<std::array<BossProjectile*, Banco::kColunas>> novas;
                for (const int i : antes) novas.push_back(mCasas[static_cast<size_t>(i)]);
                mCasas = novas;
                mProximaConsulta = agora + Banco::kDepoisDeOrdenar;
                break;
            }
        }
    }

    AtualizarCasas(agora);
}

void Thiago::ApagarCasas() {
    for (const Marca& m : mMarcas) if (m.p->GetState() == ActorState::Active) m.p->Kill();
    for (const Clarao& c : mClaroes) if (c.p->GetState() == ActorState::Active) c.p->Kill();
    mMarcas.clear();
    mClaroes.clear();
    mPendentes.clear();
}

void Thiago::AtualizarCasas(const float agora) {
    // AS CASAS QUE ACENDEM: cada uma se enche de pedacos, por um instante.
    for (auto it = mPendentes.begin(); it != mPendentes.end();) {
        if (agora < it->quando) { ++it; continue; }
        for (const Banco::Casa& casa : it->casas) {
            const Vector2 centro = CentroDaCasa(casa.linha, casa.coluna);
            const SDL_FRect campo = Campo();
            const float largura = Tabela::Espessura(campo.w, Banco::kColunas);
            const float altura = Tabela::Espessura(campo.h, Banco::kLinhas);
            const auto pedacos = DispararEm(Banco::kGatilhoDaCasa, centro);
            for (size_t k = 0; k < pedacos.size(); ++k) {
                const int i = static_cast<int>(k) % Banco::kPedacosNaLargura;
                const int j = (static_cast<int>(k) / Banco::kPedacosNaLargura) % Banco::kPedacosNaAltura;
                const Vector2 onde(centro.x + (static_cast<float>(i) + 0.5f - Banco::kPedacosNaLargura / 2.0f) * largura / Banco::kPedacosNaLargura,
                                   centro.y + (static_cast<float>(j) + 0.5f - Banco::kPedacosNaAltura / 2.0f) * altura / Banco::kPedacosNaAltura);
                pedacos[k]->SetPosition(onde);
                if (auto* rb = pedacos[k]->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
                mClaroes.push_back(Clarao{pedacos[k], agora + Banco::kDuracaoDaCasa});
            }
        }
        it = mPendentes.erase(it);
    }

    // O QUE TEM HORA PARA ACABAR, acaba.
    for (auto it = mMarcas.begin(); it != mMarcas.end();) {
        if (agora < it->ate && it->p->GetState() == ActorState::Active) { ++it; continue; }
        if (it->p->GetState() == ActorState::Active) it->p->Kill();
        it = mMarcas.erase(it);
    }
    for (auto it = mClaroes.begin(); it != mClaroes.end();) {
        if (agora < it->ate && it->p->GetState() == ActorState::Active) {
            if (auto* rb = it->p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
            ++it;
            continue;
        }
        if (it->p->GetState() == ActorState::Active) it->p->Kill();
        it = mClaroes.erase(it);
    }
}

// ---------------------------------------------------------------------------
// A FASE 2: O INSERT, O UPDATE E O DELETE
// ---------------------------------------------------------------------------

void Thiago::Apagar(const int linha) {
    if (linha < 0 || static_cast<size_t>(linha) >= mCasas.size()) return;
    for (BossProjectile* p : mCasas[static_cast<size_t>(linha)]) {
        if (!p || p->GetState() != ActorState::Active) continue;
        p->DefinirEnfeite(false);
        p->DefinirPosto("");
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, mDegrau.velocidadeDoApagado));
        // A LINHA APAGADA CAI VERMELHA. Com a cara de sempre, ela parecia um
        // dado fora do lugar - uma linha da tabela parada entre duas outras -,
        // e nao o tiro que ela passou a ser.
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetColor(255, 105, 95);
    }
    mCasas.erase(mCasas.begin() + linha);
}

void Thiago::AtualizarEscrita() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naEscrita = fsm && fsm->GetStateName() == Banco::kFaseDaEscrita;

    if (!naEscrita) {
        if (mNaEscrita) {
            ApagarCasas();
            mExecucoes.clear();
            if (!mAnuncioDaEscrita.empty()) RegistrarAnuncio(Banco::kFaseDaEscrita, mAnuncioDaEscrita);
            mAlvo.reset();
            mNaEscrita = false;
        }
        return;
    }
    if (!mNaEscrita) {
        mNaEscrita = true;
        mExecucoes.clear();
        mProximaEscrita = Banco::kPrimeiraEscrita;
        if (mAnuncioDaEscrita.empty()) mAnuncioDaEscrita = AnuncioDaFase(Banco::kFaseDaEscrita);
        mAlvo = Vector2(Banco::XDoProfessor(Banco::kColunas / 2, Campo().w), Banco::kYDoProfessor);
    }
    const float agora = fsm->GetStateTime();

    // O COMANDO NOVO: o placar o escreve, e o que ele vai mexer acende.
    if (agora >= mProximaEscrita && mExecucoes.empty()) {
        const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
        const Banco::Escrita e = Banco::ProximaEscrita(mRegistros, sorteio);
        RegistrarAnuncio(Banco::kFaseDaEscrita, Banco::Sql(e));
        mProximaEscrita = agora + mDegrau.passoDaEscrita;
        const int nome = static_cast<int>(Banco::Coluna::Nome);

        switch (e.comando) {
            case Banco::Comando::Insert: {
                // O REGISTRO NOVO SAI DO PROFESSOR para a primeira linha vazia,
                // e a linha e gravada - varrida de ponta a ponta.
                const auto linhas = Banco::Executar(mRegistros, e);
                if (linhas.empty()) break;
                const int l = linhas.front();
                mCasas.resize(mRegistros.size());
                EscreverRegistro(static_cast<size_t>(l), GetPosition());
                std::vector<Banco::Casa> casas;
                for (int c = 0; c < Banco::kColunas; ++c) casas.push_back({l, c});
                MarcarVarredura(casas, true, agora, mDegrau.avisoDaLinha);
                Varrer(Banco::kGatilhoDaSelecao, l, mDegrau.avisoDaLinha, mDegrau.velocidadeDaLinha);
                mAlvo = Vector2(Banco::XDoProfessor(nome, Campo().w), Banco::kYDoProfessor);
                break;
            }
            case Banco::Comando::Update: {
                // A NOTA DE CADA LINHA DO WHERE: a casa acende, e o valor muda.
                std::vector<Banco::Casa> casas;
                for (const int l : Banco::Linhas(mRegistros, e.onde)) casas.push_back({l, static_cast<int>(Banco::Coluna::Nota)});
                Marcar(casas, agora + mDegrau.avisoDaCasa);
                mPendentes.push_back(Pendente{casas, agora + mDegrau.avisoDaCasa});
                mExecucoes.push_back(Execucao{e, agora + mDegrau.avisoDaCasa});
                mAlvo = Vector2(Banco::XDoProfessor(static_cast<int>(e.onde.coluna), Campo().w), Banco::kYDoProfessor);
                break;
            }
            case Banco::Comando::Delete: {
                // A LINHA DO WHERE e varrida - e apagada quando a varredura a
                // atravessa.
                const auto linhas = Banco::Linhas(mRegistros, e.onde);
                std::vector<Banco::Casa> casas;
                for (const int l : linhas) for (int c = 0; c < Banco::kColunas; ++c) casas.push_back({l, c});
                MarcarVarredura(casas, true, agora, mDegrau.avisoDaLinha);
                for (const int l : linhas) Varrer(Banco::kGatilhoDaSelecao, l, mDegrau.avisoDaLinha, mDegrau.velocidadeDaLinha);
                const float travessia = (Campo().w - Banco::kMargemDaVarredura) / mDegrau.velocidadeDaLinha;
                mExecucoes.push_back(Execucao{e, agora + mDegrau.avisoDaLinha + travessia});
                mAlvo = Vector2(Banco::XDoProfessor(static_cast<int>(e.onde.coluna), Campo().w), Banco::kYDoProfessor);
                break;
            }
        }
    }

    // O QUE FOI ANUNCIADO ACONTECE: o valor novo aparece, a linha apagada cai.
    for (auto it = mExecucoes.begin(); it != mExecucoes.end();) {
        if (agora < it->quando) { ++it; continue; }
        const Banco::Escrita e = it->escrita;
        it = mExecucoes.erase(it);
        if (e.comando == Banco::Comando::Delete) {
            const auto linhas = Banco::Linhas(mRegistros, e.onde);
            for (auto l = linhas.rbegin(); l != linhas.rend(); ++l) Apagar(*l);
            Banco::Executar(mRegistros, e);
        } else if (e.comando == Banco::Comando::Update) {
            for (const int l : Banco::Executar(mRegistros, e)) {
                if (static_cast<size_t>(l) >= mCasas.size()) continue;
                BossProjectile* p = mCasas[static_cast<size_t>(l)][static_cast<size_t>(Banco::Coluna::Nota)];
                if (p && p->GetState() == ActorState::Active) {
                    if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) {
                        anim->SetAnimation(Banco::Cara(mRegistros[static_cast<size_t>(l)], Banco::Coluna::Nota));
                    }
                }
            }
        }
    }

    AtualizarCasas(agora);
}

// ---------------------------------------------------------------------------
// A FASE 3: AS TRANSACOES
// ---------------------------------------------------------------------------

namespace {
    std::uint32_t Sorteio() {
        return static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
    }
}

void Thiago::Anunciar(const int id, const int linha, const float agora) {
    if (linha < 0 || static_cast<size_t>(linha) >= mRegistros.size()) return;
    // UMA NOTA NOVA, diferente da que esta la.
    int nota = Random::GetIntRange(0, 99);
    if (nota == mRegistros[static_cast<size_t>(linha)].nota) nota = (nota + 37) % 100;
    for (TxEmCampo& t : mEmCampo) {
        if (t.id != id) continue;
        t.linhaDaVez = linha;
        t.novas.emplace_back(linha, nota);
    }
    RegistrarAnuncio(Transacao::kFaseDaTransacao,
                     Transacao::PlacarDoUpdate(id, nota, mRegistros[static_cast<size_t>(linha)].mat));

    // O UPDATE GRAVA A LINHA: ela acende e e varrida, como a do INSERT.
    std::vector<Banco::Casa> casas;
    for (int c = 0; c < Banco::kColunas; ++c) casas.push_back({linha, c});
    MarcarVarredura(casas, true, agora, mDegrau.avisoDaLinha);
    Varrer(Banco::kGatilhoDaSelecao, linha, mDegrau.avisoDaLinha, mDegrau.velocidadeDaLinha);
}

void Thiago::Gravar(const int id, const int linha) {
    if (linha < 0 || static_cast<size_t>(linha) >= mRegistros.size() || static_cast<size_t>(linha) >= mCasas.size()) return;
    TxEmCampo* tx = nullptr;
    for (TxEmCampo& t : mEmCampo) if (t.id == id) tx = &t;
    if (!tx) return;
    int nota = mRegistros[static_cast<size_t>(linha)].nota;
    for (const auto& [l, n] : tx->novas) if (l == linha) nota = n;

    // AS ESCRITAS SAEM DO PROFESSOR e ficam na linha, no meio dela, de ponta a
    // ponta: o LOCK. Atravessar a linha travada e passar entre elas.
    const Vector2 origem = GetPosition();
    const SDL_FRect campo = Campo();
    const float y = CentroDaCasa(linha, 0).y;
    for (int k = 0; k < mDegrauDasTransacoes.escritasPorLinha; ++k) {
        const Vector2 destino(campo.x + (static_cast<float>(k) + 0.5f) * campo.w / mDegrauDasTransacoes.escritasPorLinha, y);
        for (BossProjectile* p : DispararEm(Transacao::kGatilhoDaEscrita, origem)) {
            Vector2 ida = destino - origem;
            if (ida.LengthSq() > 0.0f) ida.Normalize();
            if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(ida * mDegrauDasTransacoes.velocidadeDaEscrita);
            mGravacoes.push_back(Gravacao{p, id, origem, destino, false, false});
        }
    }

    // O VALOR NOVO APARECE SUJO: escrito, mas ainda nao de verdade.
    tx->antes.emplace_back(linha, mRegistros[static_cast<size_t>(linha)].nota);
    mRegistros[static_cast<size_t>(linha)].nota = nota;
    if (BossProjectile* chip = mCasas[static_cast<size_t>(linha)][static_cast<size_t>(Banco::Coluna::Nota)]) {
        if (auto* anim = chip->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Banco::CaraSuja(nota));
    }
}

void Thiago::Terminar(const int id, const bool desfaz, const float agora) {
    TxEmCampo* tx = nullptr;
    for (TxEmCampo& t : mEmCampo) if (t.id == id) tx = &t;
    if (!tx) return;

    // O VALOR FICA LIMPO: o novo (COMMIT) ou o de antes (ROLLBACK).
    for (const auto& [linha, notaAntiga] : tx->antes) {
        if (static_cast<size_t>(linha) >= mRegistros.size()) continue;
        if (desfaz) mRegistros[static_cast<size_t>(linha)].nota = notaAntiga;
        if (static_cast<size_t>(linha) >= mCasas.size()) continue;
        if (BossProjectile* chip = mCasas[static_cast<size_t>(linha)][static_cast<size_t>(Banco::Coluna::Nota)]) {
            if (auto* anim = chip->GetComponent<DrawAnimatedComponent>()) {
                anim->SetAnimation(Banco::Cara(mRegistros[static_cast<size_t>(linha)], Banco::Coluna::Nota));
            }
        }
    }

    if (!desfaz) {
        // COMMIT: as escritas somem - o que estava escrito passou a ser de verdade.
        for (Gravacao& g : mGravacoes) if (g.tx == id && g.p->GetState() == ActorState::Active) g.p->Kill();
    } else {
        // ROLLBACK: cada linha gravada e desfeita - acende e e varrida do outro
        // lado, o log lido de tras para a frente -, e as escritas voltam com a
        // varredura, pelo caminho por onde vieram.
        for (const auto& [linha, notaAntiga] : tx->antes) {
            (void)notaAntiga;
            std::vector<Banco::Casa> casas;
            for (int c = 0; c < Banco::kColunas; ++c) casas.push_back({linha, c});
            MarcarVarredura(casas, true, agora, mDegrau.avisoDaLinha, true);
            Varrer(Transacao::kGatilhoDoDesfazer, linha, mDegrau.avisoDaLinha, mDegrau.velocidadeDaLinha);
        }
        mVoltas.push_back(Volta{id, agora + mDegrau.avisoDaLinha});
    }
    mGravacoes.erase(std::remove_if(mGravacoes.begin(), mGravacoes.end(), [](const Gravacao& g) {
        return g.p->GetState() != ActorState::Active;
    }), mGravacoes.end());

    Transacao::Liberar(mLocks, id);
    mEmCampo.erase(std::remove_if(mEmCampo.begin(), mEmCampo.end(), [id](const TxEmCampo& t) { return t.id == id; }),
                   mEmCampo.end());
}

void Thiago::AtualizarTransacoes() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool nasTransacoes = fsm && fsm->GetStateName() == Transacao::kFaseDaTransacao;

    if (!nasTransacoes) {
        if (mNasTransacoes) {
            // A FASE ACABOU NO MEIO DAS TRANSACOES: o banco desfaz as que nao
            // fecharam - o valor de antes volta -, sem o ataque da volta.
            for (const TxEmCampo& tx : mEmCampo) {
                for (auto it = tx.antes.rbegin(); it != tx.antes.rend(); ++it) {
                    const auto [linha, notaAntiga] = *it;
                    if (static_cast<size_t>(linha) >= mRegistros.size() || static_cast<size_t>(linha) >= mCasas.size()) continue;
                    mRegistros[static_cast<size_t>(linha)].nota = notaAntiga;
                    if (BossProjectile* chip = mCasas[static_cast<size_t>(linha)][static_cast<size_t>(Banco::Coluna::Nota)]) {
                        if (auto* anim = chip->GetComponent<DrawAnimatedComponent>()) {
                            anim->SetAnimation(Banco::Cara(mRegistros[static_cast<size_t>(linha)], Banco::Coluna::Nota));
                        }
                    }
                }
            }
            mEmCampo.clear();
            for (Gravacao& g : mGravacoes) if (g.p->GetState() == ActorState::Active) g.p->Kill();
            mGravacoes.clear();
            mVoltas.clear();
            mLocks.clear();
            ApagarCasas();
            if (!mAnuncioDasTransacoes.empty()) RegistrarAnuncio(Transacao::kFaseDaTransacao, mAnuncioDasTransacoes);
            mAlvo.reset();
            mNasTransacoes = false;
        }
        return;
    }
    if (!mNasTransacoes) {
        mNasTransacoes = true;
        mEmCampo.clear();
        mGravacoes.clear();
        mVoltas.clear();
        mLocks.clear();
        mProximaTransacao = Transacao::kPrimeira;
        if (mAnuncioDasTransacoes.empty()) mAnuncioDasTransacoes = AnuncioDaFase(Transacao::kFaseDaTransacao);
        mAlvo = Vector2(Banco::XDoProfessor(Banco::kColunas / 2, Campo().w), Banco::kYDoProfessor);
        // A TABELA CHEIA DE NOVO: o que o DELETE apagou na fase 2 e reinserido,
        // vindo do professor.
        while (static_cast<int>(mRegistros.size()) < Banco::kLinhas) {
            Banco::Escrita poe;
            poe.comando = Banco::Comando::Insert;
            poe.novo = Banco::NovoRegistro(mRegistros, Sorteio());
            const auto linhas = Banco::Executar(mRegistros, poe);
            if (linhas.empty()) break;
            mCasas.resize(mRegistros.size());
            EscreverRegistro(static_cast<size_t>(linhas.front()), GetPosition());
        }
    }
    const float agora = fsm->GetStateTime();
    const int linhas = static_cast<int>(mRegistros.size());

    // UMA TRANSACAO NOVA: o primeiro UPDATE numa linha que ninguem trava.
    if (agora >= mProximaTransacao && static_cast<int>(mEmCampo.size()) < mDegrauDasTransacoes.maximo) {
        const int linha = Transacao::LinhaLivre(mLocks, linhas, Sorteio());
        mProximaTransacao = agora + mDegrauDasTransacoes.novaTransacao;
        if (linha >= 0) {
            const int id = mProximoId++;
            mLocks.push_back(Transacao::Tx{id, {linha}, -1});
            mEmCampo.push_back(TxEmCampo{id, Etapa::MarcandoPrimeira, agora + mDegrau.avisoDaLinha, -1, {}, {}});
            Anunciar(id, linha, agora);
            mAlvo = Vector2(Banco::XDoProfessor(Random::GetIntRange(0, Banco::kColunas - 1), Campo().w), Banco::kYDoProfessor);
        }
    }

    // CADA TRANSACAO NA ETAPA DELA. Copia dos ids: Terminar mexe na lista.
    std::vector<int> ids;
    for (const TxEmCampo& t : mEmCampo) ids.push_back(t.id);
    for (const int id : ids) {
        TxEmCampo* tx = nullptr;
        for (TxEmCampo& t : mEmCampo) if (t.id == id) tx = &t;
        if (!tx) continue;   // abortada por um deadlock nesta mesma volta

        switch (tx->etapa) {
            case Etapa::MarcandoPrimeira:
            case Etapa::MarcandoSegunda: {
                if (agora < tx->proximo) break;
                const bool segunda = tx->etapa == Etapa::MarcandoSegunda;
                Gravar(id, tx->linhaDaVez);
                tx->etapa = segunda ? Etapa::Fechando : Etapa::Segurando;
                tx->proximo = agora + (segunda ? Transacao::kSegurando : Transacao::kEntreUpdates);
                break;
            }
            case Etapa::Segurando: {
                if (agora < tx->proximo) break;
                // O SEGUNDO UPDATE: as vezes numa linha de outra transacao.
                const auto sorteio = Sorteio();
                const bool disputa = static_cast<float>(sorteio % 1000u) / 1000.0f < mPlanoDasTransacoes.chanceDeDisputa;
                int alvo = disputa ? Transacao::LinhaDeOutra(mLocks, id, sorteio / 7u) : -1;
                if (alvo < 0) {
                    int primeira = -1;
                    for (const auto& t : mLocks) if (t.id == id && !t.segura.empty()) primeira = t.segura.front();
                    alvo = Transacao::LinhaLivre(mLocks, linhas, sorteio / 3u, primeira);
                }
                if (alvo < 0) {
                    tx->etapa = Etapa::Fechando;
                    tx->proximo = agora + Transacao::kSegurando;
                    break;
                }
                for (auto& t : mLocks) if (t.id == id) t.quer = alvo;
                tx->linhaDaVez = alvo;
                tx->etapa = Etapa::Esperando;
                const int dono = Transacao::Dono(mLocks, alvo);
                if (dono != 0) RegistrarAnuncio(Transacao::kFaseDaTransacao, Transacao::PlacarDaEspera(id, dono));
                // DEADLOCK: se a espera fecha um ciclo, a mais nova desfaz tudo.
                const auto ciclo = Transacao::CicloDeEspera(mLocks, id);
                if (!ciclo.empty()) {
                    const int vitima = Transacao::Vitima(ciclo);
                    RegistrarAnuncio(Transacao::kFaseDaTransacao, Transacao::PlacarDoDeadlock(vitima));
                    Terminar(vitima, true, agora);
                }
                break;
            }
            case Etapa::Esperando: {
                // A LINHA FICOU LIVRE: o lock passa a ser dela, e o UPDATE sai.
                int quer = -1;
                for (const auto& t : mLocks) if (t.id == id) quer = t.quer;
                if (quer < 0 || Transacao::Dono(mLocks, quer) != 0) break;
                for (auto& t : mLocks) {
                    if (t.id != id) continue;
                    t.segura.push_back(quer);
                    t.quer = -1;
                }
                tx->etapa = Etapa::MarcandoSegunda;
                tx->proximo = agora + mDegrau.avisoDaLinha;
                Anunciar(id, quer, agora);
                break;
            }
            case Etapa::Fechando: {
                if (agora < tx->proximo) break;
                const bool desfaz = Transacao::Desfaz(Sorteio(), mPlanoDasTransacoes.chanceDeRollback);
                RegistrarAnuncio(Transacao::kFaseDaTransacao, Transacao::PlacarDoFim(id, desfaz));
                Terminar(id, desfaz, agora);
                break;
            }
        }
    }

    // O ROLLBACK SOLTA AS ESCRITAS junto com a varredura que desfaz.
    for (auto it = mVoltas.begin(); it != mVoltas.end();) {
        if (agora < it->quando) { ++it; continue; }
        for (Gravacao& g : mGravacoes) {
            if (g.tx != it->tx || g.p->GetState() != ActorState::Active) continue;
            g.voltando = true;
            g.parada = false;
            Vector2 volta = g.origem - g.p->GetPosition();
            if (volta.LengthSq() > 0.0f) volta.Normalize();
            if (auto* rb = g.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(volta * mDegrauDasTransacoes.velocidadeDaEscrita);
        }
        it = mVoltas.erase(it);
    }

    // AS ESCRITAS: as que vao param na casa, as que voltam somem ao chegar.
    for (auto it = mGravacoes.begin(); it != mGravacoes.end();) {
        Gravacao& g = *it;
        if (g.p->GetState() != ActorState::Active) { it = mGravacoes.erase(it); continue; }
        auto* rb = g.p->GetComponent<RigidBodyComponent>();
        const Vector2 aqui = g.p->GetPosition();
        if (g.voltando) {
            if ((g.origem - aqui).LengthSq() < 20.0f * 20.0f) {
                g.p->Kill();
                it = mGravacoes.erase(it);
                continue;
            }
        } else if (!g.parada) {
            const Vector2 falta = g.destino - aqui;
            Vector2 ida = falta;
            if (ida.LengthSq() > 0.0f) ida.Normalize();
            const bool chegou = falta.LengthSq() < 12.0f * 12.0f
                             || (rb && Vector2::Dot(rb->GetVelocity(), falta) <= 0.0f);
            if (chegou) {
                g.p->SetPosition(g.destino);
                g.parada = true;
                if (rb) rb->SetVelocity(Vector2::Zero);
            }
        } else if (rb) {
            rb->SetVelocity(Vector2::Zero);
        }
        ++it;
    }

    AtualizarCasas(agora);
}

// ---------------------------------------------------------------------------
// A REPESCAGEM: O PRODUTO CARTESIANO
// ---------------------------------------------------------------------------

void Thiago::AtualizarProduto() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noProduto = fsm && fsm->GetStateName() == Banco::kFaseDoProduto;

    if (!noProduto) {
        if (mNoProduto) {
            ApagarCasas();
            mAgendadas.clear();
            if (!mAnuncioDoProduto.empty()) RegistrarAnuncio(Banco::kFaseDoProduto, mAnuncioDoProduto);
            mAlvo.reset();
            mNoProduto = false;
        }
        return;
    }
    if (!mNoProduto) {
        mNoProduto = true;
        mVolta = 0;
        mAgendadas.clear();
        mProximaVolta = Banco::kPrimeiroProduto;
        if (mAnuncioDoProduto.empty()) mAnuncioDoProduto = AnuncioDaFase(Banco::kFaseDoProduto);
        mAlvo = Vector2(Banco::XDoProfessor(Banco::kColunas / 2, Campo().w), Banco::kYDoProfessor);
    }
    const float agora = fsm->GetStateTime();

    // UMA VOLTA NOVA DO LACO: o placar diz o plano, e cada casa ganha a hora
    // de ser marcada - uma depois da outra, cada volta mais depressa.
    if (mAgendadas.empty() && agora >= mProximaVolta) {
        const Banco::Plano plano = Banco::PlanoDaVolta(Variacao::NaOrdem(mOrdemDosPlanos, mVolta));
        const float passo = Banco::PassoDoProduto(mVolta) * mDegrau.passoDoProduto;
        ++mVolta;
        RegistrarAnuncio(Banco::kFaseDoProduto, Banco::PlacarDoProduto(plano));
        const auto casas = Banco::Produto(plano);
        for (size_t k = 0; k < casas.size(); ++k) {
            mAgendadas.push_back(Agendada{casas[k], agora + passo * static_cast<float>(k)});
        }
        // O professor le de uma coluna sorteada, a volta inteira.
        const int coluna = Random::GetIntRange(0, Banco::kColunas - 1);
        mAlvo = Vector2(Banco::XDoProfessor(coluna, Campo().w), Banco::kYDoProfessor);
        mProximaVolta = agora + passo * static_cast<float>(casas.size()) + mDegrau.avisoDoProduto
                      + Banco::kDuracaoDaCasa + Banco::kEntreVoltas;
    }

    // A CASA DA VEZ E MARCADA, e acende kAvisoDoProduto depois.
    for (auto it = mAgendadas.begin(); it != mAgendadas.end();) {
        if (agora < it->marca) { ++it; continue; }
        Marcar({it->casa}, it->marca + mDegrau.avisoDoProduto);
        mPendentes.push_back(Pendente{{it->casa}, it->marca + mDegrau.avisoDoProduto});
        it = mAgendadas.erase(it);
    }

    AtualizarCasas(agora);
}

// ---------------------------------------------------------------------------
// OS GANCHOS
// ---------------------------------------------------------------------------

void Thiago::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                  const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

void Thiago::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O QUE O THIAGO DISPARA NUM LUGAR nasce nele - e a varredura vai na faixa
    // que ele escolheu, com o aviso que as casas acesas estao dando.
    if (mDisparandoEm) {
        params.firePosition = mOndeDisparar;
        if (auto* consulta = dynamic_cast<ConsultaAttackParams*>(&params)) {
            consulta->indice = mIndiceDaConsulta;
            consulta->aviso = mAvisoDaConsulta;
            consulta->projectileSpeed = mVelocidadeDaConsulta;
        }
        return;
    }
}
