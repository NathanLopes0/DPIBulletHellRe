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
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Random.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Tabela.h"
#include "../../Player/Player.h"
#include "BossesProjectiles/BossProjectile.h"

Thiago::Thiago(Scene* scene) : Boss(scene)
{
}

void Thiago::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
    if (EhClone()) return;
    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm || fsm->GetStateName().empty()) return;
    if (!mTabelaCriada) CriarTabela();
    AtualizarSelecao(deltaTime);
    AtualizarEscrita();
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
                             const float agora, const float aviso) {
    // A CASA APAGA QUANDO A VARREDURA PASSA POR ELA: a parede nasce na borda
    // (kMargemDaVarredura) e anda na velocidade dela, e a casa deixa de ser
    // perigo quando a parede sai pelo outro lado dela.
    const SDL_FRect campo = Campo();
    for (const Banco::Casa& casa : casas) {
        const float fim = porLinha
            ? Tabela::Inicio(campo.x, campo.w, Banco::kColunas, casa.coluna) + Tabela::Espessura(campo.w, Banco::kColunas)
            : Tabela::Inicio(campo.y, campo.h, Banco::kLinhas, casa.linha) + Tabela::Espessura(campo.h, Banco::kLinhas);
        const float inicio = porLinha ? campo.x : campo.y;
        const float velocidade = porLinha ? Banco::kVelocidadeDaLinha : Banco::kVelocidadeDaColuna;
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
        const Banco::Consulta c = Banco::Proxima(mRegistros, sorteio, mVez++);
        RegistrarAnuncio(Banco::kFaseDaSelecao, Banco::Sql(c));
        const Banco::Coluna lida = (c.tipo == Banco::Tipo::Selecao) ? c.onde.coluna : c.coluna;
        mAlvo = Vector2(Banco::XDoProfessor(static_cast<int>(lida), Campo().w), Banco::kYDoProfessor);
        mProximaConsulta = agora + Banco::kPasso;

        const auto casas = Banco::Casas(mRegistros, c);
        switch (c.tipo) {
            case Banco::Tipo::Selecao:
                // AS LINHAS DO WHERE, varridas de ponta a ponta.
                MarcarVarredura(casas, true, agora, Banco::kAvisoDaLinha);
                for (const int l : Banco::Linhas(mRegistros, c.onde)) {
                    Varrer(Banco::kGatilhoDaSelecao, l, Banco::kAvisoDaLinha, Banco::kVelocidadeDaLinha);
                }
                break;
            case Banco::Tipo::Projecao:
                // A COLUNA, varrida de cima a baixo.
                MarcarVarredura(casas, false, agora, Banco::kAvisoDaColuna);
                Varrer(Banco::kGatilhoDaProjecao, static_cast<int>(c.coluna), Banco::kAvisoDaColuna,
                       Banco::kVelocidadeDaColuna);
                break;
            case Banco::Tipo::Ambas:
                // SO OS CRUZAMENTOS: as casas acendem por inteiro.
                Marcar(casas, agora + Banco::kAvisoDaCasa);
                mPendentes.push_back(Pendente{casas, agora + Banco::kAvisoDaCasa});
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
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, Banco::kVelocidadeDoApagado));
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
        mProximaEscrita = agora + Banco::kPassoDaEscrita;
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
                MarcarVarredura(casas, true, agora, Banco::kAvisoDaLinha);
                Varrer(Banco::kGatilhoDaSelecao, l, Banco::kAvisoDaLinha, Banco::kVelocidadeDaLinha);
                mAlvo = Vector2(Banco::XDoProfessor(nome, Campo().w), Banco::kYDoProfessor);
                break;
            }
            case Banco::Comando::Update: {
                // A NOTA DE CADA LINHA DO WHERE: a casa acende, e o valor muda.
                std::vector<Banco::Casa> casas;
                for (const int l : Banco::Linhas(mRegistros, e.onde)) casas.push_back({l, static_cast<int>(Banco::Coluna::Nota)});
                Marcar(casas, agora + Banco::kAvisoDaCasa);
                mPendentes.push_back(Pendente{casas, agora + Banco::kAvisoDaCasa});
                mExecucoes.push_back(Execucao{e, agora + Banco::kAvisoDaCasa});
                mAlvo = Vector2(Banco::XDoProfessor(static_cast<int>(e.onde.coluna), Campo().w), Banco::kYDoProfessor);
                break;
            }
            case Banco::Comando::Delete: {
                // A LINHA DO WHERE e varrida - e apagada quando a varredura a
                // atravessa.
                const auto linhas = Banco::Linhas(mRegistros, e.onde);
                std::vector<Banco::Casa> casas;
                for (const int l : linhas) for (int c = 0; c < Banco::kColunas; ++c) casas.push_back({l, c});
                MarcarVarredura(casas, true, agora, Banco::kAvisoDaLinha);
                for (const int l : linhas) Varrer(Banco::kGatilhoDaSelecao, l, Banco::kAvisoDaLinha, Banco::kVelocidadeDaLinha);
                const float travessia = (Campo().w - Banco::kMargemDaVarredura) / Banco::kVelocidadeDaLinha;
                mExecucoes.push_back(Execucao{e, agora + Banco::kAvisoDaLinha + travessia});
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
        const Banco::Plano plano = Banco::PlanoDaVolta(mVolta);
        const float passo = Banco::PassoDoProduto(mVolta);
        ++mVolta;
        RegistrarAnuncio(Banco::kFaseDoProduto, Banco::PlacarDoProduto(plano));
        const auto casas = Banco::Produto(plano);
        for (size_t k = 0; k < casas.size(); ++k) {
            mAgendadas.push_back(Agendada{casas[k], agora + passo * static_cast<float>(k)});
        }
        // O professor le de uma coluna sorteada, a volta inteira.
        const int coluna = Random::GetIntRange(0, Banco::kColunas - 1);
        mAlvo = Vector2(Banco::XDoProfessor(coluna, Campo().w), Banco::kYDoProfessor);
        mProximaVolta = agora + passo * static_cast<float>(casas.size()) + Banco::kAvisoDoProduto
                      + Banco::kDuracaoDaCasa + Banco::kEntreVoltas;
    }

    // A CASA DA VEZ E MARCADA, e acende kAvisoDoProduto depois.
    for (auto it = mAgendadas.begin(); it != mAgendadas.end();) {
        if (agora < it->marca) { ++it; continue; }
        Marcar({it->casa}, it->marca + Banco::kAvisoDoProduto);
        mPendentes.push_back(Pendente{{it->casa}, it->marca + Banco::kAvisoDoProduto});
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

int Thiago::FaixaDoJogador(const ConsultaAttackParams& consulta) {

    const auto battle = dynamic_cast<Battle*>(GetScene());
    if (!battle || !battle->Alvo()) return 0;

    const SDL_FRect campo = battle->GetPlayfieldBounds();
    const Vector2 onde = battle->Alvo()->GetPosition();

    // A MESMA CONTA que a ConsultaAttack usa para posicionar a varredura, de
    // Tabela. Se este arquivo fizesse a propria divisao, bastaria um
    // arredondamento diferente para o chefe anunciar uma linha e varrer a
    // vizinha - e o jogador seria atingido por um ataque que leu certo.
    return (consulta.eixo == ConsultaAttackParams::Eixo::Linha)
               ? Tabela::FaixaDe(campo.y, campo.h, consulta.forma.linhas,  onde.y)
               : Tabela::FaixaDe(campo.x, campo.w, consulta.forma.colunas, onde.x);
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

    // Fases que nao usam ConsultaAttack caem fora daqui e seguem com o que o
    // arquivo disser - e o caso dos ataques comuns misturados as consultas.
    auto* consulta = dynamic_cast<ConsultaAttackParams*>(&params);
    if (!consulta) return;

    const int quantas = (consulta->eixo == ConsultaAttackParams::Eixo::Linha)
                            ? consulta->forma.linhas
                            : consulta->forma.colunas;
    if (quantas <= 0) return;

    if (stateName == "StateTwo") {
        // WHERE. A consulta encontra o jogador. O aviso continua valendo, entao
        // ficar parado e que mata - nao a consulta em si.
        consulta->indice = FaixaDoJogador(*consulta);
    }
    else if (stateName == "StateThree") {
        // A TRANSACAO. Metade das consultas vai na faixa do jogador e metade na
        // que ele acabou de deixar: a varredura anterior "volta". Quem aprendeu
        // na fase 2 a pular para o lado e esperar descobre que o espaco limpo e
        // justamente para onde o desfazimento vem.
        const int aquiAgora = FaixaDoJogador(*consulta);

        if (mProximaFaixa % 2 == 0) {
            consulta->indice = aquiAgora;
        }
        else {
            // A vizinha, alternando o lado para nao virar um padrao de um lado so.
            const int lado = (Random::GetIntRange(0, 1) == 0) ? -1 : 1;
            int vizinha = aquiAgora + lado;
            if (vizinha < 0) vizinha = 1;
            if (vizinha >= quantas) vizinha = quantas - 2;
            consulta->indice = (vizinha < 0) ? 0 : vizinha;
        }
        ++mProximaFaixa;
    }
    else if (stateName == "StateFinal") {
        // CROSS JOIN. Aqui NAO ha escolha de faixa: o arquivo dispara varias
        // consultas por rajada, cada uma com o proprio indice fixo, e juntas
        // elas formam o produto cartesiano. Mexer no indice aqui desmontaria a
        // treliça que o arquivo descreve.
    }
}
