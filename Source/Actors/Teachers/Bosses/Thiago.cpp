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
#include "../../../Motor/Random.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Regras/Tabela.h"
#include "BossesProjectiles/BossProjectile.h"

Thiago::Thiago(Scene* scene) : Boss(scene)
{
}

void Thiago::SortearALuta() {
    mLutaSorteada = true;
    const Dificuldade::Nivel nivel = NivelDeDificuldade();
    mDegrau = Banco::DegrauDo(nivel);
    mDegrauDaEscrita = Dml::DegrauDo(nivel);
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
    mRelogio += deltaTime;
    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm || fsm->GetStateName().empty()) return;
    if (!mTabelaCriada) CriarTabela();
    AtualizarSelecao(deltaTime);
    AtualizarEscrita();
    AtualizarTransacoes();
    AtualizarProduto();
    AtualizarTabela(deltaTime);
    AtualizarVoos(deltaTime);
}

std::optional<Vector2> Thiago::AlvoDoMovimento() const {
    return mAlvo;
}

// A tabela visivel.

Vector2 Thiago::CentroDaCasa(const int linha, const int coluna) const {
    // A mesma conta da ConsultaAttack (Tabela.h).
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
        // Atras de tudo, ate do quadro (60): e a tabela riscada no chao da sala.
        auto ator = std::make_unique<Actor>(mScene);
        mGrade = ator->AddComponent<DrawLinhasComponent>(kOrdemDaGrade);
        mScene->AddActor(std::move(ator));
    }
    mGrade->Limpar();
    const SDL_FRect campo = Campo();
    // As divisas das linhas e das colunas.
    for (int l = 1; l < Banco::kLinhas; ++l) {
        const float y = Tabela::Inicio(campo.y, campo.h, Banco::kLinhas, l);
        mGrade->Acrescentar(Vector2(campo.x, y), Vector2(campo.x + campo.w, y), kCorDaGrade);
    }
    for (int c = 1; c < Banco::kColunas; ++c) {
        const float x = Tabela::Inicio(campo.x, campo.w, Banco::kColunas, c);
        mGrade->Acrescentar(Vector2(x, campo.y + Banco::kYDoCabecalho - 14.0f), Vector2(x, campo.y + campo.h), kCorDaGrade);
    }
    // O cabecalho sublinhado: e o que separa o nome da coluna dos valores.
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
    // A tabela fica a batalha inteira: o limite de vida de um projetil parado nao vale para ela.
    for (size_t l = 0; l < mCasas.size(); ++l) {
        for (int c = 0; c < Banco::kColunas; ++c) {
            BossProjectile* p = mCasas[l][static_cast<size_t>(c)];
            if (!p || p->GetState() != ActorState::Active) continue;
            p->RenovarTempoDeVida();
            p->SetPosition(Ancora::Aproximar(p->GetPosition(), LugarDoDado(static_cast<int>(l), c), kRapidezDaCasa, deltaTime));
            if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
    }
    for (BossProjectile* p : mCabecalhos) {
        if (!p || p->GetState() != ActorState::Active) continue;
        p->RenovarTempoDeVida();
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
}

// A fase 1: o select.

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
    // A casa apaga quando a varredura passa por ela.
    const SDL_FRect campo = Campo();
    for (const Banco::Casa& casa : casas) {
        float fim = porLinha
            ? Tabela::Inicio(campo.x, campo.w, Banco::kColunas, casa.coluna) + Tabela::Espessura(campo.w, Banco::kColunas)
            : Tabela::Inicio(campo.y, campo.h, Banco::kLinhas, casa.linha) + Tabela::Espessura(campo.h, Banco::kLinhas);
        float inicio = porLinha ? campo.x : campo.y;
        if (invertida) {
            // Do outro lado: a casa apaga quando a parede passa pela borda de cima dela.
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
        // A fase acabou: o que estava aceso apaga, e o placar volta ao do arquivo.
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
                // As linhas do where, varridas de ponta a ponta.
                MarcarVarredura(casas, true, agora, mDegrau.avisoDaLinha);
                for (const int l : Banco::Linhas(mRegistros, c.onde)) {
                    Varrer(Banco::kGatilhoDaSelecao, l, mDegrau.avisoDaLinha, mDegrau.velocidadeDaLinha);
                }
                break;
            case Banco::Tipo::Projecao:
                // A coluna, varrida de cima a baixo.
                MarcarVarredura(casas, false, agora, mDegrau.avisoDaColuna);
                Varrer(Banco::kGatilhoDaProjecao, static_cast<int>(c.coluna), mDegrau.avisoDaColuna,
                       mDegrau.velocidadeDaColuna);
                break;
            case Banco::Tipo::Ambas:
                // So os cruzamentos: as casas acendem por inteiro.
                Marcar(casas, agora + mDegrau.avisoDaCasa);
                mPendentes.push_back(Pendente{casas, agora + mDegrau.avisoDaCasa});
                break;
            case Banco::Tipo::Ordenacao: {
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
    // As casas que acendem: cada uma se enche de pedacos, por um instante.
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

    // O que tem hora para acabar, acaba.
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

namespace {
    std::uint32_t Sorteio() {
        return static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
    }

    constexpr int kNota = static_cast<int>(Banco::Coluna::Nota);
}

// Os registros que voam: o INSERT, o DELETE e o UPDATE, nas fases 2 e 3.

float Thiago::Chao() const {
    const SDL_FRect campo = Campo();
    return campo.y + campo.h - 6.0f;
}

void Thiago::Arremessar(const int linha, const Vector2 de, const float espera) {
    if (linha < 0 || static_cast<size_t>(linha) >= mRegistros.size()) return;
    if (mCasas.size() < mRegistros.size()) mCasas.resize(mRegistros.size());
    const Banco::Registro r = mRegistros[static_cast<size_t>(linha)];
    for (int c = 0; c < Banco::kColunas; ++c) {
        // Escrito em fila embaixo do professor, e so entao arremessado - um valor depois do outro.
        const Vector2 escrito = de + Vector2((static_cast<float>(c) - 2.0f) * kEntreValoresEscritos, kAbaixoDoProfessor);
        for (BossProjectile* p : DispararEm(Banco::kGatilhoDoDado, escrito)) {
            p->DefinirEnfeite(true);
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Banco::Cara(r, static_cast<Banco::Coluna>(c)));
            Voo v;
            v.p = p;
            v.tipo = Voo::Tipo::Arco;
            v.de = escrito;
            v.para = LugarDoDado(linha, c);
            v.altura = Dml::kAlturaDoArco;
            v.inicio = mRelogio + espera + 0.06f * static_cast<float>(c);
            v.duracao = mDegrauDaEscrita.duracaoDoArco;
            v.mat = r.mat;
            v.coluna = c;
            mVoos.push_back(v);
        }
    }
}

void Thiago::Derrubar(const int linha) {
    if (linha < 0 || static_cast<size_t>(linha) >= mCasas.size()) return;
    for (BossProjectile* p : mCasas[static_cast<size_t>(linha)]) {
        if (!p || p->GetState() != ActorState::Active) continue;
        p->DefinirEnfeite(false);
        p->DefinirPosto("");
        if (auto* draw = p->GetComponent<DrawAnimatedComponent>()) {
            draw->SetIsVisible(true);
            draw->SetColor(255, 105, 95);
        }
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, 40.0f));
        Voo v;
        v.p = p;
        v.tipo = Voo::Tipo::Queda;
        mVoos.push_back(v);
    }
    mCasas.erase(mCasas.begin() + linha);
}

void Thiago::Espatifar(const Vector2 onde) {
    // O leque e o do ataque; aqui so a gravidade.
    for (BossProjectile* p : DispararEm(Dml::kGatilhoDoCaco, onde)) {
        Voo v;
        v.p = p;
        v.tipo = Voo::Tipo::Caco;
        mVoos.push_back(v);
    }
}

void Thiago::Chutar(const int linha) {
    if (linha < 0 || static_cast<size_t>(linha) >= mCasas.size()) return;
    BossProjectile* p = mCasas[static_cast<size_t>(linha)][static_cast<size_t>(kNota)];
    mCasas[static_cast<size_t>(linha)][static_cast<size_t>(kNota)] = nullptr;
    if (!p || p->GetState() != ActorState::Active) return;
    p->DefinirEnfeite(false);
    p->DefinirPosto("");
    if (auto* draw = p->GetComponent<DrawAnimatedComponent>()) {
        draw->SetIsVisible(true);
        draw->SetColor(255, 170, 90);
    }
    Vector2 d(0.0f, 1.0f);
    ApontarParaOAluno(p->GetPosition(), d);
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(d * mDegrauDaEscrita.velocidadeDoChute);
}

void Thiago::EscreverNota(const int linha, const Vector2 de) {
    if (linha < 0 || static_cast<size_t>(linha) >= mRegistros.size()) return;
    const Banco::Registro& r = mRegistros[static_cast<size_t>(linha)];
    const Vector2 casa = LugarDoDado(linha, kNota);
    for (BossProjectile* p : DispararEm(Banco::kGatilhoDoDado, de)) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Banco::Cara(r, Banco::Coluna::Nota));
        Voo v;
        v.p = p;
        v.tipo = Voo::Tipo::Arco;
        v.de = de;
        v.para = casa;
        v.inicio = mRelogio;
        v.duracao = std::max(0.15f, (casa - de).Length() / Dml::kVelocidadeDaEscrita);
        v.mat = r.mat;
        v.coluna = kNota;
        mVoos.push_back(v);
    }
}

void Thiago::Riscar(const int linha) {
    if (linha < 0 || static_cast<size_t>(linha) >= mCasas.size()) return;
    for (int c = 0; c < Banco::kColunas; ++c) {
        if (BossProjectile* p = mCasas[static_cast<size_t>(linha)][static_cast<size_t>(c)]) {
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetColor(255, 140, 130);
        }
        for (BossProjectile* p : DispararEm(Transacao::kGatilhoDoRascunho, CentroDaCasa(linha, c))) {
            p->DefinirEnfeite(true);
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Dml::kCaraDoRisco);
            mRiscos.push_back(p);
        }
    }
}

void Thiago::ApagarRiscos() {
    for (BossProjectile* p : mRiscos) if (p->GetState() == ActorState::Active) p->Kill();
    mRiscos.clear();
}

void Thiago::AtualizarVoos(const float deltaTime) {
    const float chao = Chao();
    std::vector<Vector2> espatifados;
    for (auto it = mVoos.begin(); it != mVoos.end();) {
        Voo& v = *it;
        if (!v.p || v.p->GetState() != ActorState::Active) { it = mVoos.erase(it); continue; }
        auto* rb = v.p->GetComponent<RigidBodyComponent>();

        if (v.tipo == Voo::Tipo::Arco) {
            // O destino acompanha a linha, que pode ter subido.
            const int l = Transacao::LinhaDe(mRegistros, v.mat);
            if (l >= 0) v.para = LugarDoDado(l, v.coluna);
            const float t = (mRelogio - v.inicio) / v.duracao;
            if (rb) rb->SetVelocity(Vector2::Zero);
            if (t < 0.0f) {
                // Ainda sendo escrito: parado, e sem machucar.
                v.p->SetPosition(v.de);
                ++it;
                continue;
            }
            if (t < 1.0f) {
                v.p->DefinirEnfeite(false);
                v.p->SetPosition(Dml::NoArco(v.de, v.para, v.altura, t));
                ++it;
                continue;
            }
            // Chegou: vira a casa, se a linha ainda existe e a casa esta vazia.
            const bool cabe = l >= 0 && static_cast<size_t>(l) < mCasas.size() && v.coluna >= 0
                           && v.coluna < Banco::kColunas
                           && mCasas[static_cast<size_t>(l)][static_cast<size_t>(v.coluna)] == nullptr;
            if (cabe) {
                v.p->SetPosition(v.para);
                v.p->DefinirEnfeite(true);
                mCasas[static_cast<size_t>(l)][static_cast<size_t>(v.coluna)] = v.p;
            } else {
                v.p->Kill();
            }
            it = mVoos.erase(it);
            continue;
        }

        // A queda e os cacos: a gravidade.
        if (rb) rb->SetVelocity(rb->GetVelocity() + Vector2(0.0f, mDegrauDaEscrita.gravidade * deltaTime));
        if (v.tipo == Voo::Tipo::Queda && v.p->GetPosition().y >= chao) {
            espatifados.emplace_back(v.p->GetPosition().x, chao);
            v.p->Kill();
            it = mVoos.erase(it);
            continue;
        }
        ++it;
    }
    for (const Vector2& onde : espatifados) Espatifar(onde);
}

// A fase 2: o INSERT, o UPDATE e o DELETE.

void Thiago::AtualizarEscrita() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naEscrita = fsm && fsm->GetStateName() == Banco::kFaseDaEscrita;

    if (!naEscrita) {
        if (mNaEscrita) {
            // O comando que ainda esperava o aviso nao acontece: a linha riscada volta ao normal.
            ApagarRiscos();
            for (auto& linha : mCasas) {
                for (BossProjectile* p : linha) {
                    if (!p || p->GetState() != ActorState::Active) continue;
                    if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetColor(255, 255, 255);
                }
            }
            mExecucoes.clear();
            ApagarCasas();
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

    // O comando novo: o placar o escreve, e o que ele vai mexer e marcado.
    if (agora >= mProximaEscrita && mExecucoes.empty()) {
        const Banco::Escrita e = Banco::ProximaEscrita(mRegistros, Sorteio());
        RegistrarAnuncio(Banco::kFaseDaEscrita, Banco::Sql(e));
        mProximaEscrita = agora + mDegrau.passoDaEscrita;

        switch (e.comando) {
            case Banco::Comando::Insert: {
                // O registro novo e escrito embaixo do professor e arremessado, valor por valor, ate a linha vazia.
                const auto linhas = Banco::Executar(mRegistros, e);
                if (linhas.empty()) break;
                const int l = linhas.front();
                mCasas.resize(mRegistros.size());
                std::vector<Banco::Casa> casas;
                for (int c = 0; c < Banco::kColunas; ++c) casas.push_back({l, c});
                Marcar(casas, agora + Dml::kEscrevendo + mDegrauDaEscrita.duracaoDoArco + 0.3f);
                Arremessar(l, GetPosition(), Dml::kEscrevendo);
                mAlvo = Vector2(Banco::XDoProfessor(Banco::kColunas / 2, Campo().w), Banco::kYDoProfessor);
                break;
            }
            case Banco::Comando::Update: {
                // A nota de cada linha do where e marcada: depois do aviso, a velha e chutada e a nova chega.
                std::vector<Banco::Casa> casas;
                for (const int l : Banco::Linhas(mRegistros, e.onde)) casas.push_back({l, kNota});
                Marcar(casas, agora + mDegrauDaEscrita.avisoDoUpdate);
                mExecucoes.push_back(Execucao{e, agora + mDegrauDaEscrita.avisoDoUpdate});
                mAlvo = Vector2(Banco::XDoProfessor(static_cast<int>(e.onde.coluna), Campo().w), Banco::kYDoProfessor);
                break;
            }
            case Banco::Comando::Delete: {
                // A linha do where fica riscada; depois do aviso, ela cai.
                for (const int l : Banco::Linhas(mRegistros, e.onde)) Riscar(l);
                mExecucoes.push_back(Execucao{e, agora + mDegrauDaEscrita.avisoDoDelete});
                mAlvo = Vector2(Banco::XDoProfessor(static_cast<int>(e.onde.coluna), Campo().w), Banco::kYDoProfessor);
                break;
            }
        }
    }

    // O que foi anunciado acontece.
    for (auto it = mExecucoes.begin(); it != mExecucoes.end();) {
        if (agora < it->quando) { ++it; continue; }
        const Banco::Escrita e = it->escrita;
        it = mExecucoes.erase(it);
        if (e.comando == Banco::Comando::Delete) {
            ApagarRiscos();
            const auto linhas = Banco::Linhas(mRegistros, e.onde);
            for (auto l = linhas.rbegin(); l != linhas.rend(); ++l) Derrubar(*l);
            Banco::Executar(mRegistros, e);
        } else if (e.comando == Banco::Comando::Update) {
            for (const int l : Banco::Executar(mRegistros, e)) {
                Chutar(l);
                EscreverNota(l, GetPosition());
            }
        }
    }

    AtualizarCasas(agora);
}

// A fase 3: as transacoes.

std::vector<Transacao::Operacao> Thiago::Pendentes() const {
    std::vector<Transacao::Operacao> ops;
    for (const Aberta& tx : mAbertas) ops.insert(ops.end(), tx.ops.begin(), tx.ops.end());
    return ops;
}

Vector2 Thiago::OndeFicaOSelo(const Aberta& tx) const {
    const SDL_FRect campo = Campo();
    const auto p = static_cast<size_t>(std::clamp(tx.posicao, 0, static_cast<int>(Transacao::kPosicoesDoSelo.size()) - 1));
    return Vector2(campo.x + campo.w * Transacao::kPosicoesDoSelo[p], campo.y + Transacao::kYDoSelo);
}

void Thiago::Preparar(Aberta& tx, const float agora) {
    Transacao::Operacao op;
    if (!Transacao::Proxima(mRegistros, Pendentes(), Sorteio(), op, mPlanoDasTransacoes)) {
        // Nada mais cabe: a transacao segue para o COMMIT com o que tem.
        tx.preparada = true;
        tx.proxima = agora + mDegrauDasTransacoes.ateOCommit;
        return;
    }
    const int indice = static_cast<int>(tx.ops.size());
    tx.ops.push_back(op);
    RegistrarAnuncio(Transacao::kFaseDaTransacao, Transacao::PlacarDaOperacao(tx.id, op));

    // O rascunho: cada casa que a operacao vai mexer, na cor da transacao.
    const bool riscado = op.tipo == Transacao::Tipo::Delete;
    const auto casas = Transacao::Casas(mRegistros, op, static_cast<int>(mRegistros.size()));
    for (size_t k = 0; k < casas.size(); ++k) {
        for (BossProjectile* p : DispararEm(Transacao::kGatilhoDoRascunho, CentroDaCasa(casas[k].linha, casas[k].coluna))) {
            p->DefinirEnfeite(true);
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Transacao::CaraDoRascunho(tx.cor, riscado));
            tx.rascunhos.push_back(Rascunho{p, indice, static_cast<int>(k)});
        }
    }

    // O UPDATE escreve o valor sujo por cima da casa: ele so vale depois do COMMIT.
    BossProjectile* sujo = nullptr;
    if (op.tipo == Transacao::Tipo::Update) {
        const int l = Transacao::LinhaDe(mRegistros, op.mat);
        if (l >= 0 && static_cast<size_t>(l) < mCasas.size()) {
            for (BossProjectile* p : DispararEm(Banco::kGatilhoDoDado, LugarDoDado(l, kNota))) {
                p->DefinirEnfeite(true);
                if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Banco::CaraSuja(op.nota));
                sujo = p;
            }
            // O limpo some enquanto o sujo esta por cima.
            if (BossProjectile* limpo = mCasas[static_cast<size_t>(l)][static_cast<size_t>(kNota)]) {
                if (auto* draw = limpo->GetComponent<DrawAnimatedComponent>()) draw->SetIsVisible(false);
            }
        }
    }
    tx.sujos.push_back(sujo);

    if (static_cast<int>(tx.ops.size()) >= mDegrauDasTransacoes.operacoes) {
        tx.preparada = true;
        tx.proxima = agora + mDegrauDasTransacoes.ateOCommit;
        if (tx.selo) {
            if (auto* anim = tx.selo->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Transacao::CaraDoSelo(tx.cor, tx.id, true));
        }
    } else {
        tx.proxima = agora + mDegrauDasTransacoes.entreOperacoes;
    }
    // O professor escreve do lado da transacao.
    const float meio = Banco::XDoProfessor(Banco::kColunas / 2, Campo().w);
    mAlvo = Vector2(meio + (OndeFicaOSelo(tx).x - meio) * 0.35f, Banco::kYDoProfessor);
}

void Thiago::Confirmar(Aberta& tx) {
    RegistrarAnuncio(Transacao::kFaseDaTransacao, Transacao::PlacarDoCommit(tx.id));

    // Primeiro o que le e o que muda no lugar, depois o que tira linha, e por fim o que poe.
    std::vector<int> deletes;
    std::vector<Banco::Registro> inserts;
    for (size_t k = 0; k < tx.ops.size(); ++k) {
        const Transacao::Operacao& op = tx.ops[k];
        const int l = Transacao::LinhaDe(mRegistros, op.mat);
        switch (op.tipo) {
            case Transacao::Tipo::SelectLinha:
                if (l >= 0) Varrer(Banco::kGatilhoDaSelecao, l, Transacao::kAvisoNoCommit, mDegrau.velocidadeDaLinha);
                break;
            case Transacao::Tipo::SelectColuna:
                Varrer(Banco::kGatilhoDaProjecao, op.coluna, Transacao::kAvisoNoCommit, mDegrau.velocidadeDaColuna);
                break;
            case Transacao::Tipo::Update: {
                if (l < 0 || static_cast<size_t>(l) >= mCasas.size()) break;
                Chutar(l);
                mRegistros[static_cast<size_t>(l)].nota = op.nota;
                BossProjectile* sujo = k < tx.sujos.size() ? tx.sujos[k] : nullptr;
                if (sujo && sujo->GetState() == ActorState::Active) {
                    // O valor sujo vira o da tabela.
                    if (auto* anim = sujo->GetComponent<DrawAnimatedComponent>()) {
                        anim->SetAnimation(Banco::Cara(mRegistros[static_cast<size_t>(l)], Banco::Coluna::Nota));
                    }
                    mCasas[static_cast<size_t>(l)][static_cast<size_t>(kNota)] = sujo;
                    tx.sujos[k] = nullptr;
                }
                break;
            }
            case Transacao::Tipo::Delete:
                if (l >= 0) deletes.push_back(l);
                break;
            case Transacao::Tipo::Insert:
                inserts.push_back(op.novo);
                break;
        }
    }
    std::sort(deletes.rbegin(), deletes.rend());
    for (const int l : deletes) {
        Derrubar(l);
        mRegistros.erase(mRegistros.begin() + l);
    }
    const Vector2 de = tx.selo ? tx.selo->GetPosition() : GetPosition();
    for (const Banco::Registro& novo : inserts) {
        if (static_cast<int>(mRegistros.size()) >= Banco::kLinhas) continue;
        mRegistros.push_back(novo);
        mCasas.resize(mRegistros.size());
        Arremessar(static_cast<int>(mRegistros.size()) - 1, de, 0.0f);
    }
    Desfazer(tx);
}

void Thiago::Desfazer(Aberta& tx) {
    for (Rascunho& r : tx.rascunhos) if (r.p && r.p->GetState() == ActorState::Active) r.p->Kill();
    tx.rascunhos.clear();
    for (BossProjectile*& p : tx.sujos) {
        if (p && p->GetState() == ActorState::Active) p->Kill();
        p = nullptr;
    }
    // O valor limpo de cada UPDATE volta a aparecer.
    for (const Transacao::Operacao& op : tx.ops) {
        if (op.tipo != Transacao::Tipo::Update) continue;
        const int l = Transacao::LinhaDe(mRegistros, op.mat);
        if (l < 0 || static_cast<size_t>(l) >= mCasas.size()) continue;
        if (BossProjectile* limpo = mCasas[static_cast<size_t>(l)][static_cast<size_t>(kNota)]) {
            if (auto* draw = limpo->GetComponent<DrawAnimatedComponent>()) draw->SetIsVisible(true);
        }
    }
    if (tx.selo && tx.selo->GetState() == ActorState::Active) tx.selo->Kill();
    tx.selo = nullptr;
}

void Thiago::Acompanhar() {
    const int vazia = static_cast<int>(mRegistros.size());
    for (Aberta& tx : mAbertas) {
        for (Rascunho& r : tx.rascunhos) {
            if (!r.p || r.p->GetState() != ActorState::Active) continue;
            const auto casas = Transacao::Casas(mRegistros, tx.ops[static_cast<size_t>(r.op)], vazia);
            if (static_cast<size_t>(r.casa) < casas.size()) {
                r.p->SetPosition(CentroDaCasa(casas[static_cast<size_t>(r.casa)].linha, casas[static_cast<size_t>(r.casa)].coluna));
            }
            if (auto* rb = r.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
        for (size_t k = 0; k < tx.sujos.size() && k < tx.ops.size(); ++k) {
            BossProjectile* p = tx.sujos[k];
            if (!p || p->GetState() != ActorState::Active) continue;
            const int l = Transacao::LinhaDe(mRegistros, tx.ops[k].mat);
            if (l >= 0) p->SetPosition(LugarDoDado(l, kNota));
            if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
        if (tx.selo && tx.selo->GetState() == ActorState::Active) {
            tx.selo->SetPosition(OndeFicaOSelo(tx));
            if (auto* rb = tx.selo->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
    }
}

void Thiago::AtualizarTransacoes() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool nasTransacoes = fsm && fsm->GetStateName() == Transacao::kFaseDaTransacao;

    if (!nasTransacoes) {
        if (mNasTransacoes) {
            // A fase acabou: o que nao teve COMMIT nao aconteceu.
            for (Aberta& tx : mAbertas) Desfazer(tx);
            mAbertas.clear();
            ApagarCasas();
            if (!mAnuncioDasTransacoes.empty()) RegistrarAnuncio(Transacao::kFaseDaTransacao, mAnuncioDasTransacoes);
            mAlvo.reset();
            mNasTransacoes = false;
        }
        return;
    }
    if (!mNasTransacoes) {
        mNasTransacoes = true;
        mAbertas.clear();
        mProximoId = 1;
        mProximaTransacao = Transacao::kPrimeira;
        if (mAnuncioDasTransacoes.empty()) mAnuncioDasTransacoes = AnuncioDaFase(Transacao::kFaseDaTransacao);
        mAlvo = Vector2(Banco::XDoProfessor(Banco::kColunas / 2, Campo().w), Banco::kYDoProfessor);
        // A tabela cheia de novo: o que o DELETE apagou na fase 2 e reinserido, vindo do professor.
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

    // BEGIN: uma transacao nova, num lugar livre, se alguma operacao ainda cabe.
    if (agora >= mProximaTransacao && static_cast<int>(mAbertas.size()) < Transacao::kMaximo) {
        mProximaTransacao = agora + mDegrauDasTransacoes.novaTransacao;
        Transacao::Operacao cabe;
        if (Transacao::Proxima(mRegistros, Pendentes(), Sorteio(), cabe, mPlanoDasTransacoes)) {
            Aberta tx;
            tx.id = mProximoId;
            mProximoId = mProximoId % Transacao::kIds + 1;
            // A cor que a outra aberta nao usa, e um lugar sorteado que ninguem ocupa.
            std::vector<int> ocupadas;
            for (int c = 0; c < Transacao::kMaximo; ++c) {
                const bool usada = std::any_of(mAbertas.begin(), mAbertas.end(), [c](const Aberta& a) { return a.cor == c; });
                if (!usada) { tx.cor = c; break; }
            }
            for (const Aberta& a : mAbertas) ocupadas.push_back(a.posicao);
            tx.posicao = Transacao::PosicaoDoSelo(ocupadas, Sorteio());
            tx.proxima = agora + Transacao::kPrimeiraOperacao;
            for (BossProjectile* p : DispararEm(Transacao::kGatilhoDoSelo, OndeFicaOSelo(tx))) {
                tx.selo = p;
                if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Transacao::CaraDoSelo(tx.cor, tx.id, false));
            }
            RegistrarAnuncio(Transacao::kFaseDaTransacao, Transacao::PlacarDoBegin(tx.id));
            mAbertas.push_back(tx);
        }
    }

    // Cada transacao na sua vez: mais uma operacao, ou o COMMIT.
    for (size_t i = 0; i < mAbertas.size();) {
        Aberta& tx = mAbertas[i];
        if (agora < tx.proxima) { ++i; continue; }
        if (!tx.preparada) {
            Preparar(tx, agora);
            ++i;
            continue;
        }
        Confirmar(tx);
        mAbertas.erase(mAbertas.begin() + static_cast<std::ptrdiff_t>(i));
    }

    Acompanhar();
    AtualizarCasas(agora);
}

// A repescagem: o produto cartesiano.

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

    // Uma volta nova do laco: o placar diz o plano, e cada casa ganha a hora de ser marcada.
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

    // A casa da vez e marcada, e acende kAvisoDoProduto depois.
    for (auto it = mAgendadas.begin(); it != mAgendadas.end();) {
        if (agora < it->marca) { ++it; continue; }
        Marcar({it->casa}, it->marca + mDegrau.avisoDoProduto);
        mPendentes.push_back(Pendente{{it->casa}, it->marca + mDegrau.avisoDoProduto});
        it = mAgendadas.erase(it);
    }

    AtualizarCasas(agora);
}

// Os ganchos.

void Thiago::OnDerrubou(BossProjectile* qual) {
    // O selo derrubado e a falha que desfaz a transacao inteira: tudo ou nada.
    for (size_t i = 0; i < mAbertas.size(); ++i) {
        if (mAbertas[i].selo != qual) continue;
        mAbertas[i].selo = nullptr;
        RegistrarAnuncio(Transacao::kFaseDaTransacao, Transacao::PlacarDoRollback(mAbertas[i].id));
        Desfazer(mAbertas[i]);
        mAbertas.erase(mAbertas.begin() + static_cast<std::ptrdiff_t>(i));
        return;
    }
}

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

    // O que o Thiago dispara num lugar nasce nele.
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
