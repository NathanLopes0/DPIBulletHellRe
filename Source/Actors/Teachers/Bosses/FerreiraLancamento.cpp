//
// A INF 216, a repescagem: o lancamento.
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

// ---- A repescagem: o lancamento ----

void Ferreira::AtualizarLancamento(const float t, const float deltaTime) {
    using namespace Lancamento;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    TrocouDeEtapa(etapa);
    Varrer(t, 0.3f, 8.0f);
    if (etapa == 0) AtualizarHitbox(te);
    else if (etapa == 1) AtualizarCrash(te);
    else if (etapa == 2) AtualizarFrota(te, deltaTime);
    else AtualizarCreditos(te, deltaTime);
}

void Ferreira::AtualizarHitbox(const float te) {
    using namespace Lancamento;
    const Degrau& g = mDegrauDoLancamento;

    // O desenho vira enfeite, e a colisao vai para um tiro sem cara, que o professor conduz.
    const auto entrar = [this](const std::vector<BossProjectile*>& novos) {
        for (BossProjectile* desenho : novos) {
            desenho->DefinirEnfeite(true);
            BossProjectile* colisor = Peca(kGatilhoDoColisor, desenho->GetPosition(), "Vazio");
            mDescolados.push_back({desenho, colisor});
        }
    };
    if (te >= 0.8f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneisDaHitbox;
        entrar(Anel(kGatilhoDoSprite, g.tirosDoAnelDaHitbox, g.velocidadeDaHitbox, "Tiro"));
    }
    if (te >= 1.4f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLequesDaHitbox;
        entrar(Leque(kGatilhoDoSprite, g.tirosDoLequeDaHitbox, 28.0f, g.velocidadeDaHitbox + 60.0f, "Tiro"));
    }

    const Vector2 fora = Descolamento(te);
    for (Descolado& d : mDescolados) {
        if (!d.desenho) {
            Matar(d.colisor);
            continue;
        }
        if (!d.colisor) continue;
        const Vector2 desenho = d.desenho->GetPosition();
        const Vector2 onde = desenho + fora;
        d.colisor->SetPosition(onde);
        Parar(d.colisor);
        Circulo(onde, 9.0f, kDebug, 10);
        Linha(desenho, onde, kDebugFraco);
    }
    mDescolados.erase(std::remove_if(mDescolados.begin(), mDescolados.end(),
                                     [](const Descolado& d) { return !d.desenho && !d.colisor; }), mDescolados.end());

    // O modo debug mostra todo colisor: o do aluno e o do professor tambem.
    for (const Player* a : Jogadores()) Circulo(a->GetPosition(), RaioDe(a) + 1.0f, kDebug, 10);
    Circulo(GetPosition(), 56.0f, kDebugFraco, 20);
    Placar(kFase, 0, "debug: a hitbox DESCOLOU. Desvie do círculo verde");
}

void Ferreira::Travar(const int k) {
    using namespace Lancamento;
    mTravou[static_cast<size_t>(k)] = true;
    mTravadaAberta = k;
    mEscolhaPendente = Escolha::Nenhuma;
    mTravados.clear();
    for (BossProjectile* p : mTiros) {
        if (!p) continue;
        mTravados.push_back({p, VelocidadeDe(p)});
        Parar(p);
    }
    // Os dois botoes, embaixo da mensagem: ESPERAR troca de lado de uma travada para a outra.
    const SDL_FRect c = Campo();
    const Vector2 meio(c.x + c.w / 2.0f, c.y + c.h * 0.42f);
    const Vector2 esquerda = meio + Vector2(-110.0f, 44.0f), direita = meio + Vector2(110.0f, 44.0f);
    const bool esperarAEsquerda = LadoDoEsperar(k) == 0;
    mBotoes[0] = Peca(kGatilhoDoBotao, esperarAEsquerda ? esquerda : direita, "Esperar");
    mBotoes[1] = Peca(kGatilhoDoBotao, esperarAEsquerda ? direita : esquerda, "Fechar");
    Escrever(Etiqueta(mEtiquetasDoDialogo, 0, 440, 26), "DPIBulletHell não está respondendo", meio + Vector2(0.0f, -28.0f));
    mClarao = 0.18f;
    mCorDoClarao = SDL_Color{255, 255, 255, 255};
    SomDoDisparo();
}

void Ferreira::Destravar(const Lancamento::Escolha escolha) {
    using namespace Lancamento;
    if (mTravadaAberta < 0) return;
    mTravadaAberta = -1;
    for (BossProjectile*& botao : mBotoes) Matar(botao);
    Esconder(mEtiquetasDoDialogo);
    Battle* b = Batalha();
    if (escolha == Escolha::Fechar) {
        // Fechar o programa: cada tiro congelado estilhaca em dois.
        for (const Travado& t : mTravados) {
            if (!Vivo(t.p)) continue;
            const float centro = mSorteio.Real(0.0f, 360.0f);
            const auto cacos = DispararEm(kGatilhoDoEstilhaco, t.p->GetPosition(), 2, centro, 180.0f, 210.0f);
            DarVelocidade(cacos, centro, 180.0f, 210.0f);
            for (BossProjectile* f : cacos) Cara(f, "Fragmento");
            t.p->Kill();
        }
        Avisar("Fechou o programa: os tiros estilhaçaram!", 1.8f);
        mClarao = 0.25f;
        mCorDoClarao = kSave;
    } else {
        const float fator = FatorDaVolta(escolha);
        for (const Travado& t : mTravados) {
            if (Vivo(t.p)) Andar(t.p, t.v * fator);
        }
        if (escolha == Escolha::Esperar) {
            if (b) b->Conquistar(kPremioPorEsperar);
            Avisar("Esperou: o jogo voltou de onde parou", 1.6f);
        } else {
            Avisar("Sem resposta: recuperando os quadros perdidos!", 1.8f);
        }
    }
    mTravados.clear();
    SomDoDisparo();
}

void Ferreira::AtualizarCrash(const float te) {
    using namespace Lancamento;
    const Degrau& g = mDegrauDoLancamento;
    const SDL_FRect c = Campo();

    const int k = TravadaEm(te);
    if (k >= 0 && !mTravou[static_cast<size_t>(k)]) Travar(k);
    if (mTravadaAberta >= 0 && mEscolhaPendente != Escolha::Nenhuma) Destravar(mEscolhaPendente);
    if (mTravadaAberta >= 0 && k != mTravadaAberta) Destravar(Escolha::Nenhuma);
    mEscolhaPendente = Escolha::Nenhuma;

    if (mTravadaAberta >= 0) {
        // Travado: nada anda, e o dialogo espera no meio do campo.
        for (const Travado& t : mTravados) Parar(t.p);
        const Vector2 meio(c.x + c.w / 2.0f, c.y + c.h * 0.42f);
        const Vector2 meia(250.0f, 86.0f);
        Preencher(meio - meia, meio + meia, SDL_Color{16, 18, 34, 235});
        Preencher(meio - meia, Vector2(meio.x + meia.x, meio.y - meia.y + 20.0f), SDL_Color{40, 70, 190, 235});
        Retangulo(meio - meia, meio + meia, kGuia);
        // O tempo que resta para escolher.
        const float resta = std::clamp(1.0f - (te - kTravadas[static_cast<size_t>(mTravadaAberta)]) / kEspera, 0.0f, 1.0f);
        Preencher(Vector2(meio.x - 200.0f, meio.y - 2.0f), Vector2(meio.x - 200.0f + 400.0f * resta, meio.y + 4.0f), kPerigo);
        Placar(kFase, 1, "O jogo travou: atire em ESPERAR (ou em FECHAR...)");
        return;
    }

    // Solto: aneis cheios, que e o que vai congelar.
    if (te >= 0.4f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneis;
        for (BossProjectile* p : Anel(kGatilhoDoAnel, g.tirosDoAnel, g.velocidade, "Tiro")) mTiros.push_back(p);
    }
    if (te >= 0.9f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreAneis * 1.7f;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, 3, 26.0f, g.velocidade + 45.0f, "Tiro")) mTiros.push_back(p);
    }
    Placar(kFase, 1, "Build de lançamento: nada pode dar errado");
}

void Ferreira::AtualizarFrota(const float te, const float deltaTime) {
    using namespace Lancamento;
    const Degrau& g = mDegrauDoLancamento;
    const SDL_FRect c = Campo();
    Battle* b = Batalha();
    const Vector2 topo(c.x + c.w / 2.0f, c.y + 170.0f);
    const float folga = (c.w - static_cast<float>(kColunasDaFrota - 1) * kEspacoX) / 2.0f - 40.0f;
    const int total = kFileiras * kColunasDaFrota;
    const auto indice = [](const int fileira, const int coluna) { return static_cast<size_t>(fileira * kColunasDaFrota + coluna); };
    const Vector2 eu = GetPosition();
    const std::string onda = std::to_string(mOndaDaFrota + 1);

    // A chegada de uma onda: os lugares piscam e os bugs aparecem - ja com o god mode ligado.
    if (!mFrotaMontada && !mJanelaAberta) {
        mGodMode = true;
        if (mFrotaChegaEm < 0.0f) mFrotaChegaEm = te + kChegadaDaFrota;
        if (Aceso(te, 0.12f)) {
            for (int f = 0; f < kFileiras; ++f) {
                for (int col = 0; col < kColunasDaFrota; ++col) Circulo(LugarNaFrota(f, col, topo, Marcha{}), 12.0f, kDebugFraco, 8);
            }
        }
        if (te >= mFrotaChegaEm) {
            mFrotaMontada = true;
            mPassosDaFrota = 0.0f;
            mProximoHotfix = te + g.hotfix;
            mFrota.assign(static_cast<size_t>(total), nullptr);
            for (int f = 0; f < kFileiras; ++f) {
                for (int col = 0; col < kColunasDaFrota; ++col) {
                    mFrota[indice(f, col)] = Peca(kGatilhoDoBug, LugarNaFrota(f, col, topo, Marcha{}), "Bug");
                }
            }
            if (mOndaDaFrota > 0) Avisar("new Wave(" + onda + "): mais rápida, e o god mode de volta", 1.6f);
            SomDoDisparo();
        }
        Placar(kFase, 2, mOndaDaFrota == 0 ? "genero = nave: os bugs do lançamento chegaram"
                                           : "onda " + onda + " a caminho");
        return;
    }

    // A marcha: de um lado ao outro, um degrau para baixo a cada batida na borda - e mais rapida quanto
    // menos bugs sobram, e a cada onda.
    int vivos = 0;
    for (const BossProjectile* p : mFrota) vivos += p ? 1 : 0;
    if (vivos > 0) mPassosDaFrota += deltaTime / EntrePassos(g.entrePassos, vivos, total, mOndaDaFrota);
    const Marcha m = MarchaEm(static_cast<int>(mPassosDaFrota), folga);
    std::vector<size_t> deBaixo;
    std::vector<int> vazios;
    for (int col = 0; col < kColunasDaFrota; ++col) {
        bool achou = false;
        for (int f = kFileiras - 1; f >= 0; --f) {
            BossProjectile* p = mFrota[indice(f, col)];
            if (!p) {
                vazios.push_back(static_cast<int>(indice(f, col)));
                continue;
            }
            p->SetPosition(LugarNaFrota(f, col, topo, m));
            p->RenovarTempoDeVida();
            Parar(p);
            if (!achou) deBaixo.push_back(indice(f, col));
            achou = true;
        }
    }
    // O bug de baixo de uma coluna solta um tiro.
    if (!deBaixo.empty() && te >= mProximaQueda) {
        mProximaQueda = te + g.entreQuedas * std::pow(0.85f, static_cast<float>(mOndaDaFrota));
        const size_t qual = deBaixo[static_cast<size_t>(mSorteio.Inteiro(0, static_cast<int>(deBaixo.size()) - 1))];
        Um(kGatilhoDaQueda, mFrota[qual]->GetPosition() + Vector2(0.0f, 18.0f), "Tiro", Vector2(0.0f, g.velocidadeDaQueda));
    }

    // O god mode: cada bug vivo o segura por um fio, e a gaiola do modo debug fica em volta dele.
    EtiquetaDeCodigo* rotulo = Etiqueta(mEtiquetasDaFrota, 0, 180, 26);
    if (vivos > 0) {
        mGodMode = true;
        for (const BossProjectile* p : mFrota) {
            if (p) Tracejada(eu + Vector2(0.0f, 30.0f), p->GetPosition(), kDebugFraco, 7.0f);
        }
        Circulo(eu, 74.0f + 3.0f * Math::Sin(mRelogio * 7.0f), kDebug, 28);
        Circulo(eu, 80.0f, kDebugFraco, 28);
        // Em god mode ele protege a frota: um leque mirado de tanto em tanto, mais rapido a cada onda.
        if (te >= mProximoLeque) {
            mProximoLeque = te + g.entreLequesDaFrota * std::pow(0.85f, static_cast<float>(mOndaDaFrota));
            Leque(kGatilhoDoTiro, g.tirosDoLequeDaFrota, 34.0f, g.velocidade + 20.0f, "Tiro");
        }
        if (rotulo) {
            rotulo->Pintar(SDL_Color{120, 255, 120, 160}, SDL_Color{10, 12, 24, 215}, SDL_Color{150, 255, 170, 255});
            Escrever(rotulo, "godMode = true", eu + Vector2(eu.x > Campo().x + Campo().w - 260.0f ? -170.0f : 170.0f, 10.0f));
        }
    }

    // O hotfix: um lugar vazio pisca, o fio do professor vai ate ele, e o bug volta.
    if (vivos > 0 && vivos < total && mHotfixLugar < 0 && te >= mProximoHotfix) {
        mHotfixLugar = LugarDoHotfix(vazios, mSorteio);
        mHotfixEm = te + kAvisoDoHotfix;
    }
    if (mHotfixLugar >= 0) {
        const int f = mHotfixLugar / kColunasDaFrota, col = mHotfixLugar % kColunasDaFrota;
        const Vector2 onde = LugarNaFrota(f, col, topo, m);
        if (vivos == 0 || mFrota[static_cast<size_t>(mHotfixLugar)]) {
            mHotfixLugar = -1;   // a frota acabou antes do patch: ele nao tem onde aplicar
        } else if (te < mHotfixEm) {
            Tracejada(eu + Vector2(0.0f, 30.0f), onde, kMagenta, 6.0f);
            if (Aceso(te, 0.07f)) Circulo(onde, 16.0f, kMagenta, 10);
        } else {
            mFrota[static_cast<size_t>(mHotfixLugar)] = Peca(kGatilhoDoBug, onde, "Bug");
            if (b) b->Faiscar(onde, Hud::Faisca::Coleta);
            mHotfixLugar = -1;
            mProximoHotfix = te + g.hotfix;
            ++vivos;
        }
    }

    // Zerada a frota, o god mode cai: a janela.
    if (vivos == 0 && !mJanelaAberta) {
        mJanelaAberta = true;
        mGodMode = false;
        mJanelaAte = te + g.janela;
        if (b) b->Conquistar(kPremioDaFrotaLimpa);
        mClarao = 0.2f;
        mCorDoClarao = kOuro;
        Avisar("0 bugs: godMode = false - ATIRE NELE, cada acerto vale o dobro!", 1.8f);
        SomDoDisparo();
    }
    if (mJanelaAberta) {
        // Sem frota, ele se defende atirando; a barra embaixo dele e o tempo da janela.
        if (te >= mProximoLeque) {
            mProximoLeque = te + 0.9f;
            Leque(kGatilhoDoTiro, 4, 30.0f, g.velocidade + 40.0f, "Tiro");
        }
        const float u = std::clamp((mJanelaAte - te) / std::max(g.janela, 0.1f), 0.0f, 1.0f);
        const Vector2 a(eu.x - 70.0f, eu.y + 66.0f), z(eu.x + 70.0f, eu.y + 74.0f);
        Retangulo(a, z, kOuro);
        Preencher(a, Vector2(Mistura(a.x, z.x, u), z.y), kOuro);
        if (Aceso(te, 0.15f)) Circulo(eu, 70.0f, kPerigo, 24);
        if (rotulo) {
            rotulo->Pintar(SDL_Color{255, 214, 90, 160}, SDL_Color{10, 12, 24, 215}, SDL_Color{255, 214, 90, 255});
            Escrever(rotulo, "godMode = false", eu + Vector2(eu.x > Campo().x + Campo().w - 260.0f ? -170.0f : 170.0f, 10.0f));
        }
        if (te >= mJanelaAte) {
            // A onda seguinte: mais rapida, e o god mode de volta.
            mJanelaAberta = false;
            mFrotaMontada = false;
            mFrotaChegaEm = -1.0f;
            ++mOndaDaFrota;
            mGodMode = true;
        }
        Placar(kFase, 2, "godMode = false: ATIRE NO PROFESSOR (x2)!");
        return;
    }
    Placar(kFase, 2, "godMode = true enquanto houver bug: faltam " + std::to_string(vivos) + " (onda " + onda + ")");
}

void Ferreira::AtualizarCreditos(const float te, const float deltaTime) {
    using namespace Lancamento;
    const Degrau& g = mDegrauDoLancamento;
    const SDL_FRect c = Campo();
    const int blocos = std::max(1, static_cast<int>(c.w / kBloco));
    const float sobra = (c.w - static_cast<float>(blocos) * kBloco) / 2.0f;
    const auto xDoBloco = [&](const int bloco) { return c.x + sobra + (static_cast<float>(bloco) + 0.5f) * kBloco; };

    // Uma linha nova sobe do rodape: uma parede de blocos com um vao, e o texto no meio.
    if (te >= 0.8f && te >= mProximaLinha && mLinhasFeitas < static_cast<int>(kCreditos.size())) {
        mProximaLinha = te + g.entreLinhas;
        LinhaDeCredito l;
        l.y = c.y + c.h + 26.0f;
        l.vao = VaoDaLinha(blocos, mSorteio);
        l.texto = mLinhasFeitas++;
        l.etiqueta = static_cast<size_t>(l.texto) % kEtiquetasDosCreditos;
        // A ultima linha (o obrigado) sobe sozinha, sem parede.
        const bool comParede = !UltimaLinha(l.texto);
        for (int bloco = 0; comParede && bloco < blocos; ++bloco) {
            if (NoVao(bloco, l.vao)) continue;
            if (BossProjectile* p = Peca(kGatilhoDoCredito, Vector2(xDoBloco(bloco), l.y), "Bloco")) {
                l.blocos.push_back(p);
                l.lugares.push_back(bloco);
            }
        }
        Escrever(Etiqueta(mEtiquetasDosCreditos, l.etiqueta, 388, 26), kCreditos[static_cast<size_t>(l.texto)],
                 Vector2(c.x + c.w / 2.0f, l.y));
        mCreditos.push_back(std::move(l));
    }

    for (LinhaDeCredito& l : mCreditos) {
        l.y -= g.subida * deltaTime;
        for (size_t i = 0; i < l.blocos.size(); ++i) {
            if (!l.blocos[i]) continue;
            l.blocos[i]->SetPosition(Vector2(xDoBloco(l.lugares[i]), l.y));
            Parar(l.blocos[i]);
        }
        if (l.etiqueta < mEtiquetasDosCreditos.size() && mEtiquetasDosCreditos[l.etiqueta]) {
            mEtiquetasDosCreditos[l.etiqueta]->SetPosition(Vector2(c.x + c.w / 2.0f, l.y));
            mEtiquetasDosCreditos[l.etiqueta]->Mostrar(l.y > c.y + 20.0f);
        }
        // O vao, marcado.
        if (l.blocos.empty()) continue;
        const float x0 = c.x + sobra + static_cast<float>(l.vao) * kBloco;
        Preencher(Vector2(x0, l.y - 12.0f), Vector2(x0 + static_cast<float>(kVaoDosCreditos) * kBloco, l.y + 12.0f), kSeguro);
    }
    // A linha que passou do alto sai.
    while (!mCreditos.empty() && mCreditos.front().y < c.y - 20.0f) {
        const size_t etiqueta = mCreditos.front().etiqueta;
        if (etiqueta < mEtiquetasDosCreditos.size() && mEtiquetasDosCreditos[etiqueta]) mEtiquetasDosCreditos[etiqueta]->Mostrar(false);
        MatarTodos(mCreditos.front().blocos);
        mCreditos.erase(mCreditos.begin());
    }

    if (te >= 2.0f && te >= mProximoLeque) {
        mProximoLeque = te + 1.5f;
        Leque(kGatilhoDoTiro, 1, 0.0f, g.velocidade, "Tiro");
    }
    Placar(kFase, 3, "Créditos: passe pelos vãos. Obrigado por jogar!");
}
