//
// Created by nslop on 16/09/2024.
//

#include "Battle.h"

#include "../../Aluno/Progresso.h"
#include "../../Regras/Nota.h"

#include <random>

#include "BattleHUD.h"
#include "DesenhoSobreOsAlunos.h"
#include "EfeitosDaBatalha.h"
#include "FaixaDaNota.h"
#include "MolduraDoCampo.h"
#include "PausaDaBatalha.h"
#include "PlacarDaFase.h"
#include "RelogioDaFase.h"
#include "ProjectileManager.h"
#include "TelaDoResultado.h"
#include "../../Motor/AudioSystem.h"
#include "../../Motor/Font.h"
#include "../../Interface/Pausa.h"
#include "../../Interface/Resultado.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teachers/Boss.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Actors/Teachers/BossFactory/IBossFactory.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Actors/Teachers/ExtraPointItem.h"
#include "../../Regras/PontoExtra.h"
#include "../../Regras/Raspao.h"
#include "../../Regras/Cobertura.h"
#include "../../Aluno/Matricula.h"
#include "../../Regras/Destrutivel.h"
#include "../../Actors/Orientador/Orientador.h"
#include "../../Curso/MateriasArquivo.h"

#include "../../Motor/CaminhosArquivo.h"
#include "../../Motor/Fontes.h"
#include "DevolutivaDaBatalha.h"
#include "RegistroDaTentativa.h"
#include "RelatorioDaSimulacao.h"
#include "ClaroesDaMoldura.h"
#include "TelaDeCarregamento.h"
#include "../../Components/AIComponents/FSMComponent.h"

// Quantas instâncias de cada tipo de projétil são pré-aquecidas no pool antes da batalha começar.

static constexpr int kPreAquecidosPorTipo = 300;

namespace {

/// O colisor de um jogador, ou nullptr se nao houver jogador ou colisor.
const CircleColliderComponent* ColisorDe(const Player* jogador) {
    return jogador ? jogador->GetComponent<CircleColliderComponent>() : nullptr;
}

} // namespace

Battle::Battle(Game* game, const int selectedStage)
    : Scene(game, SceneType::Battle)
    , mConta(Progresso::kNotaInicial)
    , mMateria(selectedStage)
{
    if (game) mCondicao = game->Condicao();
    mMira = Papeis::Mira(mCondicao.Ligada(Experimento::Mecanica::Papeis));
}

Battle::~Battle() {
    Avisar([&](OuvinteDaBatalha& o) { o.AoTerminar(*this, Tentativas::Resultado::Abandonou, false); });
}

//region LoadFunctions
void Battle::Load() {

    // 1.
    mProjeteis = std::make_unique<ProjectileManager>(this);

    // O registro da tentativa ouve a batalha desde o comeco.
    mOuvintes.push_back(std::make_unique<RegistroDaTentativa>());
    // E o relatorio da simulacao, quando pedido (--relatorio).
    if (mGame->ConfiguracaoDaSimulacao().relatorio) {
        mOuvintes.push_back(std::make_unique<RelatorioDaSimulacao>(mGame->ConfiguracaoDaSimulacao().sairNoFim));
    }
    // E a devolutiva, que a tela do resultado mostra no fim.
    auto devolutiva = std::make_unique<DevolutivaDaBatalha>(mGame->QuantosJogadores(), mCondicao);
    mDevolutiva = devolutiva.get();
    mOuvintes.push_back(std::move(devolutiva));

    // Toda batalha comeca em 40, mesmo que o aluno ja tenha tirado mais nesta materia, e sem os
    // acertos de ontem: quem foi atingido ontem nao perde a nota cheia de hoje.
    mConta = Nota::Conta(Progresso::kNotaInicial, mGame->QuantosJogadores());
    mAvisoDeFaseLimpa = 0.0f;

    // O modo e a semente antes do chefe: a fabrica e o chefe podem perguntar pelos dois.
    mNivel = mGame->NivelDeDificuldade();
    // A simulacao prende a semente da luta (--variacao, ou a --semente); o jogo, nao.
    const auto& sim = mGame->ConfiguracaoDaSimulacao();
    mSemente = sim.variacao ? *sim.variacao
             : sim.semente  ? *sim.semente
                            : static_cast<std::uint32_t>(std::random_device{}());
    SDL_Log("BATALHA: semente %u, modo %s", mSemente, Dificuldade::Chave(mNivel));

    // 2.
    CriarJogadores();
    CriarChefe();
    CriarOrientador();
    CriarHUD();
    // A base e privada: a conversao para as regras so vale aqui dentro, e nao no make_unique.
    Colisoes::Regras& regras = *this;
    mColisoes = std::make_unique<Colisoes>(mJogadores, mCorpos, mPontosExtras, *mEfeitos, regras);
    mDesenhoSobreOsAlunos = std::make_unique<DesenhoSobreOsAlunos>(*this);
    mPausa = std::make_unique<PausaDaBatalha>(*this, mFonteDaNota.get(), mGame->JeitoDoRodape(), GetPlayfieldBounds());

    // Tocando desde o Load, ela comecava por cima de uma tela parada.

    // 3.
    ComecarCarregamento();

}
void Battle::CriarChefe() {

    if (IBossFactory *factory = mGame->GetFactory(mMateria)) {

        auto boss = factory->CreateBoss(this);
        mChefe = dynamic_cast<Boss*>(this->AddActor(std::move(boss)));

        // O original e o primeiro corpo.
        mCorpos.clear();
        if (mChefe) mCorpos.push_back(mChefe);

        mChefe->SetPosition(Vector2(static_cast<float>(mGame->GetWindowWidth())/2.0f,
                                        static_cast<float>(mGame->GetWindowHeight())/6.0f));
    } else {
        // %d, e nao %s: mMateria e o enum int, ou seja um inteiro.
        SDL_Log("Erro fatal: Nenhuma BossFactory encontrada para a materia %d, "
                "voltando pra StageSelect", static_cast<int>(mMateria));
        mGame->RequestSceneChange(SceneType::StageSelect);
    }
}
void Battle::CriarOrientador() {

    mOrientador = nullptr;

    // So na materia que pede orientacao.
    const Materias::Lista& materias = Materias::Carregadas();
    const Materias::Materia* aqui = materias.Por(mMateria);
    if (!aqui || !aqui->orientacao) return;

    const int materia = mGame->Orientador();
    if (materia < 0) return;
    const Materias::Materia* dele = materias.Por(materia);
    IBossFactory* fabrica = mGame->GetFactory(static_cast<size_t>(materia));
    if (!dele || !fabrica) {
        SDL_Log("ORIENTADOR: a materia %d nao tem professor com batalha - o TCC segue sem orientador.",
                materia);
        return;
    }

    auto orientador = std::make_unique<Orientador>(this, dele->professor,
                                                   Caminhos::Asset(fabrica->Retrato() + ".png"));
    mOrientador = dynamic_cast<Orientador*>(AddActor(std::move(orientador)));
    if (mOrientador) mOrientador->Pousar();
}

void Battle::CriarHUD() {

    // Cria a HUD para segurar todas as infos.
    mHUD = std::make_unique<BattleHUD>(this);
    mMoldura = std::make_unique<MolduraDoCampo>(*this, GetPlayfieldBounds(), mGame->GetWindowWidth(),
                                                mGame->GetWindowHeight(), mJogadores);
    mEfeitos = std::make_unique<EfeitosDaBatalha>(*this);
    mOuvintes.push_back(std::make_unique<ClaroesDaMoldura>(mMoldura->Desenho()));

    // A capsula do tempo segue o professor, e o BattleHUD a esvazia.
    if (mChefe) {
        mRelogio = std::make_unique<RelogioDaFase>(*this, *mChefe);
        mHUD->SetTimeBar(mRelogio->Capsula(), mRelogio->DesenhoDaCapsula());
    } else {
        SDL_Log("ERRO: CriarHUD chamado antes do mChefe ser criado!");
    }
    // A fonte da faixa e tambem a dos menus e da tela de carregamento.
    mFonteDaNota = std::make_unique<Font>();
    mFonteDaNota->Load(Caminhos::Asset(Fontes::kJogo));
    mFaixa = std::make_unique<FaixaDaNota>(*this, mFonteDaNota.get(), mGame->GetWindowWidth(),
                                           mGame->GetWindowHeight(), mConta.Valor(), mNivel);
    // O quadro da sala, no canto de cima a esquerda.
    mFonteDoPlacar = std::make_unique<Font>();
    mFonteDoPlacar->Load(Caminhos::Asset(Fontes::kPlacar));
    mPlacar = std::make_unique<PlacarDaFase>(*this, mFonteDoPlacar.get(), mMoldura->AreaDoAluno().esquerda);
    CriarIntercambio();

}
void Battle::CriarJogadores() {

    // Um aluno por sessao.
    const size_t quantos = mGame->QuantosJogadores();

    const SDL_FRect campo = GetPlayfieldBounds();

    for (size_t quem = 0; quem < quantos; ++quem) {

        auto player = std::make_unique<Player>(this, quem);

        // Nasce dentro do campo, e nao numa fracao da janela.
        const auto alturaDoAluno = static_cast<float>(
            player->GetComponent<DrawAnimatedComponent>()->GetSpriteHeight());

        // Lado a lado, repartindo a largura em partes iguais.
        const float fatia = campo.w / static_cast<float>(quantos + 1);
        const float x = campo.x + fatia * static_cast<float>(quem + 1);
        const float y = campo.y + campo.h - alturaDoAluno;

        player->SetPosition(Vector2(x, y));
        mJogadores.push_back(dynamic_cast<Player*>(this->AddActor(std::move(player))));
    }
}
void Battle::AtualizarPlacar() {
    if (!mPlacar || !mChefe) return;

    // FaseAtual, e nao o nome que a maquina de estados acabou de trocar.
    mPlacar->Mostrar(Intercambio::Avisando(mRelogioDoIntercambio) ? mChefe->AvisoDoIntercambio()
                                                                  : mChefe->AnuncioDaFase(mChefe->FaseAtual()));
}

void Battle::CriarIntercambio() {

    // Um marcador por aluno, do tamanho dele.
    for (const Player* jogador : mJogadores) {
        if (!jogador) continue;
        const auto* sprite = jogador->GetComponent<DrawAnimatedComponent>();
        const int largura = sprite ? sprite->GetSpriteWidth()  : 32;
        const int altura  = sprite ? sprite->GetSpriteHeight() : 48;

        auto marcador = std::make_unique<Actor>(this);
        auto contorno = marcador->AddComponent<DrawCaixaComponent>(
            largura + 8, altura + 8, SDL_Color{235, 235, 225, 220}, SDL_Color{0, 0, 0, 0},
            2, kOrdemDoMarcador);
        contorno->SetIsVisible(false);

        mMarcadoresDoIntercambio.push_back(marcador.get());
        this->AddActor(std::move(marcador));
    }
}

void Battle::AtualizarIntercambio(const float deltaTime) {
    if (!mChefe) return;

    // O relogio e da fase.
    const std::string& fase = mChefe->FaseAtual();
    if (fase != mFaseDoIntercambio) {
        mFaseDoIntercambio = fase;
        mRelogioDoIntercambio = Intercambio::Relogio{};
        if (const auto ic = mChefe->IntercambioDaFase(fase)) {
            mRelogioDoIntercambio.aCada = ic->first;
            mRelogioDoIntercambio.aviso = ic->second;
        }
    }

    const bool trocou = Intercambio::Avancar(mRelogioDoIntercambio, deltaTime);

    // O destino e recalculado todo quadro, e nao congelado quando o aviso acende.
    std::vector<Vector2> posicoes;
    for (const Player* jogador : mJogadores) {
        if (jogador) posicoes.push_back(jogador->GetPosition());
    }
    const auto destinos = Intercambio::Destinos(
        posicoes, static_cast<float>(mGame->GetWindowWidth()));

    if (trocou) {
        for (size_t i = 0; i < mJogadores.size() && i < destinos.size(); ++i) {
            if (mJogadores[i]) mJogadores[i]->SetPosition(destinos[i]);
        }
        // O chefe fica sabendo, depois que os alunos ja estao no lugar novo.
        mChefe->OnIntercambio();
    }

    const bool aceso = Intercambio::Avisando(mRelogioDoIntercambio);
    for (size_t i = 0; i < mMarcadoresDoIntercambio.size(); ++i) {
        Actor* marcador = mMarcadoresDoIntercambio[i];
        if (!marcador) continue;
        if (i < destinos.size()) marcador->SetPosition(destinos[i]);
        marcador->GetComponent<DrawCaixaComponent>()->SetIsVisible(aceso && i < destinos.size());
    }
}

void Battle::ComecarCarregamento() {

    mTiposACarregar.clear();

    if (mChefe) {
        // Pega exatamente os nomes que este Boss registrou.
        mTiposACarregar = mChefe->GetProjectileFactoryNames();
    }

    mTiposNoTotal = static_cast<int>(mTiposACarregar.size());

    mCarregando = true;

    // Pausa o Player durante o carregamento.
    for (Player* jogador : mJogadores) {
        if (jogador) jogador->SetState(ActorState::Paused);
    }

    mTelaDeCarregamento = std::make_unique<TelaDeCarregamento>(*this, Materias::Carregadas().Por(mMateria), mNivel,
                                                               mFonteDaNota.get(), mFonteDoPlacar.get());
}

void Battle::AvancarCarregamento(const float deltaTime) {

    // Processa um tipo de projétil inteiro nesta chamada.
    if (!mTiposACarregar.empty()) {
        const std::string nextName = mTiposACarregar.back();
        mTiposACarregar.pop_back();
        if (ProjectileFactory* factory = mChefe ? mChefe->GetProjectileFactory(nextName) : nullptr) {
            factory->Prewarm(this, mChefe, kPreAquecidosPorTipo);
        }
    }

    // O primeiro quadro carrega o Load inteiro no deltaTime.
    const float feito = mTiposNoTotal > 0
        ? static_cast<float>(mTiposNoTotal - static_cast<int>(mTiposACarregar.size())) /
          static_cast<float>(mTiposNoTotal)
        : 1.0f;
    mTelaDeCarregamento->Avancar(feito, std::min(deltaTime, 0.05f));
    if (mTelaDeCarregamento->Acabou()) TerminarCarregamento();
}

void Battle::TerminarCarregamento() {

    mCarregando = false;

    mTelaDeCarregamento->Remover();
    mTelaDeCarregamento.reset();

    if (mGame->GetAudio() && !mMusica.IsValid()) {
        mMusica = mGame->GetAudio()->PlaySound("backgroundmusic.ogg", true);
        mGame->GetAudio()->SetSoundVolume(mMusica, 32);
    }

    // Reativa o Player, pausado em ComecarCarregamento().
    for (Player* jogador : mJogadores) {
        if (jogador) jogador->SetState(ActorState::Active);
    }

    Avisar([&](OuvinteDaBatalha& o) { o.AoComecar(*this); });

    if (mChefe) {
        mChefe->Start();
    }
}

//endregion LoadFunctions
//region UpdateFunctions
void Battle::OnUpdate(float deltaTime) {

    // ---- A simulacao (Simulacao.h): a pausa, o foco e o fim, num quadro dado ----
    {
        const auto& sim = mGame->ConfiguracaoDaSimulacao();
        const int quadro = mGame->Quadro();
        if (quadro == sim.pausaNoQuadro) AbrirPausa();
        if (quadro == sim.focoNoQuadro) AoPerderOFoco();
        if (quadro == sim.fimNoQuadro) {
            if (sim.fimComNota) mConta.Definir(*sim.fimComNota);
            EncerrarBatalha(sim.fimAprovado);
        }
    }

    // ---- Tela de Carregamento (pré-aquecimento dos pools de projétil) ----
    if (mCarregando) {
        AvancarCarregamento(deltaTime);
        return;
    }

    // ---- A pausa: a cena ja nao atualizou os atores (Scene::mAtoresCongelados), ----
    if (mPausa->Parada()) {
        Avisar([&](OuvinteDaBatalha& o) { o.AoPassarOTempo(*this, deltaTime, true); });
        if (mPausa->Avancar(deltaTime)) Retomar();
        return;
    }

    // ---- O fim da luta: o painel do resultado esta na tela (EncerrarBatalha) ----
    if (mAcabando) {
        // O ClearBossProjectiles() de EncerrarBatalha apenas marca os projeteis.
        if (mProjeteis) mProjeteis->Cleanup();
        if (mResultado) mResultado->Avancar(deltaTime);
        return;
    }

    // A cena base já atualiza todos os atores.
    Avisar([&](OuvinteDaBatalha& o) { o.AoPassarOTempo(*this, deltaTime, false); });
    mRelogioDaLuta += deltaTime;

    // Atualiza os sistemas principais apenas da Battle (os atores são atualizados pela Scene)
    if (mProjeteis) mProjeteis->Update(deltaTime);

    // Orquestra a checagem de colisões.
    VerificarColisoes();
    mMira.Avancar(deltaTime);
    mDesenhoSobreOsAlunos->Atualizar(deltaTime, mJogadores, FocoDaMira(), mGame && mGame->MostrarHitbox());
    MarcarOCaminhoDosAlunos();
    if (mMoldura) mMoldura->Atualizar(mJogadores, mCorpos, mRede, mFonteDoPlacar.get());
    mEfeitos->AvancarOndas(deltaTime, mProjeteis->GetBossProjectiles());
    mEfeitos->Desenhar(deltaTime, mFaixa->Ponta(mConta.Valor()));

    if (mHUD) mHUD->OnUpdate(deltaTime, mChefe && mChefe->Congelado());
    if (mRede > 0.0f) mRede = std::max(0.0f, mRede - deltaTime);

    if (mRelogio && mChefe && mHUD) mRelogio->Seguir(*mChefe, mHUD->Restante());
    mFaixa->Atualizar(mConta.Valor(), mAvisoDeFaseLimpa > 0.0f);

    AtualizarIntercambio(deltaTime);
    AtualizarPlacar();

    // Purga os ponteiros observadores de pontos extras.
    mPontosExtras.erase(std::remove_if(mPontosExtras.begin(), mPontosExtras.end(),
        [](const ExtraPointItem* item) {
            return item->GetState() == ActorState::Destroy;
        }), mPontosExtras.end());

    if (mAvisoDeFaseLimpa > 0.0f) mAvisoDeFaseLimpa -= deltaTime;
    Avisar([&](OuvinteDaBatalha& o) { o.AoFimDoQuadro(*this); });

    // Se a nota cair a 0, termina a fase.
    if (mConta.Valor() <= 0 && !mGame->ConfiguracaoDaSimulacao().imortal) {
        EncerrarBatalha(false);
    }

}
//endregion UpdateFunctions

void Battle::OnProcessInput(const Uint8* keyState) {
    // Nao chame ProcessInput dos jogadores aqui.

    // O ESC e borda, e nao estado: segurado, ele abriria e fecharia a pausa a cada quadro.
    const bool esc = keyState[SDL_SCANCODE_ESCAPE] != 0;
    const bool escNovo = mBordaEsc.Bateu(esc);

    if (mAcabando) {
        if (mResultado) mResultado->Processar(keyState);
        return;
    }
    if (!mPausa) return;
    if (mPausa->Aberta()) {
        switch (mPausa->Processar(keyState, escNovo)) {
            case PausaDaBatalha::Pedido::Recomecar:
                // A mesma materia, de novo e do zero: a troca de cena refaz a batalha com o que o Game guarda.
                SairDaBatalha(SceneType::Battle);
                break;
            case PausaDaBatalha::Pedido::Sair:
                SairDaBatalha(SceneType::StageSelect);
                break;
            case PausaDaBatalha::Pedido::Nada:
                break;
        }
        return;
    }
    // Na contagem, o ESC reabre o menu: quem se arrependeu de voltar nao precisa esperar o 1.
    if (escNovo && Pausa::PodePausar(mGame->NoGabinete(), EmCombate())) AbrirPausa();
}

void Battle::AoPerderOFoco() {
    if (mPausa && Pausa::PausaAoPerderOFoco(mGame->NoGabinete(), EmCombate(), mPausa->Aberta())) AbrirPausa();
}

//region Pausa
void Battle::AbrirPausa() {
    if (!mPausa) return;
    mPausa->Abrir();
    mAtoresCongelados = true;

    // O aluno para onde esta.
    for (Player* jogador : mJogadores) {
        if (jogador) if (auto* rb = jogador->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
    if (mGame->GetAudio()) mGame->GetAudio()->PauseSound(mMusica);
}

void Battle::Retomar() {
    mAtoresCongelados = false;
    if (mGame->GetAudio()) mGame->GetAudio()->ResumeSound(mMusica);
}

void Battle::SairDaBatalha(const SceneType destino) {
    // Quem sai pela pausa desistiu - inclusive no Recomecar.
    Avisar([&](OuvinteDaBatalha& o) { o.AoTerminar(*this, Tentativas::Resultado::Abandonou, false); });
    if (mProjeteis) {
        mProjeteis->ClearBossProjectiles();
        mProjeteis->Cleanup();
    }
    for (Boss* corpo : Corpos()) if (corpo) corpo->AoTerminarABatalha();
    mGame->RequestSceneChange(destino);
}
//endregion Pausa

void Battle::VerificarColisoes() {
    Avisar([&](OuvinteDaBatalha& o) { o.AoAntesDasColisoes(*this); });
    if (mColisoes && mProjeteis) {
        mColisoes->Verificar(mChefe, mProjeteis->GetBossProjectiles(), mProjeteis->GetPlayerProjectiles());
    }
}

void Battle::AlunoAcertado(Player& aluno, const std::string& causa, const float multiplicador, const bool porUmTiro) {
    const float perda = mConta.Descer(multiplicador);
    Avisar([&](OuvinteDaBatalha& o) { o.AoPerderNota(*this, aluno, causa, perda); });
    Avisar([&](OuvinteDaBatalha& o) { o.AoSerAcertado(*this, aluno, porUmTiro); });
}

void Battle::AlunoRaspou(Player& aluno, const Vector2& onde) {
    // A medida conta sempre; o experimento sem raspao so nao paga nem acende.
    Avisar([&](OuvinteDaBatalha& o) { o.AoRaspar(*this, aluno); });
    if (!mCondicao.Ligada(Experimento::Mecanica::Raspao)) return;
    mConta.Subir(mConta.PorJogador(Raspao::kGanho));
    mEfeitos->Faiscar(onde, Hud::Faisca::Raspao);
}

void Battle::ProfessorAcertado(const PlayerProjectile& tiro, const CircleColliderComponent& corpo, const bool ouviu) {
    // Acerto de um jogador: repartido.
    Avisar([&](OuvinteDaBatalha& o) { o.AoAcertarOProfessor(*this, tiro.Dono(), ouviu); });

    if (ouviu) {
        // A duvida perguntada de perto vale mais (Nota::FatorDePerto) - a do aluno; a do orientador, nao.
        const int quem = PosicaoDoAluno(tiro.Dono());
        // Quem pergunta incomoda, e chama a mira (Papeis.h).
        if (quem >= 0) mMira.Acertou(static_cast<size_t>(quem));
        const float fatorDaDistancia = quem >= 0 ? Nota::FatorDePerto((tiro.Dono()->GetPosition() -
                                                                       corpo.GetCenter()).Length())
                                                 : 1.0f;
        // Perguntou de perto: conta pela distancia; o experimento sem o perto so nao paga.
        if (quem >= 0 && fatorDaDistancia >= Nota::kFatorDaRespostaDupla) {
            const Player& aluno = *mJogadores[static_cast<size_t>(quem)];
            Avisar([&](OuvinteDaBatalha& o) { o.AoAcertarDePerto(*this, aluno); });
        }
        const float fator = mCondicao.Ligada(Experimento::Mecanica::Perto) ? fatorDaDistancia : 1.0f;
        mConta.Subir(mConta.PorJogador(Nota::kGanhoPorAcerto * fator));
        // A duvida respondida: o "?" vira um "!" na cor de quem perguntou - "!!" de perto.
        mEfeitos->Faiscar(tiro.GetPosition(),
                          fator >= Nota::kFatorDaRespostaDupla ? Hud::Faisca::RespostaDePerto : Hud::Faisca::Resposta,
                          quem);
    }

    // Todo acerto carrega o poder do orientador, blindado ou nao.
    if (mOrientador) mOrientador->Carregar();
}

void Battle::PontoExtraPego(Player& aluno, const bool primeiroDoQuadro) {
    Avisar([&](OuvinteDaBatalha& o) { o.AoPegarPontoExtra(*this, aluno); });
    // O som toca uma vez por quadro, por mais Pontos Extras que se peguem nele.
    if (primeiroDoQuadro) mGame->GetAudio()->PlaySound("powerup.wav", false);
}

Cobertura::Aluno Battle::AlunoDaCobertura(const Player* aluno) const {
    const auto* colisor = aluno ? ColisorDe(aluno) : nullptr;
    // O que nada atinge (o escudo, a piscada) nao precisa de cobertura.
    if (!colisor || aluno->EstaInvencivel()) return Cobertura::Aluno{};
    return Cobertura::Aluno{colisor->GetCenter(), colisor->GetRadius(), true};
}

bool Battle::ProjetilDerrubado(const PlayerProjectile& tiro, const BossProjectile& projetil) {
    // So na dupla, e so o tiro de um aluno.
    const int quem = PosicaoDoAluno(tiro.Dono());
    if (quem < 0 || mJogadores.size() < 2) return false;
    const Player* parceiro = mJogadores[quem == 0 ? 1 : 0];
    const auto* colisorDoProjetil = projetil.GetComponent<CircleColliderComponent>();
    if (!colisorDoProjetil) return false;
    const auto* rb = projetil.GetComponent<RigidBodyComponent>();
    const Vector2 velocidade = rb ? rb->GetVelocity() : Vector2::Zero;
    // "Ia nele": passaria na faixa do raspao dele, ou mais perto.
    if (!Cobertura::VaiNele(colisorDoProjetil->GetCenter(), velocidade, colisorDoProjetil->GetRadius(),
                            AlunoDaCobertura(parceiro))) return false;

    // A medida conta sempre; o experimento sem cobertura so nao paga (Experimento.h).
    Avisar([&](OuvinteDaBatalha& o) { o.AoCobrir(*this, *mJogadores[static_cast<size_t>(quem)], *parceiro); });
    if (!mCondicao.Ligada(Experimento::Mecanica::Cobertura)) return false;

    // E da dupla: pago inteiro, sem a divisao.
    mConta.Subir(Cobertura::kGanho);
    mEfeitos->Faiscar(colisorDoProjetil->GetCenter(), Hud::Faisca::Cobertura, quem);
    // O valeu: sobe de quem foi coberto, na cor de quem cobriu - os dois veem que contou, na hora.
    if (mCondicao.Ligada(Experimento::Mecanica::Valeu)) {
        mDesenhoSobreOsAlunos->Valeu(parceiro, static_cast<size_t>(quem));
    }
    return true;
}

void Battle::Sinalizar(const Player& quem) {
    // So na dupla, e fora do TCC: la o Botao 3 e o poder do orientador.
    if (mJogadores.size() < 2 || mOrientador || mAcabando || mCarregando) return;
    const int i = PosicaoDoAluno(&quem);
    if (i < 0 || i > 1) return;
    if (!Comunicacao::PodeSinalizar(mRelogioDaLuta, mUltimoSinal[static_cast<size_t>(i)])) return;
    mUltimoSinal[static_cast<size_t>(i)] = mRelogioDaLuta;
    // A medida conta sempre; o experimento sem sinal so nao mostra nem toca.
    Avisar([&](OuvinteDaBatalha& o) { o.AoSinalizar(*this, quem); });
    if (!mCondicao.Ligada(Experimento::Mecanica::Sinal)) return;
    mDesenhoSobreOsAlunos->Sinal(&quem, static_cast<size_t>(i));
    if (mGame && mGame->GetAudio()) mGame->GetAudio()->PlaySound("sinal.wav", false);
}

void Battle::MarcarOCaminhoDosAlunos() {
    // Sozinho, nao ha quem cubra, e nenhum projetil brilha.
    if (mJogadores.size() < 2 || !mProjeteis) return;
    std::vector<Cobertura::Aluno> alunos;
    for (const Player* p : mJogadores) alunos.push_back(AlunoDaCobertura(p));
    const Uint8 alfa = Hud::AlfaDoBrilho(mDesenhoSobreOsAlunos->Pulso());
    const bool brilha = mCondicao.Ligada(Experimento::Mecanica::Brilho);

    for (const auto& projetil : mProjeteis->GetBossProjectiles()) {
        if (projetil->GetState() != ActorState::Active) continue;
        auto* desenho = projetil->GetComponent<DrawAnimatedComponent>();
        if (!desenho) continue;
        int quem = -1;
        // So o que tem o contorno vermelho: o que o tiro derruba.
        const auto* colisor = projetil->GetComponent<CircleColliderComponent>();
        if (colisor && Destrutivel::Pode(projetil->Derrubavel(), projetil->Blindado(), projetil->Abrigo(),
                                         projetil->Enfeite())) {
            const auto* rb = projetil->GetComponent<RigidBodyComponent>();
            quem = Cobertura::AlunoNoCaminho(colisor->GetCenter(), rb ? rb->GetVelocity() : Vector2::Zero,
                                             colisor->GetRadius(), alunos);
        }
        // Uma chance de cobertura por projetil e aluno - brilhe ele ou nao (Experimento.h).
        if (quem >= 0 && projetil->PrimeiraVezNoCaminhoDe(static_cast<size_t>(quem))) {
            const Player& alvo = *mJogadores[static_cast<size_t>(quem)];
            Avisar([&](OuvinteDaBatalha& o) { o.AoIrNoAluno(*this, alvo); });
        }
        if (quem < 0 || !brilha) {
            desenho->DefinirBrilho(false);
            continue;
        }
        const Hud::Rgb c = Hud::CorDoAluno(static_cast<size_t>(quem));
        desenho->DefinirBrilho(true, SDL_Color{c.r, c.g, c.b, alfa}, Hud::kAlcanceDoBrilho);
    }
}

Player* Battle::JogadorEm(const size_t posicao) const {
    return posicao < mJogadores.size() ? mJogadores[posicao] : nullptr;
}

Player* Battle::Alvo() const {

    // Quem passa a vez e RevezarAlvo, chamado por quem sabe que um disparo comecou; quem prende a
    // mira e o incomodo (IndiceDoAlvo).
    if (mJogadores.empty()) return nullptr;
    const size_t i = IndiceDoAlvo();
    return i < mJogadores.size() ? mJogadores[i] : mJogadores.front();
}

void Battle::DarAVez(const size_t posicao) {
    mMira.DarAVez(posicao, mJogadores.size());
}

void Battle::RevezarAlvo() {
    mMira.Revezar(mJogadores.size());
}

/// Segundos que o aviso de fase limpa fica na tela. Curto: ele informa, nao
/// comemora - e o jogador esta no meio de uma batalha.
static constexpr float kDuracaoDoAvisoDeFaseLimpa = 2.5f;

void Battle::AoMudarDeFase(const float novaDuracao) {

    // A ordem importa.
    EncerrarFase();

    if (mHUD) {
        mHUD->ResetTimeBar(novaDuracao);
    }

    // O FSM ja esta na fase nova quando avisa (FSMComponent::SetState).
    const auto* fsm = mChefe ? mChefe->GetComponent<FSMComponent>() : nullptr;
    const std::string fase = fsm ? fsm->GetStateName() : "";
    const std::string anuncio = mChefe ? mChefe->AnuncioDaFase(fase) : "";
    Avisar([&](OuvinteDaBatalha& o) { o.AoTrocarDeFase(*this, fase, anuncio); });
}

void Battle::EncerrarFase() {

    // Depois que a batalha acabou, nao ha mais fase a fechar.
    if (mAcabando) return;

    // A conta paga o bonus da fase limpa (e nao paga nada na entrada da primeira).
    if (!mConta.FecharFase()) return;
    Avisar([&](OuvinteDaBatalha& o) { o.AoLimparFase(*this); });

    // O aviso existe para a nota nao subir por motivo invisivel.
    mAvisoDeFaseLimpa = kDuracaoDoAvisoDeFaseLimpa;
}

SDL_FRect Battle::GetPlayfieldBounds() const
{
    // O campo acaba onde a faixa da nota comeca, e esse numero e o da barra - nao uma copia dele.
    return {
        0.0f, // x
        0.0f, // y
        static_cast<float>(mGame->GetWindowWidth()), // w
        static_cast<float>(mGame->GetWindowHeight() - FaixaDaNota::kDoFundo) // h
    };
}

void Battle::DividirChefe(const std::string& faseDoClone) {

    if (!mChefe || mCorpos.size() > 1) return;   // ja dividido, ou sem original

    IBossFactory* fabrica = mGame->GetFactory(static_cast<size_t>(mMateria));
    if (!fabrica) {
        SDL_Log("BATALHA: a fase pediu divisao, mas esta materia nao tem fabrica de chefe.");
        return;
    }

    // O clone e um chefe inteiro da mesma fabrica, e e isso que o deixa barato.
    auto clone = fabrica->CreateBoss(this);
    if (!clone) return;

    clone->MarcarComoClone();
    clone->SetInitialState(faseDoClone);

    // Nasce ao lado do original, e nao em cima.
    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const Vector2 ondeEstava = mChefe->GetPosition();
    clone->SetPosition(Vector2(largura - ondeEstava.x, ondeEstava.y));

    Boss* bruto = dynamic_cast<Boss*>(this->AddActor(std::move(clone)));
    if (!bruto) return;

    // Start() e o que dispara a FSM e os primeiros ataques.
    bruto->Start();
    mCorpos.push_back(bruto);

    SDL_Log("BATALHA: o chefe se dividiu. O segundo corpo usa os ataques de \"%s\".",
            faseDoClone.c_str());
}

void Battle::JuntarCorpos() {

    // O original fica, os extras saem.
    for (size_t i = mCorpos.size(); i-- > 1; ) {
        if (mCorpos[i]) {

            // Antes de destruir o corpo, tira os tiros dele de cena.
            if (mProjeteis) {
                mProjeteis->DescartarProjeteisDe(mCorpos[i]);
            }
            mCorpos[i]->SetState(ActorState::Destroy);
        }
        mCorpos.pop_back();
    }
}

Player* Battle::OutroJogador(const Player* quem) const {

    // Com um jogador so nao ha outro, e quem pergunta trata o nulo.
    if (mJogadores.size() < 2) return nullptr;

    for (size_t i = 0; i < mJogadores.size(); ++i) {
        if (mJogadores[i] == quem) return mJogadores[(i + 1) % mJogadores.size()];
    }
    return nullptr;
}

void Battle::AoGastarPontoExtra(Player* quemGastou) {

    // O mesmo nos dois modos, e repartido: e acao de um jogador.
    mConta.GastarPontoExtra();
    if (quemGastou) Avisar([&](OuvinteDaBatalha& o) { o.AoUsarPontoExtra(*this, *quemGastou); });

    Player* oOutro = OutroJogador(quemGastou);

    if (!oOutro) {
        // Sozinho, o Ponto Extra limpa a tela, como sempre fez.
        const auto* colisor = ColisorDe(quemGastou);
        mEfeitos->SoltarOnda(colisor ? colisor->GetCenter() : quemGastou->GetPosition());
        mEfeitos->Faiscar(colisor ? colisor->GetCenter() : quemGastou->GetPosition(), Hud::Faisca::Coleta);
        return;
    }

    // Em dupla, o Ponto Extra e do outro, e nao limpa a tela.
    oOutro->Proteger(PontoExtra::kImunidadeDoPresente);
    mDesenhoSobreOsAlunos->Feixe(quemGastou, oOutro);

    // A sincronia: os dois juntos, e a onda do solo sai do meio dos dois, alem dos escudos.
    const int quem = PosicaoDoAluno(quemGastou);
    const int outro = PosicaoDoAluno(oOutro);
    if (quem < 0 || outro < 0 || quem > 1 || outro > 1) return;
    if (!PontoExtra::EmSincronia(mRelogioDaLuta, mUltimoPontoExtra[static_cast<size_t>(outro)])) {
        mUltimoPontoExtra[static_cast<size_t>(quem)] = mRelogioDaLuta;
        return;
    }
    // Cada gasto vale uma sincronia so.
    mUltimoPontoExtra = {PontoExtra::kNuncaGastou, PontoExtra::kNuncaGastou};
    // A medida conta sempre; o experimento sem sincronia so nao solta a onda.
    Avisar([&](OuvinteDaBatalha& o) { o.AoSincronizar(*this); });
    if (!mCondicao.Ligada(Experimento::Mecanica::Sincronia)) return;
    const auto* a = ColisorDe(quemGastou);
    const auto* b = ColisorDe(oOutro);
    const Vector2 meio = ((a ? a->GetCenter() : quemGastou->GetPosition()) +
                          (b ? b->GetCenter() : oOutro->GetPosition())) * 0.5f;
    mEfeitos->SoltarOnda(meio);
    mEfeitos->Faiscar(meio, Hud::Faisca::Coleta);
}

void Battle::Atravessou(const Vector2& de, const Vector2& para) {
    mEfeitos->Faiscar(de, Hud::Faisca::Coleta);
    mEfeitos->Faiscar(para, Hud::Faisca::Coleta);
    Avisar([&](OuvinteDaBatalha& o) { o.AoAtravessarAParede(*this); });
}

int Battle::PosicaoDoAluno(const Actor* ator) const {
    for (size_t i = 0; i < mJogadores.size(); ++i) {
        if (ator && mJogadores[i] == ator) return static_cast<int>(i);
    }
    return -1;
}

void Battle::SoltarPontoExtra(Vector2 position) {
    auto item = std::make_unique<ExtraPointItem>(this);
    item->SetPosition(position);

    mPontosExtras.emplace_back(item.get());

    AddActor(std::move(item));
}

void Battle::EncerrarBatalha(bool aprovado) {
    if (mAcabando) return; // Já está acabando, ignora chamadas duplicadas
    Avisar([&](OuvinteDaBatalha& o) { o.AoDecidir(*this, aprovado); });

    // A ultima fase tambem paga.
    EncerrarFase();

    mAcabando = true;

    // O resumo da tela de resultado.
    Resultado::Resumo resumo;
    resumo.nota = mConta.Valor();
    resumo.aprovado = aprovado;
    resumo.temFicha = mGame->ProgressoEGravado(0);
    resumo.recordeAntes = mGame->RecordeDaPartida(mMateria);
    resumo.fasesLimpas = mConta.FasesLimpas();
    resumo.acertosSofridos = mConta.AcertosSofridos();
    // A devolutiva, para quem reprovou: onde a nota mais caiu e o que mais acertou.
    if (!aprovado && mCondicao.Ligada(Experimento::Mecanica::Devolutiva)) {
        resumo.piorFase = mDevolutiva->Caderno().PiorFase();
        resumo.causas = mDevolutiva->Caderno().Causas();
        if (resumo.causas.size() > static_cast<size_t>(Devolutiva::kCausasNaTela)) {
            resumo.causas.resize(static_cast<size_t>(Devolutiva::kCausasNaTela));
        }
    }
    // A matricula; quem nao tem uma (o visitante) e o aluno da posicao dele - a cor diz qual.
    const auto nomeDe = [this](const size_t i) {
        const std::string& m = mGame->MatriculaDe(i);
        return (m.empty() || m == Matricula::kVisitante) ? "Aluno " + std::to_string(i + 1) : m;
    };
    // A justica, na dupla: a parte de cada um, com a matricula dele.
    if (mDevolutiva->Partes().size() >= 2) {
        resumo.partes = mDevolutiva->Partes();
        for (size_t i = 0; i < resumo.partes.size(); ++i) resumo.partes[i].quem = nomeDe(i);
        // E o papel de cada um, pelo que mais fez (Devolutiva::Titulos).
        if (mCondicao.Ligada(Experimento::Mecanica::Titulos)) resumo.titulos = Devolutiva::Titulos(resumo.partes);
    }

    // Quem ja usava o capelo antes desta nota (Personagens::Formatura).
    std::vector<bool> formadoAntes;
    for (size_t i = 0; i < mJogadores.size(); ++i) formadoAntes.push_back(mGame->Formado(i));

    // O unico ponto em que a nota vai para o armazenamento persistente.
    mGame->RegistrarNota(mMateria, mConta.Valor());

    // Quem se formou agora ganha o capelo ali mesmo, e a tela diz.
    if (aprovado) {
        bool todosGravados = true;
        for (size_t i = 0; i < mJogadores.size(); ++i) {
            if (formadoAntes[i] || !mGame->Formado(i)) continue;
            resumo.formados.push_back(nomeDe(i));
            todosGravados = todosGravados && mGame->ProgressoEGravado(i);
            if (mJogadores[i]) mJogadores[i]->RecomporPersonagem();
        }
        resumo.formaturaGravada = !resumo.formados.empty() && todosGravados;
    }
    // E a tentativa, com o resultado.
    Avisar([&](OuvinteDaBatalha& o) { o.AoFimDoQuadro(*this); });
    const auto resultado = aprovado ? Tentativas::Resultado::Aprovado : Tentativas::Resultado::Reprovado;
    Avisar([&](OuvinteDaBatalha& o) { o.AoTerminar(*this, resultado, mConta.Valor() > 0.0f); });
    for (auto& actor : mActors) {
        actor->SetState(ActorState::Paused);
    }

    // 1.
    if (mProjeteis) {
        mProjeteis->ClearBossProjectiles();
        // Se tiver função para limpar projéteis do player, chame aqui também.
    }
    // E o que o chefe desenhou no campo sem ser projetil (Boss.h).
    for (Boss* corpo : Corpos()) if (corpo) corpo->AoTerminarABatalha();

    // 2.
    for (Player* jogador : mJogadores) {
        if (jogador) jogador->DefinirInvencivel(true);
    }

    // 3.
    mResultado = std::make_unique<TelaDoResultado>(
        *this, *mGame, mFonteDaNota.get(), mJogadores.size(), mCondicao.Ligada(Experimento::Mecanica::Toque),
        [this]() { Avisar([&](OuvinteDaBatalha& o) { o.AoTocarAqui(*this); }); });
    mResultado->Mostrar(resumo, mChefe);
}

float Battle::TempoNoResultado() const {
    return mResultado ? mResultado->Tempo() : 0.0f;
}
