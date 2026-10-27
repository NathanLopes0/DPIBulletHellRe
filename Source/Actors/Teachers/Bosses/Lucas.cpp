//
// O chefe da INF 321 - Programacao Web.
//

#include "Lucas.h"

#include <algorithm>
#include <cmath>

#include "../../../Attacks/AttackParameters/CaixaAttackParams.h"
#include "../../../Attacks/AttackParameters/FlexAttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {
    /// A tag guardada ainda e o mesmo tiro se a idade dela bate com a da caixa:
    /// o pool devolve o projetil com o relogio zerado.
    constexpr float kFolgaDaIdade = 0.1f;
}

Lucas::Lucas(Scene* scene) : Boss(scene) {
}

void Lucas::SortearALuta() {
    mLutaSorteada = true;
    mSorteioDoDom = SorteioDaFase(Dom::kFaseDoDom);
    mSorteioDoFlex = SorteioDaFase(kFaseDoFlex);
    mDegrauDoDom = Dom::DegrauDo(NivelDeDificuldade());
    mDegrauDoElixir = Elixir::DegrauDo(NivelDeDificuldade());
}

void Lucas::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    AtualizarDom();
    AtualizarFlex();
    AtualizarElixir();
}

void Lucas::AtualizarFlex() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noFlex = fsm && fsm->GetStateName() == kFaseDoFlex && !EhClone();
    if (!noFlex) {
        if (mNoFlex) mAlvo.reset();
        mNoFlex = false;
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    if (!mNoFlex) {
        mNoFlex = true;
        mXDoComecoDaVarredura = GetPosition().x;
    }
    // O PROFESSOR VARRE O ALTO, devagar no Normal e mais depressa no Dificil
    // (Flex::XDaVarredura): o movimento "Conduzido" vai atras do ponto.
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const float velocidade = NoNivel(Flex::kVarredura, Flex::kVarreduraNoDificil);
    mAlvo = Vector2(Flex::XDaVarredura(fsm->GetStateTime(), mXDoComecoDaVarredura, velocidade, largura),
                    Flex::kYDaVarredura);
}


void Lucas::OnProjetilDerrubado() {
    ++mParaReiniciar;
}

Vector2 Lucas::PosicaoDoAluno() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene); battle && battle->Alvo()) {
        return battle->Alvo()->GetPosition();
    }
    return GetPosition() + GetDirectionToPlayer() * GetDistanceToPlayer();
}

std::vector<BossProjectile*> Lucas::DispararDe(const char* gatilho, const Vector2 onde,
                                               const std::optional<float> rumo, const float escala) {
    mDisparandoDe = true;
    mOndeDisparar = onde;
    if (rumo) {
        mRumoDoDisparo = *rumo;
    } else {
        const Vector2 d = PosicaoDoAluno() - onde;
        mRumoDoDisparo = Math::ToDegrees(Math::Atan2(d.y, d.x));
    }
    mEscalaDoDisparo = escala;
    mDisparados.clear();
    Disparar(gatilho);
    mDisparandoDe = false;
    return mDisparados;
}

// ---------------------------------------------------------------------------
// FASE 1 - O DOCUMENTO
// ---------------------------------------------------------------------------

void Lucas::AtualizarDom() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noDom = fsm && fsm->GetStateName() == Dom::kFaseDoDom && !EhClone();

    if (!noDom) {
        // A FASE ACABOU: as caixas seguem como estao, o placar volta ao nome
        // dela, e o professor deixa de ter para onde ir.
        if (mNoDom) {
            if (!mAnuncioDoDom.empty()) RegistrarAnuncio(Dom::kFaseDoDom, mAnuncioDoDom);
            mCaixas.clear();
            mEventos.clear();
            mAlvo.reset();
        }
        mNoDom = false;
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());

    if (!mNoDom) {
        mNoDom = true;
        mEventos.clear();
        mProximo = 0;
        mProximoEvento = Dom::kPrimeiro;
        if (mAnuncioDoDom.empty()) mAnuncioDoDom = AnuncioDaFase(Dom::kFaseDoDom);
        RegistrarAnuncio(Dom::kFaseDoDom, mAnuncioDoDom);
    }

    const auto lugar = [&](const int elemento) {
        const Dom::Ponto& c = mDocumento.elementos[static_cast<size_t>(elemento)].centro;
        return Vector2(c.x, c.y);
    };

    const float agora = fsm->GetStateTime();
    while (agora >= mProximoEvento) {
        // UM DOCUMENTO NOVO quando o anterior fechou a raiz.
        if (mProximo >= mEventos.size()) {
            mDocumento = Dom::Gerar(mSorteioDoDom, largura);
            mEventos = Dom::Eventos(mDocumento);
            mCaixas.assign(mDocumento.elementos.size(), Caixa{});
            mProximo = 0;
        }
        const Dom::Evento e = mEventos[mProximo++];
        const Dom::Elemento& el = mDocumento.elementos[static_cast<size_t>(e.elemento)];
        Caixa& caixa = mCaixas[static_cast<size_t>(e.elemento)];

        switch (e.tipo) {
            case Dom::Tipo::Abre:
                // ABRE: a caixa nasce no lugar do elemento, menor a cada geracao.
                caixa.tags = DispararDe(Dom::kGatilhoDaAbertura, lugar(e.elemento), 90.0f,
                                        Dom::Encolher(el.profundidade));
                caixa.aberta = agora;
                break;
            case Dom::Tipo::Fecha:
                // FECHA: a caixa vira do avesso - cada tag que ainda e dela
                // volta pelo centro e sai do outro lado, mais depressa.
                for (BossProjectile* tag : caixa.tags) {
                    if (!tag || tag->GetState() != ActorState::Active) continue;
                    if (std::fabs(tag->TempoDeVida() - (agora - caixa.aberta)) > kFolgaDaIdade) continue;
                    if (auto* rb = tag->GetComponent<RigidBodyComponent>()) rb->SetVelocity(rb->GetVelocity() * -Dom::kVolta);
                }
                caixa.tags.clear();
                break;
            case Dom::Tipo::Vazio:
                // O VAZIO ABRE E FECHA DE UMA VEZ: um anel.
                DispararDe(Dom::kGatilhoDoVazio, lugar(e.elemento), 90.0f);
                break;
        }
        RegistrarAnuncio(Dom::kFaseDoDom, Dom::Placar(mDocumento, e));
        mProximoEvento += mDegrauDoDom.porEvento;
        if (mProximo >= mEventos.size()) mProximoEvento += Dom::kEntreDocumentos;
    }

    // O PROFESSOR FICA EM CIMA DA RAIZ do documento que escreve (Dom.h).
    mAlvo = mDocumento.elementos.empty() ? Vector2(largura / 2.0f, Dom::kYDaRaiz) : lugar(0);



}

// ---------------------------------------------------------------------------
// FASE 3 - AS MENSAGENS
// ---------------------------------------------------------------------------

void Lucas::AtualizarElixir() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noElixir = fsm && fsm->GetStateName() == Elixir::kFaseDoElixir && !EhClone();
    if (!noElixir) {
        if (mNoElixir && !mAnuncioDoElixir.empty()) RegistrarAnuncio(Elixir::kFaseDoElixir, mAnuncioDoElixir);
        mNoElixir = false;
        return;
    }
    if (!mNoElixir) {
        mNoElixir = true;
        mProximaMensagem = Elixir::kPrimeira;
        mMensagens = 0;
        if (mAnuncioDoElixir.empty()) mAnuncioDoElixir = AnuncioDaFase(Elixir::kFaseDoElixir);
    }
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return;

    const float agora = fsm->GetStateTime();
    if (agora < mProximaMensagem) return;
    mProximaMensagem += mDegrauDoElixir.entreMensagens;

    // OS PROCESSOS VIVOS, da esquerda para a direita: o rodizio anda pela
    // fileira deles, e a mensagem vem de um lugar diferente a cada vez.
    std::vector<BossProjectile*> processos;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (!p || p->GetState() != ActorState::Active || !p->Derrubavel()) continue;
        processos.push_back(p.get());
    }
    std::sort(processos.begin(), processos.end(),
              [](const BossProjectile* a, const BossProjectile* b) { return a->GetPosition().x < b->GetPosition().x; });
    const int quem = Elixir::Remetente(mMensagens, static_cast<int>(processos.size()));
    if (quem < 0) return;
    DispararDe(Elixir::kGatilhoDaMensagem, processos[static_cast<size_t>(quem)]->GetPosition(), std::nullopt);
    ++mMensagens;
    RegistrarAnuncio(Elixir::kFaseDoElixir, Elixir::Placar(mMensagens));
}

// ---------------------------------------------------------------------------

void Lucas::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoDe) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mDisparados.push_back(b);
    }
}

void Lucas::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O QUE SAI DE UM LUGAR (a caixa de um elemento, a mensagem de um
    // processo) sai de onde o Lucas manda, no rumo dele. A caixa do filho e
    // menor que a do pai: o tamanho e as tags encolhem juntos, e o espaco
    // entre duas tags fica o mesmo.
    if (mDisparandoDe) {
        params.firePosition = mOndeDisparar;
        params.centralAngle = mRumoDoDisparo;
        if (auto* caixa = dynamic_cast<CaixaAttackParams*>(&params)) {
            if (mMeiaAlturaDoArquivo < 0.0f) {
                mMeiaAlturaDoArquivo = caixa->meiaAlturaInicial;
                mTagsDoArquivo = params.numProjectiles;
            }
            caixa->meiaAlturaInicial = mMeiaAlturaDoArquivo * mEscalaDoDisparo;
            params.numProjectiles = std::max(8, static_cast<int>(std::lround(static_cast<float>(mTagsDoArquivo) * mEscalaDoDisparo)));
        }
        return;
    }

    // O SUPERVISOR DEVOLVE OS PROCESSOS QUE CAIRAM, na rodada seguinte da fase
    // 3 e na mesma rajada dos novos. Escrito como TOTAL, a partir do numero do
    // arquivo: params e o mesmo objeto a cada disparo (Boss.h), e somar sobre o
    // anterior faria a rodada crescer para sempre.
    if (stateName == Elixir::kFaseDoElixir) {
        if (mProcessosDoArquivo < 0) mProcessosDoArquivo = params.numProjectiles;
        const int voltam = std::min(mParaReiniciar, kReiniciosPorRodada);
        params.numProjectiles = mProcessosDoArquivo + voltam;
        mParaReiniciar -= voltam;
        return;
    }

    // O FLEXBOX SAI DE ONDE PAROU. Cada disparo nasce no arranjo em que o
    // anterior terminou e se rearranja para uma regra sorteada, nunca a mesma
    // (Flex::Proxima). E alterna a direcao: a FILEIRA cai do professor, a
    // COLUNA atravessa a tela de lado, na altura do aluno, entrando por um
    // lado sorteado (Flex::DirecaoDoDisparo).
    auto* flex = dynamic_cast<FlexAttackParams*>(&params);
    if (!flex) return;

    if (!mGuardouOFlex) {
        mUltimoArranjo = flex->de;
        mLarguraDaFileira = flex->largura;
        mItensDaFileira = params.numProjectiles;

        mGuardouOFlex = true;
    }
    flex->de = mUltimoArranjo;
    flex->para = Flex::Proxima(mSorteioDoFlex, mUltimoArranjo);
    mUltimoArranjo = flex->para;

    const Flex::Direcao direcao = Flex::DirecaoDoDisparo(mSorteioDoFlex, mDisparosDoFlex++,
        NoNivel(Flex::kCadaQuantasColunas, Flex::kCadaQuantasColunasNoDificil));
    flex->coluna = direcao.coluna;
    flex->sentido = direcao.sentido;
    flex->largura = direcao.coluna ? Flex::kAlturaDaColuna : mLarguraDaFileira;
    params.numProjectiles = direcao.coluna ? Flex::kItensDaColuna : mItensDaFileira;
    if (direcao.coluna && mScene && mScene->GetGame()) {
        const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
        params.firePosition = Vector2(direcao.sentido > 0.0f ? -Flex::kForaDaTela : largura + Flex::kForaDaTela,
                                      Flex::kYDaColuna);
    }

    // E O PLACAR ESCREVE A REGRA DESTE DISPARO. Ele nasce no arranjo velho e
    // so muda depois de "aos" segundos: e o tempo de ler o quadro e saber
    // para onde ele vai se abrir.
    RegistrarAnuncio(stateName, "flex-direction: " + Flex::NomeDaDirecao(direcao.coluna) +
                                "\njustify-content: " + Flex::Nome(flex->para));
}
