//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#include "Vladimir.h"

#include <algorithm>
#include <limits>

#include "../../../Random.h"
#include "../../../Attacks/Alfabeto.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Attacks/Fita.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"

Vladimir::Vladimir(Scene* scene) : Boss(scene) {
}

void Vladimir::SortearALuta() {
    mLutaSorteada = true;
    const Dificuldade::Nivel nivel = NivelDeDificuldade();
    mDegrauDaGramatica = Gramatica::DegrauDo(nivel);
    mDegrauDaFita = Fita::DegrauDo(nivel);
    mDegrauDaParada = Parada::DegrauDo(nivel);
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Fita::kFaseDaFita);
        mPlanoDaFita = Fita::Sortear(sorteio);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Parada::kFaseDaParada);
        mPlanoDaParada = Parada::Sortear(sorteio);
    }
}

void Vladimir::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    AtualizarAutomato();
    AtualizarGramatica(deltaTime);
    AtualizarFita(deltaTime);
    AtualizarParada(deltaTime);
}

void Vladimir::AtualizarFita(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naFita = fsm && fsm->GetStateName() == Fita::kFaseDaFita && !EhClone();
    if (!naFita) {
        if (mNaFita) {
            if (!mAnuncioDaFita.empty()) RegistrarAnuncio(Fita::kFaseDaFita, mAnuncioDaFita);
            mAlvo.reset();
        }
        mNaFita = false;
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const auto ponta = [&](const int sentido) {
        return Vector2(sentido > 0 ? largura - Fita::kMargem : Fita::kMargem, Fita::kAlturaDaCabeca);
    };

    if (!mNaFita) {
        // A CABECA COMECA INDO PARA A DIREITA, em q0, escrevendo desde ja.
        mNaFita = true;
        mSentidoDaFita = 1;
        mPausaNaPonta = 0.0f;
        mProximaEscrita = mPlanoDaFita.primeiraEscrita;
        if (mAnuncioDaFita.empty()) mAnuncioDaFita = AnuncioDaFase(Fita::kFaseDaFita);
        mAlvo = ponta(mSentidoDaFita);
        RegistrarAnuncio(Fita::kFaseDaFita, Fita::Transicao(true));
    }

    // NA PONTA: parada, sem escrever, ate a pausa acabar - e entao volta.
    if (mPausaNaPonta > 0.0f) {
        mPausaNaPonta -= deltaTime;
        if (mPausaNaPonta <= 0.0f) {
            mSentidoDaFita = -mSentidoDaFita;
            mAlvo = ponta(mSentidoDaFita);
            mProximaEscrita = mPlanoDaFita.primeiraEscrita;
            RegistrarAnuncio(Fita::kFaseDaFita, Fita::Transicao(mSentidoDaFita > 0));
        }
        return;
    }

    // CHEGOU A PONTA: le o branco e troca de estado.
    if ((GetPosition() - ponta(mSentidoDaFita)).LengthSq() <= 1.0f) {
        mPausaNaPonta = mPlanoDaFita.pausaNaPonta;
        RegistrarAnuncio(Fita::kFaseDaFita, Fita::Branco(mSentidoDaFita > 0));
        return;
    }

    // ANDANDO, ESCREVE: uma casa a cada escrita do degrau (Fita::kEscrita no
    // Normal).
    mProximaEscrita -= deltaTime;
    if (mProximaEscrita <= 0.0f) {
        Disparar(Fita::kGatilhoDaEscrita);
        mProximaEscrita += mDegrauDaFita.escrita;
    }
}

void Vladimir::AtualizarParada(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naParada = fsm && fsm->GetStateName() == Parada::kFaseDaParada && !EhClone();
    if (!naParada) {
        if (mNaParada) {
            if (!mAnuncioDaParada.empty()) RegistrarAnuncio(Parada::kFaseDaParada, mAnuncioDaParada);
            mNaParada = false;
            mAlvo.reset();
        }
        return;
    }
    if (!mNaParada) {
        mNaParada = true;
        mX = 0;
        mProximoPassoDaParada = Parada::kPrimeiro;
        mXDaParada = GetPosition().x;
        mSentidoDaParada = mPlanoDaParada.sentido;
        mProgramaDaVez = 0;
        if (mAnuncioDaParada.empty()) mAnuncioDaParada = AnuncioDaFase(Parada::kFaseDaParada);
        RegistrarAnuncio(Parada::kFaseDaParada, mAnuncioDaParada);
    }

    // DE LADO A LADO, devagar, e na ALTURA DO VALOR DE X: o granizo.
    if (mScene && mScene->GetGame()) {
        const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
        mXDaParada += mSentidoDaParada * kVarreduraDaParada * deltaTime;
        if (mXDaParada > largura - kMargemDaParada) { mXDaParada = largura - kMargemDaParada; mSentidoDaParada = -1.0f; }
        if (mXDaParada < kMargemDaParada)          { mXDaParada = kMargemDaParada;          mSentidoDaParada = 1.0f; }
    }
    mAlvo = Vector2(mXDaParada, Parada::Altura(mX == 0 ? 1u : mX));

    const float agora = fsm->GetStateTime();
    if (agora < mProximoPassoDaParada) return;

    if (mX == 0) {
        // UM PROGRAMA NOVO, da proxima semente da luta.
        mX = Parada::SementeDaVez(mPlanoDaParada, mProgramaDaVez++);
        mProximoPassoDaParada += mDegrauDaParada.passo;
    } else if (!Parada::Parou(mX)) {
        // UM PASSO: atira conforme a paridade - a metade sao "a" retos, o
        // 3x + 1 sao "b" que perseguem -, e da o passo.
        Disparar(mX % 2u == 0u ? Parada::kGatilhoDaMetade : Parada::kGatilhoDoTriplo, -1, mDegrauDaParada.rapidez);
        mX = Parada::Proximo(mX);
        mProximoPassoDaParada += mDegrauDaParada.passo;
    } else {
        // PAROU: o anel, e depois de um tempo, outro programa.
        Disparar(Parada::kGatilhoDeParar);
        mX = 0;
        mProximoPassoDaParada += Parada::kParado;
        RegistrarAnuncio(Parada::kFaseDaParada, Parada::Placar(1u));
        return;
    }
    RegistrarAnuncio(Parada::kFaseDaParada, Parada::Placar(mX));
}

std::string Vladimir::AvisoDoIntercambio() const {
    return Alfabeto::kAvisoDaTroca;
}

std::optional<Vector2> Vladimir::AlvoDoMovimento() const {
    return mAlvo;
}

std::string Vladimir::CaraDoEstado(const int q, const bool aceso) {
    return "Q" + std::to_string(q) + (aceso ? "Aceso" : "");
}

Vector2 Vladimir::CentroDoEstado(const int q) {
    const auto i = static_cast<size_t>(q < 0 || q >= Automato::kEstados ? 0 : q);
    return Vector2(Automato::kXDosEstados[i], Automato::kYDosEstados[i]);
}

std::vector<BossProjectile*> Vladimir::NoPosto(const char* posto) const {
    std::vector<BossProjectile*> achados;
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return achados;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == posto) achados.push_back(p.get());
    }
    return achados;
}

void Vladimir::OnIntercambio() {
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return;

    // TODA LETRA EM VOO TROCA DE LETRA E DE COMPORTAMENTO, no quadro em que os
    // alunos trocam de lugar. O "a" ganha o Tracking do "b", e o "b" perde o
    // dele e segue reto no rumo em que estava - o modulo nao muda nos dois
    // casos, entao nenhuma letra acelera de surpresa.
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (!p || p->GetState() != ActorState::Active) continue;
        auto* anim = p->GetComponent<DrawAnimatedComponent>();
        if (!anim || !Alfabeto::Troca(anim->GetAnimation(), p->Posto(), p->Enfeite())) continue;

        const std::string nova = Alfabeto::Trocar(anim->GetAnimation());
        anim->SetAnimation(nova);
        if (nova == Alfabeto::kB) {
            p->insertMotion<TrackingBehavior>(kAtrasoDaTroca, kForcaDaTroca, kDuracaoDaTroca);
        } else {
            p->DescartarMotion();
        }
    }

    // E O QUE O AUTOMATO AINDA VAI LER TAMBEM TROCA: o placar muda, e com ele
    // o caminho do professor. O que ja foi lido fica como foi.
    if (mNoAutomato && mMomento == Momento::Lendo) {
        Automato::Trocar(mLeitura);
        RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
    }
}

void Vladimir::Formar(const char* gatilho, const Vector2 onde, std::string cara) {
    mFormando = true;
    mDisparoDoFormando = onde;
    mCaraDoFormando = std::move(cara);
    Disparar(gatilho);
    mFormando = false;
}

void Vladimir::AtualizarAutomato() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noAutomato = fsm && fsm->GetStateName() == Automato::kFaseDoAutomato && !EhClone();

    if (!noAutomato) {
        // A FASE ACABOU: o automato se desfaz, o placar volta ao que o arquivo
        // diz, e o professor deixa de ter para onde ir.
        if (mNoAutomato) {
            for (BossProjectile* p : NoPosto(Automato::kPostoDosEstados)) p->Kill();
            if (!mAnuncioDoAutomato.empty()) RegistrarAnuncio(Automato::kFaseDoAutomato, mAnuncioDoAutomato);
            mNoAutomato = false;
            mAlvo.reset();
        }
        return;
    }
    if (!mNoAutomato) {
        mNoAutomato = true;
        mAutomatoFormado = false;
        mMomento = Momento::Anuncio;
        mProximoPasso = kPrimeiraPalavra;
        mLeitura = Automato::Leitura{};
        if (mAnuncioDoAutomato.empty()) mAnuncioDoAutomato = AnuncioDaFase(Automato::kFaseDoAutomato);
        RegistrarAnuncio(Automato::kFaseDoAutomato, mAnuncioDoAutomato);
        mAlvo = CentroDoEstado(Automato::kInicial);
    }
    const float agora = fsm->GetStateTime();

    if (!mAutomatoFormado && agora >= kFormaOAutomato) {
        Formar(Automato::kGatilhoDoDiagrama, Vector2(Automato::kXDoDiagrama, Automato::kYDoDiagrama), "");
        for (int q = 0; q < Automato::kEstados; ++q) {
            Formar(Automato::kGatilhoDosEstados, CentroDoEstado(q), CaraDoEstado(q, false));
        }
        mAutomatoFormado = true;
    }

    if (agora >= mProximoPasso) {
        mProximoPasso += (mMomento == Momento::Lendo && Automato::Terminou(mLeitura)) ? kVeredito : kPasso;

        if (mMomento != Momento::Lendo) {
            // UMA PALAVRA NOVA, sempre do estado inicial: o professor volta ao
            // q0 sem atirar, e o placar mostra o que ele vai ler.
            const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
            mLeitura = Automato::Leitura{Automato::Sortear(sorteio), 0, Automato::kInicial};
            mMomento = Momento::Lendo;
            mAlvo = CentroDoEstado(Automato::kInicial);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
        } else if (!Automato::Terminou(mLeitura)) {
            // UM SIMBOLO: a letra sai do estado em que ele esta, e ele voa.
            const Automato::Passo passo = Automato::Ler(mLeitura);
            Disparar(passo.letra == 'a' ? Automato::kGatilhoDoA : Automato::kGatilhoDoB);
            mAlvo = CentroDoEstado(passo.para);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
        } else {
            // O VEREDITO: aceita, e o estado de aceitacao estoura num anel; ou
            // rejeita, e saem perseguidores de onde ele parou.
            Disparar(Automato::Aceita(mLeitura.estado) ? Automato::kGatilhoDeAceitar
                                                       : Automato::kGatilhoDeRejeitar);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Veredito(mLeitura));
            mMomento = Momento::Veredito;
        }
    }

    // O ESTADO EM QUE ELE ESTA FICA ACESO - lendo, ou no veredito.
    const bool lendo = mMomento != Momento::Anuncio;
    for (BossProjectile* p : NoPosto(Automato::kPostoDosEstados)) {
        auto* anim = p->GetComponent<DrawAnimatedComponent>();
        if (!anim || anim->GetAnimation() == Automato::kCaraDoDiagrama) continue;
        for (int q = 0; q < Automato::kEstados; ++q) {
            if ((p->GetPosition() - CentroDoEstado(q)).LengthSq() > 1.0f) continue;
            const std::string cara = CaraDoEstado(q, lendo && q == mLeitura.estado);
            if (anim->GetAnimation() != cara) anim->SetAnimation(cara);
        }
    }
}

void Vladimir::AtualizarGramatica(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naGramatica = fsm && fsm->GetStateName() == Gramatica::kFaseDaGramatica && !EhClone();

    // AS PALAVRAS VAZIAS SOMEM no tempo delas, dentro ou fora da fase.
    for (auto& [vazia, falta] : mVazias) {
        falta -= deltaTime;
        if (falta <= 0.0f && vazia->GetState() == ActorState::Active) vazia->Kill();
    }
    mVazias.erase(std::remove_if(mVazias.begin(), mVazias.end(),
                                 [](const auto& v) { return v.second <= 0.0f || v.first->GetState() != ActorState::Active; }),
                  mVazias.end());

    if (!naGramatica) {
        // A FASE ACABOU: o S que ainda voa segue reto, sem derivar mais, e o
        // placar volta ao que o arquivo diz.
        if (mNaGramatica) {
            mVariaveis.clear();
            mFormas.clear();
            if (!mAnuncioDaGramatica.empty()) RegistrarAnuncio(Gramatica::kFaseDaGramatica, mAnuncioDaGramatica);
            mNaGramatica = false;
        }
        return;
    }
    if (!mNaGramatica) {
        mNaGramatica = true;
        if (mAnuncioDaGramatica.empty()) mAnuncioDaGramatica = AnuncioDaFase(Gramatica::kFaseDaGramatica);
    }

    // QUEM NAO ESTA MAIS EM CAMPO SAI DA CONTA: o S derrubado pelo aluno, o que
    // saiu da tela. E o S que o professor acabou de atirar entra, com o sorteio.
    const auto emCampo = NoPosto(Gramatica::kPostoDaDerivacao);
    for (auto it = mVariaveis.begin(); it != mVariaveis.end();) {
        const bool esta = std::find(emCampo.begin(), emCampo.end(), it->first) != emCampo.end();
        it = esta ? std::next(it) : mVariaveis.erase(it);
    }
    for (BossProjectile* s : emCampo) {
        if (mVariaveis.count(s)) continue;
        const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
        const int cadeia = mProximaCadeia++;
        mVariaveis[s] = Variavel{0.0f, Gramatica::SortearDerivacoes(sorteio), cadeia};
        mFormas[cadeia] = Gramatica::kCaraDaVariavel;
        mCadeiaDoPlacar = cadeia;
        RegistrarAnuncio(Gramatica::kFaseDaGramatica, mAnuncioDaGramatica);
    }

    // OS QUE CHEGARAM A IDADE DE DERIVAR, separados antes: derivar mexe na conta.
    std::vector<BossProjectile*> derivam;
    for (auto& [s, v] : mVariaveis) {
        v.idade += deltaTime;
        if (v.idade >= mDegrauDaGramatica.derivar) derivam.push_back(const_cast<BossProjectile*>(s));
    }

    for (BossProjectile* s : derivam) {
        const Variavel v = mVariaveis[s];
        mVariaveis.erase(s);
        std::string& forma = mFormas[v.cadeia];
        const std::string antes = forma;

        Vector2 rumo(0.0f, 1.0f);
        if (const auto* rb = s->GetComponent<RigidBodyComponent>(); rb && rb->GetVelocity().LengthSq() > 0.0f) {
            rumo = rb->GetVelocity();
            rumo.Normalize();
        }

        if (v.restantes > 0) {
            // S -> aSb: o par sai aberto em volta do rumo, e o S novo segue nele.
            mDerivando = true;
            mDisparoDaDerivacao = s->GetPosition();
            mRumoDaDerivacao = Math::ToDegrees(Math::Atan2(rumo.y, rumo.x));
            Disparar(Gramatica::kGatilhoDoPar);
            mNascida = nullptr;
            Disparar(Gramatica::kGatilhoDaVariavel);
            if (mNascida) mVariaveis[mNascida] = Variavel{0.0f, v.restantes - 1, v.cadeia};
            mDerivando = false;
            s->Kill();
            forma = Gramatica::Derivar(forma);
        } else {
            // S -> e: o S vira a palavra vazia, que nao machuca e some logo.
            s->DefinirPosto("");
            s->DefinirEnfeite(true);
            if (auto* anim = s->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Gramatica::kCaraDaVazia);
            if (auto* rb = s->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
            s->DescartarComportamentos();
            mVazias.emplace_back(s, Gramatica::kVaziaNaTela);
            forma = Gramatica::Esvaziar(forma);
        }
        if (v.cadeia == mCadeiaDoPlacar) {
            RegistrarAnuncio(Gramatica::kFaseDaGramatica, Gramatica::Passo(antes, forma));
        }
    }
}

void Vladimir::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                    const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);

    // O S QUE NASCEU DE UMA DERIVACAO: e ele que herda a conta do pai. E todo
    // S que nasce sai da conta antiga: o objeto do pool pode ter sido um S que
    // o aluno derrubou neste mesmo quadro, e herdaria a idade dele.
    if (stateName == Gramatica::kFaseDaGramatica) {
        for (auto& p : projectiles) {
            auto* s = dynamic_cast<BossProjectile*>(p.get());
            if (!s || s->Posto() != Gramatica::kPostoDaDerivacao) continue;
            mVariaveis.erase(s);
            if (mDerivando) mNascida = s;
        }
        return;
    }
    if (!mFormando) return;

    // O DIAGRAMA E OS ESTADOS SAO O MAPA, e nao tiro: ninguem se machuca neles,
    // e o tiro do aluno os atravessa.
    for (auto& p : projectiles) {
        auto* parte = dynamic_cast<BossProjectile*>(p.get());
        if (!parte) continue;
        parte->DefinirEnfeite(true);
        if (auto* anim = parte->GetComponent<DrawAnimatedComponent>(); anim && !mCaraDoFormando.empty()) {
            anim->SetAnimation(mCaraDoFormando);
        }
    }
}

void Vladimir::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // NA GRAMATICA, o que nasce de uma derivacao sai do S, no rumo dele; e o
    // par de S que o professor atira sai aberto em volta do aluno - o meio do
    // leque e ele, e os dois S passam um de cada lado (ver fases.json).
    if (stateName == Gramatica::kFaseDaGramatica) {
        if (mDerivando) {
            params.firePosition = mDisparoDaDerivacao;
            params.centralAngle = mRumoDaDerivacao;
            return;
        }
        const Vector2 d = GetDirectionToPlayer();
        params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
        return;
    }

    if (stateName != Automato::kFaseDoAutomato && stateName != Parada::kFaseDaParada) return;

    // O MAPA NASCE ONDE FICA.
    if (mFormando) {
        params.firePosition = mDisparoDoFormando;
        return;
    }

    // A LETRA LIDA SAI MIRADA NO ALUNO, com um tiro no meio do leque: num leque
    // par o meio cai entre dois tiros, e quem fica parado ficaria na fresta
    // (ver Ricardo::CustomizeAttackParams). O anel da aceitacao ignora o angulo.
    // Vale igual para os passos do programa da repescagem.
    const Vector2 d = GetDirectionToPlayer();
    const bool par = params.numProjectiles > 1 && params.numProjectiles % 2 == 0;
    const float meioPasso = par ? 0.5f * params.angle / static_cast<float>(params.numProjectiles - 1) : 0.0f;
    params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x)) + meioPasso;
}
