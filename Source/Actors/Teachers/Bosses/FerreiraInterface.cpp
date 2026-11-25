//
// A INF 216, fase 1: a interface - a nota editada, a barra, a camera e o inimigo.
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

// ---- Fase 1: a interface ----

void Ferreira::AtualizarHud(const float t, const float deltaTime) {
    using namespace HudHostil;
    const Degrau& g = mDegrauDoHud;
    const int etapa = JogosDigitais::Etapa(kDuracoes, t);
    const float te = JogosDigitais::NaEtapa(kDuracoes, t);
    TrocouDeEtapa(etapa);
    Varrer(t, 0.3f, 9.0f);

    if (etapa == kEtapaDaAbertura) {
        AtualizarAbertura(te);
        return;
    }
    if (etapa == kEtapaDoInimigo) {
        AtualizarInimigo(te, deltaTime, ComoOInimigo{});
        return;
    }
    if (etapa == kEtapaDaCamera) {
        AtualizarCamera(te, deltaTime);
        return;
    }
    AtualizarBarra(etapa, te);
    // Os leques so comecam com a barra no campo: o susto do rodape e so dela.
    const bool arrancada = etapa == kEtapaDaBarraDeitada && te < kTremorDaBarra + kArrancadaDaBarra;
    if (!arrancada && te >= mProximoLeque) {
        mProximoLeque = te + g.entreLeques;
        Leque(kGatilhoDoLeque, g.tirosDoLeque, 26.0f, g.velocidade, "Tiro");
    }
    const FaixaDaNota* faixa = Faixa();
    const float nota = std::clamp(faixa ? faixa->Mostrada() : Nota(), 0.0f, 100.0f);
    const std::string cheia = ::Nota::ECheia(nota) ? "100% cheia: NÃO há vazio!" : Inteiro(nota) + "% cheia: passe pelo VAZIO";
    if (arrancada) {
        Placar(kFase, static_cast<size_t>(etapa), "hud.nota.soltar(): a barra da sua nota saiu da HUD!");
    } else {
        Placar(kFase, static_cast<size_t>(etapa), (etapa == kEtapaDaBarraDeitada ? "A barra é a sua nota, " : "Agora ela vem de lado, ") + cheia);
    }
}

void Ferreira::TocarSom(const char* som) {
    if (mScene && mScene->GetGame() && mScene->GetGame()->GetAudio()) mScene->GetGame()->GetAudio()->PlaySound(som, false);
}

void Ferreira::DesenharOVisor(const SDL_Color cor) {
    // As quatro quinas do visor de uma camera, grossas: o campo inteiro e o que ela filma.
    const SDL_FRect c = Campo();
    constexpr float kDentro = 14.0f, kBraco = 74.0f;
    const Vector2 quinas[4] = {Vector2(c.x + kDentro, c.y + kDentro), Vector2(c.x + c.w - kDentro, c.y + kDentro),
                               Vector2(c.x + c.w - kDentro, c.y + c.h - kDentro), Vector2(c.x + kDentro, c.y + c.h - kDentro)};
    for (int k = 0; k < 4; ++k) {
        const float sx = (k == 0 || k == 3) ? 1.0f : -1.0f, sy = k < 2 ? 1.0f : -1.0f;
        for (int e = 0; e < 4; ++e) {
            const float o = static_cast<float>(e);
            Linha(quinas[k] + Vector2(0.0f, sy * o), quinas[k] + Vector2(sx * kBraco, sy * o), cor);
            Linha(quinas[k] + Vector2(sx * o, 0.0f), quinas[k] + Vector2(sx * o, sy * kBraco), cor);
        }
    }
}

void Ferreira::AtualizarCamera(const float te, const float deltaTime) {
    using namespace HudHostil;
    const Degrau& g = mDegrauDoHud;
    const SDL_FRect c = Campo();
    const JogosDigitais::Campo cp = CampoPuro();
    const std::vector<Player*> alunos = Jogadores();
    const Camera momento = CameraEm(te);
    Esconder(mEtiquetasDaCamera);
    if (!mCameraAvisada) {
        // O aviso tem som: ninguem pode ser pego de surpresa.
        mCameraAvisada = true;
        mCamera = Vector2::Zero;
        TocarSom("sinal.wav");
    }

    // O aluno: no aviso ele ainda anda; no fim dele, desliza ate a ancora, e dali nao sai.
    if (mDeslizaDe.size() != alunos.size()) {
        mDeslizaDe.clear();
        for (const Player* a : alunos) mDeslizaDe.push_back(a ? a->GetPosition() : Vector2::Zero);
    }
    const float desliza = kAvisoDaCamera - kDeslizaAteAAncora;
    for (size_t i = 0; i < alunos.size(); ++i) {
        Player* a = alunos[i];
        if (!a) continue;
        const Vector2 ancora = AncoraDaCamera(cp, i, alunos.size());
        if (te < desliza) {
            mDeslizaDe[i] = a->GetPosition();
            // Onde ele vai ficar: o pino na ancora, e o tracejado ate ela.
            Circulo(ancora, 20.0f + 3.0f * std::sin(mRelogio * 10.0f), kOuro, 20);
            Linha(ancora + Vector2(-8.0f, 0.0f), ancora + Vector2(8.0f, 0.0f), kOuro);
            Linha(ancora + Vector2(0.0f, -8.0f), ancora + Vector2(0.0f, 8.0f), kOuro);
            Tracejada(a->GetPosition(), ancora, kOuro, 8.0f);
            continue;
        }
        const float u = std::clamp((te - desliza) / kDeslizaAteAAncora, 0.0f, 1.0f);
        a->SegurarNaCamera(mDeslizaDe[i] + (ancora - mDeslizaDe[i]) * (u * u * (3.0f - 2.0f * u)));
        // Preso: o tripe embaixo dele.
        const Vector2 pe = a->GetPosition() + Vector2(0.0f, 30.0f);
        Linha(pe, pe + Vector2(-16.0f, 18.0f), kGuia);
        Linha(pe, pe + Vector2(16.0f, 18.0f), kGuia);
        Linha(pe, pe + Vector2(0.0f, 20.0f), kGuia);
    }

    // A camera: o manche (na dupla, a media dos dois) a move, e o mundo anda o contrario na tela.
    if (momento != Camera::Aviso) {
        if (!mCameraLigada) {
            mCameraLigada = true;
            TocarSom("toque.wav");
        }
        Vector2 intencao = Vector2::Zero;
        int presos = 0;
        for (const Player* a : alunos) {
            if (a && a->NaCamera()) {
                intencao += a->IntencaoDaCamera();
                ++presos;
            }
        }
        if (presos > 0) intencao = intencao * (1.0f / static_cast<float>(presos));
        const Vector2 passo = PassoDoMundo(intencao, deltaTime);
        mCamera -= passo;
        if (Battle* b = Batalha()) {
            if (ProjectileManager* tiros = b->GerenteDeProjeteis()) {
                for (const auto& p : tiros->GetBossProjectiles()) {
                    if (p && p->GetState() == ActorState::Active) p->SetPosition(p->GetPosition() + passo);
                }
            }
        }
        // O professor anda com o mundo (e a varredura dele tambem), sem sair da tela.
        const Vector2 solto = GetPosition() + passo;
        const Vector2 preso = ProfessorNaTela(solto, cp, 48.0f, kFundoDoProfessor);
        SetPosition(preso);
        if (mAlvo) mAlvo = ProfessorNaTela(*mAlvo - mCamera, cp, 48.0f, kFundoDoProfessor);
        if ((solto - preso).Length() > 0.5f) {
            // A borda segura o professor: a camera nao sai da arena.
            const Vector2 d = JogosDigitais::Rumo(preso, solto);
            const Vector2 lado(-d.y, d.x);
            const Vector2 na = preso + d * 34.0f;
            for (int k = -1; k <= 1; ++k) Linha(na + lado * 28.0f + d * (2.0f * k), na - lado * 28.0f + d * (2.0f * k), kPerigo);
        }
    }

    // A grade do fundo anda com a camera: e ela que mostra que o mundo e que se move.
    {
        constexpr float kPasso = 90.0f;
        const float forca = momento == Camera::Aviso ? std::clamp(te / kAvisoDaCamera, 0.0f, 1.0f) : 1.0f;
        const SDL_Color grade{150, 200, 255, static_cast<Uint8>(38.0f * forca)};
        const float ox = kPasso - std::fmod(std::fmod(mCamera.x, kPasso) + kPasso, kPasso);
        const float oy = kPasso - std::fmod(std::fmod(mCamera.y, kPasso) + kPasso, kPasso);
        for (float x = c.x + ox; x < c.x + c.w; x += kPasso) Linha(Vector2(x, c.y), Vector2(x, c.y + c.h), grade);
        for (float y = c.y + oy; y < c.y + c.h; y += kPasso) Linha(Vector2(c.x, y), Vector2(c.x + c.w, y), grade);
    }

    // O visor: vermelho e piscando no aviso, branco com a camera livre, ambar na volta.
    const bool pisca = std::fmod(mRelogio, 0.3f) < 0.18f;
    const SDL_Color vermelho{255, 70, 60, 240}, branco{235, 240, 255, 210}, ambar{255, 190, 70, 240};
    if (momento == Camera::Aviso) DesenharOVisor(pisca ? vermelho : SDL_Color{255, 70, 60, 110});
    else if (momento == Camera::Livre) DesenharOVisor(branco);
    else DesenharOVisor(pisca ? ambar : SDL_Color{255, 190, 70, 90});
    // O REC, no canto de baixo (o de cima e do quadro), e o centro da camera.
    const Vector2 rec(c.x + 46.0f, c.y + c.h - 76.0f);
    if (std::fmod(mRelogio, 1.0f) < 0.6f) {
        for (float r = 1.0f; r <= 8.0f; r += 1.0f) Circulo(rec, r, vermelho, 14);
    }
    Escrever(Etiqueta(mEtiquetasDaCamera, 0, 150, 24), "REC · CÂMERA", rec + Vector2(92.0f, 0.0f));
    if (momento != Camera::Aviso) {
        const Vector2 meio(c.x + c.w / 2.0f, c.y + c.h / 2.0f);
        Linha(meio + Vector2(-12.0f, 0.0f), meio + Vector2(12.0f, 0.0f), kGuia);
        Linha(meio + Vector2(0.0f, -12.0f), meio + Vector2(0.0f, 12.0f), kGuia);
        const auto numero = [](const float v) {
            const int n = static_cast<int>(std::lround(v));
            return (n > 0 ? "+" : "") + std::to_string(n);
        };
        Escrever(Etiqueta(mEtiquetasDaCamera, 2, 170, 24), "x " + numero(mCamera.x) + "  y " + numero(mCamera.y),
                 rec + Vector2(92.0f, -28.0f));
    }

    // O aviso: a contagem e um icone de camera, sem uma palavra - o que muda, o aluno descobre.
    if (momento == Camera::Aviso) {
        const Vector2 meio(c.x + c.w / 2.0f, c.y + c.h * 0.42f);
        if (const int n = ContagemDaCamera(te); n > 0) SeteSegmentos(std::to_string(n), meio + Vector2(-28.0f, -80.0f), 110.0f, vermelho);
        // A camera: o corpo, o visor em cima, a lente e a luz do REC.
        const Vector2 icone = meio + Vector2(0.0f, 100.0f);
        for (int e = 0; e < 3; ++e) {
            const float o = static_cast<float>(e);
            Retangulo(icone + Vector2(-46.0f + o, -28.0f + o), icone + Vector2(46.0f - o, 28.0f - o), branco);
            Retangulo(icone + Vector2(-18.0f + o, -40.0f + o), icone + Vector2(10.0f - o, -28.0f), branco);
        }
        for (float r = 15.0f; r <= 18.0f; r += 1.0f) Circulo(icone, r, branco, 24);
        Circulo(icone, 8.0f + 1.5f * std::sin(mRelogio * 8.0f), branco, 16);
        if (pisca) for (float r = 1.0f; r <= 5.0f; r += 1.0f) Circulo(icone + Vector2(34.0f, -16.0f), r, vermelho, 12);
    }

    // Os tiros: o anel do professor e o leque mirado; e as caixas paradas do mundo, que so a camera move.
    if (te >= kAvisoDaCamera + 0.3f) {
        if (te >= mProximoAnel) {
            mProximoAnel = te + g.entreAneisDaCamera;
            Anel(kGatilhoDoAnelDaCamera, g.tirosDoAnelDaCamera, g.velocidadeDaCamera, "Tiro");
        }
        if (te >= mProximoLeque) {
            mProximoLeque = te + g.entreLequesDaCamera;
            Leque(kGatilhoDoLeque, 3, 22.0f, g.velocidade, "Tiro");
        }
        if (momento == Camera::Livre && te >= mProximaCaixa) {
            mProximaCaixa = te + g.entreCaixas;
            for (int tentativa = 0; tentativa < 14; ++tentativa) {
                const Vector2 onde(mSorteio.Real(c.x + 60.0f, c.x + c.w - 60.0f), mSorteio.Real(c.y + 140.0f, c.y + c.h - 60.0f));
                bool longe = true;
                for (const Player* a : alunos) longe = longe && (!a || (a->GetPosition() - onde).Length() >= kLongeDaCaixa);
                if (!longe) continue;
                if (BossProjectile* p = Peca(kGatilhoDaCaixa, onde, "Caixa")) {
                    p->DefinirEnfeite(true);
                    mCaixasDaCamera.push_back(CaixaDaCamera{p, te});
                }
                break;
            }
        }
    }
    for (CaixaDaCamera& cx : mCaixasDaCamera) {
        if (!Vivo(cx.p)) {
            cx.p = nullptr;
            continue;
        }
        const float idade = te - cx.nasceu;
        if (idade >= g.vidaDaCaixa) {
            Matar(cx.p);
            continue;
        }
        if (idade < kCaixaAcende) {
            // Ainda nao machuca: so o contorno, piscando.
            const Vector2 p = cx.p->GetPosition();
            if (pisca) Retangulo(p - Vector2(20.0f, 20.0f), p + Vector2(20.0f, 20.0f), kPerigo);
        } else {
            cx.p->DefinirEnfeite(false);
        }
    }
    mCaixasDaCamera.erase(std::remove_if(mCaixasDaCamera.begin(), mCaixasDaCamera.end(),
                                         [](const CaixaDaCamera& cx) { return !cx.p; }),
                          mCaixasDaCamera.end());

    // O quadro so diz o que o codigo faz, e nao o que fazer.
    if (momento == Camera::Aviso) Placar(kFase, kEtapaDaCamera, "camera.ligar()");
    else if (momento == Camera::Livre) Placar(kFase, kEtapaDaCamera, "camera.ligar(): REC");
    else Placar(kFase, kEtapaDaCamera, "camera.desligar()");
}

void Ferreira::AtualizarAbertura(const float te) {
    using namespace HudHostil;
    FaixaDaNota* faixa = Faixa();
    const Vector2 placa = faixa ? faixa->LugarDoNumero() : Vector2(Campo().x + Campo().w / 2.0f, Campo().y + Campo().h);
    // Uma vez por luta: se a fase 1 voltar (o ciclo das fases), a nota ja foi reescrita.
    if (mNotaEditada) {
        if (faixa) faixa->EsconderNumero(false);
        Placar(kFase, kEtapaDaAbertura, "nota = 15: agora a luta e de verdade");
        return;
    }
    if (mNotaAntesDaEdicao < 0.0f) mNotaAntesDaEdicao = Nota();
    const NumeroDaAbertura n = NumeroEm(te, placa, GetPosition(), mNotaAntesDaEdicao);

    if (n.momento == Momento::Devolvido) {
        // O numero novo encaixa na placa: dali em diante, a nota e 15.
        mNotaEditada = true;
        if (Battle* b = Batalha()) {
            b->DefinirNota(kNotaDaAbertura);
            b->Faiscar(placa, Hud::Faisca::Coleta);
        }
        if (faixa) faixa->EsconderNumero(false);
        mClarao = 0.2f;
        mCorDoClarao = kMagenta;
        Avisar("hud.nota = 15. Agora sim: bem-vindo a INF 216", 1.8f);
        SomDoDisparo();
        return;
    }
    if (faixa) faixa->EsconderNumero(n.momento != Momento::NaPlaca || Aceso(te, 0.08f));

    // O numero vai na camada de cima, como o contador do FPS: o jogo inteiro e dele.
    DrawLinhasComponent* doCampo = mLinhas;
    if (mLinhasDoContador) mLinhas = mLinhasDoContador;
    char texto[16];
    std::snprintf(texto, sizeof(texto), "%.2f", static_cast<double>(n.valor));
    const float altura = 22.0f * n.tamanho;
    // A largura de um caractere e a do ponto, como SeteSegmentos as escreve, para centrar o numero.
    float largura = 0.0f;
    for (const char* ch = texto; *ch; ++ch) largura += *ch == '.' ? altura / 4.0f : altura * 0.5f + std::max(2.0f, altura / 8.0f) * 1.6f;
    Vector2 canto = n.onde - Vector2(largura / 2.0f, altura / 2.0f);
    if (n.momento == Momento::NaPlaca) canto = canto + Vector2(3.0f * Math::Sin(mRelogio * 70.0f), 0.0f);
    const bool piscando = n.momento == Momento::Reescrevendo && Aceso(te, 0.09f);
    const SDL_Color cor = n.momento == Momento::Reescrevendo ? (piscando ? SDL_Color{255, 60, 230, 255} : SDL_Color{255, 240, 255, 255})
                                                             : SDL_Color{240, 244, 255, 255};
    if (n.momento != Momento::NaPlaca) {
        Preencher(canto - Vector2(10.0f, 8.0f), canto + Vector2(largura + 10.0f, altura + 8.0f), SDL_Color{8, 10, 20, 225});
        Retangulo(canto - Vector2(10.0f, 8.0f), canto + Vector2(largura + 10.0f, altura + 8.0f), kMagenta);
    }
    SeteSegmentos(texto, canto, altura, cor);
    if (n.momento == Momento::Reescrevendo) {
        // O cursor de quem edita, piscando depois do numero.
        if (Aceso(te, 0.25f)) Preencher(canto + Vector2(largura + 4.0f, 0.0f), canto + Vector2(largura + 8.0f, altura), kMagenta);
    }
    mLinhas = doCampo;
    // O fio: o professor puxa o numero da HUD ate ele, e o empurra de volta.
    if (n.momento == Momento::Indo || n.momento == Momento::Voltando || n.momento == Momento::NaPlaca) {
        Tracejada(GetPosition() + Vector2(0.0f, 40.0f), n.onde, kMagenta, 10.0f);
    }
    switch (n.momento) {
        case Momento::NaPlaca:      Placar(kFase, kEtapaDaAbertura, "hud.nota: este número é meu"); break;
        case Momento::Indo:         Placar(kFase, kEtapaDaAbertura, "hud.nota.pegar()"); break;
        case Momento::Reescrevendo: Placar(kFase, kEtapaDaAbertura, "nota = " + Inteiro(mNotaAntesDaEdicao) + " -> nota = " + Inteiro(n.valor)); break;
        case Momento::Voltando:     Placar(kFase, kEtapaDaAbertura, "hud.nota.devolver(15)"); break;
        case Momento::Devolvido:    break;
    }
}

void Ferreira::DevolverABarra() {
    MatarTodos(mBarra);
    if (FaixaDaNota* faixa = Faixa(); faixa && faixa->Solta()) faixa->Devolver();
}

void Ferreira::AtualizarBarra(const int etapa, const float te) {
    using namespace HudHostil;
    FaixaDaNota* faixa = Faixa();
    if (!faixa) return;
    const PoseDaBarra pose = Pose(etapa, te, CampoPuro(), faixa->CentroDoEncaixe(), faixa->ComprimentoNoEncaixe(), mDegrauDoHud);
    if (pose.noEncaixe && etapa == kEtapaDaBarraDePe) {
        // De volta ao rodape: a HUD tem a barra de novo.
        DevolverABarra();
        return;
    }
    faixa->Pousar(pose.centro, pose.comprimento, pose.angulo, pose.espelhada);
    mPoseDaBarra = pose;

    // O que machuca: um segmento sem cara embaixo de cada pedaco cheio do que a barra MOSTRA - a
    // nota que se ve, e nao a que ela ainda vai alcancar.
    const float mostrada = faixa->Mostrada();
    const int n = std::max(1, static_cast<int>(pose.comprimento / kSegmento));
    if (static_cast<int>(mBarra.size()) != n) {
        MatarTodos(mBarra);
        mBarra.assign(static_cast<size_t>(n), nullptr);
    }
    mSegmentosDaBarra = n;
    for (int k = 0; k < n; ++k) {
        BossProjectile*& p = mBarra[static_cast<size_t>(k)];
        if (!pose.perigo || !Cheio(k, n, mostrada, pose.espelhada)) {
            Matar(p);
            continue;
        }
        const Vector2 onde = CentroDoSegmento(pose, k, n);
        if (!p) p = Peca(kGatilhoDaBarra, onde, "Vazia");
        if (p) {
            p->SetPosition(onde);
            Parar(p);
        }
    }

    // O puxao: o professor a arranca do rodape.
    if (etapa == kEtapaDaBarraDeitada && te >= kTremorDaBarra * 0.4f && te < kTremorDaBarra + kArrancadaDaBarra) {
        Tracejada(GetPosition() + Vector2(0.0f, 40.0f), pose.centro, kOuro, 9.0f);
        Circulo(pose.centro, 20.0f, kOuro, 14);
    }
    if (!pose.perigo) return;

    // O guia: a faixa por onde se passa, do tamanho do pedaco vazio, e a linha onde o cheio acaba.
    const int cheios = Cheios(mostrada, n);
    if (cheios >= n) return;
    const int primeiro = pose.espelhada ? 0 : cheios;
    const int ultimo = pose.espelhada ? n - cheios - 1 : n - 1;
    const Vector2 a = CentroDoSegmento(pose, primeiro, n) - pose.eixo * (kSegmento / 2.0f);
    const Vector2 b = CentroDoSegmento(pose, ultimo, n) + pose.eixo * (kSegmento / 2.0f);
    const Vector2 corte = pose.espelhada ? b : a;
    const SDL_FRect c = Campo();
    if (std::fabs(pose.eixo.x) > 0.5f) {
        Preencher(Vector2(a.x, c.y), Vector2(b.x, c.y + c.h), kSeguro);
        Tracejada(Vector2(corte.x, c.y), Vector2(corte.x, c.y + c.h), kGuia, 10.0f);
    } else {
        Preencher(Vector2(c.x, a.y), Vector2(c.x + c.w, b.y), kSeguro);
        Tracejada(Vector2(c.x, corte.y), Vector2(c.x + c.w, corte.y), kGuia, 10.0f);
    }
}

void Ferreira::QuebrarABarra() {
    using namespace HudHostil;
    mBarraAcertada = false;
    mQuebrada = 0.0f;
    Battle* b = Batalha();
    FaixaDaNota* faixa = Faixa();
    const float mostrada = faixa ? faixa->Mostrada() : Nota();
    mCacos = Cacos(mPoseDaBarra, mSegmentosDaBarra, mostrada);
    mNotaAntesDaQuebra = Nota();
    // A barra sai de campo, e com ela o que o professor tinha atirado: agora so os cacos caem.
    MatarTodos(mBarra);
    for (BossProjectile*& p : mTiros) Matar(p);
    mTiros.clear();
    if (faixa) {
        mCorDosCacos = faixa->CorDoCheio();
        faixa->Quebrar();
    }
    if (b) {
        for (int k = 0; k < mSegmentosDaBarra; k += 2) {
            b->Faiscar(CentroDoSegmento(mPoseDaBarra, k, mSegmentosDaBarra), Hud::Faisca::Acerto);
        }
        // Os cacos nao machucam: o que sobrar no campo tambem nao, ate a luta acabar.
        b->CapturarAcertos(kQuebraDaBarra + 0.5f, [](Player&) {});
    }
    mClarao = 0.3f;
    mCorDoClarao = SDL_Color{255, 40, 60, 255};
    Avisar("hud.nota.destruir(): você quebrou a sua própria nota!", kQuebraDaBarra + 1.0f);
    SomDoDisparo();
}

void Ferreira::AtualizarQuebra(const float deltaTime) {
    using namespace HudHostil;
    mQuebrada += deltaTime;
    const SDL_Color cheio = mCorDosCacos;
    const SDL_Color vazio{120, 120, 150, 255};
    for (const Caco& c : mCacos) {
        const Vector2 centro = OndeDoCaco(c, mQuebrada);
        if (!NoCampo(centro, 40.0f)) continue;
        const float a = Math::ToRadians(c.giro * mQuebrada);
        const Vector2 u = Vector2(Math::Cos(a), Math::Sin(a)) * (c.lado / 2.0f);
        const Vector2 v(-u.y, u.x);
        const SDL_Color cor = c.cheio ? cheio : vazio;
        Linha(centro - u - v, centro + u - v, cor);
        Linha(centro + u - v, centro + u + v, cor);
        Linha(centro + u + v, centro - u + v, cor);
        Linha(centro - u + v, centro - u - v, cor);
        Linha(centro - u - v, centro + u + v, cor);
    }
    Placar(kFase, static_cast<size_t>(std::max(0, mEtapaAnterior)), "hud.nota = null");
    Battle* b = Batalha();
    if (!b || !b->EmCombate()) return;
    if (QuebraAcabou(mQuebrada)) {
        b->DestruirANota(kFraseDaNotaDestruida);
    } else {
        b->DefinirNota(NotaNaQuebra(mNotaAntesDaQuebra, mQuebrada));
    }
}

// ---- Fase 1, a ultima etapa: o inimigo e a IA dele ----

int Ferreira::BlocosVivos() const {
    int n = 0;
    for (const BossProjectile* p : mCorpo) n += Vivo(p) ? 1 : 0;
    return n;
}

void Ferreira::MontarOInimigo() {
    using namespace IaDoInimigo;
    MatarTodos(mCorpo);
    for (const Vector2& lugar : LugaresDosBlocos()) {
        BossProjectile* p = Peca(HudHostil::kGatilhoDoInimigo, mCentroDoInimigo + lugar, "Textura");
        // Nascendo, os blocos ainda se juntam: nao machucam.
        if (p) p->DefinirEnfeite(true);
        mCorpo.push_back(p);
    }
    for (BossProjectile*& o : mOlhos) Matar(o);
    PorOsOlhos();
    mCorpoMontado = true;
}

void Ferreira::PorOsOlhos() {
    using namespace IaDoInimigo;
    const auto lugares = LugaresDosOlhos();
    for (size_t k = 0; k < mOlhos.size() && k < lugares.size(); ++k) {
        if (Vivo(mOlhos[k])) continue;
        mOlhos[k] = Peca(HudHostil::kGatilhoDoOlho, mCentroDoInimigo + lugares[k], "Olho");
        if (mOlhos[k]) mOlhos[k]->DefinirEnfeite(!Machuca(mIa.estado));
    }
}

void Ferreira::DesmontarOInimigo() {
    // O que ele pagou e ainda nao entrou na nota entra agora.
    if (mPremioDoInimigo > 0.0f) {
        if (Battle* b = Batalha()) b->Conquistar(mPremioDoInimigo);
        mPremioDoInimigo = 0.0f;
    }
    MatarTodos(mCorpo);
    for (BossProjectile*& o : mOlhos) Matar(o);
    Matar(mPoder);
    mCorpoMontado = false;
    mIa = IaDoInimigo::Ia{};
    mVelocidadeDoInimigo = Vector2::Zero;
    mOlhoAtingido = false;
    mApanhava = {};
    mProximoPoder = -1.0f;
    mMarca.clear();
    mMarcaAte = -1.0f;
}

void Ferreira::AoTrocarDeEstado(const IaDoInimigo::Estado de, const IaDoInimigo::Estado para, const float te) {
    using namespace IaDoInimigo;
    Battle* b = Batalha();
    if (para == Estado::Alerta) {
        mMarca = "!";
        mMarcaAte = mRelogio + kAlerta;
        SomDoDisparo();
    }
    if (de == Estado::Persegue && para == Estado::Patrulha) {
        mMarca = "?";
        mMarcaAte = mRelogio + 1.0f;
    }
    if (para == Estado::Atordoado) {
        mClarao = 0.1f;
        mCorDoClarao = SDL_Color{255, 230, 120, 255};
    }
    // Os olhos voltam quando ele acorda.
    if (de == Estado::Atordoado) PorOsOlhos();
    if (para == Estado::Fugindo) {
        mClarao = 0.15f;
        mCorDoClarao = SDL_Color{255, 214, 90, 255};
        Avisar(mFase == RegrasTrocadas::kFase ? "regras.inverter(): o fantasma virou moeda - pegue!"
                                               : "powerUp.usar(): agora ELE foge de você!",
               1.3f);
        SomDoDisparo();
    }
    if (de == Estado::Fugindo) {
        // O power-up acabou (ou ele foi derrotado): o proximo vem depois de um respiro.
        mProximoPoder = te + mDegrauDoInimigo.entrePoderes;
        PorOsOlhos();
    }
    if (para == Estado::Derrotado) {
        if (b) b->Conquistar(mPremioDoInimigo + kPremioPorDerrotar);
        mPremioDoInimigo = 0.0f;
        for (BossProjectile*& o : mOlhos) {
            if (Vivo(o) && b) b->Faiscar(o->GetPosition(), Hud::Faisca::Coleta);
            Matar(o);
        }
        MatarTodos(mCorpo);
        mCorpoMontado = false;
        mClarao = 0.25f;
        mCorDoClarao = SDL_Color{255, 214, 90, 255};
        Avisar("inimigo.destruir(): + " + Inteiro(kPremioPorDerrotar) + " de bônus", 1.6f);
        SomDoDisparo();
    }
}

void Ferreira::AtualizarInimigo(const float te, const float deltaTime, const ComoOInimigo& como) {
    using namespace IaDoInimigo;
    const IaDoInimigo::Degrau& g = mDegrauDoInimigo;
    const JogosDigitais::Campo campo = CampoPuro();
    const auto ronda = Ronda(campo);
    Battle* b = Batalha();
    const std::vector<Player*> alunos = Jogadores();

    // O alvo da IA: o aluno mais perto.
    const Player* alvo = nullptr;
    float distancia = 1e9f;
    for (const Player* a : alunos) {
        const float d = (a->GetPosition() - mCentroDoInimigo).Length();
        if (d < distancia) {
            distancia = d;
            alvo = a;
        }
    }
    const Vector2 ondeAluno = alvo ? alvo->GetPosition() : PosicaoDoAluno();

    // Nascendo, o corpo aparece no canto mais longe do aluno.
    if (mIa.estado == Estado::Nascendo && !mCorpoMontado) {
        mCantoDaRonda = Ninho(campo, ondeAluno);
        mCentroDoInimigo = ronda[static_cast<size_t>(mCantoDaRonda)];
        distancia = alunos.empty() ? 1e9f : (ondeAluno - mCentroDoInimigo).Length();
        MontarOInimigo();
        mClarao = 0.08f;
        mCorDoClarao = SDL_Color{255, 60, 230, 255};
        SomDoDisparo();
    }
    if (como.fase1 && mProximoPoder < 0.0f) mProximoPoder = g.primeiroPoder;

    // O power-up: nasce longe dele e espera; quem encosta faz ele fugir. Enquanto ele nasce, ou
    // depois de derrotado, o power-up espera.
    bool pegou = false;
    if (Vivo(mPoder)) {
        const bool valeAgora = mIa.estado != Estado::Nascendo && mIa.estado != Estado::Derrotado;
        for (const Player* a : alunos) {
            if (!valeAgora || (a->GetPosition() - mPoder->GetPosition()).Length() > RaioDe(a) + kToque + 6.0f) continue;
            pegou = true;
            if (b) b->Faiscar(mPoder->GetPosition(), Hud::Faisca::Coleta);
            Matar(mPoder);
            break;
        }
    } else if (como.fase1 && te >= mProximoPoder && mIa.estado != Estado::Fugindo &&
               te < HudHostil::kDuracoes[HudHostil::kEtapaDoInimigo] - 1.5f) {
        mPoder = Peca(HudHostil::kGatilhoDoPoder, LugarDoPoder(mSorteio, campo, mCentroDoInimigo), "Poder");
        if (mPoder) mPoder->DefinirEnfeite(true);
        mProximoPoder = 1e9f;
    }

    // O corpo acabou de acertar alguem: o aluno que comecou a piscar agora, encostado nele.
    bool acertou = false;
    for (size_t i = 0; i < alunos.size() && i < mApanhava.size(); ++i) {
        const bool apanha = alunos[i]->AcabouDeApanhar();
        const Vector2 d = alunos[i]->GetPosition() - mCentroDoInimigo;
        if (apanha && !mApanhava[i] && std::fabs(d.x) <= kMeiaLargura + 10.0f && std::fabs(d.y) <= kMeiaAltura + 10.0f) {
            acertou = true;
        }
        mApanhava[i] = apanha;
    }

    // A IA percebe e decide.
    Percepcao p;
    p.distancia = distancia;
    // O susto de fora (a inversao) faz o mesmo que o power-up, com o tempo dele.
    p.pegouOPoder = pegou || como.susto > 0.0f;
    p.susto = pegou ? 0.0f : como.susto;
    p.olhoAtingido = mOlhoAtingido;
    p.acertouOAluno = acertou && Machuca(mIa.estado);
    p.blocos = mCorpoMontado ? BlocosVivos() : 0;
    mOlhoAtingido = false;
    const Estado antes = mIa.estado;
    if (Passo(mIa, p, deltaTime, g)) {
        AoTrocarDeEstado(antes, mIa.estado, te);
        // Quem acertou o aluno recua: a ronda segue para o canto mais longe dele.
        if (p.acertouOAluno && mIa.estado == Estado::Patrulha) {
            mCantoDaRonda = (Ninho(campo, ondeAluno) + 3) % 4;
            mMarca.clear();
        }
    }

    // Onde ele anda: a ronda pelos cantos, atras do aluno, ou para longe dele.
    Vector2 rumo = Vector2::Zero;
    if (mIa.estado == Estado::Patrulha) {
        const Vector2 proximo = ronda[static_cast<size_t>((mCantoDaRonda + 1) % 4)];
        if ((proximo - mCentroDoInimigo).Length() < 14.0f) mCantoDaRonda = (mCantoDaRonda + 1) % 4;
        rumo = JogosDigitais::Rumo(mCentroDoInimigo, ronda[static_cast<size_t>((mCantoDaRonda + 1) % 4)]);
    } else if (mIa.estado == Estado::Persegue) {
        rumo = JogosDigitais::Rumo(mCentroDoInimigo, ondeAluno);
    } else if (mIa.estado == Estado::Fugindo) {
        const JogosDigitais::Campo arena{ronda[0].x, ronda[0].y, ronda[2].x - ronda[0].x, ronda[2].y - ronda[0].y};
        rumo = JogosDigitais::Fugir(mCentroDoInimigo, ondeAluno, arena, 10.0f);
    }
    // No FPS ele anda sem deltaTime: o passo dele e o do quadro.
    mVelocidadeDoInimigo = rumo * (Rapidez(mIa, g) * como.passo);
    if (mCorpoMontado) mCentroDoInimigo = NaArena(mCentroDoInimigo + mVelocidadeDoInimigo * deltaTime, campo);

    // O corpo e os olhos no lugar. Nascendo, os blocos vem do meio para o lugar; no alerta, ele treme.
    const bool fugindo = mIa.estado == Estado::Fugindo;
    const bool pisca = Pisca(mIa, mRelogio);
    const bool machuca = Machuca(mIa.estado);
    const float junta = mIa.estado == Estado::Nascendo ? std::clamp(mIa.noEstado / kNascendo, 0.0f, 1.0f) : 1.0f;
    // O fantasma flutua - o corpo sobe e desce, e a barra ondula embaixo.
    const Vector2 tremor = (mIa.estado == Estado::Alerta ? Vector2(4.0f * Math::Sin(mRelogio * 70.0f), 0.0f) : Vector2::Zero) +
                           Vector2(0.0f, Flutua(mRelogio));
    const auto lugares = LugaresDosBlocos();
    for (size_t k = 0; k < mCorpo.size() && k < lugares.size(); ++k) {
        BossProjectile* bloco = mCorpo[k];
        if (!bloco) continue;
        const Vector2 onda(0.0f, NaBarra(lugares[k]) ? Onda(Coluna(lugares[k]), mRelogio) : 0.0f);
        bloco->SetPosition(mCentroDoInimigo + tremor + onda + lugares[k] * (0.2f + 0.8f * junta));
        // Nas regras ele vive mais que um tiro (Projectile::kTempoDeVidaMaximo): o corpo nao expira.
        bloco->RenovarTempoDeVida();
        Andar(bloco, mVelocidadeDoInimigo);
        Cara(bloco, fugindo && !pisca ? "Moeda" : "Textura");
        bloco->DefinirEnfeite(!machuca);
    }
    const auto lugaresDosOlhos = LugaresDosOlhos();
    for (size_t k = 0; k < mOlhos.size() && k < lugaresDosOlhos.size(); ++k) {
        BossProjectile* olho = mOlhos[k];
        if (!olho) continue;
        olho->SetPosition(mCentroDoInimigo + tremor + lugaresDosOlhos[k] * (0.2f + 0.8f * junta));
        olho->RenovarTempoDeVida();
        Andar(olho, mVelocidadeDoInimigo);
        const float dx = ondeAluno.x - olho->GetPosition().x;
        const bool tonto = fugindo || mIa.estado == Estado::Atordoado;
        Cara(olho, tonto ? "Assustado" : (dx < -40.0f ? "OlhoEsquerda" : (dx > 40.0f ? "OlhoDireita" : "Olho")));
        // Fugindo, o olho e moeda tambem: o tiro passa.
        olho->DefinirEnfeite(!machuca);
    }

    // Fugindo, cada bloco que o aluno encosta e dele.
    if (fugindo) {
        for (BossProjectile*& bloco : mCorpo) {
            if (!bloco) continue;
            for (const Player* a : alunos) {
                if ((bloco->GetPosition() - a->GetPosition()).Length() > RaioDe(a) + kToque) continue;
                if (b) b->Faiscar(bloco->GetPosition(), Hud::Faisca::Coleta);
                mPremioDoInimigo += kPremioPorBloco;
                Matar(bloco);
                break;
            }
        }
        if (mPremioDoInimigo >= 0.75f) {
            if (b) b->Conquistar(mPremioDoInimigo);
            mPremioDoInimigo = 0.0f;
        }
    }

    // A IA a mostra, como no modo debug de quem faz o jogo: o raio de visao na ronda, a linha ate o
    // alvo na perseguicao, o tempo do power-up na fuga, as estrelas do atordoado.
    const Vector2 eu = mCentroDoInimigo;
    if (mIa.estado == Estado::Patrulha) {
        for (int k = 0; k < 40; k += 2) {
            const float a0 = Math::TwoPi * static_cast<float>(k) / 40.0f, a1 = Math::TwoPi * static_cast<float>(k + 1) / 40.0f;
            Linha(eu + Vector2(Math::Cos(a0), Math::Sin(a0)) * g.raioDeVisao, eu + Vector2(Math::Cos(a1), Math::Sin(a1)) * g.raioDeVisao,
                  kMagentaFraco);
        }
    } else if (mIa.estado == Estado::Persegue && alvo) {
        Tracejada(eu, ondeAluno, kMagentaFraco, 10.0f);
    } else if (fugindo) {
        const float u = std::clamp(mIa.susto / std::max(g.susto, 0.1f), 0.0f, 1.0f);
        const Vector2 a(eu.x - kMeiaLargura, eu.y + kMeiaAltura + 10.0f), z(eu.x + kMeiaLargura, eu.y + kMeiaAltura + 16.0f);
        Retangulo(a, z, kOuro);
        Preencher(a, Vector2(Mistura(a.x, z.x, u), z.y), kOuro);
        for (const Player* aluno : alunos) Circulo(aluno->GetPosition(), 24.0f + 2.0f * Math::Sin(mRelogio * 10.0f), kOuro, 16);
    } else if (mIa.estado == Estado::Atordoado) {
        for (int k = 0; k < 3; ++k) {
            const float a = mRelogio * 5.0f + Math::TwoPi * static_cast<float>(k) / 3.0f;
            Circulo(eu + Vector2(Math::Cos(a) * 46.0f, -kMeiaAltura - 8.0f + Math::Sin(a) * 9.0f), 5.0f, kOuro, 6);
        }
    } else if (mIa.estado == Estado::Derrotado) {
        // O respawn: o contorno do corpo no canto onde ele vai nascer.
        const Vector2 ninho = ronda[static_cast<size_t>(Ninho(campo, ondeAluno))];
        if (Aceso(te, 0.25f)) {
            const Vector2 a = ninho - Vector2(kMeiaLargura, kMeiaAltura), z = ninho + Vector2(kMeiaLargura, kMeiaAltura);
            Tracejada(a, Vector2(z.x, a.y), kMagenta);
            Tracejada(Vector2(z.x, a.y), z, kMagenta);
            Tracejada(z, Vector2(a.x, z.y), kMagenta);
            Tracejada(Vector2(a.x, z.y), a, kMagenta);
        }
    }
    if (Vivo(mPoder)) Circulo(mPoder->GetPosition(), 20.0f + 4.0f * Math::Sin(mRelogio * 8.0f), kOuro, 14);

    // O estado em cima dele (ou o "!" e o "?" de um instante), e o rotulo do power-up.
    const bool comCorpo = mCorpoMontado && mIa.estado != Estado::Derrotado;
    EtiquetaDeCodigo* estado = Etiqueta(mEtiquetasDoInimigo, 0, 150, 26);
    if (estado && comCorpo) {
        const bool marca = mRelogio < mMarcaAte && !mMarca.empty();
        const SDL_Color tinta = marca || mIa.estado == Estado::Persegue || mIa.estado == Estado::Alerta
                                    ? SDL_Color{255, 110, 110, 255}
                                    : (fugindo ? SDL_Color{255, 214, 90, 255}
                                               : (mIa.estado == Estado::Atordoado ? SDL_Color{255, 240, 150, 255}
                                                                                   : SDL_Color{150, 255, 170, 255}));
        estado->Pintar(SDL_Color{255, 60, 230, 160}, SDL_Color{10, 12, 24, 215}, tinta);
        Escrever(estado, marca ? mMarca : std::string(Nome(mIa.estado)), eu + Vector2(0.0f, -kMeiaAltura - 22.0f));
    } else if (estado) {
        estado->Mostrar(false);
    }
    EtiquetaDeCodigo* rotulo = Etiqueta(mEtiquetasDoInimigo, 1, 120, 24);
    if (rotulo && Vivo(mPoder)) {
        rotulo->Pintar(SDL_Color{255, 214, 90, 160}, SDL_Color{10, 12, 24, 215}, SDL_Color{255, 214, 90, 255});
        Escrever(rotulo, "POWER-UP", mPoder->GetPosition() + Vector2(0.0f, -34.0f));
    } else if (rotulo) {
        rotulo->Mostrar(false);
    }

    // Na fase 1, o professor continua: os leques, menos que no resto da prova. Nas regras, a etapa
    // e quem atira, e o placar e dela.
    if (!como.fase1) return;
    if (te >= 1.0f && te >= mProximoLeque) {
        mProximoLeque = te + mDegrauDoHud.entreLeques * 1.4f;
        Leque(HudHostil::kGatilhoDoLeque, mDegrauDoHud.tirosDoLeque, 26.0f, mDegrauDoHud.velocidade, "Tiro");
    }

    std::string texto;
    switch (mIa.estado) {
        case Estado::Nascendo:  texto = "new Inimigo(): a IA dele é uma máquina de estados"; break;
        case Estado::Patrulha:  texto = "PATRULHA: fique fora do raio de visão dele"; break;
        case Estado::Alerta:    texto = "ALERTA (!): ele te viu - CORRA!"; break;
        case Estado::Persegue:
            texto = Vivo(mPoder) ? "PERSEGUE: pegue o POWER-UP!" : "PERSEGUE: atire no OLHO dele, o ponto fraco";
            break;
        case Estado::Atordoado: texto = "ATORDOADO: o olho era o ponto fraco"; break;
        case Estado::Fugindo:
            texto = pisca || mIa.susto <= kPiscaNoFim ? "FUGINDO... o power-up está acabando!"
                                                      : "FUGINDO: encoste nos blocos - cada um é nota!";
            break;
        case Estado::Derrotado:
            texto = "inimigo.destruir(): respawn em " + std::to_string(static_cast<int>(std::ceil(kRespawn - mIa.noEstado)));
            break;
    }
    Placar(HudHostil::kFase, HudHostil::kEtapaDoInimigo, texto);
}
