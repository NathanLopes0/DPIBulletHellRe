//
// Created by nslop on 16/09/2024.
//

#include "Battle.h"

#include "../../Aluno/Progresso.h"
#include "../../Regras/Nota.h"

#include <random>

#include "BattleHUD.h"
#include "FaixaDaNota.h"
#include "ProjectileManager.h"
#include "../../Motor/AudioSystem.h"
#include "../../Motor/Font.h"
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
#include "../../Components/DrawComponents/ProgressBarComponent.h"
#include "../../Components/DrawComponents/DrawFundoDoCampoComponent.h"
#include "../../Components/DrawComponents/DrawMolduraDoCampoComponent.h"
#include "../../Components/DrawComponents/DrawQuadroComponent.h"
#include "../../Components/DrawComponents/DrawRelogioDaFaseComponent.h"
#include "../../Actors/Teachers/ExtraPointItem.h"
#include "../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../Regras/PontoExtra.h"
#include "../../Regras/Raspao.h"
#include "../../Regras/Cobertura.h"
#include "../TextoDeCena.h"
#include "../../Aluno/Matricula.h"
#include "../../Actors/Teachers/BossFactory/BossProjectileFactory/FabricaDeProjetil.h"
#include "../../Regras/Destrutivel.h"
#include "../../Actors/Orientador/Orientador.h"
#include "../../Curso/MateriasArquivo.h"
#include "../../Personagem/ComporPersonagem.h"
#include "../../Personagem/PersonagensArquivo.h"
#include "../../Entrada/Painel.h"

#include "../../Motor/CaminhosArquivo.h"
#include "../../Motor/Fontes.h"
#include "RegistroDaTentativa.h"
#include "RelatorioDaSimulacao.h"
#include "ClaroesDaMoldura.h"
#include "TelaDeCarregamento.h"
#include "../PainelDeMenu.h"
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
    CriarPausa();
    mPartes.assign(mJogadores.size(), Devolutiva::Parte{});

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
    CriarFundo();
    CriarMoldura();
    if (mMoldura) mOuvintes.push_back(std::make_unique<ClaroesDaMoldura>(*mMoldura));
    CriarBarraDeTempo();
    // A fonte da faixa e tambem a dos menus e da tela de carregamento.
    mFonteDaNota = std::make_unique<Font>();
    mFonteDaNota->Load(Caminhos::Asset(Fontes::kJogo));
    mFaixa = std::make_unique<FaixaDaNota>(*this, mFonteDaNota.get(), mGame->GetWindowWidth(),
                                           mGame->GetWindowHeight(), mConta.Valor(), mNivel);
    CriarPlacar();
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
void Battle::CriarBarraDeTempo() {

    // Cria o Actor que vai segurar a barra que mostra o tempo do estado.
    auto barActor = std::make_unique<Actor>(this);

    if (!mChefe) {
        SDL_Log("ERRO: CriarHUD chamado antes do mChefe ser criado!");
        return;
    }

    auto bossSprite = mChefe->GetComponent<DrawAnimatedComponent>();
    auto bossHeight = bossSprite->GetSpriteHeight();

    auto barCompPtr = barActor->AddComponent<DrawRelogioDaFaseComponent>(
        12, static_cast<int>(static_cast<float>(bossHeight) / 1.4f), 200);

    Actor* barActorPtr = barActor.get();

    // Adiciona ator à cena.
    this->AddActor(std::move(barActor));

    // Dá poder ao HUD pra modificar a barra.
    mHUD->SetTimeBar(barActorPtr, barCompPtr);

    // Salva o ponteiro do Actor que tem a barra de progresso na cena Battle, pra poder seguir o Boss.
    mBarraDeTempoAtor = barActorPtr;

    // Dá load no reloginho que tem embaixo da barra.
    CriarRelogioDaFase();
}
void Battle::CriarRelogioDaFase() {

    // Cria o ator que vai segurar a animação do relógior.
    auto clockActor = std::make_unique<Actor>(this);
    auto clockActorPtr = clockActor.get();

    std::string clockSpriteSheet = Caminhos::Asset("Icons/TestTimeIcon.png");
    std::string clockSpriteData = Caminhos::Asset("Icons/TestTimeIcon.json");

    auto clockComp = clockActor->AddComponent<DrawAnimatedComponent>(clockSpriteSheet, clockSpriteData, 201);

    clockComp->AddAnimation("Idle", {0});
    clockComp->AddAnimation("Active", {0,1,2,3,4,5});
    clockComp->SetAnimation("Active");

    this->AddActor(std::move(clockActor));
    mRelogioAtor = clockActorPtr;

}
void Battle::CriarPlacar() {

    // O quadro da sala, no canto de cima a esquerda.
    mFonteDoPlacar = std::make_unique<Font>();
    mFonteDoPlacar->Load(Caminhos::Asset(Fontes::kPlacar));

    auto placar = std::make_unique<Actor>(this);

    // Verde de quadro-negro e moldura de giz.
    auto quadro = placar->AddComponent<DrawQuadroComponent>(kPlacarLargura, kPlacarAltura, kOrdemDoPlacar);
    quadro->SetIsVisible(false);

    // A caixa do texto e menor que o quadro, e o ajuste ao texto so encolhe.
    auto texto = placar->AddComponent<DrawTextComponent>(
        " ", mFonteDoPlacar.get(), kPlacarLargura - 24, kPlacarAltura - 6,
        kTamanhoDoPlacar, kOrdemDoPlacar + 1);
    texto->SetAjustarAoTexto(true);
    texto->SetIsVisible(false);

    // Depois do trilho da esquerda, e nao colado na borda da tela.
    placar->SetPosition(Vector2(mAreaDoAluno.esquerda + static_cast<float>(kPlacarMargem + kPlacarLargura / 2) - 4.0f,
                                static_cast<float>(kPlacarMargem + kPlacarAltura / 2)));

    mPlacarAtor = placar.get();
    this->AddActor(std::move(placar));
}

void Battle::AtualizarPlacar() {
    if (!mPlacarAtor || !mChefe) return;

    // FaseAtual, e nao o nome que a maquina de estados acabou de trocar.
    const std::string anuncio = Intercambio::Avisando(mRelogioDoIntercambio)
                                    ? mChefe->AvisoDoIntercambio()
                                    : mChefe->AnuncioDaFase(mChefe->FaseAtual());
    if (anuncio == mTextoDoPlacar) return;
    mTextoDoPlacar = anuncio;

    const bool aceso = !anuncio.empty();

    // So escreve quando ha o que escrever.
    auto* texto = mPlacarAtor->GetComponent<DrawTextComponent>();
    if (aceso) texto->SetText(anuncio);
    texto->SetIsVisible(aceso);
    auto* quadro = mPlacarAtor->GetComponent<DrawQuadroComponent>();
    quadro->SetIsVisible(aceso);
    if (aceso) quadro->Apagar();
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

void Battle::CriarResultado(const int linhas, const bool comToque) {
    std::vector<std::string> opcoes;
    for (const Resultado::Opcao o : Resultado::kOpcoes) opcoes.emplace_back(Resultado::Texto(o));

    // Sem o ESC no rodape: a luta acabou, e quem escolhe e o painel.
    std::vector<Painel::Acao> acoes = {Painel::Manche(Painel::Jogador::Um, "escolher"),
                                       {Painel::Jogador::Um, Painel::Botao::Um, "confirmar"}};
    // O toca aqui se ensina no proprio rodape: o Botao 3 dos dois, juntos.
    if (comToque) {
        acoes.emplace_back("BOTAO 3 JUNTOS",
                           std::string(Painel::NomeDaTecla(Painel::Jogador::Um, Painel::Botao::Tres)) + " + " +
                               Painel::NomeDaTecla(Painel::Jogador::Dois, Painel::Botao::Tres),
                           "toca aqui");
    }
    const std::string rodape = Painel::Rodape(acoes, mGame->JeitoDoRodape());

    // Mais alta que a da pausa: cabem as linhas do resumo entre o titulo e as opcoes.
    PainelDeMenu::Medidas medidas;
    medidas.larguraDaPlaca = kLarguraDoResultado;
    medidas.alturaDaPlaca = PainelDeMenu::AlturaPara(linhas);
    medidas.ordem = kOrdemDosMenus;
    mPainelDoResultado = std::make_unique<PainelDeMenu>(*this, mFonteDaNota.get(), " ", opcoes, rodape,
                                                        linhas, medidas);
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
    if (mPausa.aberto || Retomando()) {
        Avisar([&](OuvinteDaBatalha& o) { o.AoPassarOTempo(*this, deltaTime, true); });
        if (Retomando()) {
            mRetomada -= deltaTime;
            if (Retomando()) {
                mNumeroDaRetomada->GetComponent<DrawTextComponent>()->SetText(
                    std::to_string(Pausa::Contagem(mRetomada)));
            } else {
                Retomar();
            }
        }
        return;
    }

    // ---- O fim da luta: o painel do resultado esta na tela (EncerrarBatalha) ----
    if (mAcabando) {
        mTempoNoFim += deltaTime;

        // O ClearBossProjectiles() de EncerrarBatalha apenas marca os projeteis.
        if (mProjeteis) mProjeteis->Cleanup();

        AtualizarToque(deltaTime);

        if (Resultado::VoltaSozinha(mGame->NoGabinete(), mTempoNoFim)) {
            mGame->RequestSceneChange(SceneType::StageSelect);
        }

        return;
    }

    // A cena base já atualiza todos os atores.
    Avisar([&](OuvinteDaBatalha& o) { o.AoPassarOTempo(*this, deltaTime, false); });
    mRelogioDaLuta += deltaTime;

    // Atualiza os sistemas principais apenas da Battle (os atores são atualizados pela Scene)
    if (mProjeteis) mProjeteis->Update(deltaTime);

    // Orquestra a checagem de colisões.
    VerificarColisoes();
    mIncomodo.Avancar(deltaTime);
    AtualizarDesenhoSobreOsAlunos(deltaTime);
    MarcarOCaminhoDosAlunos();
    AtualizarMoldura();
    AtualizarOndas(deltaTime);
    AtualizarEfeitos(deltaTime);

    if (mHUD) mHUD->OnUpdate(deltaTime, mChefe && mChefe->Congelado());
    if (mRede > 0.0f) mRede = std::max(0.0f, mRede - deltaTime);

    AtualizarBarraDeTempo();
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
void Battle::AtualizarBarraDeTempo() const {
    if (!mChefe || !mBarraDeTempoAtor) return;

    Vector2 bossPos = mChefe->GetPosition();

    auto bossSprite = mChefe->GetComponent<DrawAnimatedComponent>();
    auto bossWidth = bossSprite->GetSpriteWidth();
    auto bossHeight = bossSprite->GetSpriteHeight();

    float offSetX = static_cast<float>(bossWidth) / 2.5f;
    float offSetY = static_cast<float>(bossHeight) / 4.f;

    mBarraDeTempoAtor->SetPosition(Vector2(bossPos.x + offSetX,
                                    bossPos.y - offSetY));

    mRelogioAtor->SetPosition(mBarraDeTempoAtor->GetPosition() + Vector2(6, static_cast<float>(bossHeight) / 1.4f + 4.0f));

    // O relogio fica vermelho junto com a capsula, nos ultimos segundos.
    if (auto* relogio = mRelogioAtor->GetComponent<DrawAnimatedComponent>(); relogio && mHUD) {
        const bool urgente = Hud::Urgente(mHUD->Restante());
        const bool aceso = urgente && std::fmod(mHUD->Restante(), 0.5f) > 0.25f;
        if (aceso) relogio->SetColor(255, 110, 100);
        else relogio->SetColor(255, 255, 255);
    }
}
//endregion UpdateFunctions

void Battle::OnProcessInput(const Uint8* keyState) {
    // Nao chame ProcessInput dos jogadores aqui.

    // O ESC e borda, e nao estado: segurado, ele abriria e fecharia a pausa a cada quadro.
    const bool esc = keyState[SDL_SCANCODE_ESCAPE] != 0;
    const bool escNovo = mBordaEsc.Bateu(esc);

    if (mAcabando) {
        ProcessarResultado(keyState);
        return;
    }
    if (mPausa.aberto) {
        ProcessarPausa(keyState, escNovo);
        return;
    }
    // Na contagem, o ESC reabre o menu: quem se arrependeu de voltar nao precisa esperar o 1.
    if (escNovo && Pausa::PodePausar(mGame->NoGabinete(), EmCombate())) AbrirPausa();
}

void Battle::AoPerderOFoco() {
    if (Pausa::PausaAoPerderOFoco(mGame->NoGabinete(), EmCombate(), mPausa.aberto)) AbrirPausa();
}

Battle::BatidasDoMenu Battle::LerMenu(const Uint8* keyState) {
    // Qualquer um dos dois mexe no menu.
    bool cima = false, baixo = false, confirmar = false;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        cima = cima || Painel::Apertado(keyState, quem, Painel::Direcao::Cima);
        baixo = baixo || Painel::Apertado(keyState, quem, Painel::Direcao::Baixo);
        confirmar = confirmar || Painel::Apertado(keyState, quem, Painel::Botao::Um);
    }
    confirmar = confirmar || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];

    BatidasDoMenu b;
    b.cima = mBordaCima.Bateu(cima);
    b.baixo = mBordaBaixo.Bateu(baixo);
    b.confirmar = mBordaConfirmar.Bateu(confirmar);
    return b;
}

//region Pausa
void Battle::CriarPausa() {
    std::vector<std::string> opcoes;
    for (const Pausa::Opcao o : Pausa::kOpcoes) opcoes.emplace_back(Pausa::Texto(o));

    // O rodape sai do painel, como o de toda tela (Painel.h).
    const std::string rodape = Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher"),
                                               {Painel::Jogador::Um, Painel::Botao::Um, "confirmar"},
                                               {std::string("ESC"), std::string(), std::string("continuar")}},
                                              mGame->JeitoDoRodape());
    PainelDeMenu::Medidas medidas;
    medidas.ordem = kOrdemDosMenus;
    mPainelDaPausa = std::make_unique<PainelDeMenu>(*this, mFonteDaNota.get(), "PAUSA", opcoes, rodape, 0, medidas);

    // O numero da contagem, no meio do campo, sobre uma placa escura pequena.
    const SDL_FRect campo = GetPlayfieldBounds();
    auto numero = std::make_unique<Actor>(this);
    numero->SetPosition(Vector2(campo.x + campo.w / 2.0f, campo.y + campo.h / 2.0f));
    numero->AddComponent<DrawCaixaComponent>(130, 130, SDL_Color{120, 140, 180, 230}, SDL_Color{12, 13, 17, 200},
                                             3, kOrdemDosMenus)->SetIsVisible(false);
    auto* dc = numero->AddComponent<DrawTextComponent>(" ", mFonteDaNota.get(), 110, 110, 72, kOrdemDosMenus + 1);
    dc->SetAjustarAoTexto(true);
    dc->SetIsVisible(false);
    mNumeroDaRetomada = AddActor(std::move(numero));
}

void Battle::AbrirPausa() {
    Pausa::Abrir(mPausa);
    mAtoresCongelados = true;

    // A contagem, se estava correndo, para: ao fechar de novo ela recomeca do 3.
    mRetomada = 0.0f;
    mNumeroDaRetomada->GetComponent<DrawCaixaComponent>()->SetIsVisible(false);
    mNumeroDaRetomada->GetComponent<DrawTextComponent>()->SetIsVisible(false);

    // Quem estava segurando o manche ou o tiro quando apertou ESC nao anda no menu por isso.
    SegurarMenu();

    // O aluno para onde esta.
    for (Player* jogador : mJogadores) {
        if (jogador) if (auto* rb = jogador->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
    if (mGame->GetAudio()) mGame->GetAudio()->PauseSound(mMusica);

    mPainelDaPausa->Selecionar(mPausa.selecionada);
    mPainelDaPausa->Mostrar(true);
}

void Battle::FecharPausa() {
    Pausa::Fechar(mPausa);
    mPainelDaPausa->Mostrar(false);

    // A luta nao volta ainda.
    mRetomada = Pausa::kRetomada;
    auto* texto = mNumeroDaRetomada->GetComponent<DrawTextComponent>();
    texto->SetText(std::to_string(Pausa::Contagem(mRetomada)));
    texto->SetIsVisible(true);
    mNumeroDaRetomada->GetComponent<DrawCaixaComponent>()->SetIsVisible(true);
}

void Battle::Retomar() {
    mRetomada = 0.0f;
    mNumeroDaRetomada->GetComponent<DrawCaixaComponent>()->SetIsVisible(false);
    mNumeroDaRetomada->GetComponent<DrawTextComponent>()->SetIsVisible(false);
    mAtoresCongelados = false;
    if (mGame->GetAudio()) mGame->GetAudio()->ResumeSound(mMusica);
}

void Battle::ProcessarPausa(const Uint8* keyState, const bool escNovo) {
    const BatidasDoMenu b = LerMenu(keyState);
    if (escNovo) {
        FecharPausa();
        return;
    }
    if (b.cima) Pausa::Mover(mPausa, -1);
    if (b.baixo) Pausa::Mover(mPausa, 1);
    mPainelDaPausa->Selecionar(mPausa.selecionada);
    if (b.confirmar) EscolherNaPausa();
}

void Battle::EscolherNaPausa() {
    switch (Pausa::Escolhida(mPausa)) {
        case Pausa::Opcao::Continuar:
            FecharPausa();
            break;
        case Pausa::Opcao::Recomecar:
            // A mesma materia, de novo e do zero: a troca de cena refaz a batalha com o que o Game guarda.
            SairDaBatalha(SceneType::Battle);
            break;
        case Pausa::Opcao::Sair:
            SairDaBatalha(SceneType::StageSelect);
            break;
    }
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

//region Resultado
void Battle::MostrarResultado(const Resultado::Resumo& resumo) {
    mResultadoSelecionado = 0;
    // O tiro que estava apertado quando a luta acabou nao escolhe nada.
    SegurarMenu();
    const std::vector<std::string> linhas = Resultado::Linhas(resumo);
    // A dupla aprovada comemora: os dois avatares no painel, e o toca aqui (Comunicacao.h).
    const bool duplaAprovada = mJogadores.size() >= 2 && resumo.aprovado;
    mPodeTocar = duplaAprovada;
    CriarResultado(static_cast<int>(linhas.size()), duplaAprovada && mCondicao.Ligada(Experimento::Mecanica::Toque));
    // A formatura e dourada, o titulo e a linha de quem ganhou o capelo.
    const bool formou = Resultado::Formou(resumo);
    mPainelDoResultado->SetTitulo(Resultado::Titulo(resumo),
                                  formou ? Color::Gold : resumo.aprovado ? Color::LightGreen : Color::LightPink);
    mPainelDoResultado->SetLinhas(linhas, Resultado::LinhaEmDestaque(resumo));
    if (formou) mPainelDoResultado->ColorirLinha(Resultado::LinhaDaFormatura(resumo), Color::Gold);
    if (formou || duplaAprovada) MostrarAlunosNoResultado();
    // A justica: cada aluno na cor dele.
    for (size_t i = 0; i < resumo.partes.size(); ++i) {
        const Hud::Rgb c = Hud::CorDoAluno(i);
        mPainelDoResultado->ColorirLinha(Resultado::LinhaDaParte(resumo, i),
                                         Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f));
    }
    mPainelDoResultado->Selecionar(mResultadoSelecionado);
    mPainelDoResultado->Mostrar(true);
    // A devolutiva: o desenho do que mais acertou, na linha dela.
    const int linhaDasCausas = Resultado::LinhaDasCausas(resumo);
    if (linhaDasCausas >= 0) MostrarCausas(resumo.causas, mPainelDoResultado->OndeDaLinha(linhaDasCausas));
}

void Battle::MostrarAlunosNoResultado() {
    if (mJogadores.empty() || !mPainelDoResultado) return;
    const Vector2 titulo = mPainelDoResultado->OndeDoTitulo();
    const size_t ultimo = mJogadores.size() - 1;
    // O primeiro aluno a esquerda, o ultimo a direita: sozinho, ele e os dois.
    const std::pair<size_t, float> lados[] = {{0, -1.0f}, {ultimo, 1.0f}};
    for (const auto& [quem, lado] : lados) {
        const Personagens::Composta c = Personagens::Compor(mGame, Personagens::Carregado(),
                                                            mGame->AparenciaDe(quem), mGame->Formado(quem));
        if (!c.ok) continue;
        auto formando = std::make_unique<Actor>(this);
        formando->SetPosition(titulo + Vector2(lado * kAfastamentoDoFormando, 0.0f));
        formando->SetScale(kEscalaDoFormando);
        auto* desenho = formando->AddComponent<DrawAnimatedComponent>(c.chaveDaTextura, c.atlas, kOrdemDosMenus + 4);
        desenho->AddAnimation("Andando", {0, 1, 2, 3});
        desenho->SetAnimFPS(6.0f);
        desenho->SetAnimation("Andando");
        const Vector2 base = formando->GetPosition();
        mAvataresNoResultado.push_back(AvatarNoResultado{AddActor(std::move(formando)), base});
    }
}

void Battle::LerToque(const Uint8* keyState) {
    if (!mPodeTocar || mJogadores.size() < 2) return;
    for (size_t i = 0; i < 2; ++i) {
        const auto quem = i == 0 ? Painel::Jogador::Um : Painel::Jogador::Dois;
        const bool apertado = Painel::Apertado(keyState, quem, Painel::Botao::Tres);
        // O botao que vinha segurado da luta (o sinal) nao conta: so o aperto depois da trava.
        if (apertado && !mTresAntes[i] && mTempoNoFim >= Resultado::kTravaDoMenu) mUltimoTres[i] = mTempoNoFim;
        mTresAntes[i] = apertado;
    }
    if (Comunicacao::Juntos(mUltimoTres[0], mUltimoTres[1]) &&
        mTempoNoFim - mUltimoToque >= Comunicacao::kEsperaDoToque) {
        TocarAqui();
    }
}

void Battle::TocarAqui() {
    // Cada par de apertos vale um toque so.
    mUltimoTres = {Comunicacao::kNuncaApertou, Comunicacao::kNuncaApertou};
    mUltimoToque = mTempoNoFim;
    // A medida conta sempre; o experimento sem o toque so nao pula, nao estoura e nao toca.
    Avisar([&](OuvinteDaBatalha& o) { o.AoTocarAqui(*this); });
    if (!mCondicao.Ligada(Experimento::Mecanica::Toque)) return;
    for (AvatarNoResultado& a : mAvataresNoResultado) a.pulo = 0.0f;
    mEstouro = 0.0f;
    if (!mDesenhoDoToque) {
        auto ator = std::make_unique<Actor>(this);
        mDesenhoDoToque = ator->AddComponent<DrawLinhasComponent>(kOrdemDosMenus + 5);
        AddActor(std::move(ator));
    }
    if (mGame && mGame->GetAudio()) mGame->GetAudio()->PlaySound("toque.wav", false);
}

void Battle::AtualizarToque(const float deltaTime) {
    // O pulo dos dois, juntos.
    for (AvatarNoResultado& a : mAvataresNoResultado) {
        if (!a.ator) continue;
        a.pulo = std::min(a.pulo + deltaTime, Comunicacao::kDuracaoDoPulo);
        a.ator->SetPosition(a.base - Vector2(0.0f, Comunicacao::AlturaDoPulo(a.pulo)));
    }
    if (!mDesenhoDoToque) return;
    mDesenhoDoToque->Limpar();
    mEstouro = std::min(mEstouro + deltaTime, Comunicacao::kDuracaoDoEstouro);
    const float alfa = Comunicacao::AlfaDoEstouro(mEstouro);
    if (alfa <= 0.0f || !mPainelDoResultado) return;

    // O estouro dourado entre os dois: raios que abrem do meio do titulo, e um anel.
    const Vector2 meio = mPainelDoResultado->OndeDoTitulo();
    const float abre = Comunicacao::AberturaDoEstouro(mEstouro);
    const SDL_Color ouro{255, 215, 60, static_cast<Uint8>(255.0f * alfa)};
    constexpr int kRaios = 16;
    for (int r = 0; r < kRaios; ++r) {
        const float angulo = Math::TwoPi * static_cast<float>(r) / static_cast<float>(kRaios);
        const Vector2 direcao(Math::Cos(angulo), Math::Sin(angulo));
        // So os leques de cima e de baixo: dos lados estao os dois avatares, que pulam.
        if (std::abs(direcao.y) < 0.6f) continue;
        // De fora do titulo para fora, sem chegar na linha de baixo: emoldura a palavra sem cobri-la.
        const Vector2 escala(150.0f, 34.0f);
        const float perto = 0.95f + 0.25f * abre;
        const float longe = 1.1f + 0.4f * abre;
        const Vector2 de(meio.x + direcao.x * escala.x * perto, meio.y + direcao.y * escala.y * perto);
        const Vector2 ate(meio.x + direcao.x * escala.x * longe, meio.y + direcao.y * escala.y * longe);
        mDesenhoDoToque->Acrescentar(de, ate, ouro);
    }
}

void Battle::MostrarCausas(const std::vector<Devolutiva::Causa>& causas, const Vector2& onde) {
    // "Mais te acertou:" a esquerda; depois, cada causa: o desenho do projetil e quantas vezes.
    const int ordem = kOrdemDosMenus + 4;
    CriarTexto(*this, mFonteDaNota.get(), "Mais te acertou:", onde + Vector2(-150.0f, 0.0f), 26, 230, 34, ordem);
    float x = onde.x - 10.0f;
    for (const Devolutiva::Causa& c : causas) {
        const std::string vezes = "\xc3\x97" + std::to_string(c.acertos);
        const auto* fabrica = mChefe ? dynamic_cast<FabricaDeProjetil*>(mChefe->GetProjectileFactory(c.causa)) : nullptr;
        if (c.causa == Devolutiva::kEsbarrao || !fabrica) {
            // O esbarrao nao tem desenho de projetil: o nome dele.
            CriarTexto(*this, mFonteDaNota.get(), "encostar " + vezes, Vector2(x + 50.0f, onde.y), 26, 150, 34, ordem);
            x += 170.0f;
            continue;
        }
        auto icone = std::make_unique<Actor>(this);
        icone->SetPosition(Vector2(x, onde.y));
        auto* desenho = icone->AddComponent<DrawAnimatedComponent>(fabrica->CaminhoDoSprite(), fabrica->CaminhoDosDados(), ordem);
        for (const auto& anim : fabrica->Descricao().animacoes) desenho->AddAnimation(anim.nome, anim.quadros);
        desenho->SetAnimation(fabrica->Descricao().animacaoInicial);
        // Do tamanho de uma linha, por maior que seja o projetil.
        const float lado = static_cast<float>(std::max(desenho->GetSpriteWidth(), desenho->GetSpriteHeight()));
        // Pixel art cresce bem: o projetil miudo vai ate quatro vezes, o grande encolhe.
        icone->SetScale(lado > 0.0f ? std::min(4.0f, kLadoDoIcone / lado) : 1.0f);
        AddActor(std::move(icone));
        CriarTexto(*this, mFonteDaNota.get(), vezes, Vector2(x + 48.0f, onde.y), 26, 80, 34, ordem);
        x += 120.0f;
    }
}

void Battle::AnotarPerda(const Player& jogador, const std::string& causa, const float perda) {
    mCaderno.Perder(causa, perda);
    const int quem = PosicaoDoAluno(&jogador);
    if (quem >= 0 && static_cast<size_t>(quem) < mPartes.size()) ++mPartes[static_cast<size_t>(quem)].sofridos;
}

void Battle::ProcessarResultado(const Uint8* keyState) {
    // Lido mesmo na trava (Resultado::kTravaDoMenu).
    const BatidasDoMenu b = LerMenu(keyState);
    LerToque(keyState);
    if (mTempoNoFim < Resultado::kTravaDoMenu) return;

    if (!mPainelDoResultado) return;
    if (b.cima) mResultadoSelecionado = Resultado::Mover(mResultadoSelecionado, -1);
    if (b.baixo) mResultadoSelecionado = Resultado::Mover(mResultadoSelecionado, 1);
    mPainelDoResultado->Selecionar(mResultadoSelecionado);
    if (b.confirmar) EscolherNoResultado();
}

void Battle::EscolherNoResultado() {
    // A nota ja esta na ficha e a tentativa ja esta nos dados (EncerrarBatalha).
    switch (Resultado::kOpcoes[mResultadoSelecionado]) {
        case Resultado::Opcao::TentarDeNovo:
            mGame->RequestSceneChange(SceneType::Battle);
            break;
        case Resultado::Opcao::Voltar:
            mGame->RequestSceneChange(SceneType::StageSelect);
            break;
    }
}
//endregion Resultado
void Battle::VerificarColisoes() {
    Avisar([&](OuvinteDaBatalha& o) { o.AoAntesDasColisoes(*this); });
    if (mJogadores.empty() || !mChefe) return;

    const auto *bossCollider = mChefe->GetComponent<CircleColliderComponent>();
    if (!bossCollider) return;

    const auto &bossProjectiles = mProjeteis->GetBossProjectiles();
    const auto &playerProjectiles = mProjeteis->GetPlayerProjectiles();

    // A ordem das regras importa: cada uma ve o que a anterior ja derrubou.
    ColisoesDoAbrigo(bossProjectiles);
    ColisoesDoEscudo(bossProjectiles);
    ColisoesTiroDoChefeNoAluno(bossProjectiles);
    ColisoesTiroDoAlunoNosProjeteis(playerProjectiles, bossProjectiles);
    ColisoesTiroDoAlunoNoChefe(playerProjectiles);
    ColisoesDeContato();
    ColisoesComPontoExtra();
}

void Battle::ColisoesDoAbrigo(const std::vector<std::unique_ptr<BossProjectile>>& bossProjectiles) {
    for (const auto &abrigo : bossProjectiles) {
        if (abrigo->GetState() != ActorState::Active || !abrigo->Abrigo()) continue;
        const auto *colisorDoAbrigo = abrigo->GetComponent<CircleColliderComponent>();
        if (!colisorDoAbrigo) continue;
        for (const auto &bossProj : bossProjectiles) {
            if (bossProj->GetState() != ActorState::Active) continue;
            if (bossProj->Abrigo() || bossProj->Enfeite() || !bossProj->Posto().empty()) continue;
            const auto *outro = bossProj->GetComponent<CircleColliderComponent>();
            if (outro && colisorDoAbrigo->Intersect(*outro)) bossProj->Kill();
        }
    }
}

void Battle::ColisoesDoEscudo(const std::vector<std::unique_ptr<BossProjectile>>& bossProjectiles) {
    for (Player* jogador : mJogadores) {
        if (!jogador || jogador->TempoDeImunidade() <= 0.0f) continue;
        const auto* colisor = ColisorDe(jogador);
        const Vector2 centro = colisor ? colisor->GetCenter() : jogador->GetPosition();
        for (const auto &bossProj : bossProjectiles) {
            if (bossProj->GetState() != ActorState::Active) continue;
            if (bossProj->Abrigo() || bossProj->Enfeite() || !bossProj->Posto().empty()) continue;
            const auto *outro = bossProj->GetComponent<CircleColliderComponent>();
            if (outro && PontoExtra::NoEscudo(centro, outro->GetCenter(), outro->GetRadius())) bossProj->Kill();
        }
    }
}

void Battle::ColisoesTiroDoChefeNoAluno(const std::vector<std::unique_ptr<BossProjectile>>& bossProjectiles) {
    for (Player* jogador : mJogadores) {
        const auto *playerCollider = ColisorDe(jogador);
        if (!playerCollider) continue;

        for (const auto &bossProj: bossProjectiles) {
            if (bossProj->GetState() != ActorState::Active) continue;
            if (bossProj->Enfeite()) continue;   // o cursor da sprint e a mao, nao um tiro
            if (bossProj->Abrigo()) continue;    // a capivara nao machuca ninguem
            if (const auto *bossProjCollider = bossProj->GetComponent<CircleColliderComponent>()) {
                // O mesmo tiro nao acerta duas vezes seguidas (Acerto.h).
                if (playerCollider->Intersect(*bossProjCollider) && !jogador->EstaInvencivel() &&
                    !jogador->JaFoiAcertadoPor(bossProj.get(), bossProj->TempoDeVida())) {
                    jogador->DefinirInvencivel(true);
                    jogador->FoiAcertadoPor(bossProj.get(), bossProj->TempoDeVida());
                    jogador->OnCollision(bossProj.get());
                    bossProj->OnCollision(jogador);

                    // Nem todo acerto e a mesma coisa.
                    if (bossProj->Acorrenta()) {
                        jogador->Acorrentar(bossProj->SegundosDeCorrente(),
                                            bossProj->FatorDaCorrente());
                    }
                    AnotarPerda(*jogador, bossProj->Tipo(), mConta.Descer(bossProj->MultiplicadorDaPerda()));

                    Faiscar(playerCollider->GetCenter(), Hud::Faisca::Dano);
                    Avisar([&](OuvinteDaBatalha& o) { o.AoSerAcertado(*this, *jogador, true); });
                } else {
                    Raspar(*jogador, *playerCollider, *bossProj, *bossProjCollider);
                }
            }
        }
    }
}

Cobertura::Aluno Battle::AlunoDaCobertura(const Player* aluno) const {
    const auto* colisor = aluno ? ColisorDe(aluno) : nullptr;
    // O que nada atinge (o escudo, a piscada) nao precisa de cobertura.
    if (!colisor || aluno->EstaInvencivel()) return Cobertura::Aluno{};
    return Cobertura::Aluno{colisor->GetCenter(), colisor->GetRadius(), true};
}

bool Battle::Cobrir(const PlayerProjectile& tiro, const BossProjectile& projetil) {
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
    if (static_cast<size_t>(quem) < mPartes.size()) ++mPartes[static_cast<size_t>(quem)].coberturas;
    Faiscar(colisorDoProjetil->GetCenter(), Hud::Faisca::Cobertura, quem);
    // O valeu: sobe de quem foi coberto, na cor de quem cobriu - os dois veem que contou, na hora.
    if (mCondicao.Ligada(Experimento::Mecanica::Valeu)) {
        mValeus.push_back(ValeuEmCampo{parceiro, static_cast<size_t>(quem), 0.0f});
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
    mSinais.push_back(SinalEmCampo{&quem, static_cast<size_t>(i), 0.0f});
    if (mGame && mGame->GetAudio()) mGame->GetAudio()->PlaySound("sinal.wav", false);
}

void Battle::MarcarOCaminhoDosAlunos() {
    // Sozinho, nao ha quem cubra, e nenhum projetil brilha.
    if (mJogadores.size() < 2 || !mProjeteis) return;
    std::vector<Cobertura::Aluno> alunos;
    for (const Player* p : mJogadores) alunos.push_back(AlunoDaCobertura(p));
    const Uint8 alfa = Hud::AlfaDoBrilho(mPulsoDoEscudo);
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

void Battle::DesenharComunicacao(const float deltaTime) {
    const auto centroDe = [](const Player* p) {
        const auto* c = ColisorDe(p);
        return c ? c->GetCenter() : p->GetPosition();
    };
    const auto corDe = [](const size_t quem, const float alfa) {
        const Hud::Rgb c = Hud::CorDoAluno(quem);
        return SDL_Color{c.r, c.g, c.b, static_cast<Uint8>(255.0f * alfa)};
    };

    // O sinal: um anel que abre em volta de quem chamou, e um segundo, mais apagado, logo dentro.
    for (SinalEmCampo& s : mSinais) s.t += deltaTime;
    mSinais.erase(std::remove_if(mSinais.begin(), mSinais.end(),
        [](const SinalEmCampo& s) { return Comunicacao::AlfaDoSinal(s.t) <= 0.0f; }), mSinais.end());
    for (const SinalEmCampo& s : mSinais) {
        if (!s.quem) continue;
        const float raio = Comunicacao::RaioDoSinal(s.t);
        const float alfa = Comunicacao::AlfaDoSinal(s.t);
        for (const auto& [a, b] : PontoExtra::Anel(centroDe(s.quem), raio, 28)) {
            mDesenhoSobreOsAlunos->Acrescentar(a, b, corDe(s.cor, alfa));
        }
        for (const auto& [a, b] : PontoExtra::Anel(centroDe(s.quem), raio - 5.0f, 28)) {
            mDesenhoSobreOsAlunos->Acrescentar(a, b, corDe(s.cor, alfa * 0.5f));
        }
    }

    // O valeu: o coracao sobe da cabeca de quem foi coberto, na cor de quem cobriu, e apaga.
    for (ValeuEmCampo& v : mValeus) v.t += deltaTime;
    mValeus.erase(std::remove_if(mValeus.begin(), mValeus.end(),
        [](const ValeuEmCampo& v) { return Comunicacao::AlfaDoValeu(v.t) <= 0.0f; }), mValeus.end());
    constexpr float p = static_cast<float>(Comunicacao::kPixelDoCoracao);
    for (const ValeuEmCampo& v : mValeus) {
        if (!v.coberto) continue;
        const Vector2 meio = centroDe(v.coberto) -
                             Vector2(0.0f, Comunicacao::kAlturaDoValeu + Comunicacao::SubidaDoValeu(v.t));
        const Vector2 canto = meio - Vector2(p * Comunicacao::kLarguraDoCoracao, p * Comunicacao::kAlturaDoCoracao) * 0.5f;
        const SDL_Color cor = corDe(v.cor, Comunicacao::AlfaDoValeu(v.t));
        for (const Comunicacao::Faixa& f : Comunicacao::Coracao()) {
            for (int dy = 0; dy < Comunicacao::kPixelDoCoracao; ++dy) {
                const float y = canto.y + p * static_cast<float>(f.linha) + static_cast<float>(dy);
                mDesenhoSobreOsAlunos->Acrescentar(Vector2(canto.x + p * static_cast<float>(f.de), y),
                                                   Vector2(canto.x + p * static_cast<float>(f.ate + 1) - 1.0f, y),
                                                   cor);
            }
        }
    }
}

void Battle::Raspar(Player& jogador, const CircleColliderComponent& colisor, BossProjectile& projetil,
                    const CircleColliderComponent& colisorDoProjetil) {
    // O que esta parado num posto (a pilha, a tabela) e o que nem anda nao e um desvio; e quem esta
    // invencivel nao arrisca nada.
    if (jogador.EstaInvencivel() || !projetil.Posto().empty()) return;
    // A conta da faixa antes de procurar o corpo rigido: quase todo projetil passa longe.
    if (!Raspao::NaFaixa(colisor.GetCenter(), colisor.GetRadius(), colisorDoProjetil.GetCenter(),
                         colisorDoProjetil.GetRadius())) return;
    const auto* rb = projetil.GetComponent<RigidBodyComponent>();
    if (!rb || rb->GetVelocity().Length() < Raspao::kRapidezMinima) return;
    if (jogador.JaRaspou(&projetil, projetil.TempoDeVida())) return;

    jogador.Raspou(&projetil, projetil.TempoDeVida());
    // A medida conta sempre; o experimento sem raspao so nao paga nem acende.
    Avisar([&](OuvinteDaBatalha& o) { o.AoRaspar(*this, jogador); });
    if (!mCondicao.Ligada(Experimento::Mecanica::Raspao)) return;
    mConta.Subir(mConta.PorJogador(Raspao::kGanho));
    Faiscar(Raspao::OndeFaisca(colisor.GetCenter(), colisorDoProjetil.GetCenter()), Hud::Faisca::Raspao);
}

void Battle::ColisoesTiroDoAlunoNosProjeteis(const std::vector<std::unique_ptr<PlayerProjectile>>& playerProjectiles, const std::vector<std::unique_ptr<BossProjectile>>& bossProjectiles) {
    for (const auto &playerProj: playerProjectiles) {
        if (playerProj->GetState() != ActorState::Active) continue;
        const auto *tiro = playerProj->GetComponent<CircleColliderComponent>();
        if (!tiro) continue;

        // A promessa do Lucas (Promise.all, no TCC) desfaz e segue.
        if (playerProj->Promessa()) {
            for (const auto &bossProj: bossProjectiles) {
                if (bossProj->GetState() != ActorState::Active) continue;
                if (bossProj->Enfeite() || bossProj->Abrigo() || bossProj->Blindado() || !bossProj->Posto().empty()) continue;
                const auto *alvo = bossProj->GetComponent<CircleColliderComponent>();
                if (!alvo || !playerProj->Alcanca(*alvo)) continue;
                if (!playerProj->DesfazPelaPrimeiraVez(bossProj.get())) continue;

                if (bossProj->Derrubavel()) {
                    if (bossProj->Golpear()) {
                        if (!Cobrir(*playerProj, *bossProj)) Faiscar(bossProj->GetPosition(), Hud::Faisca::Derrubada);
                        bossProj->OnCollision(playerProj.get());
                        if (mChefe) {
                            mChefe->OnProjetilDerrubado();
                            mChefe->OnDerrubou(bossProj.get());
                        }
                    }
                } else {
                    Faiscar(bossProj->GetPosition(), Hud::Faisca::Dissolvido);
                    bossProj->Kill();
                }
            }
            continue;
        }

        for (const auto &bossProj: bossProjectiles) {
            if (bossProj->GetState() != ActorState::Active) continue;
            if (!bossProj->Derrubavel() && !bossProj->Blindado() && !bossProj->Abrigo()) continue;
            if (bossProj->Enfeite()) continue;   // o tiro passa pelo cursor
            const auto *alvo = bossProj->GetComponent<CircleColliderComponent>();
            if (!alvo || !playerProj->Alcanca(*alvo)) continue;

            if (!Destrutivel::Pode(bossProj->Derrubavel(), bossProj->Blindado(), bossProj->Abrigo(), bossProj->Enfeite())) {
                playerProj->Murchar();
                break;
            }
            playerProj->OnCollision(bossProj.get());
            {
                // A resistencia: so o ultimo golpe derruba.
                if (bossProj->Golpear()) {
                    if (!Cobrir(*playerProj, *bossProj)) Faiscar(bossProj->GetPosition(), Hud::Faisca::Derrubada);
                    bossProj->OnCollision(playerProj.get());
                    if (mChefe) {
                        mChefe->OnProjetilDerrubado();
                        mChefe->OnDerrubou(bossProj.get());
                    }
                } else if (mChefe) {
                    Faiscar(playerProj->GetPosition(), Hud::Faisca::Acerto);
                    mChefe->OnProjetilBarrado();
                }
            }
            break;   // um tiro derruba um processo so, ou some numa historia so
        }
    }
}

void Battle::ColisoesTiroDoAlunoNoChefe(const std::vector<std::unique_ptr<PlayerProjectile>>& playerProjectiles) {
    for (const auto &playerProj: playerProjectiles) {
        if (playerProj->GetState() != ActorState::Active) continue;

        const auto *playerProjCollider = playerProj->GetComponent<CircleColliderComponent>();
        if (!playerProjCollider) continue;

        for (Boss* corpo : mCorpos) {
            const auto* colisorDoCorpo = corpo ? corpo->GetComponent<CircleColliderComponent>()
                                               : nullptr;
            if (!colisorDoCorpo) continue;

            if (playerProj->Alcanca(*colisorDoCorpo)) {
                // A promessa que ja volta passa pelo Hugo: ela ja resolveu.
                if (playerProj->Promessa() && playerProj->Voltando()) continue;

                corpo->OnCollision(playerProj.get());
                // O chefe blindado nao ouve a pergunta.
                const bool ouviu = corpo->AceitaDano();
                if (playerProj->Promessa()) playerProj->Voltar();
                else if (ouviu) playerProj->OnCollision(corpo);
                else playerProj->Murchar();

                // Acerto de um jogador: repartido.
                Avisar([&](OuvinteDaBatalha& o) { o.AoAcertarOProfessor(*this, playerProj->Dono(), ouviu); });

                if (ouviu) {
                    // A duvida perguntada de perto vale mais (Nota::FatorDePerto) - a do aluno; a do
                    // orientador, nao.
                    const int quem = PosicaoDoAluno(playerProj->Dono());
                    // Quem pergunta incomoda, e chama a mira (Papeis.h).
                    if (quem >= 0) mIncomodo.Acertou(static_cast<size_t>(quem));
                    if (quem >= 0 && static_cast<size_t>(quem) < mPartes.size()) ++mPartes[static_cast<size_t>(quem)].acertos;
                    const float fatorDaDistancia = quem >= 0 ? Nota::FatorDePerto((playerProj->Dono()->GetPosition() -
                                                                                   colisorDoCorpo->GetCenter()).Length())
                                                             : 1.0f;
                    // Perguntou de perto: conta pela distancia; o experimento sem o perto so nao paga.
                    if (quem >= 0 && fatorDaDistancia >= Nota::kFatorDaRespostaDupla) {
                        const Player& aluno = *mJogadores[static_cast<size_t>(quem)];
                        Avisar([&](OuvinteDaBatalha& o) { o.AoAcertarDePerto(*this, aluno); });
                    }
                    const float fator = mCondicao.Ligada(Experimento::Mecanica::Perto) ? fatorDaDistancia : 1.0f;
                    mConta.Subir(mConta.PorJogador(Nota::kGanhoPorAcerto * fator));
                    // A duvida respondida: o "?" vira um "!" na cor de quem perguntou - "!!" de perto.
                    Faiscar(playerProj->GetPosition(),
                            fator >= Nota::kFatorDaRespostaDupla ? Hud::Faisca::RespostaDePerto : Hud::Faisca::Resposta,
                            quem);
                }

                // Todo acerto carrega o poder do orientador, blindado ou nao.
                if (mOrientador) mOrientador->Carregar();

                // Um tiro acerta um corpo so.
                break;
            }
        }
    }
}

void Battle::ColisoesDeContato() {
    for (Player* jogador : mJogadores) {
        const auto *playerCollider = ColisorDe(jogador);
        if (!playerCollider) continue;

        for (Boss* corpo : mCorpos) {
            const auto* colisorDoCorpo = corpo ? corpo->GetComponent<CircleColliderComponent>()
                                               : nullptr;
            if (!colisorDoCorpo) continue;

            if (playerCollider->Intersect(*colisorDoCorpo) && !jogador->EstaInvencivel() &&
                !jogador->JaFoiAcertadoPor(corpo, 0.0f)) {
                jogador->DefinirInvencivel(true);
                jogador->FoiAcertadoPor(corpo, 0.0f);
                jogador->OnCollision(corpo);
                corpo->OnCollision(jogador);
                AnotarPerda(*jogador, Devolutiva::kEsbarrao, mConta.Descer());
                Avisar([&](OuvinteDaBatalha& o) { o.AoSerAcertado(*this, *jogador, false); });
            }
        }
    }
}

void Battle::ColisoesComPontoExtra() {
    bool playedPowerUpSound = false;

    for (Player* jogador : mJogadores) {
        const auto *playerCollider = ColisorDe(jogador);
        if (!playerCollider) continue;

        for (const auto item : mPontosExtras) {
            if (item->GetState() != ActorState::Active) continue;

            if (const auto itemCollider = item->GetComponent<CircleColliderComponent>(); itemCollider && playerCollider->Intersect(*itemCollider)) {

                // 1.
                jogador->GanharPontoExtra();
                Faiscar(playerCollider->GetCenter(), Hud::Faisca::Coleta);
                Avisar([&](OuvinteDaBatalha& o) { o.AoPegarPontoExtra(*this, *jogador); });

                // 2.
                if (!playedPowerUpSound) {
                    // Toca só se ainda não tocou neste frame.
                    mGame->GetAudio()->PlaySound("powerup.wav", false);
                    playedPowerUpSound = true;
                }

                // 3.
                item->SetState(ActorState::Destroy);
            }
        }
    }

    // A nota nao e mais escrita aqui.
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
    if (posicao < mJogadores.size()) {
        mVezDoAlvo = posicao;
        mVezDada = true;
    }
}

void Battle::RevezarAlvo() {

    // A vez dada valia para um disparo, e ele ja saiu.
    mVezDada = false;

    // O "% tamanho" e o que faz isto nao dar nada com um jogador: 0 + 1 = 1, e 1 % 1 = 0.
    if (mJogadores.empty()) { mVezDoAlvo = 0; return; }
    mVezDoAlvo = (mVezDoAlvo + 1) % mJogadores.size();
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
    mCaderno.ComecarFase(anuncio, fase == "StateFinal");
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
        mOndas.push_back(Onda{colisor ? colisor->GetCenter() : quemGastou->GetPosition(), 0.0f});
        Faiscar(colisor ? colisor->GetCenter() : quemGastou->GetPosition(), Hud::Faisca::Coleta);
        return;
    }

    // Em dupla, o Ponto Extra e do outro, e nao limpa a tela.
    oOutro->Proteger(PontoExtra::kImunidadeDoPresente);
    mFeixes.push_back(FeixeDoPontoExtra{quemGastou, oOutro, 0.0f});

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
    mOndas.push_back(Onda{meio, 0.0f});
    Faiscar(meio, Hud::Faisca::Coleta);
}

void Battle::AtualizarDesenhoSobreOsAlunos(const float deltaTime) {
    const bool hitbox = mGame && mGame->MostrarHitbox();
    if (!mDesenhoSobreOsAlunos) {
        if (mJogadores.size() < 2 && !hitbox) return;   // so a dupla tem feixe e escudo
        auto ator = std::make_unique<Actor>(this);
        mDesenhoSobreOsAlunos = ator->AddComponent<DrawLinhasComponent>(kOrdemSobreOsAlunos);
        AddActor(std::move(ator));
    }
    mDesenhoSobreOsAlunos->Limpar();

    auto centroDe = [](const Player* p) {
        const auto* c = ColisorDe(p);
        return c ? c->GetCenter() : p->GetPosition();
    };

    // Os feixes: a cabeca corre ate o parceiro, e depois a linha se apaga.
    for (FeixeDoPontoExtra& f : mFeixes) f.t += deltaTime;
    mFeixes.erase(std::remove_if(mFeixes.begin(), mFeixes.end(),
        [](const FeixeDoPontoExtra& f) { return !PontoExtra::FeixeAceso(f.t); }), mFeixes.end());
    for (const FeixeDoPontoExtra& f : mFeixes) {
        if (!f.de || !f.ate) continue;
        const Vector2 de = centroDe(f.de), ate = centroDe(f.ate);
        const PontoExtra::Feixe feixe = PontoExtra::FeixeEm(de, ate, f.t);
        const auto alfa = static_cast<Uint8>(110.0f * feixe.rastro);
        if (alfa > 0) mDesenhoSobreOsAlunos->Acrescentar(de, ate, SDL_Color{150, 210, 255, alfa});
        if (feixe.cabeca) {
            // A cabeca tem corpo: tres tracos lado a lado, o do meio mais claro.
            Vector2 lado(ate.y - de.y, de.x - ate.x);
            if (lado.LengthSq() > 0.0f) lado.Normalize();
            mDesenhoSobreOsAlunos->Acrescentar(feixe.cabecaDe + lado * 2.0f, feixe.cabecaAte + lado * 2.0f, SDL_Color{150, 210, 255, 200});
            mDesenhoSobreOsAlunos->Acrescentar(feixe.cabecaDe - lado * 2.0f, feixe.cabecaAte - lado * 2.0f, SDL_Color{150, 210, 255, 200});
            mDesenhoSobreOsAlunos->Acrescentar(feixe.cabecaDe, feixe.cabecaAte, SDL_Color{235, 250, 255, 255});
        }
    }

    mPulsoDoEscudo += deltaTime;
    for (const Player* p : mJogadores) {
        if (!p || p->TempoDeImunidade() <= 0.0f) continue;
        const float pulso = 0.5f + 0.5f * Math::Sin(mPulsoDoEscudo * (p->TempoDeImunidade() < 0.8f ? 22.0f : 8.0f));
        const auto alfa = static_cast<Uint8>(120.0f + 120.0f * pulso);
        // Dois aneis, o de dentro mais apagado: a borda do escudo tem espessura.
        for (const auto& [a, b] : PontoExtra::Anel(centroDe(p), PontoExtra::kRaioDoEscudo)) {
            mDesenhoSobreOsAlunos->Acrescentar(a, b, SDL_Color{120, 190, 255, alfa});
        }
        for (const auto& [a, b] : PontoExtra::Anel(centroDe(p), PontoExtra::kRaioDoEscudo - 3.0f)) {
            mDesenhoSobreOsAlunos->Acrescentar(a, b, SDL_Color{120, 190, 255, static_cast<Uint8>(alfa / 2)});
        }
    }

    DesenharComunicacao(deltaTime);

    // A mira presa (Papeis.h): um "v" vermelho em cima de quem o professor esta olhando. So quando ela
    // esta presa - enquanto os dois revezam ele piscaria a cada disparo, e nao diria nada.
    const int foco = FocoDaMira();
    if (foco >= 0 && static_cast<size_t>(foco) < mJogadores.size() && mJogadores[static_cast<size_t>(foco)]) {
        const Vector2 c = centroDe(mJogadores[static_cast<size_t>(foco)]) + Vector2(0.0f, -kAlturaDaMira);
        const float pulso = 0.5f + 0.5f * Math::Sin(mPulsoDoEscudo * 6.0f);
        const auto alfa = static_cast<Uint8>(170.0f + 85.0f * pulso);
        const auto v = [&](const float dy, const float abre, const SDL_Color cor) {
            mDesenhoSobreOsAlunos->Acrescentar(c + Vector2(-abre, dy - abre * 0.8f), c + Vector2(0.0f, dy), cor);
            mDesenhoSobreOsAlunos->Acrescentar(c + Vector2(abre, dy - abre * 0.8f), c + Vector2(0.0f, dy), cor);
        };
        for (float dy = -1.0f; dy <= 3.0f; dy += 1.0f) v(dy, 9.0f, SDL_Color{20, 22, 34, static_cast<Uint8>(alfa * 0.7f)});
        for (float dy = 0.0f; dy <= 2.0f; dy += 1.0f) v(dy, 7.0f, SDL_Color{255, 80, 70, alfa});
    }

    // A hitbox (opcao da tela de Opcoes): o circulo de colisao de verdade, e um ponto no meio.
    if (!hitbox) return;
    for (const Player* p : mJogadores) {
        const auto* c = p ? ColisorDe(p) : nullptr;
        if (!c) continue;
        const SDL_Color vermelho{255, 70, 70, 230};
        for (const auto& [a, b] : PontoExtra::Anel(c->GetCenter(), c->GetRadius(), 16)) {
            mDesenhoSobreOsAlunos->Acrescentar(a, b, vermelho);
        }
        const Vector2 m = c->GetCenter();
        mDesenhoSobreOsAlunos->Acrescentar(m - Vector2(2.0f, 0.0f), m + Vector2(2.0f, 0.0f), SDL_Color{255, 255, 255, 255});
    }
}

// A moldura do campo e os efeitos.

void Battle::CriarFundo() {
    // O caderno quadriculado embaixo de tudo, do tamanho do campo.
    auto fundo = std::make_unique<Actor>(this);
    fundo->AddComponent<DrawFundoDoCampoComponent>(GetPlayfieldBounds(), kOrdemDoFundo);
    AddActor(std::move(fundo));
}

void Battle::CriarMoldura() {
    // A area do aluno sai do mesmo tamanho que o Player::PrenderNoCampo usa.
    const SDL_FRect campo = GetPlayfieldBounds();
    float largura = 32.0f, altura = 48.0f;
    if (!mJogadores.empty() && mJogadores.front()) {
        if (const auto* sprite = mJogadores.front()->GetComponent<DrawAnimatedComponent>()) {
            largura = static_cast<float>(sprite->GetSpriteWidth());
            altura = static_cast<float>(sprite->GetSpriteHeight());
        }
    }
    mAreaDoAluno = Hud::AreaDoAluno({campo.x, campo.y, campo.x + campo.w, campo.y + campo.h}, largura, altura);

    auto moldura = std::make_unique<Actor>(this);
    mMoldura = moldura->AddComponent<DrawMolduraDoCampoComponent>(kOrdemDaMoldura);
    mMoldura->Definir(static_cast<float>(mGame->GetWindowWidth()), static_cast<float>(mGame->GetWindowHeight()),
                      mAreaDoAluno);
    AddActor(std::move(moldura));

    auto efeitos = std::make_unique<Actor>(this);
    mEfeitos = efeitos->AddComponent<DrawLinhasComponent>(kOrdemDosEfeitos);
    AddActor(std::move(efeitos));
}

void Battle::AtualizarMoldura() {
    if (!mMoldura) return;
    std::vector<std::pair<Vector2, Vector2>> alunos;
    for (const Player* p : mJogadores) {
        if (!p) continue;
        const auto* sprite = p->GetComponent<DrawAnimatedComponent>();
        if (!sprite) continue;
        alunos.emplace_back(p->GetPosition(), Vector2(static_cast<float>(sprite->GetSpriteWidth()) / 2.0f,
                                                      static_cast<float>(sprite->GetSpriteHeight()) / 2.0f));
    }
    mMoldura->SetAlunos(std::move(alunos));

    std::vector<float> xs;
    for (const Boss* corpo : mCorpos) {
        if (corpo && corpo->GetState() == ActorState::Active) {
            xs.push_back(std::clamp(corpo->GetPosition().x, mAreaDoAluno.esquerda, mAreaDoAluno.direita));
        }
    }
    while (mRotulosDoProfessor.size() < xs.size() && mFonteDoPlacar) {
        auto ator = std::make_unique<Actor>(this);
        auto* texto = ator->AddComponent<DrawTextComponent>("prof", mFonteDoPlacar.get(), 60, 18, 14, kOrdemDaMoldura + 2);
        texto->SetAjustarAoTexto(true);
        texto->SetColor(Vector3(1.0f, 0.45f, 0.4f));
        mRotulosDoProfessor.push_back(AddActor(std::move(ator)));
    }
    for (size_t i = 0; i < mRotulosDoProfessor.size(); ++i) {
        auto* texto = mRotulosDoProfessor[i]->GetComponent<DrawTextComponent>();
        const bool aceso = i < xs.size();
        if (texto) texto->SetIsVisible(aceso);
        if (aceso) mRotulosDoProfessor[i]->SetPosition(Vector2(xs[i], mAreaDoAluno.baixo + 13.0f));
    }
    mMoldura->SetProfessores(std::move(xs));
    mMoldura->SetRede(mRede);
}

void Battle::Atravessou(const Vector2& de, const Vector2& para) {
    Faiscar(de, Hud::Faisca::Coleta);
    Faiscar(para, Hud::Faisca::Coleta);
    Avisar([&](OuvinteDaBatalha& o) { o.AoAtravessarAParede(*this); });
}

void Battle::AtualizarOndas(const float deltaTime) {
    for (Onda& o : mOndas) {
        o.t += deltaTime;
        const float raio = PontoExtra::RaioDaOnda(o.t);
        if (!mProjeteis) continue;
        // O mesmo filtro do ClearAtaquesDoChefe.
        for (const auto& p : mProjeteis->GetBossProjectiles()) {
            if (!p || p->Enfeite()) continue;
            const auto estado = p->GetState();
            if (estado != ActorState::Active && estado != ActorState::Paused) continue;
            if ((p->GetPosition() - o.centro).Length() > raio) continue;
            const Vector2 onde = p->GetPosition();
            p->Kill();
            Faiscar(onde, Hud::Faisca::Dissolvido);
            if (mMoedas.size() < kMaximoDeMoedas) mMoedas.push_back(Moeda{onde, 0.0f});
        }
    }
    mOndas.erase(std::remove_if(mOndas.begin(), mOndas.end(),
                                [](const Onda& o) { return o.t >= PontoExtra::kDuracaoDaOnda; }),
                 mOndas.end());
    for (Moeda& m : mMoedas) m.t += deltaTime;
    mMoedas.erase(std::remove_if(mMoedas.begin(), mMoedas.end(),
                                 [](const Moeda& m) { return m.t >= kVooDaMoeda; }),
                  mMoedas.end());
}

void Battle::Faiscar(const Vector2& onde, const Hud::Faisca tipo, const int quem) {
    if (mFaiscas.size() >= kMaximoDeFaiscas) mFaiscas.erase(mFaiscas.begin());
    mFaiscas.push_back({onde, tipo, 0.0f, quem});
}

int Battle::PosicaoDoAluno(const Actor* ator) const {
    for (size_t i = 0; i < mJogadores.size(); ++i) {
        if (ator && mJogadores[i] == ator) return static_cast<int>(i);
    }
    return -1;
}

void Battle::AtualizarEfeitos(const float deltaTime) {
    if (!mEfeitos) return;
    mEfeitos->Limpar();
    for (FaiscaEmCampo& f : mFaiscas) f.idade += deltaTime;
    mFaiscas.erase(std::remove_if(mFaiscas.begin(), mFaiscas.end(),
                                  [](const FaiscaEmCampo& f) { return f.idade >= Hud::Vida(f.tipo); }),
                   mFaiscas.end());

    const auto alfa = [](const float a) { return static_cast<Uint8>(std::clamp(a, 0.0f, 1.0f) * 255.0f); };
    const auto raios = [this](const Vector2& c, const float de, const float ate, const int n, const float giro,
                              const SDL_Color cor) {
        for (int k = 0; k < n; ++k) {
            const float a = giro + 2.0f * Math::Pi * static_cast<float>(k) / static_cast<float>(n);
            const Vector2 u(Math::Cos(a), Math::Sin(a));
            mEfeitos->Acrescentar(c + u * de, c + u * ate, cor);
        }
    };
    const auto anel = [this](const Vector2& c, const float r, const SDL_Color cor) {
        for (const auto& [a, b] : PontoExtra::Anel(c, r, 24)) mEfeitos->Acrescentar(a, b, cor);
    };

    for (const FaiscaEmCampo& f : mFaiscas) {
        const float r = Hud::Raio(f.tipo, f.idade);
        const float b = Hud::Brilho(f.tipo, f.idade);
        // O giro de cada uma sai de onde ela nasceu: duas lado a lado nao sao a mesma estrela repetida.
        const float giro = f.onde.x * 0.37f + f.onde.y * 0.11f;
        switch (f.tipo) {
            case Hud::Faisca::Acerto:
                // Miuda e branca: sao dez por segundo, e nao podem cobrir o chefe.
                raios(f.onde, r * 0.45f, r, 5, giro, SDL_Color{255, 250, 220, alfa(0.9f * b)});
                break;
            case Hud::Faisca::Derrubada:
                raios(f.onde, r * 0.35f, r, 8, giro, SDL_Color{255, 160, 60, alfa(b)});
                anel(f.onde, r * 0.7f, SDL_Color{255, 210, 120, alfa(0.6f * b)});
                break;
            case Hud::Faisca::Dano:
                anel(f.onde, r, SDL_Color{255, 70, 60, alfa(b)});
                anel(f.onde, r - 3.0f, SDL_Color{255, 70, 60, alfa(0.5f * b)});
                raios(f.onde, r * 0.25f, r * 0.6f, 8, giro, SDL_Color{255, 120, 100, alfa(0.8f * b)});
                break;
            case Hud::Faisca::Coleta:
                anel(f.onde, r, SDL_Color{255, 215, 80, alfa(b)});
                raios(f.onde, r * 0.8f, r * 1.15f, 4, giro + f.idade * 6.0f, SDL_Color{255, 240, 160, alfa(b)});
                break;
            case Hud::Faisca::Dissolvido:
                // O tiro vira brilho: uma estrela dourada de quatro pontas.
                raios(f.onde, 0.0f, r, 4, giro, SDL_Color{255, 225, 120, alfa(b)});
                raios(f.onde, 0.0f, r * 0.55f, 4, giro + Math::Pi / 4.0f, SDL_Color{255, 250, 210, alfa(b)});
                break;
            case Hud::Faisca::Resposta:
            case Hud::Faisca::RespostaDePerto: {
                // O "!" sobe e some, na cor do aluno, com o miolo claro e um contorno escuro. De perto
                // sao dois: "!!".
                const Hud::Rgb c = Hud::CorDoAluno(f.quem < 0 ? 9 : static_cast<size_t>(f.quem));
                const SDL_Color cor{c.r, c.g, c.b, alfa(b)};
                const SDL_Color miolo{255, 255, 240, alfa(b)};
                const SDL_Color contorno{20, 22, 34, alfa(0.85f * b)};
                const Vector2 p = f.onde + Vector2(0.0f, -10.0f - r);
                const auto exclamacao = [&](const float x0, const int largura, const float folga, const SDL_Color cor) {
                    for (int dx = -largura; dx <= largura; ++dx) {
                        const float x = x0 + static_cast<float>(dx);
                        mEfeitos->Acrescentar(p + Vector2(x, -16.0f - folga), p + Vector2(x, -4.0f + folga), cor);
                        mEfeitos->Acrescentar(p + Vector2(x, -1.0f - folga), p + Vector2(x, 2.0f + folga), cor);
                    }
                };
                const bool dois = f.tipo == Hud::Faisca::RespostaDePerto;
                constexpr float kLados[] = {-5.0f, 5.0f};
                for (int i = 0; i < (dois ? 2 : 1); ++i) {
                    const float x0 = dois ? kLados[i] : 0.0f;
                    exclamacao(x0, 3, 1.0f, contorno);
                    exclamacao(x0, 2, 0.0f, cor);
                    exclamacao(x0, 0, -1.0f, miolo);
                }
                break;
            }
            case Hud::Faisca::Cobertura: {
                // A derrubada dourada, com o anel na cor de quem cobriu: salvou o parceiro.
                const Hud::Rgb c = Hud::CorDoAluno(f.quem < 0 ? 9 : static_cast<size_t>(f.quem));
                raios(f.onde, r * 0.3f, r, 8, giro, SDL_Color{255, 220, 110, alfa(b)});
                anel(f.onde, r * 0.75f, SDL_Color{c.r, c.g, c.b, alfa(0.9f * b)});
                anel(f.onde, r * 0.75f - 2.0f, SDL_Color{255, 245, 200, alfa(0.6f * b)});
                break;
            }
            case Hud::Faisca::Raspao:
                // Tres riscos azul-claros, na borda do aluno, com um contorno escuro que os separa do
                // desenho dele: passou perto.
                raios(f.onde, r * 0.25f, r, 3, giro, SDL_Color{20, 22, 34, alfa(0.6f * b)});
                raios(f.onde + Vector2(1.0f, 0.0f), r * 0.25f, r, 3, giro, SDL_Color{20, 22, 34, alfa(0.6f * b)});
                raios(f.onde + Vector2(0.0f, -1.0f), r * 0.3f, r, 3, giro, SDL_Color{170, 225, 255, alfa(b)});
                break;
        }
    }

    for (const Onda& o : mOndas) {
        const float raio = PontoExtra::RaioDaOnda(o.t);
        const float some = 1.0f - o.t / PontoExtra::kDuracaoDaOnda;
        const int lados = std::clamp(static_cast<int>(raio / 6.0f), 24, 160);
        for (int k = 0; k < 3; ++k) {
            const float rk = raio - static_cast<float>(k) * 7.0f;
            if (rk <= 0.0f) continue;
            const SDL_Color cor{255, static_cast<Uint8>(225 - 25 * k), static_cast<Uint8>(120 - 30 * k),
                                alfa(some * (1.0f - 0.3f * static_cast<float>(k)))};
            for (const auto& [a, b] : PontoExtra::Anel(o.centro, rk, lados)) mEfeitos->Acrescentar(a, b, cor);
        }
    }

    // As moedas voam ate a ponta da barra da nota, num arco: o ganho tem de onde vir.
    const Vector2 ponta = mFaixa->Ponta(mConta.Valor());
    for (const Moeda& m : mMoedas) {
        const float u = std::clamp(m.t / kVooDaMoeda, 0.0f, 1.0f);
        const float e = u * u;                                  // devagar, depois depressa
        const Vector2 meio = (m.de + ponta) * 0.5f + Vector2(0.0f, -140.0f);
        const Vector2 p = m.de * ((1.0f - e) * (1.0f - e)) + meio * (2.0f * e * (1.0f - e)) + ponta * (e * e);
        const SDL_Color ouro{255, 215, 80, alfa(1.0f - 0.3f * u)};
        mEfeitos->Acrescentar(p + Vector2(-4.0f, 0.0f), p + Vector2(0.0f, -4.0f), ouro);
        mEfeitos->Acrescentar(p + Vector2(0.0f, -4.0f), p + Vector2(4.0f, 0.0f), ouro);
        mEfeitos->Acrescentar(p + Vector2(4.0f, 0.0f), p + Vector2(0.0f, 4.0f), ouro);
        mEfeitos->Acrescentar(p + Vector2(0.0f, 4.0f), p + Vector2(-4.0f, 0.0f), ouro);
    }
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
        resumo.piorFase = mCaderno.PiorFase();
        resumo.causas = mCaderno.Causas();
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
    if (mPartes.size() >= 2) {
        resumo.partes = mPartes;
        for (size_t i = 0; i < resumo.partes.size(); ++i) resumo.partes[i].quem = nomeDe(i);
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
    mTempoNoFim = 0.0f;

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
    MostrarResultado(resumo);
}
