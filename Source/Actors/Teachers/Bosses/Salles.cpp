//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"

#include <algorithm>
#include <limits>

#include "../../../Game.h"
#include "../../../Random.h"
#include "../../../Attacks/Ancora.h"
#include "../../../Actors/Player/Player.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"


Salles::Salles(Scene *scene) : Boss(scene)
{

}

void Salles::OnUpdate(float deltaTime) {
    Boss::OnUpdate(deltaTime);
    AtualizarCapivara(deltaTime);
    AtualizarPilha(deltaTime);
    AtualizarHash(deltaTime);
    AtualizarArvore(deltaTime);
}

void Salles::DispararDaArvore(const char* gatilho, const Vector2 onde, const bool mirar) {
    mDisparandoDaArvore = true;
    mMirandoDaArvore = mirar;
    mDisparoDaArvore = onde;
    mCriadaNaArvore = nullptr;
    Disparar(gatilho);
    mDisparandoDaArvore = false;
    mMirandoDaArvore = false;
}

void Salles::AtualizarArestas() {
    // A ARESTA DE QUEM SAIU SOME; a de quem entrou (e tem pai) nasce.
    for (auto it = mArestas.begin(); it != mArestas.end();) {
        if (mNosDaArvore.count(it->first) && mNosDaArvore.count(it->first / 2)) { ++it; continue; }
        for (BossProjectile* p : it->second) if (p->GetState() == ActorState::Active) p->Kill();
        it = mArestas.erase(it);
    }
    for (const auto& [i, chave] : mNosDaArvore) {
        if (i == 1 || mArestas.count(i) || !mNosDaArvore.count(i / 2)) continue;
        std::vector<BossProjectile*> pontos;
        for (int k = 1; k <= Arvore::kPontosPorAresta; ++k) {
            const float t = static_cast<float>(k) / static_cast<float>(Arvore::kPontosPorAresta + 1);
            const Vector2 pai(Arvore::X(i / 2), Arvore::Y(i / 2));
            const Vector2 filho(Arvore::X(i), Arvore::Y(i));
            DispararDaArvore(Arvore::kGatilhoDaAresta, pai + (filho - pai) * t, false);
            if (!mCriadaNaArvore) continue;
            mCriadaNaArvore->DefinirEnfeite(true);
            pontos.push_back(mCriadaNaArvore);
        }
        mArestas[i] = pontos;
    }
}

void Salles::DerrubarSubarvore(const int indice) {
    for (const int i : Arvore::Subarvore(mNosDaArvore, indice)) {
        const auto it = mProjeteisDaArvore.find(i);
        if (it != mProjeteisDaArvore.end()) {
            BossProjectile* p = it->second;
            if (p->GetState() == ActorState::Active) {
                p->DefinirPosto("");
                if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, 280.0f));
            }
            mProjeteisDaArvore.erase(it);
        }
        mNosDaArvore.erase(i);
    }
}

void Salles::AtualizarArvore(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naArvore = fsm && fsm->GetStateName() == Arvore::kFaseDaArvore && !EhClone();

    if (!naArvore) {
        // A FASE ACABOU: a arvore cai inteira, e a chave que descia tambem.
        if (mNaArvore) {
            DerrubarSubarvore(1);
            for (auto& [i, p] : mProjeteisDaArvore) {
                if (p->GetState() == ActorState::Active) {
                    p->DefinirPosto("");
                    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, 280.0f));
                }
            }
            mProjeteisDaArvore.clear();
            mNosDaArvore.clear();
            AtualizarArestas();
            if (mDescida.chave && mDescida.chave->GetState() == ActorState::Active) mDescida.chave->Kill();
            mDescida = Descida{};
            if (!mAnuncioDaArvore.empty()) RegistrarAnuncio(Arvore::kFaseDaArvore, mAnuncioDaArvore);
            mNaArvore = false;
        }
        return;
    }
    if (!mNaArvore) {
        mNaArvore = true;
        mNosDaArvore.clear();
        mProjeteisDaArvore.clear();
        mDescida = Descida{};
        mProximaOperacaoDaArvore = Arvore::kPrimeira;
        if (mAnuncioDaArvore.empty()) mAnuncioDaArvore = AnuncioDaFase(Arvore::kFaseDaArvore);
    }
    const float agora = fsm->GetStateTime();

    // O NO DERRUBADO A TIRO LEVA A SUBARVORE JUNTO. Visto do mais alto para o
    // mais fundo: derrubar um pai ja derruba os filhos que estavam na fila.
    std::vector<int> derrubados;
    for (const auto& [i, p] : mProjeteisDaArvore) {
        if (p->GetState() != ActorState::Active || p->Posto() != Arvore::kPostoDaArvore) derrubados.push_back(i);
    }
    for (const int i : derrubados) if (mNosDaArvore.count(i)) DerrubarSubarvore(i);

    // A DESCIDA EM CURSO: um no por vez - o no visitado atira no aluno, e o
    // placar diz a comparacao. No fim, a chave entra como folha, ou cai.
    if (mDescida.ativa && mDescida.chave && mDescida.chave->GetState() != ActorState::Active) {
        mDescida.ativa = false;                       // o aluno derrubou a chave que descia
        mProximaOperacaoDaArvore = agora + Arvore::kEntreOperacoes;
    }
    if (mDescida.ativa && agora >= mDescida.proximo) {
        mDescida.proximo += Arvore::kEmCadaNo;
        const int k = mDescida.op.chave;
        if (mDescida.passo + 1 < mDescida.caminho.size()) {
            const int i = mDescida.caminho[mDescida.passo++];
            // O no do caminho pode ter caido no meio da descida, a tiro: ai nao
            // ha quem atire nem com quem comparar.
            if (const auto no = mNosDaArvore.find(i); no != mNosDaArvore.end()) {
                DispararDaArvore(Arvore::kGatilhoDaVisita, Vector2(Arvore::X(i), Arvore::Y(i)), true);
                RegistrarAnuncio(Arvore::kFaseDaArvore, Arvore::PlacarDaComparacao(k, no->second));
            }
        } else {
            const int fim = mDescida.caminho.back();
            // CABE se esta dentro da profundidade E o pai ainda esta la: uma
            // subarvore que caiu no meio da descida deixa o lugar sem pai.
            const bool cabe = Arvore::Cabe(fim) && (fim == 1 || mNosDaArvore.count(fim / 2) == 1);
            const auto achado = mNosDaArvore.find(fim);
            const bool achou = achado != mNosDaArvore.end() && achado->second == k;
            BossProjectile* chave = mDescida.chave;
            if (mDescida.op.tipo == Arvore::Tipo::Inserir && cabe && !mNosDaArvore.count(fim)) {
                // ENTRA: a chave vira a folha daquele lugar.
                mNosDaArvore[fim] = k;
                mProjeteisDaArvore[fim] = chave;
                chave->DefinirPosto(Arvore::kPostoDaArvore);
            } else if (mDescida.op.tipo == Arvore::Tipo::Inserir) {
                // ALTURA DEMAIS: a chave cai da arvore, em cima do aluno.
                chave->DefinirPosto("");
                Vector2 d(0.0f, 1.0f);
                if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
                    d = batalha->Alvo()->GetPosition() - chave->GetPosition();
                    if (d.LengthSq() > 0.0f) d.Normalize();
                }
                if (auto* rb = chave->GetComponent<RigidBodyComponent>()) rb->SetVelocity(d * kVelocidadeDaQueda);
            } else {
                // A BUSCA: achou, o no achado estoura; nao achou, o NULL cai do
                // lugar vazio. A chave que buscava some.
                if (achou) DispararDaArvore(Arvore::kGatilhoDoAchado, Vector2(Arvore::X(fim), Arvore::Y(fim)), false);
                else DispararDaArvore(Arvore::kGatilhoDoNulo, chave->GetPosition(), true);
                chave->Kill();
            }
            RegistrarAnuncio(Arvore::kFaseDaArvore, Arvore::PlacarDoFim(mDescida.op, achou, cabe));
            mDescida.ativa = false;
            mProximaOperacaoDaArvore = agora + Arvore::kEntreOperacoes;
        }
    }

    // UMA OPERACAO NOVA: a chave sai do professor e comeca a descer.
    if (!mDescida.ativa && agora >= mProximaOperacaoDaArvore) {
        const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
        const Arvore::Operacao op = Arvore::Proxima(mNosDaArvore, sorteio);
        DispararDaArvore(Arvore::kGatilhoDaChave, GetPosition(), false);
        if (mCriadaNaArvore) {
            if (auto* anim = mCriadaNaArvore->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Hash::CaraDaChave(op.chave));
            // A CHAVE QUE BUSCA NAO MACHUCA: e a pergunta, e nao um tiro.
            mCriadaNaArvore->DefinirEnfeite(op.tipo == Arvore::Tipo::Buscar);
            mDescida = Descida{};
            mDescida.ativa = true;
            mDescida.op = op;
            mDescida.caminho = Arvore::Caminho(mNosDaArvore, op.chave);
            mDescida.proximo = agora + Arvore::kEmCadaNo;
            mDescida.chave = mCriadaNaArvore;
            RegistrarAnuncio(Arvore::kFaseDaArvore, Arvore::PlacarDaOperacao(op));
        }
    }

    AtualizarArestas();

    // CADA NO NO LUGAR DELE, e a chave que desce em cima do no da vez.
    for (const auto& [i, p] : mProjeteisDaArvore) {
        p->SetPosition(Ancora::Aproximar(p->GetPosition(), Vector2(Arvore::X(i), Arvore::Y(i)), kRapidezDaChave, deltaTime));
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
    if (mDescida.ativa && mDescida.chave) {
        const int i = mDescida.caminho[std::min(mDescida.passo, mDescida.caminho.size() - 1)];
        const bool ultimo = mDescida.passo + 1 >= mDescida.caminho.size();
        const Vector2 lugar(Arvore::X(i), Arvore::Y(i) - (ultimo ? 0.0f : kAcimaDoNo));
        mDescida.chave->SetPosition(Ancora::Aproximar(mDescida.chave->GetPosition(), lugar, kRapidezDaDescida, deltaTime));
        if (auto* rb = mDescida.chave->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
}

void Salles::DispararDoHash(const char* gatilho, const Vector2 onde, const bool mirar) {
    mDisparandoDoHash = true;
    mMirandoDoHash = mirar;
    mDisparoDoHash = onde;
    mCriadaNoHash = nullptr;
    Disparar(gatilho);
    mDisparandoDoHash = false;
    mMirandoDoHash = false;
}

void Salles::FormarBaldes(const int m) {
    for (BossProjectile* b : mBaldes) if (b->GetState() == ActorState::Active) b->Kill();
    mBaldes.clear();
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    for (int i = 0; i < m; ++i) {
        DispararDoHash(Hash::kGatilhoDosBaldes, Vector2(Hash::CentroDoBalde(i, m, largura), Hash::kYDosBaldes));
        if (!mCriadaNoHash) continue;
        mCriadaNoHash->DefinirEnfeite(true);
        if (auto* anim = mCriadaNoHash->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Hash::CaraDoBalde(i));
        mBaldes.push_back(mCriadaNoHash);
    }
}

void Salles::RefazerListas() {
    std::vector<int> naLista(static_cast<size_t>(mTamanho), 0);
    for (Chave& c : mChaves) {
        c.balde = Hash::Balde(c.valor, mTamanho);
        c.posicao = naLista[static_cast<size_t>(c.balde)]++;
    }
}

void Salles::AtualizarHash(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noHash = fsm && fsm->GetStateName() == Hash::kFaseDoHash && !EhClone();

    if (!noHash) {
        // A FASE ACABOU: a tabela cai inteira, e os baldes somem.
        if (mNoHash) {
            for (const Chave& c : mChaves) {
                c.p->DefinirPosto("");
                if (auto* rb = c.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, 280.0f));
            }
            mChaves.clear();
            for (BossProjectile* b : mBaldes) if (b->GetState() == ActorState::Active) b->Kill();
            mBaldes.clear();
            mBusca = Busca{};
            if (!mAnuncioDoHash.empty()) RegistrarAnuncio(Hash::kFaseDoHash, mAnuncioDoHash);
            mAlvo.reset();
            mNoHash = false;
        }
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());

    if (!mNoHash) {
        mNoHash = true;
        mChaves.clear();
        mBaldes.clear();
        mBusca = Busca{};
        mTamanho = Hash::kTamanhos[0];
        mRehashPendente = false;
        mProximaOperacaoDoHash = Hash::kPrimeira;
        if (mAnuncioDoHash.empty()) mAnuncioDoHash = AnuncioDaFase(Hash::kFaseDoHash);
        mAlvo = Vector2(largura / 2.0f, kAlturaNoHash);
        FormarBaldes(mTamanho);
    }
    const float agora = fsm->GetStateTime();

    // QUEM SAIU DA TABELA SAI DA CONTA - a chave que o aluno derrubou -, e as
    // listas se refazem.
    const auto antes = mChaves.size();
    mChaves.erase(std::remove_if(mChaves.begin(), mChaves.end(), [](const Chave& c) {
        return c.p->GetState() != ActorState::Active || c.p->Posto() != Hash::kPostoDasChaves;
    }), mChaves.end());
    if (mChaves.size() != antes) RefazerListas();

    // A BUSCA EM CURSO: um no por vez atira para baixo; no fim, o achado sai
    // atirado no aluno, ou o NULL cai pela coluna.
    if (mBusca.ativa && agora >= mBusca.proximo) {
        mBusca.proximo += Hash::kEntreNos;
        if (mBusca.lidos < mBusca.nos.size()) {
            BossProjectile* no = mBusca.nos[mBusca.lidos++];
            if (no->GetState() == ActorState::Active) DispararDoHash(Hash::kGatilhoDoPercurso, no->GetPosition(), true);
        } else {
            if (mBusca.achou && !mBusca.nos.empty() && mBusca.nos.back()->GetState() == ActorState::Active) {
                BossProjectile* achado = mBusca.nos.back();
                achado->DefinirPosto("");
                Vector2 d(0.0f, 1.0f);
                if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
                    d = batalha->Alvo()->GetPosition() - achado->GetPosition();
                    if (d.LengthSq() > 0.0f) d.Normalize();
                }
                if (auto* rb = achado->GetComponent<RigidBodyComponent>()) rb->SetVelocity(d * kVelocidadeDoAchado);
            } else if (!mBusca.achou) {
                DispararDoHash(Hash::kGatilhoDoNulo, mBusca.fim);
            }
            mBusca.ativa = false;
        }
    }

    // A OPERACAO - menos durante uma busca, que le a lista ate o fim.
    if (agora >= mProximaOperacaoDoHash && !mBusca.ativa) {
        mProximaOperacaoDoHash += Hash::kPasso;

        if (mRehashPendente) {
            // O REHASH: a tabela cresce, e toda chave muda de balde.
            const int novo = Hash::ProximoTamanho(mTamanho);
            RegistrarAnuncio(Hash::kFaseDoHash, Hash::PlacarDoRehash(mTamanho, novo));
            mTamanho = novo;
            FormarBaldes(mTamanho);
            RefazerListas();
            mRehashPendente = false;
            mAlvo = Vector2(largura / 2.0f, kAlturaNoHash);
        } else {
            std::vector<int> naTabela;
            for (const Chave& c : mChaves) naTabela.push_back(c.valor);
            const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
            const Hash::Operacao op = Hash::Proxima(naTabela, sorteio);
            const int balde = Hash::Balde(op.chave, mTamanho);
            mAlvo = Vector2(Hash::CentroDoBalde(balde, mTamanho, largura), kAlturaNoHash);

            if (op.tipo == Hash::Tipo::Inserir) {
                DispararDoHash(Hash::kGatilhoDaChave, GetPosition());
                if (mCriadaNoHash) {
                    if (auto* anim = mCriadaNoHash->GetComponent<DrawAnimatedComponent>()) {
                        anim->SetAnimation(Hash::CaraDaChave(op.chave));
                    }
                    mChaves.push_back(Chave{mCriadaNoHash, op.chave, 0, 0});
                    RefazerListas();
                }
                RegistrarAnuncio(Hash::kFaseDoHash, Hash::PlacarDaInsercao(op.chave, mTamanho));
                mRehashPendente = Hash::PrecisaDeRehash(static_cast<int>(mChaves.size()), mTamanho);
            } else {
                // A BUSCA LE A LISTA DO BALDE do comeco ate a chave - ou ate o fim.
                std::vector<const Chave*> lista;
                for (const Chave& c : mChaves) if (c.balde == balde) lista.push_back(&c);
                std::sort(lista.begin(), lista.end(), [](const Chave* a, const Chave* b) { return a->posicao < b->posicao; });
                mBusca = Busca{};
                mBusca.ativa = true;
                mBusca.chave = op.chave;
                mBusca.proximo = agora + Hash::kEntreNos;
                for (const Chave* c : lista) {
                    mBusca.nos.push_back(c->p);
                    if (c->valor == op.chave) { mBusca.achou = true; break; }
                }
                mBusca.fim = Vector2(Hash::CentroDoBalde(balde, mTamanho, largura),
                                     Hash::AlturaNaLista(static_cast<int>(lista.size())));
                RegistrarAnuncio(Hash::kFaseDoHash, Hash::PlacarDaBusca(op.chave, mBusca.achou));
            }
        }
    }

    // CADA CHAVE NO LUGAR DELA, pendurada no balde: a que acabou de entrar sai
    // do professor, e no rehash todas atravessam para a lista nova.
    for (const Chave& c : mChaves) {
        const Vector2 lugar(Hash::CentroDoBalde(c.balde, mTamanho, largura), Hash::AlturaNaLista(c.posicao));
        c.p->SetPosition(Ancora::Aproximar(c.p->GetPosition(), lugar, kRapidezDaChave, deltaTime));
        if (auto* rb = c.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
}

std::optional<Vector2> Salles::AlvoDoMovimento() const {
    return mAlvo;
}

std::vector<BossProjectile*> Salles::NoPosto(const char* posto) const {
    std::vector<BossProjectile*> achados;
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return achados;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == posto) achados.push_back(p.get());
    }
    return achados;
}

void Salles::DerrubarPilha() {
    // A PILHA CAI ABERTA EM LEQUE, cada bloco num angulo: a do topo no meio, e
    // as outras para os lados - uma chuva, e nao uma coluna so.
    const int n = static_cast<int>(mPilha.size());
    for (int k = 0; k < n; ++k) {
        BossProjectile* b = mPilha[static_cast<size_t>(k)];
        b->DefinirPosto("");
        const float abertura = n > 1 ? (static_cast<float>(k) / static_cast<float>(n - 1) - 0.5f) * 50.0f : 0.0f;
        const float a = Math::ToRadians(90.0f + abertura);
        if (auto* rb = b->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(Math::Cos(a), Math::Sin(a)) * 260.0f);
    }
    mPilha.clear();
    mValores.clear();
}

void Salles::AtualizarPilha(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naPilha = fsm && fsm->GetStateName() == Pilha::kFaseDaPilha && !EhClone();

    // OS BLOCOS QUE SAIRAM NUM POP ESTOURAM na hora deles, dentro ou fora da
    // fase: um pop do fim da fase ainda esta voando.
    for (auto& p : mPopados) {
        if (p.bloco->GetState() != ActorState::Active) { p.idade = -1.0f; continue; }
        p.idade += deltaTime;
        if (p.idade < Pilha::kEstouraEm) continue;
        if (p.valor > 0) {
            mEstourando = true;
            mDisparoDoEstouro = p.bloco->GetPosition();
            mLascas = p.valor;
            Disparar(Pilha::kGatilhoDoEstouro);
            mEstourando = false;
        }
        p.bloco->Kill();
        p.idade = -1.0f;
    }
    mPopados.erase(std::remove_if(mPopados.begin(), mPopados.end(), [](const Popado& p) { return p.idade < 0.0f; }),
                   mPopados.end());

    if (!naPilha) {
        // A FASE ACABOU: a pilha cai, e o professor deixa de ter coluna.
        if (mNaPilha) {
            DerrubarPilha();
            if (!mAnuncioDaPilha.empty()) RegistrarAnuncio(Pilha::kFaseDaPilha, mAnuncioDaPilha);
            mAlvo.reset();
            mNaPilha = false;
        }
        return;
    }
    if (!mNaPilha) {
        mNaPilha = true;
        mPilha.clear();
        mValores.clear();
        mProximaOperacao = Pilha::kPrimeira;
        mProximaMudanca = kParadoNaColuna;
        mColuna = 1;
        mAlvo = Vector2(kColunasDaPilha[1], kAlturaDaPilha);
        if (mAnuncioDaPilha.empty()) mAnuncioDaPilha = AnuncioDaFase(Pilha::kFaseDaPilha);
    }
    const float agora = fsm->GetStateTime();

    // QUEM SAIU DA PILHA SAI DA CONTA: o bloco que o aluno derrubou a tiro.
    for (size_t k = 0; k < mPilha.size();) {
        if (mPilha[k]->GetState() != ActorState::Active || mPilha[k]->Posto() != Pilha::kPostoDaPilha) {
            mPilha.erase(mPilha.begin() + static_cast<std::ptrdiff_t>(k));
            mValores.erase(mValores.begin() + static_cast<std::ptrdiff_t>(k));
        } else {
            ++k;
        }
    }

    // DE COLUNA EM COLUNA, parando em cada uma: parado, ele leva tiro; e
    // andando, a pilha pendurada varre o campo.
    if (agora >= mProximaMudanca) {
        mProximaMudanca += kParadoNaColuna;
        const int salto = Random::GetIntRange(1, 2);
        mColuna = (mColuna + salto) % static_cast<int>(kColunasDaPilha.size());
        mAlvo = Vector2(kColunasDaPilha[static_cast<size_t>(mColuna)], kAlturaDaPilha);
    }

    // A OPERACAO DO PROGRAMA.
    if (agora >= mProximaOperacao) {
        mProximaOperacao += Pilha::kPasso;
        const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
        const Pilha::Operacao op = Pilha::Proxima(static_cast<int>(mPilha.size()), sorteio);
        const int topo = mValores.empty() ? 0 : mValores.back();

        if (op.tipo == Pilha::Tipo::Push) {
            mEmpurrando = true;
            mEmpurrado = nullptr;
            Disparar(Pilha::kGatilhoDoPush);
            mEmpurrando = false;
            if (mEmpurrado) {
                if (auto* anim = mEmpurrado->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Pilha::CaraDoBloco(op.valor));
                mPilha.push_back(mEmpurrado);
                mValores.push_back(op.valor);
            }
        } else if (op.tipo == Pilha::Tipo::Pop && !mPilha.empty()) {
            // O TOPO SAI MIRADO NO ALUNO, e estoura no caminho.
            BossProjectile* b = mPilha.back();
            b->DefinirPosto("");
            Vector2 d(0.0f, 1.0f);
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
                d = batalha->Alvo()->GetPosition() - b->GetPosition();
                if (d.LengthSq() > 0.0f) d.Normalize();
            }
            if (auto* rb = b->GetComponent<RigidBodyComponent>()) rb->SetVelocity(d * Pilha::kVelocidadeDoPop);
            mPopados.push_back(Popado{b, 0.0f, mValores.back()});
            mPilha.pop_back();
            mValores.pop_back();
        } else if (op.tipo == Pilha::Tipo::Overflow) {
            DerrubarPilha();
        }
        RegistrarAnuncio(Pilha::kFaseDaPilha, Pilha::Placar(op, topo));
    }

    // CADA BLOCO NO LUGAR DELE, embaixo do professor: o 0 colado nele, o topo
    // la embaixo. Parados - sao armadura, e nao tiro.
    for (size_t k = 0; k < mPilha.size(); ++k) {
        BossProjectile* b = mPilha[k];
        const Vector2 lugar = GetPosition() + Vector2(0.0f, Pilha::AbaixoDoProfessor(static_cast<int>(k)));
        b->SetPosition(Ancora::Aproximar(b->GetPosition(), lugar, kRapidezDoBloco, deltaTime));
        if (auto* rb = b->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
}

void Salles::AtualizarCapivara(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm || EhClone()) return;
    const std::string& fase = fsm->GetStateName();
    if (fase.empty()) return;

    // O AVISO NO PLACAR SAI SOZINHO, no tempo dele - na fase em que foi dado.
    if (mAvisoDaCapivara > 0.0f) {
        mAvisoDaCapivara -= deltaTime;
        if (mAvisoDaCapivara <= 0.0f) RegistrarAnuncio(mFaseDaCapivara, mAnuncioAntesDaCapivara);
    }

    // UMA FASE NOVA, UM SORTEIO NOVO: se ela vem nesta, quando e de que lado.
    if (fase != mFaseDaCapivara) {
        if (mAvisoDaCapivara > 0.0f) RegistrarAnuncio(mFaseDaCapivara, mAnuncioAntesDaCapivara);
        mAvisoDaCapivara = 0.0f;
        mFaseDaCapivara = fase;
        const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
        mVisita = Capivara::Sortear(sorteio);
        mVeio = false;
    }
    if (!mVisita.vem || mVeio || fsm->GetStateTime() < mVisita.quando) return;

    // ELA APARECE, e o placar avisa.
    mVeio = true;
    mDisparandoCapivara = true;
    Disparar(Capivara::kGatilho);
    mDisparandoCapivara = false;
    mAnuncioAntesDaCapivara = AnuncioDaFase(fase);
    RegistrarAnuncio(fase, Capivara::kAnuncio);
    mAvisoDaCapivara = Capivara::kAnuncioNaTela;
}

void Salles::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                  const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);

    // O QUE A ARVORE CRIOU: a chave que vai descer.
    if (mDisparandoDaArvore) {
        if (!projectiles.empty() && !mCriadaNaArvore) mCriadaNaArvore = dynamic_cast<BossProjectile*>(projectiles.front().get());
        return;
    }

    // O QUE O HASH CRIOU: o balde ou a chave que o Salles acabou de pedir.
    if (mDisparandoDoHash) {
        if (!projectiles.empty()) mCriadaNoHash = dynamic_cast<BossProjectile*>(projectiles.front().get());
        return;
    }

    // O BLOCO QUE O PUSH CRIOU: e ele que entra na pilha.
    if (mEmpurrando) {
        for (auto& p : projectiles) {
            if (auto* b = dynamic_cast<BossProjectile*>(p.get()); b && b->Posto() == Pilha::kPostoDaPilha) mEmpurrado = b;
        }
        return;
    }
    if (!mDisparandoCapivara) return;

    // ELA ANDA OLHANDO PARA ONDE VAI.
    for (auto& p : projectiles) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) {
            anim->SetAnimation(mVisita.daEsquerda ? Capivara::kAnimacaoDireita : Capivara::kAnimacaoEsquerda);
        }
    }
}

void Salles::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // A CAPIVARA ENTRA POR UMA BORDA E ATRAVESSA ATE A OUTRA, reta, na altura
    // dela - e nao sai do professor nem no angulo da fase.
    if (mDisparandoCapivara) {
        const float largura = (mScene && mScene->GetGame())
                                  ? static_cast<float>(mScene->GetGame()->GetWindowWidth()) : 1200.0f;
        params.firePosition = Vector2(mVisita.daEsquerda ? -Capivara::kForaDaTela : largura + Capivara::kForaDaTela,
                                      Capivara::kAltura);
        params.centralAngle = mVisita.daEsquerda ? 0.0f : 180.0f;
        return;
    }

    // NA TABELA HASH, cada coisa nasce onde o Salles diz: o balde no lugar
    // dele, a chave no professor, o tiro do percurso no no lido, o NULL no fim
    // da lista. Os angulos sao os do arquivo (para baixo).
    if (stateName == Hash::kFaseDoHash) {
        if (!mDisparandoDoHash) return;
        params.firePosition = mDisparoDoHash;
        // O NO LIDO ATIRA NO ALUNO, com um tiro no meio do leque.
        if (mMirandoDoHash) {
            Vector2 d(0.0f, 1.0f);
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
                d = batalha->Alvo()->GetPosition() - mDisparoDoHash;
                if (d.LengthSq() > 0.0f) d.Normalize();
            }
            params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
        }
        return;
    }

    // NA ARVORE, o que sai de um no sai dele - mirado no aluno, quando e tiro.
    if (stateName == Arvore::kFaseDaArvore) {
        if (!mDisparandoDaArvore) return;
        params.firePosition = mDisparoDaArvore;
        if (mMirandoDaArvore) {
            Vector2 d(0.0f, 1.0f);
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
                d = batalha->Alvo()->GetPosition() - mDisparoDaArvore;
                if (d.LengthSq() > 0.0f) d.Normalize();
            }
            params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
        }
        return;
    }

    // O POP ESTOURA ONDE O BLOCO ESTA, em tantas lascas quanto o numero dele.
    // O total e escrito inteiro a cada disparo (params e o mesmo objeto - ver
    // Boss.h). E o push nasce no professor, parado: quem o leva ao lugar e a
    // pilha.
    if (stateName == Pilha::kFaseDaPilha) {
        if (mEstourando) {
            // MIRADO NO ALUNO, com uma lasca no meio do leque quando o numero e
            // par (ver Ricardo::CustomizeAttackParams).
            params.firePosition = mDisparoDoEstouro;
            params.numProjectiles = mLascas;
            Vector2 d(0.0f, 1.0f);
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
                d = batalha->Alvo()->GetPosition() - mDisparoDoEstouro;
                if (d.LengthSq() > 0.0f) d.Normalize();
            }
            const bool par = mLascas > 1 && mLascas % 2 == 0;
            const float meioPasso = par ? 0.5f * params.angle / static_cast<float>(mLascas - 1) : 0.0f;
            params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x)) + meioPasso;
        }
        return;
    }

    // A REPESCAGEM sorteia o meio de cada anel de capivaras.
    if (stateName == "StateFinal") {
        params.centralAngle = Random::GetFloatRange(0.f, 180.f);
    }
}




