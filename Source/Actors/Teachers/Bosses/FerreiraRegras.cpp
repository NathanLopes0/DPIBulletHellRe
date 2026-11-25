//
// A INF 216, fase 3: as regras - a nota, o campo, o tempo e o estado.
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

// ---- Fase 3: as regras ----

void Ferreira::AtualizarRegras(const float t, const float deltaTime) {
    using namespace RegrasTrocadas;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    // O fantasma da fase 1 volta e passa de uma etapa para a outra; o save fica sem ele.
    if (etapa >= kInimigoAte && mInimigoFica && mCorpoMontado) {
        if (Battle* b = Batalha()) {
            for (BossProjectile* bloco : mCorpo) {
                if (Vivo(bloco)) b->Faiscar(bloco->GetPosition(), Hud::Faisca::Dissolvido);
            }
        }
        Avisar("inimigo.desativar(): o fantasma sai de cena - por enquanto", 1.4f);
    }
    mInimigoFica = etapa < kInimigoAte;
    TrocouDeEtapa(etapa);
    Varrer(t, 0.32f, 8.0f);
    if (etapa == kEtapaDaInversao) AtualizarInversao(te);
    else if (etapa == kEtapaDoFogoAmigo) AtualizarFogoAmigo(te);
    else if (etapa == kEtapaDoFps) AtualizarFps(te, deltaTime);
    else AtualizarSaveLoad(te, deltaTime);
    if (etapa < kInimigoAte) {
        ComoOInimigo como;
        como.fase1 = false;
        // Invertidas as regras, o tiro e moeda - e o fantasma tambem: ele foge ate elas voltarem.
        if (etapa == kEtapaDaInversao && RegraEm(te) == Regra::Invertida && mIa.estado != IaDoInimigo::Estado::Fugindo) {
            como.susto = std::max(0.5f, kVoltou - te);
        }
        // No FPS, ele anda sem deltaTime.
        if (etapa == kEtapaDoFps) como.passo = SemDeltaTime(mFps);
        AtualizarInimigo(t, deltaTime, como);
    }
}

void Ferreira::AtualizarInversao(const float te) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const Regra r = RegraEm(te);
    const bool moeda = TiroEMoeda(r);
    const bool pisca = MoedaPisca(r, te);
    mCobrando = AcertoCobra(r);
    Battle* b = Batalha();
    const std::vector<Player*> alunos = Jogadores();

    // Invertido, o tiro e moeda: tem a classe do aluno (passa por ele) e paga a quem encosta.
    for (Player* a : alunos) a->DefinirClasse(moeda ? kClasseDaMoeda : -1);
    for (BossProjectile*& p : mTiros) {
        if (!p) continue;
        p->DefinirClasse(moeda ? kClasseDaMoeda : -1);
        Cara(p, moeda && !pisca ? "Moeda" : "Tiro");
        if (!moeda) continue;
        for (const Player* a : alunos) {
            if ((p->GetPosition() - a->GetPosition()).Length() > RaioDe(a) + kToqueDaMoeda + 8.0f) continue;
            if (b) b->Faiscar(p->GetPosition(), Hud::Faisca::Coleta);
            mPremioDasMoedas += kPremioDaMoeda;
            Matar(p);
            break;
        }
    }
    if (mPremioDasMoedas >= 0.9f) {
        if (b) b->Conquistar(mPremioDasMoedas);
        mPremioDasMoedas = 0.0f;
    }

    // O professor atira igual nas duas regras: o que muda e o que o tiro vale.
    if (te >= 0.6f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneis;
        for (BossProjectile* p : Anel(kGatilhoDoAnel, g.tirosDoAnel, g.velocidade, moeda ? "Moeda" : "Tiro")) {
            p->DefinirClasse(moeda ? kClasseDaMoeda : -1);
            mTiros.push_back(p);
        }
    }
    if (te >= 1.0f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, g.tirosDoLeque, 40.0f, g.velocidade + 30.0f, moeda ? "Moeda" : "Tiro")) {
            p->DefinirClasse(moeda ? kClasseDaMoeda : -1);
            mTiros.push_back(p);
        }
    }

    // Sem texto: o professor fica vermelho, com um sinal de menos (acertar custa), e o aluno ganha
    // um anel dourado com um mais (pegar paga). No aviso, os dois piscam.
    if (mCobrando || (r == Regra::Aviso && Aceso(te, 0.15f))) {
        const Vector2 eu = GetPosition();
        Circulo(eu, 76.0f + 3.0f * Math::Sin(mRelogio * 9.0f), kPerigo, 24);
        Preencher(eu + Vector2(-24.0f, 80.0f), eu + Vector2(24.0f, 88.0f), kPerigo);
        for (const Player* a : alunos) {
            const Vector2 onde = a->GetPosition();
            Circulo(onde, 26.0f + 2.0f * Math::Sin(mRelogio * 9.0f), kOuro, 16);
            Preencher(onde + Vector2(-9.0f, -44.0f), onde + Vector2(9.0f, -40.0f), kOuro);
            Preencher(onde + Vector2(-2.0f, -51.0f), onde + Vector2(2.0f, -33.0f), kOuro);
        }
    }
    switch (r) {
        case Regra::Aviso: Placar(kFase, 0, "regras.inverter() em 3, 2, 1..."); break;
        case Regra::Invertida: Placar(kFase, 0, "INVERTIDO: pegue as moedas e NÃO atire"); break;
        case Regra::Voltando: Placar(kFase, 0, "regras.restaurar(): as moedas viram TIROS!"); break;
        case Regra::Normal: Placar(kFase, 0, "Regras de sempre: atire e desvie"); break;
    }
}

void Ferreira::AtualizarFogoAmigo(const float te) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const SDL_FRect c = Campo();
    const bool ligado = te >= kAvisoDoFogoAmigo;
    const Battle* b = Batalha();
    ProjectileManager* gerente = b ? b->GerenteDeProjeteis() : nullptr;

    // O tiro do aluno que sai por cima (o que errou o professor) volta por baixo, uma vez.
    if (ligado && gerente) {
        for (const auto& p : gerente->GetPlayerProjectiles()) {
            if (!p || p->GetState() != ActorState::Active) continue;
            const Vector2 onde = p->GetPosition();
            if (!SaiuPorCima(onde.y, c.y) || !mVoltaram.insert(p->Id()).second) continue;
            Um(kGatilhoDoRetorno, OndeVolta(onde.x, c.y + c.h), "Seu", Vector2(0.0f, -kVelocidadeDoRetorno));
        }
    }

    // Os leques do professor tambem dao a volta: o que sai por baixo reaparece em cima, uma vez.
    if (ligado && te >= mProximoLeque) {
        mProximoLeque = te + g.entreTiros;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, 3, 30.0f, g.velocidade, "Tiro")) {
            p->DefinirMargemDeSaida(2.0f, 1.0f);
            mQueDaoAVolta.push_back({p, false});
        }
    }
    for (QueDaAVolta& q : mQueDaoAVolta) {
        const Vector2 onde = q.p->GetPosition();
        if (!q.voltou && SaiuPorBaixo(onde.y, c.y + c.h)) {
            q.voltou = true;
            q.p->SetPosition(Vector2(onde.x, c.y - 8.0f));
        } else if (!NoCampo(onde, 40.0f)) {
            Matar(q.p);
        }
    }
    mQueDaoAVolta.erase(std::remove_if(mQueDaoAVolta.begin(), mQueDaoAVolta.end(), [](const QueDaAVolta& q) { return !q.p; }),
                        mQueDaoAVolta.end());

    // As duas bordas ligadas: o que sai por uma entra pela outra.
    const SDL_Color cor = ligado ? kPerigo : kVazio;
    Tracejada(Vector2(c.x, c.y + 4.0f), Vector2(c.x + c.w, c.y + 4.0f), cor, 10.0f);
    Tracejada(Vector2(c.x, c.y + c.h - 4.0f), Vector2(c.x + c.w, c.y + c.h - 4.0f), cor, 10.0f);
    for (int k = 0; k < 8; ++k) {
        const float x = c.x + c.w * (static_cast<float>(k) + 0.5f) / 8.0f;
        Seta(Vector2(x, c.y + 28.0f), Vector2(x, c.y + 8.0f), cor);
        Seta(Vector2(x, c.y + c.h - 6.0f), Vector2(x, c.y + c.h - 26.0f), cor);
    }
    Placar(kFase, 1, ligado ? "fogoAmigo = true: tiro que ERRA volta por baixo" : "fogoAmigo = true em 3, 2, 1...");
}

float Ferreira::SeteSegmentos(const std::string& texto, const Vector2 canto, const float altura, const SDL_Color cor) {
    // Cada caractere tem meia altura de largura e um traco de um oitavo dela.
    const float w = altura * 0.5f, e = std::max(2.0f, altura / 8.0f), meio = altura / 2.0f;
    float x = canto.x;
    const float y = canto.y;
    for (const char ch : texto) {
        if (ch == ' ') {
            x += w * 0.6f;
            continue;
        }
        if (ch == '.') {
            Preencher(Vector2(x, y + altura - e), Vector2(x + e, y + altura), cor);
            x += altura / 4.0f;
            continue;
        }
        const auto s = RegrasTrocadas::Segmentos(ch);
        if (s[0]) Preencher(Vector2(x + e, y), Vector2(x + w - e, y + e), cor);
        if (s[1]) Preencher(Vector2(x + w - e, y + e), Vector2(x + w, y + meio - e / 2.0f), cor);
        if (s[2]) Preencher(Vector2(x + w - e, y + meio + e / 2.0f), Vector2(x + w, y + altura - e), cor);
        if (s[3]) Preencher(Vector2(x + e, y + altura - e), Vector2(x + w - e, y + altura), cor);
        if (s[4]) Preencher(Vector2(x, y + meio + e / 2.0f), Vector2(x + e, y + altura - e), cor);
        if (s[5]) Preencher(Vector2(x, y + e), Vector2(x + e, y + meio - e / 2.0f), cor);
        if (s[6]) Preencher(Vector2(x + e, y + meio - e / 2.0f), Vector2(x + w - e, y + meio + e / 2.0f), cor);
        x += w + e * 1.6f;
    }
    return x - canto.x;
}

void Ferreira::AtualizarFps(const float te, const float deltaTime) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const SDL_FRect c = Campo();
    const std::vector<Player*> alunos = Jogadores();

    // O FPS e do aluno: quem segura o foco liga o modo economia. Em dupla, ele so cai inteiro com os
    // dois focados.
    int focados = 0;
    for (const Player* a : alunos) focados += a->Focado() ? 1 : 0;
    const float fracao = alunos.empty() ? 0.0f : static_cast<float>(focados) / static_cast<float>(alunos.size());
    // Quem solta o foco (depois de segura-lo um pouco) arranca: o impulso para correr ate o vao.
    for (size_t i = 0; i < alunos.size() && i < mArranques.size(); ++i) {
        if (Impulsiona(mArranques[i], alunos[i]->Focado(), deltaTime)) {
            alunos[i]->Impulsionar(kImpulso, kFatorDoImpulso);
            if (Battle* b = Batalha()) b->Faiscar(alunos[i]->GetPosition(), Hud::Faisca::Coleta);
        }
    }
    // Soltar o foco tem um respiro: o FPS segura kFolgaAoSoltar antes de subir.
    const float valendo = FocadosComFolga(mFolgaDoFoco, fracao, deltaTime);
    if (te >= kComecaOFps) mFps = PassoDoFps(mFps, FpsAlvo(valendo, g.fpsMaximo), g.fpsMaximo, deltaTime);
    const float fator = SemDeltaTime(mFps);
    mRelogioSemDt += deltaTime * fator;
    if (te >= mProximaAmostra) {
        mProximaAmostra = te + 0.05f;
        mHistoricoDoFps.push_back(mFps);
        if (mHistoricoDoFps.size() > kAmostrasDoGrafico) mHistoricoDoFps.erase(mHistoricoDoFps.begin());
    }

    // O vermelho: a espiral de dois bracos, feita sem deltaTime - dispara e gira pelo relogio dela, e
    // anda o que esta escrito nela vezes o FPS. O desenho e sempre o mesmo; so o tempo dele muda.
    if (te >= kComecaOFps) {
        if (mProximaEspiral <= 0.0f) mProximaEspiral = mRelogioSemDt;
        for (int k = 0; k < 4 && mRelogioSemDt >= mProximaEspiral; ++k) {
            mProximaEspiral += g.entreEspirais;
            mGiroDaEspiral = std::fmod(mGiroDaEspiral + g.giroDaEspiral, 360.0f);
            auto par = DispararEm(kGatilhoDoVermelho, GetPosition() + Vector2(0.0f, 30.0f), 2, mGiroDaEspiral, 180.0f,
                                  g.velocidadeVermelha);
            DarVelocidade(par, mGiroDaEspiral, 180.0f, g.velocidadeVermelha);
            for (BossProjectile* p : par) {
                Cara(p, "SemDt");
                mVermelhos.push_back({p, VelocidadeDe(p)});
            }
        }
    }
    // A cada quadro o vermelho anda o que o FPS manda, e o rastro diz o quanto.
    for (Vermelho& v : mVermelhos) {
        if (!Vivo(v.p)) continue;
        Andar(v.p, v.v * fator);
        const float escrita = v.v.Length();
        if (escrita < 1.0f) continue;
        const Vector2 onde = v.p->GetPosition();
        Linha(onde - v.v * (std::min(70.0f, escrita * fator * 0.08f) / escrita), onde, kRastro);
    }

    // O azul: a parede com um vao, que desce com deltaTime - na mesma velocidade em qualquer FPS. Para
    // chegar ao vao e preciso correr, e quem corre solta o foco.
    if (te >= kComecaOFps + 0.8f && te >= mProximaParede) {
        mProximaParede = te + g.entreParedes;
        // O vao novo: longe do anterior (focado nao se chega), mas ao alcance de quem solta o foco.
        const float longe = c.w / 3.0f;
        mVaoDaParede = VaoDaParede(mSorteio, mVaoDaParede, c.x + 10.0f, c.x + c.w - 10.0f, longe,
                                   VaoAlcancavel(kAlunoSemFoco, g.entreParedes, longe));
        for (float x = c.x + kEntreTirosDaParede / 2.0f; x < c.x + c.w; x += kEntreTirosDaParede) {
            if (std::fabs(x - mVaoDaParede) < kVaoDaParede / 2.0f) continue;
            Um(kGatilhoDoAzul, Vector2(x, c.y + 150.0f), "ComDt", Vector2(0.0f, g.velocidadeAzul));
        }
        SomDoDisparo();
    }

    // Quem segura o foco ganha o anel frio: o jogo esta em camera lenta por causa dele. Quem arranca
    // deixa um rastro de velocidade, para tras do rumo.
    for (const Player* a : alunos) {
        if (a->Focado()) Circulo(a->GetPosition(), 24.0f + 2.0f * Math::Sin(mRelogio * 6.0f), kFrio, 16);
        if (!a->Impulsionado()) continue;
        const auto* corpo = a->GetComponent<RigidBodyComponent>();
        const Vector2 v = corpo ? corpo->GetVelocity() : Vector2::Zero;
        if (v.Length() < 1.0f) continue;
        const Vector2 rumo = v * (1.0f / v.Length()), lado(-rumo.y, rumo.x);
        for (int k = -1; k <= 1; ++k) {
            const Vector2 de = a->GetPosition() + lado * (9.0f * static_cast<float>(k)) - rumo * 12.0f;
            Linha(de, de - rumo * 34.0f, kFrio);
        }
    }

    // O contador, no canto de cima a direita, como o de quem mede o jogo: frio em camera lenta, quente
    // quando o vermelho corre. Embaixo, o grafico dos ultimos segundos.
    const float u = NoGrafico(mFps, g.fpsMaximo);
    const auto mistura = [](const SDL_Color a, const SDL_Color b, const float k) {
        const float q = std::clamp(k, 0.0f, 1.0f);
        const auto m = [q](const Uint8 x, const Uint8 y) {
            return static_cast<Uint8>(std::lround(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * q));
        };
        return SDL_Color{m(a.r, b.r), m(a.g, b.g), m(a.b, b.b), m(a.a, b.a)};
    };
    const SDL_Color morno{240, 244, 255, 240}, quente{255, 90, 70, 245};
    const SDL_Color cor = u < 0.5f ? mistura(kFrio, morno, u * 2.0f) : mistura(morno, quente, (u - 0.5f) * 2.0f);
    const Vector2 canto(c.x + c.w - 262.0f, c.y + 18.0f);
    // O contador vai na camada de cima: o professor passa por baixo dele.
    DrawLinhasComponent* doCampo = mLinhas;
    if (mLinhasDoContador) mLinhas = mLinhasDoContador;
    Preencher(canto - Vector2(12.0f, 10.0f), canto + Vector2(250.0f, 112.0f), SDL_Color{8, 10, 20, 235});
    Retangulo(canto - Vector2(12.0f, 10.0f), canto + Vector2(250.0f, 112.0f), SDL_Color{cor.r, cor.g, cor.b, 120});
    const float largura = SeteSegmentos(std::to_string(static_cast<int>(std::lround(mFps))), canto, 46.0f, cor);
    SeteSegmentos("FPS", canto + Vector2(largura + 10.0f, 20.0f), 26.0f, cor);
    const Vector2 g0(canto.x, canto.y + 60.0f), g1(canto.x + 238.0f, canto.y + 100.0f);
    Retangulo(g0, g1, SDL_Color{150, 160, 200, 110});
    const float y60 = g1.y - (g1.y - g0.y) * NoGrafico(kFpsDeReferencia, g.fpsMaximo);
    Tracejada(Vector2(g0.x, y60), Vector2(g1.x, y60), SDL_Color{240, 244, 255, 80}, 6.0f);
    const float passo = (g1.x - g0.x) / static_cast<float>(kAmostrasDoGrafico - 1);
    for (size_t k = 1; k < mHistoricoDoFps.size(); ++k) {
        const float a0 = g1.y - (g1.y - g0.y) * NoGrafico(mHistoricoDoFps[k - 1], g.fpsMaximo);
        const float a1 = g1.y - (g1.y - g0.y) * NoGrafico(mHistoricoDoFps[k], g.fpsMaximo);
        Linha(Vector2(g0.x + passo * static_cast<float>(k - 1), a0), Vector2(g0.x + passo * static_cast<float>(k), a1), cor);
    }
    mLinhas = doCampo;

    if (te < kComecaOFps) Placar(kFase, kEtapaDoFps, "fps.limite = off: segure o FOCO e o jogo cai para 30 FPS");
    else Placar(kFase, kEtapaDoFps, "VERMELHO sem deltaTime: corre com o FPS. AZUL com deltaTime: não muda");
}

void Ferreira::AtualizarSaveLoad(const float te, const float deltaTime) {
    using namespace RegrasTrocadas;
    const Degrau& g = mDegrauDasRegras;
    const JogosDigitais::Campo campo = CampoPuro();
    const std::vector<Player*> alunos = Jogadores();

    if (te >= 0.5f && te >= mProximoAnel) {
        mProximoAnel = te + g.entreAneisDoMotor;
        for (BossProjectile* p : Anel(kGatilhoDoAnel, g.tirosDoAnelDoMotor, g.velocidadeDoMotor, "Tiro")) mTiros.push_back(p);
    }
    if (te >= 0.9f && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLequesDoMotor;
        for (BossProjectile* p : Leque(kGatilhoDoTiro, g.tirosDoLequeDoMotor, 28.0f, g.velocidadeDoMotor + 70.0f, "Tiro")) mTiros.push_back(p);
    }

    const float agora = Nota();
    // Onde o disquete anda: o campo sem a faixa do professor, com a folga do raio dele. A fuga conhece
    // essas bordas, e escorrega por elas.
    const float folga = kRaioDoSave + 4.0f;
    const JogosDigitais::Campo area{campo.x + folga, campo.y + 150.0f, campo.w - 2.0f * folga, campo.h - 150.0f - folga};
    // O save: a nota de agora e onde cada aluno esta. O disquete nasce um pouco acima do aluno.
    if (!mSalvo.tem && HoraDeSalvar(te, mUltimoLoad)) {
        mSalvo = Salvo{};
        mFuga = RegrasTrocadas::Fuga{};
        mSalvo.tem = true;
        mSalvo.nota = agora;
        for (const Player* a : alunos) mSalvo.alunos.push_back(a->GetPosition());
        const Vector2 aluno = alunos.empty() ? PosicaoDoAluno() : alunos.front()->GetPosition();
        Matar(mSave);
        mSave = Peca(kGatilhoDoDisquete, JogosDigitais::Prender(aluno + Vector2(0.0f, -110.0f), area, 20.0f, 20.0f), "Save");
        if (mSave) mSave->DefinirEnfeite(true);
        mClarao = 0.2f;
        mCorDoClarao = kSave;
        Avisar("quicksave(): o save guardou a nota " + Inteiro(mSalvo.nota), 1.4f);
        SomDoDisparo();
    }
    if (mSalvo.tem && !mSave) mSalvo.tem = false;   // o disquete sumiu por outro caminho

    if (mSalvo.tem) {
        mSalvo.ha += deltaTime;
        // O aluno mais perto: e dele que o save foge, ou e ele que o save persegue.
        const Player* alvo = nullptr;
        float distancia = 1e9f;
        for (const Player* a : alunos) {
            const float d = (a->GetPosition() - mSave->GetPosition()).Length();
            if (d < distancia) {
                distancia = d;
                alvo = a;
            }
        }
        const Humor humor = HumorDoSave(mSalvo.nota, agora);
        const Vector2 eu = mSave->GetPosition();
        const Vector2 ondeAluno = alvo ? alvo->GetPosition() : PosicaoDoAluno();
        // O tiro do aluno que acerta o disquete fica nele: o drive engasga (RegrasTrocadas::PassoDoSave).
        int tiros = 0;
        if (const Battle* batalha = Batalha(); batalha && batalha->GerenteDeProjeteis()) {
            for (const auto& tiro : batalha->GerenteDeProjeteis()->GetPlayerProjectiles()) {
                if (!tiro || tiro->GetState() != ActorState::Active || tiro->Murchando()) continue;
                if ((tiro->GetPosition() - eu).Length() > kRaioDoSave + 6.0f) continue;
                ++tiros;
                tiro->Kill();
            }
        }
        // A fuga e vencivel: encurralado num canto ele trava, e o tiro o faz andar aos trancos.
        const Vector2 passo = mSalvo.ha >= kGravando
                                  ? PassoDoSave(mFuga, humor, eu, ondeAluno, area, RapidezDoSave(mSalvo.nota, agora), tiros, deltaTime)
                                  : Vector2::Zero;
        mSave->SetPosition(JogosDigitais::Prender(eu + passo, area, 0.0f, 0.0f));
        Parar(mSave);
        // Gravando, ele pisca; depois, a cara diz o humor: verde foge (vale mais), vermelho persegue.
        const bool gravando = mSalvo.ha < kGravando;
        Cara(mSave, gravando ? (Aceso(mSalvo.ha, 0.1f) ? "Save" : "Foge")
                             : (humor == Humor::Foge ? "Foge" : (humor == Humor::Persegue ? "Persegue" : "Save")));

        // Onde o load leva: o lugar de cada aluno no save, e a linha ate ele.
        for (size_t i = 0; i < mSalvo.alunos.size(); ++i) {
            const Vector2 onde = mSalvo.alunos[i];
            Circulo(onde, 18.0f, kSave, 14);
            Tracejada(onde + Vector2(-12.0f, 0.0f), onde + Vector2(12.0f, 0.0f), kSave, 4.0f);
            Tracejada(onde + Vector2(0.0f, -12.0f), onde + Vector2(0.0f, 12.0f), kSave, 4.0f);
        }
        const SDL_Color cor = humor == Humor::Foge ? SDL_Color{110, 230, 150, 220}
                                                   : (humor == Humor::Persegue ? SDL_Color{255, 100, 100, 220} : kSave);
        Circulo(mSave->GetPosition(), 26.0f + 2.0f * Math::Sin(mRelogio * 9.0f), cor, 16);
        if (mFuga.travado > 0.0f) {
            // Encurralado: os cantos de uma gaiola em volta dele - a hora de pega-lo.
            const Vector2 s = mSave->GetPosition();
            for (int k = 0; k < 4; ++k) {
                const float sx = k % 2 == 0 ? -1.0f : 1.0f, sy = k < 2 ? -1.0f : 1.0f;
                const Vector2 q = s + Vector2(sx * 32.0f, sy * 32.0f);
                Linha(q, q - Vector2(sx * 13.0f, 0.0f), kOuro);
                Linha(q, q - Vector2(0.0f, sy * 13.0f), kOuro);
            }
        } else if (mFuga.engasgo > 0.0f) {
            // Engasgado: o drive lendo, um arco que gira em volta.
            const Vector2 s = mSave->GetPosition();
            for (int k = 0; k < 6; ++k) {
                const float a0 = mRelogio * 14.0f + static_cast<float>(k) * 0.35f;
                Linha(s + Vector2(Math::Cos(a0), Math::Sin(a0)) * 32.0f, s + Vector2(Math::Cos(a0 + 0.3f), Math::Sin(a0 + 0.3f)) * 32.0f,
                      kSave);
            }
        }

        // A nota que ele guarda, em cima dele, na cor do humor.
        if (EtiquetaDeCodigo* e = Etiqueta(mEtiquetasDoSave, 0, 110, 24)) {
            e->Pintar(cor, SDL_Color{10, 12, 24, 215}, SDL_Color{cor.r, cor.g, cor.b, 255});
            Escrever(e, "SAVE " + Inteiro(mSalvo.nota), mSave->GetPosition() + Vector2(0.0f, -36.0f));
        }

        // O toque e o quick load.
        if (!gravando) {
            for (const Player* a : alunos) {
                if (!Encostou(mSave->GetPosition(), a->GetPosition(), RaioDe(a))) continue;
                Carregar(te);
                break;
            }
        }
        if (mSalvo.tem) {
            const std::string notas = "o save tem " + Inteiro(mSalvo.nota) + ", você tem " + Inteiro(agora);
            switch (humor) {
                case Humor::Foge: Placar(kFase, 3, notas + ": ele FOGE - encurrale num canto, ou atire nele!"); break;
                case Humor::Persegue: Placar(kFase, 3, notas + ": ele te PERSEGUE - fuja!"); break;
                case Humor::Parado: Placar(kFase, 3, notas + ": ele espera"); break;
            }
        }
    }
    if (!mSalvo.tem) {
        Esconder(mEtiquetasDoSave);
        Placar(kFase, 3, te > kUltimoSave ? "sem mais saves: só desvie" : "quicksave() a caminho: o save vai guardar a sua nota");
    }
}

void Ferreira::Carregar(const float te) {
    Battle* b = Batalha();
    const std::vector<Player*> alunos = Jogadores();
    for (size_t i = 0; i < alunos.size() && i < mSalvo.alunos.size(); ++i) {
        alunos[i]->SetPosition(mSalvo.alunos[i]);
        alunos[i]->Proteger(0.6f);
    }
    mFuga = RegrasTrocadas::Fuga{};
    const float antes = Nota();
    if (b) b->DefinirNota(mSalvo.nota);
    Matar(mSave);
    mSalvo.tem = false;
    mUltimoLoad = te;
    Esconder(mEtiquetasDoSave);
    mClarao = 0.25f;
    mCorDoClarao = SDL_Color{255, 255, 255, 255};
    Avisar("quickload(): a nota foi de " + Inteiro(antes) + " para " + Inteiro(mSalvo.nota), 1.6f);
    SomDoDisparo();
}
