//
// A INF 216, fase 2: a fisica - o jogo de plataforma.
//

#include "Ferreira.h"

#include <algorithm>
#include <cmath>

#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Regras/Nota.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/FaixaDaNota.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "../../Player/PlayerProjectile.h"
#include "BossesProjectiles/BossProjectile.h"
#include "PainelDeCodigo.h"

#include "FerreiraComum.h"

using namespace FerreiraComum;

// ---- Fase 2: a fisica ----

void Ferreira::SoltarAGravidade() {
    for (Player* a : Jogadores()) a->DefinirChao(std::nullopt);
}

float Ferreira::ChaoDoAluno() const {
    for (const Player* a : Jogadores()) {
        if (const auto* d = a->GetComponent<DrawAnimatedComponent>()) return static_cast<float>(d->GetSpriteHeight());
    }
    return 64.0f;
}

void Ferreira::PrepararLado(const int etapa) {
    using namespace Gravidade;
    const Lado lado = mOrdem[static_cast<size_t>(etapa)];
    const Degrau& g = mDegrauDaGravidade;
    switch (DesafioDo(lado)) {
        case Desafio::Pulos:
            mPilhasPorVir = Pulos(mSorteioDaFisica, g);
            break;
        case Desafio::Lancas:
            mLancasPorVir = Lancas(mSorteioDaFisica, g);
            break;
        case Desafio::Chuva:
            mQuedasPorVir = Chuva(mSorteioDaFisica, g, Comprimento(lado, CampoPuro()));
            break;
        case Desafio::Esteira:
            mBuracosPorVir = Buracos(mSorteioDaFisica, g);
            mSentidoDaEsteira = mSorteioDaFisica.Chance(0.5f) ? 1.0f : -1.0f;
            break;
    }
}

void Ferreira::DesenharOChao(const Lado l, const float chao, const float pes, const SDL_Color cor, const bool aviso) {
    using namespace Gravidade;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    const Vector2 a = Ponto(l, c, chao, 0.0f, -pes), b = Ponto(l, c, chao, comprimento, -pes);
    if (aviso) {
        Tracejada(a, b, cor, 12.0f);
        return;
    }
    // A faixa entre os pes do aluno e a borda do campo, a linha de cima, clara, e as juntas - na
    // camada de baixo, sob o placar e os tiros.
    if (!mLinhasDoChao) return;
    const Vector2 borda = Ponto(l, c, chao, comprimento, -chao);
    mLinhasDoChao->Preencher(Vector2(std::min(a.x, borda.x), std::min(a.y, borda.y)),
                             Vector2(std::max(a.x, borda.x), std::max(a.y, borda.y)), kChaoFundo);
    mLinhasDoChao->Acrescentar(a, b, cor);
    for (float u = 0.0f; u <= comprimento; u += 56.0f) {
        mLinhasDoChao->Acrescentar(Ponto(l, c, chao, u, -pes), Ponto(l, c, chao, u, -chao), kChaoJunta);
    }
}

void Ferreira::AtualizarFisica(const float t, const float deltaTime) {
    using namespace Gravidade;
    const Degrau& g = mDegrauDaGravidade;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    if (TrocouDeEtapa(etapa)) PrepararLado(etapa);
    const Lado lado = mOrdem[static_cast<size_t>(etapa)];
    const bool virou = te >= kVirada;
    const JogosDigitais::Campo c = CampoPuro();
    const float chao = ChaoDoAluno(), pes = chao / 2.0f;

    // A gravidade: antes da virada, a do lado anterior - no primeiro, ainda se voa.
    std::optional<Lado> vale;
    if (virou) vale = lado;
    else if (etapa > 0) vale = mOrdem[static_cast<size_t>(etapa - 1)];
    for (Player* a : Jogadores()) a->DefinirChao(vale);

    // O professor fica no ceu do chao que vale; na virada, some e aparece no ceu novo.
    if (virou && mVirada != etapa) {
        mVirada = etapa;
        SetPosition(Ponto(lado, c, chao, Comprimento(lado, c) / 2.0f, Profundidade(lado, c, chao) - 60.0f));
        mClarao = 0.15f;
        mCorDoClarao = SDL_Color{200, 170, 255, 255};
        SomDoDisparo();
    }
    if (vale) {
        const float comprimento = Comprimento(*vale, c);
        const float u = comprimento / 2.0f + (comprimento / 2.0f - 150.0f) * Math::Sin(Math::TwoPi * t / 8.0f);
        mAlvo = Ponto(*vale, c, chao, u, Profundidade(*vale, c, chao) - 60.0f);
        DesenharOChao(*vale, chao, pes, kChaoLinha, false);
    } else {
        Varrer(t, 0.3f, 8.0f);
    }

    // Antes da virada: a seta no meio do campo e o chao novo, piscando.
    if (!virou) {
        if (Aceso(te, 0.12f)) {
            DesenharOChao(lado, chao, pes, kPerigo, true);
            SetaGrande(Vector2(c.x + c.w / 2.0f, c.y + c.h / 2.0f), Plataforma::Direcao(lado), 150.0f, kPerigo);
        }
        return;
    }

    switch (DesafioDo(lado)) {
        case Desafio::Pulos:   AtualizarPulos(te, deltaTime, lado, chao, pes); break;
        case Desafio::Lancas:  AtualizarLancas(te, deltaTime, lado, chao, pes); break;
        case Desafio::Chuva:   AtualizarChuva(te, deltaTime, lado, chao, pes); break;
        case Desafio::Esteira: AtualizarEsteira(te, deltaTime, lado, chao, pes); break;
    }

    // Do ceu, o professor atira de tanto em tanto: o desafio e o chao.
    if (te >= kComecaODesafio + 0.5f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        Leque(kGatilhoDoTiro, g.tirosDoLeque, 30.0f, g.velocidade, "Tiro");
    }
}

void Ferreira::AtualizarPulos(const float te, const float dt, const Lado l, const float chao, const float pes) {
    using namespace Gravidade;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    // As pilhas que chegam: comecam logo fora da ponta de onde vem.
    while (mProximaPilha < mPilhasPorVir.size() && te >= mPilhasPorVir[mProximaPilha].quando) {
        const Pilha& p = mPilhasPorVir[mProximaPilha++];
        PilhaViva v;
        v.sentido = p.sentido;
        v.velocidade = p.velocidade;
        v.altura = p.altura;
        v.largura = p.largura;
        v.u = p.sentido > 0.0f ? -kPeca * static_cast<float>(p.largura) - 10.0f : comprimento + 10.0f;
        const Vector2 andando = Plataforma::AoLongo(l) * (p.sentido * p.velocidade);
        for (int i = 0; i < p.largura; ++i) {
            for (int j = 0; j < p.altura; ++j) {
                const Vector2 onde = Ponto(l, c, chao, v.u + (static_cast<float>(i) + 0.5f) * kPeca, AlturaDaPeca(j, pes));
                v.caixas.push_back(PecaAndando(kGatilhoDaCaixa, onde, "Caixa", andando));
            }
        }
        mPilhas.push_back(std::move(v));
    }
    // Elas andam sozinhas; aqui so se acompanha onde esta a primeira.
    for (PilhaViva& v : mPilhas) {
        v.u += v.sentido * v.velocidade * dt;
        for (const BossProjectile* caixa : v.caixas) {
            if (!caixa) continue;
            v.u = NoChao(l, c, chao, caixa->GetPosition()).x - 0.5f * kPeca;
            break;
        }
    }
    // A que saiu pela outra ponta, sai.
    mPilhas.erase(std::remove_if(mPilhas.begin(), mPilhas.end(), [&](PilhaViva& v) {
        const bool saiu = v.sentido > 0.0f ? v.u > comprimento + 40.0f
                                           : v.u + kPeca * static_cast<float>(v.largura) < -40.0f;
        if (saiu) MatarTodos(v.caixas);
        return saiu;
    }), mPilhas.end());
}

void Ferreira::AtualizarLancas(const float te, const float dt, const Lado l, const float chao, const float pes) {
    using namespace Gravidade;
    (void)pes;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    const Vector2 ao = Plataforma::AoLongo(l);

    // O aviso, na ponta de onde ela vem e na altura dela: vermelha a de baixo, dourada a de cima.
    for (size_t k = mProximaLanca; k < mLancasPorVir.size(); ++k) {
        const Lanca& x = mLancasPorVir[k];
        if (te < x.quando - kAvisoDaLanca) break;
        if (!Aceso(te, 0.08f)) continue;
        const SDL_Color cor = x.h == kLancaBaixa ? kPerigo : kOuro;
        const Vector2 p = Ponto(l, c, chao, x.sentido > 0.0f ? 16.0f : comprimento - 16.0f, x.h);
        Seta(p - ao * (x.sentido * 20.0f), p + ao * (x.sentido * 20.0f), cor);
        Circulo(p, 11.0f, cor, 10);
    }
    while (mProximaLanca < mLancasPorVir.size() && te >= mLancasPorVir[mProximaLanca].quando) {
        const Lanca& x = mLancasPorVir[mProximaLanca++];
        LancaViva v;
        v.sentido = x.sentido;
        v.velocidade = x.velocidade;
        v.h = x.h;
        v.u = x.sentido > 0.0f ? -20.0f : comprimento + 20.0f;
        v.p = PecaAndando(kGatilhoDaLanca, Ponto(l, c, chao, v.u, v.h), "Lanca", ao * (x.sentido * x.velocidade));
        if (!v.p) continue;
        // O desenho aponta para a direita; o ator gira ao contrario do SDL (DrawAnimatedComponent).
        const Vector2 d = ao * x.sentido;
        v.p->SetRotation(-Math::Atan2(d.y, d.x));
        mLancas.push_back(v);
        SomDoDisparo();
    }
    for (LancaViva& v : mLancas) {
        v.u += v.sentido * v.velocidade * dt;
        if (v.p) v.u = NoChao(l, c, chao, v.p->GetPosition()).x;
    }
    mLancas.erase(std::remove_if(mLancas.begin(), mLancas.end(), [&](LancaViva& v) {
        const bool fora = !v.p || v.u < -60.0f || v.u > comprimento + 60.0f;
        if (fora) Matar(v.p);
        return fora;
    }), mLancas.end());
}

void Ferreira::AtualizarChuva(const float te, const float dt, const Lado l, const float chao, const float pes) {
    using namespace Gravidade;
    const Degrau& g = mDegrauDaGravidade;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    const float ceu = Profundidade(l, c, chao) + 30.0f;
    const float noChao = AlturaDaPeca(0, pes);
    const float giro = -Math::ToRadians(Plataforma::Giro(l));
    Battle* b = Batalha();
    const std::vector<Player*> alunos = Jogadores();

    // A moeda tem a classe do aluno: passa por ele e paga a quem encosta.
    for (Player* a : alunos) a->DefinirClasse(kClasseDaMoedaDaChuva);
    while (mProximaDaChuva < mQuedasPorVir.size() && te >= mQuedasPorVir[mProximaDaChuva].quando) {
        const Queda& q = mQuedasPorVir[mProximaDaChuva++];
        QuedaViva v;
        v.u = q.u;
        v.h = ceu;
        v.moeda = q.moeda;
        const Vector2 caindo = Plataforma::Direcao(l) * g.queda;
        v.p = q.moeda ? PecaAndando(kGatilhoDaMoeda, Ponto(l, c, chao, v.u, v.h), "Moeda", caindo)
                      : PecaAndando(kGatilhoDoBloco, Ponto(l, c, chao, v.u, v.h), "Bloco", caindo);
        if (!v.p) continue;
        if (q.moeda) v.p->DefinirClasse(kClasseDaMoedaDaChuva);
        else v.p->SetRotation(giro);
        mQuedas.push_back(v);
    }
    for (QuedaViva& v : mQuedas) {
        if (!v.p) continue;
        v.h = NoChao(l, c, chao, v.p->GetPosition()).y;
        // A sombra no chao, embaixo de cada um.
        const SDL_Color cor = v.moeda ? kOuro : kPerigo;
        Linha(Ponto(l, c, chao, v.u - 14.0f, -pes + 3.0f), Ponto(l, c, chao, v.u + 14.0f, -pes + 3.0f), cor);
        Linha(Ponto(l, c, chao, v.u - 8.0f, -pes + 6.0f), Ponto(l, c, chao, v.u + 8.0f, -pes + 6.0f), cor);
        if (v.h <= noChao) {
            // No chao, o bloco estilhaca em duas fagulhas, uma para cada lado; a moeda que ninguem pegou some.
            if (!v.moeda) {
                for (const float s : {-1.0f, 1.0f}) {
                    FagulhaViva f;
                    f.u = v.u;
                    f.sentido = s;
                    f.p = PecaAndando(kGatilhoDaFagulha, Ponto(l, c, chao, f.u, noChao), "Fagulha",
                                      Plataforma::AoLongo(l) * (s * g.fagulha));
                    if (f.p) mFagulhas.push_back(f);
                }
                SomDoDisparo();
            }
            Matar(v.p);
            continue;
        }
        if (!v.moeda) continue;
        for (const Player* a : alunos) {
            if ((v.p->GetPosition() - a->GetPosition()).Length() > RaioDe(a) + 26.0f) continue;
            if (b) {
                b->Faiscar(v.p->GetPosition(), Hud::Faisca::Coleta);
                b->Conquistar(kPremioDaMoedaDaChuva);
            }
            Matar(v.p);
            break;
        }
    }
    mQuedas.erase(std::remove_if(mQuedas.begin(), mQuedas.end(), [](const QuedaViva& v) { return !v.p; }), mQuedas.end());
    for (FagulhaViva& f : mFagulhas) {
        f.u += f.sentido * g.fagulha * dt;
        if (f.p) f.u = NoChao(l, c, chao, f.p->GetPosition()).x;
    }
    mFagulhas.erase(std::remove_if(mFagulhas.begin(), mFagulhas.end(), [&](FagulhaViva& f) {
        const bool fora = !f.p || f.u < -40.0f || f.u > comprimento + 40.0f;
        if (fora) Matar(f.p);
        return fora;
    }), mFagulhas.end());
}

void Ferreira::AtualizarEsteira(const float te, const float dt, const Lado l, const float chao, const float pes) {
    using namespace Gravidade;
    const Degrau& g = mDegrauDaGravidade;
    const JogosDigitais::Campo c = CampoPuro();
    const float comprimento = Comprimento(l, c);
    const Vector2 ao = Plataforma::AoLongo(l);
    const float s = mSentidoDaEsteira;
    const float giro = -Math::ToRadians(Plataforma::Giro(l));
    const float noChao = AlturaDaPeca(0, pes);
    const float moedor = s > 0.0f ? comprimento - kRecuoDoMoedor : kRecuoDoMoedor;

    // O moedor, na ponta de chegada: uma coluna de caixas mais alta que o pulo, tremendo.
    if (mMoedor.empty()) {
        for (int j = 0; j < kPecasDoMoedor; ++j) mMoedor.push_back(Peca(kGatilhoDaCaixa, Ponto(l, c, chao, moedor, AlturaDaPeca(j, pes)), "Caixa"));
    }
    for (size_t j = 0; j < mMoedor.size(); ++j) {
        if (!mMoedor[j]) continue;
        const float tremor = 3.0f * Math::Sin(mRelogio * 40.0f + static_cast<float>(j));
        mMoedor[j]->SetPosition(Ponto(l, c, chao, moedor + tremor, AlturaDaPeca(static_cast<int>(j), pes)));
        Parar(mMoedor[j]);
    }

    // A esteira leva quem esta no chao; no ar, nao.
    for (Player* a : Jogadores()) {
        if (a->NoChao()) a->SetPosition(a->GetPosition() + ao * (s * g.esteira * dt));
    }

    // Os buracos: fileiras de espinhos que vem com o chao, da ponta de saida.
    while (mProximoBuraco < mBuracosPorVir.size() && te >= mBuracosPorVir[mProximoBuraco].quando) {
        const Buraco& x = mBuracosPorVir[mProximoBuraco++];
        BuracoVivo v;
        v.u = s > 0.0f ? -kPeca * static_cast<float>(x.espinhos) : comprimento;
        for (int k = 0; k < x.espinhos; ++k) {
            BossProjectile* p = PecaAndando(kGatilhoDoEspinho, Ponto(l, c, chao, v.u + (static_cast<float>(k) + 0.5f) * kPeca, noChao),
                                            "Espinho", ao * (s * g.esteira));
            if (p) p->SetRotation(giro);
            v.espinhos.push_back(p);
        }
        mBuracos.push_back(std::move(v));
    }
    // Os espinhos andam com a esteira; aqui so se acompanha onde esta o primeiro.
    for (BuracoVivo& v : mBuracos) {
        v.u += s * g.esteira * dt;
        for (size_t k = 0; k < v.espinhos.size(); ++k) {
            if (!v.espinhos[k]) continue;
            v.u = NoChao(l, c, chao, v.espinhos[k]->GetPosition()).x - (static_cast<float>(k) + 0.5f) * kPeca;
            break;
        }
    }
    // O buraco que chegou ao moedor e moido.
    mBuracos.erase(std::remove_if(mBuracos.begin(), mBuracos.end(), [&](BuracoVivo& v) {
        const float fim = v.u + kPeca * static_cast<float>(v.espinhos.size());
        const bool moido = s > 0.0f ? fim > moedor - kPeca / 2.0f : v.u < moedor + kPeca / 2.0f;
        if (moido) MatarTodos(v.espinhos);
        return moido;
    }), mBuracos.end());

    // As setas da esteira, andando com ela, dentro do chao.
    constexpr float kPasso = 64.0f;
    const float fase = std::fmod(mRelogio * g.esteira, kPasso);
    for (float u = -kPasso; u <= comprimento + kPasso; u += kPasso) {
        const float aqui = u + s * fase;
        if (aqui < 8.0f || aqui > comprimento - 8.0f) continue;
        const Vector2 p = Ponto(l, c, chao, aqui, -pes - (chao - pes) / 2.0f);
        if (!mLinhasDoChao) continue;
        const Vector2 ponta = p + ao * (s * 10.0f), lado(-ao.y, ao.x);
        mLinhasDoChao->Acrescentar(p - ao * (s * 10.0f), ponta, kChaoLinha);
        mLinhasDoChao->Acrescentar(ponta, ponta - ao * (s * 7.0f) + lado * 5.0f, kChaoLinha);
        mLinhasDoChao->Acrescentar(ponta, ponta - ao * (s * 7.0f) - lado * 5.0f, kChaoLinha);
    }
}
