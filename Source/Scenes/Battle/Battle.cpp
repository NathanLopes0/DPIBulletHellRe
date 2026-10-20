//
// Created by nslop on 16/09/2024.
//

#include "SDL.h"
#include "Battle.h"

#include "../../Progresso.h"
#include "../../Nota.h"

#include <locale>
#include <sstream>
#include <iomanip>

#include "BattleHUD.h"
#include "ProjectileManager.h"
#include "../../AudioSystem.h"
#include "../../Json.h"
#include "../../Font.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teachers/Boss.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Actors/Teachers/BossFactory/IBossFactory.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Components/DrawComponents/ProgressBarComponent.h"
#include "../../Actors/Teachers/ExtraPointItem.h"

#include "../../CaminhosArquivo.h"

// Quantas instâncias de CADA tipo de projétil são pré-aquecidas no pool
// antes da batalha começar (ver Battle::LoadBoss / PrewarmProjectilePools).

// O pool CRESCE sozinho quando falta
// (Acquire cria um novo se estiver vazio), entao este numero e so uma
// otimizacao de partida, nunca um limite de projeteis simultaneos. 
static constexpr int kProjectilePrewarmCountPerType = 300;

namespace {

/// O colisor de um jogador, ou nullptr se nao houver jogador ou colisor.
///
/// Existe porque as tres regras de colisao que sao de jogador pedem a mesma
/// coisa, e com a lista isso passou a acontecer uma vez por jogador por regra.
const CircleColliderComponent* ColisorDe(const Player* jogador) {
    return jogador ? jogador->GetComponent<CircleColliderComponent>() : nullptr;
}

} // namespace



Battle::Battle(Game* game, const int selectedStage)
    : Scene(game, SceneType::Battle)
    , mGrade(40.0f)
    , mStage(selectedStage)
{
    //SDL_Log(" BATTLE - Construtor");
}

Battle::~Battle() = default;

//region LoadFunctions
void Battle::Load() {

    //SDL_Log(" BATTLE - LOAD - Iniciando");
    // 1. Criar os sistemas primeiro
    mProjectileManager = std::make_unique<ProjectileManager>(this);

    // TODA batalha comeca em 40, mesmo que o aluno ja tenha tirado mais nesta
    // materia. Antes ela comecava na ultima nota, o que fazia quem tinha ido bem
    // comecar perto de passar - a fase virava mais facil justamente para quem ja
    // tinha passado. A ultima nota continua guardada na ficha, como informacao
    // para o professor; ela so nao e mais o ponto de partida.
    mGrade = Progresso::kNotaInicial;

    // E o mesmo vale para o dano: a contagem de acertos zera junto com a nota,
    // porque o teto e desta tentativa. Quem foi atingido ontem nao perde a nota
    // cheia de hoje.
    mAcertosSofridos = 0;
    mAcertosNaFase = 0;
    mPrimeiraFase = true;
    mAvisoDeFaseLimpa = 0.0f;

    // 2. Criar os atores principais
    LoadPlayer();
    LoadBoss();
    LoadHUD();
    LoadEndScreen();

    if (mGame->GetAudio()) {
        mMusicHandle = mGame->GetAudio()->PlaySound("backgroundmusic.ogg", true);
        mGame->GetAudio()->SetSoundVolume(mMusicHandle, 32);
    }

    // 3. Pré-aquecer os pools de projétil ANTES de iniciar a lógica de
    // ataque do Boss. mBoss->Start() (que dispara a FSM e os primeiros
    // ataques) só é chamado quando o carregamento incremental terminar —
    // ver FinishLoadingProjectilePools(). Enquanto isso, mIsLoading fica
    // true e OnUpdate() desenha/avança a tela de carregamento em vez da
    // lógica normal de gameplay.
    StartLoadingProjectilePools();

}
void Battle::LoadBoss() {

    if (IBossFactory *factory = mGame->GetFactory(mStage)) {

        auto boss = factory->CreateBoss(this);
        mBoss = dynamic_cast<Boss*>(this->AddActor(std::move(boss)));

        // O ORIGINAL E O PRIMEIRO CORPO. A lista existe mesmo sem divisao
        // nenhuma: e o que faz as regras de colisao terem UM caminho so, em vez
        // de um para "normal" e outro para "dividido".
        mCorpos.clear();
        if (mBoss) mCorpos.push_back(mBoss);

        mBoss->SetPosition(Vector2(static_cast<float>(mGame->GetWindowWidth())/2.0f,
                                        static_cast<float>(mGame->GetWindowHeight())/6.0f));
        // O pré-aquecimento dos pools de projétil (que já foi cogitado aqui)
        // acontece de forma incremental em StartLoadingProjectilePools() /
        // UpdateLoadingStep(), chamado a partir de Load(), DEPOIS que
        // LoadBoss() retorna — porque precisa acontecer espalhado ao longo
        // de vários frames (ver comentário em Load()), não de forma síncrona
        // aqui dentro.
    } else {
        // %d, e nao %s: mStage e o enum int, ou seja um inteiro. Com %s o
        // SDL_Log tratava o numero como um char* e tentava ler a string no endereco
        // 2 - segmentation fault DENTRO da mensagem de erro. As materias sem
        // fabrica (seis das onze, hoje) derrubavam o jogo aqui, e o defeito
        // sobreviveu porque so roda quando o erro acontece.
        SDL_Log("Erro fatal: Nenhuma BossFactory encontrada para a materia %d, "
                "voltando pra StageSelect", static_cast<int>(mStage));
        mGame->RequestSceneChange(SceneType::StageSelect);
    }
}
void Battle::LoadHUD() {

    //Cria a HUD para segurar todas as infos
    mHUD = std::make_unique<BattleHUD>(this);
    LoadTimeBar();
    LoadGradeBar();

}
void Battle::LoadPlayer() {

    // UM ALUNO POR SESSAO. Quantas sessoes ha foi decidido no menu, e a
    // batalha nao pergunta se e co-op: ela percorre o que existe. E isso que
    // mantem o jogo de um jogador sendo o caso normal, sem "if (co-op)".
    const size_t quantos = mGame->QuantosJogadores();

    const SDL_FRect campo = GetPlayfieldBounds();

    for (size_t quem = 0; quem < quantos; ++quem) {

        auto player = std::make_unique<Player>(this, quem);

        // NASCE DENTRO DO CAMPO, e nao numa fracao da JANELA.
        //
        // Era 7/8 da altura da janela, ou seja 700. Com o campo acabando onde a
        // faixa da nota comeca, o chao do aluno passou a ser 682 - e ele nascia
        // ABAIXO dele, para ser empurrado de volta pelo limite no primeiro
        // quadro. Um empurrao que ninguem pediu, e que mudaria de tamanho a
        // cada vez que a faixa da nota mudasse de altura.
        const auto alturaDoAluno = static_cast<float>(
            player->GetComponent<DrawAnimatedComponent>()->GetSpriteHeight());

        // LADO A LADO, repartindo a largura em partes iguais: com um, a conta
        // da o meio da tela, que e exatamente onde ele sempre nasceu. Nao ha
        // um caso para um e outro para dois.
        const float fatia = campo.w / static_cast<float>(quantos + 1);
        const float x = campo.x + fatia * static_cast<float>(quem + 1);
        const float y = campo.y + campo.h - alturaDoAluno;

        player->SetPosition(Vector2(x, y));
        mJogadores.push_back(dynamic_cast<Player*>(this->AddActor(std::move(player))));
    }
}
void Battle::LoadTimeBar() {

    // Cria o Actor que vai segurar a barra que mostra o tempo do estado
    auto barActor = std::make_unique<Actor>(this);

    if (!mBoss) {
        SDL_Log("ERRO: LoadHUD chamado antes do mBoss ser criado!");
        return;
    }

    auto bossSprite = mBoss->GetComponent<DrawAnimatedComponent>();
    auto bossHeight = bossSprite->GetSpriteHeight();

    auto barCompPtr = barActor->AddComponent<ProgressBarComponent>();

    // Configura a barra que acabamos de colocar no boss
    barCompPtr->SetOrientation(ProgressBarComponent::Orientation::Vertical);
    barCompPtr->SetSize(10, static_cast<int>(static_cast<float>(bossHeight) / 1.4f));
    barCompPtr->SetColors({40, 40, 40, 255}, {255, 191, 0, 255});

    Actor* barActorPtr = barActor.get();

    // Adiciona ator à cena
    this->AddActor(std::move(barActor));

    // Dá poder ao HUD pra modificar a barra
    mHUD->SetTimeBar(barActorPtr, barCompPtr);

    // Salva o ponteiro do Actor que tem a barra de progresso na cena Battle, pra poder seguir o Boss
    mHUDTimeBarActor = barActorPtr;

    // Dá load no reloginho que tem embaixo da barra
    LoadTimeClock();
}
void Battle::LoadTimeClock() {

    // Cria o ator que vai segurar a animação do relógior
    auto clockActor = std::make_unique<Actor>(this);
    auto clockActorPtr = clockActor.get();

    std::string clockSpriteSheet = Caminhos::Asset("Icons/TestTimeIcon.png");
    std::string clockSpriteData = Caminhos::Asset("Icons/TestTimeIcon.json");

    auto clockComp = clockActor->AddComponent<DrawAnimatedComponent>(clockSpriteSheet, clockSpriteData, 201);

    clockComp->AddAnimation("Idle", {0});
    clockComp->AddAnimation("Active", {0,1,2,3,4,5});
    clockComp->SetAnimation("Active");

    this->AddActor(std::move(clockActor));
    mHUDClockActor = clockActorPtr;

}
void Battle::LoadGradeBar() {
    auto gradeBarActor = std::make_unique<Actor>(this);

    const int windowWidth = mGame->GetWindowWidth();
    const int windowHeight = mGame->GetWindowHeight();

    // A BARRA RECUADA, e nao de ponta a ponta. Encostada nas bordas ela parecia
    // um risco no monitor; com a sobra dos lados e a moldura, vira uma peca da
    // tela. Os numeros vem de Battle.h - ver o comentario de kBarraAltura.
    const int barraX = kBarraMargemLateral;
    const int barraY = windowHeight - kBarraDoFundo;
    const int barraLargura = windowWidth - 2 * kBarraMargemLateral;

    gradeBarActor->SetPosition(Vector2(static_cast<float>(barraX), static_cast<float>(barraY)));

    auto gradeBarComp = gradeBarActor->AddComponent<ProgressBarComponent>();
    gradeBarComp->SetOrientation(ProgressBarComponent::Orientation::Horizontal);
    gradeBarComp->SetSize(barraLargura, kBarraAltura);

    // O acabamento e desta barra, e nao da de tempo: ver SetAcabamento.
    gradeBarComp->SetAcabamento(true);

    mGradeBarActor = gradeBarActor.get();
    this->AddActor(std::move(gradeBarActor));

    // Configurar Actor e fonte que vai ter a nota no meio da barra
    mGradeBarFont = std::make_unique<Font>();
    mGradeBarFont->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
    auto gradeBarTextActor = std::make_unique<Actor>(this);

    // A CAIXA E 200x32, E NAO 36x25. Dois defeitos diferentes moravam aqui.
    //
    // A LARGURA de 36 esticava o texto: o desenho padrao preenche a caixa
    // declarada, e "100.00" espremido em 36px saia com digitos finos e altos que
    // mal davam para ler. As duas casas decimais so servem se der para le-las.
    //
    // A ALTURA tem de ser MAIOR que o tamanho da fonte, nunca a altura da barra.
    // Com o ajuste ao texto ligado, uma caixa baixa demais nao corta: ENCOLHE. A
    // 24 pontos o texto nasce com 28px de altura, entao a caixa de 25 o reduzia a
    // 0,893 - e reduzir uma fonte de pixel art por um fator quebrado apaga colunas
    // inteiras de pixels. Era isso que transformava o "A" de FINAL num "F".
    // O "+8" e a mesma folga que MainMenu, Identificacao e Opcoes ja usavam.
    constexpr int tamanhoDaNota = 24;
    // A PLACA PRIMEIRO, com ordem menor: a lista e desenhada da menor para a
    // maior, entao ela fica atras do numero. Sem ela o branco do numero cai em
    // cima do laranja do preenchimento e some.
    gradeBarTextActor->AddComponent<DrawCaixaComponent>(
        kPlacaLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca);

    auto gradeText = gradeBarTextActor->AddComponent<DrawTextComponent>(
        "omg", mGradeBarFont.get(), 200, tamanhoDaNota + 8, tamanhoDaNota, 304);
    gradeText->SetAjustarAoTexto(true);

    // NO MEIO DA BARRA, e nao numa conta solta: o texto seguia a altura antiga
    // por um caminho que ninguem via, e mover a barra o teria deixado para tras.
    const auto textPosX = static_cast<float>(windowWidth) / 2.f;
    const auto textPosY = static_cast<float>(barraY + kBarraAltura / 2);
    gradeBarTextActor->SetPosition(Vector2(textPosX, textPosY));

    mGradeTextActor = gradeBarTextActor.get();
    this->AddActor(std::move(gradeBarTextActor));

    // O anuncio do teste final, escondido ate a nota chegar a 100.
    //
    // MORA NA FAIXA DA NOTA, e nao no meio da tela, embora o meio seja mais
    // bonito. Quem acabou de chegar a 100 e exatamente quem tem mais a perder
    // com um projetil escondido atras de um texto de comemoracao: um acerto
    // derruba a nota cheia, e com tres o teto tranca em 99,99 para sempre. A
    // faixa ja e area de HUD, entao o anuncio nao custa um pixel de campo.
    auto avisoActor = std::make_unique<Actor>(this);
    auto placaDoTeste = avisoActor->AddComponent<DrawCaixaComponent>(
        kPlacaDoAvisoLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca);
    placaDoTeste->SetIsVisible(false);

    auto aviso = avisoActor->AddComponent<DrawTextComponent>(
        "TESTE FINAL", mGradeBarFont.get(), 260, tamanhoDaNota + 8, tamanhoDaNota, 304);
    aviso->SetAjustarAoTexto(true);
    aviso->SetColor(Color::Gold);
    aviso->SetIsVisible(false);

    avisoActor->SetPosition(Vector2(static_cast<float>(windowWidth) / 4.f, textPosY));

    mTesteFinalActor = avisoActor.get();
    this->AddActor(std::move(avisoActor));

    // O aviso de fase limpa, na DIREITA da faixa - o TESTE FINAL ocupa a
    // esquerda, e os dois podem estar acesos ao mesmo tempo numa corrida boa.
    auto limpaActor = std::make_unique<Actor>(this);
    auto placaDaLimpa = limpaActor->AddComponent<DrawCaixaComponent>(
        kPlacaDoAvisoLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca);
    placaDaLimpa->SetIsVisible(false);

    auto limpa = limpaActor->AddComponent<DrawTextComponent>(
        "FASE LIMPA", mGradeBarFont.get(), 260, tamanhoDaNota + 8, tamanhoDaNota, 304);
    limpa->SetAjustarAoTexto(true);
    limpa->SetColor(Color::LightGreen);
    limpa->SetIsVisible(false);

    limpaActor->SetPosition(Vector2(static_cast<float>(windowWidth) * 3.f / 4.f, textPosY));

    mFaseLimpaActor = limpaActor.get();
    this->AddActor(std::move(limpaActor));
}
void Battle::LoadEndScreen() {
    auto textActor = std::make_unique<Actor>(this);

    // Centraliza na tela
    float w = mGame->GetWindowWidth();
    float h = mGame->GetWindowHeight();
    textActor->SetPosition(Vector2(w / 2.0f, h / 2.0f));

    // Cria o componente de texto (Tamanho 72 para ficar grande!)
    // Nota: Texto inicial vazio ou espaço, pois vamos mudar dinamicamente
    auto dc = textActor->AddComponent<DrawTextComponent>(" ", mGradeBarFont.get(), 400, 100, 72, 300);

    // Começa invisível
    dc->SetIsVisible(false);

    mEndTextActor = textActor.get();
    AddActor(std::move(textActor));
}

void Battle::StartLoadingProjectilePools() {

    mLoadingPendingFactoryNames.clear();

    if (mBoss) {
        // Pega exatamente os nomes que este Boss registrou
        mLoadingPendingFactoryNames = mBoss->GetProjectileFactoryNames();
    }

    mLoadingTotalTypes = static_cast<int>(mLoadingPendingFactoryNames.size());

    if (mLoadingTotalTypes == 0) {
        // Nenhuma factory para aquecer (não deveria acontecer com um Boss
        // válido, mas por segurança pulamos direto para o gameplay).
        FinishLoadingProjectilePools();
        return;
    }

    mIsLoading = true;

    // Pausa o Player durante o carregamento: Actor::Update() e
    // Scene::ProcessInput() já respeitam ActorState::Active/Inactive, então
    // isso já impede movimento/disparo do jogador sem precisar de nenhum
    // mecanismo novo. Revertido em FinishLoadingProjectilePools().
    for (Player* jogador : mJogadores) {
        if (jogador) jogador->SetState(ActorState::Paused);
    }

    // Cria o texto de progresso (mesmo padrão de LoadEndScreen).
    auto textActor = std::make_unique<Actor>(this);
    float w = mGame->GetWindowWidth();
    float h = mGame->GetWindowHeight();
    textActor->SetPosition(Vector2(w / 2.0f, h / 2.0f));
    auto dc = textActor->AddComponent<DrawTextComponent>("Carregando...", mGradeBarFont.get(), 400, 100, 48, 300);
    dc->SetIsVisible(true);
    mLoadingTextActor = textActor.get();
    AddActor(std::move(textActor));
}

void Battle::UpdateLoadingStep() {

    if (mLoadingPendingFactoryNames.empty()) {
        FinishLoadingProjectilePools();
        return;
    }

    // Processa UM tipo de projétil inteiro nesta chamada (ver justificativa
    // em Battle.h, próximo a mIsLoading).
    const std::string nextName = mLoadingPendingFactoryNames.back();
    mLoadingPendingFactoryNames.pop_back();

    if (ProjectileFactory* factory = mBoss ? mBoss->GetProjectileFactory(nextName) : nullptr) {
        factory->Prewarm(this, mBoss, kProjectilePrewarmCountPerType);
    }

    // Atualiza o texto de progresso.
    const int done = mLoadingTotalTypes - static_cast<int>(mLoadingPendingFactoryNames.size());
    if (mLoadingTextActor) {
        std::ostringstream ss;
        ss << "Carregando... " << done << " de " << mLoadingTotalTypes;
        mLoadingTextActor->GetComponent<DrawTextComponent>()->SetText(ss.str());
    }

    if (mLoadingPendingFactoryNames.empty()) {
        FinishLoadingProjectilePools();
    }
}

void Battle::FinishLoadingProjectilePools() {

    mIsLoading = false;

    if (mLoadingTextActor) {
        if (auto dc = mLoadingTextActor->GetComponent<DrawTextComponent>()) {
            dc->SetIsVisible(false);
        }
        mLoadingTextActor = nullptr; // O Actor continua vivo em mActors; só paramos de referenciá-lo aqui.
    }

    // Reativa o Player, pausado em StartLoadingProjectilePools().
    for (Player* jogador : mJogadores) {
        if (jogador) jogador->SetState(ActorState::Active);
    }

    // Só agora, com os pools já aquecidos, a lógica de ataque do Boss
    // começa de fato (ver comentário em Load()).
    if (mBoss) {
        mBoss->Start();
    }
}

//endregion LoadFunctions
//region UpdateFunctions
void Battle::OnUpdate(float deltaTime) {

    // -- Tela de Carregamento (pré-aquecimento dos pools de projétil) --
    // Enquanto os pools ainda estão sendo aquecidos, NÃO rodamos a lógica
    // normal de gameplay (nem o Boss começou a atacar ainda — Start() só é
    // chamado em FinishLoadingProjectilePools()). Processa 1 tipo de
    // projétil por frame.
    if (mIsLoading) {
        UpdateLoadingStep();
        return;
    }

    // -- Lógica de Fim de Jogo
    if (mIsEnding) {
        mEndTimer += deltaTime;

        // O ClearBossProjectiles() de FinishBattle apenas MARCA os projeteis;
        // quem de fato os devolve ao pool e o Cleanup. Como este ramo retorna
        // antes de mProjectileManager->Update(), a limpeza precisa acontecer
        // explicitamente aqui.
        if (mProjectileManager) mProjectileManager->Cleanup();

        if (mEndTimer > 4.f) {
            mGame->RequestSceneChange(SceneType::StageSelect);
        }

        return;
    }

    // A cena base já atualiza todos os atores.
    // Aqui podemos adicionar lógica específica da batalha.

    // Atualiza os sistemas principais apenas da Battle (os atores são atualizados pela Scene)
    if (mProjectileManager) mProjectileManager->Update(deltaTime);

    // Orquestra a checagem de colisões
    CheckCollisions();

    if (mHUD) mHUD->OnUpdate(deltaTime);

    TimeBarUpdate();
    GradeBarUpdate();

    // Purga os ponteiros observadores de pontos extras. Precisa rodar DEPOIS de
    // CheckCollisions (que e quem marca um item coletado como Destroy) e ANTES
    // do RemoveDeadActors da Scene, que roda ao final do Update. Assim nenhum
    // ponteiro de mExtraPoints sobrevive ao objeto que ele aponta.
    // Isto NAO destroi o item: quem faz isso e a Scene, que e a dona dele.
    mExtraPoints.erase(std::remove_if(mExtraPoints.begin(), mExtraPoints.end(),
        [](const ExtraPointItem* item) {
            return item->GetState() == ActorState::Destroy;
        }), mExtraPoints.end());

    if (mAvisoDeFaseLimpa > 0.0f) mAvisoDeFaseLimpa -= deltaTime;

    // Se a nota cair a 0, termina a fase
    if (mGrade <= 0) {
        FinishBattle(false);
    }

}
void Battle::TimeBarUpdate() const {
    if (!mBoss || !mHUDTimeBarActor) return;

    Vector2 bossPos = mBoss->GetPosition();

    auto bossSprite = mBoss->GetComponent<DrawAnimatedComponent>();
    auto bossWidth = bossSprite->GetSpriteWidth();
    auto bossHeight = bossSprite->GetSpriteHeight();

    float offSetX = static_cast<float>(bossWidth) / 2.5f;
    float offSetY = static_cast<float>(bossHeight) / 4.f;

    mHUDTimeBarActor->SetPosition(Vector2(bossPos.x + offSetX,
                                    bossPos.y - offSetY));

    mHUDClockActor->SetPosition(mHUDTimeBarActor->GetPosition() + Vector2(5, static_cast<float>(bossHeight) / 1.4f));
}
void Battle::GradeBarUpdate() {
    auto gradeComp = mGradeBarActor->GetComponent<ProgressBarComponent>();
    if (!gradeComp) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "GradeBarActor not found");
        return;
    }

    const float fillPercent = mGrade / 100.f;
    gradeComp->SetFillPercent(fillPercent);

    // -- DEFINIÇÃO DA PALETA DE CORES -- //

    // O SULCO, e dele que saem a moldura e a sombra (ver SetAcabamento). Era
    // {20,20,20}, quase preto: sobre o preto da tela a barra nao tinha contorno
    // nenhum, e clarear a moldura a partir de um quase-preto nao chega a lugar
    // nenhum. Puxado para o azul pelo mesmo motivo do fundo das telas de menu -
    // um cinza neutro deixa a faixa amarelada ao lado do laranja da nota.
    static const SDL_Color BG_COLOR = {26,28,38,255};

    // STATE1 - REPROVADE
    static const SDL_Color FAIL_START = {100, 0, 0, 255};
    static const SDL_Color FAIL_END = {255, 0, 0, 255};

    // STATE2 - FINAL
    static const SDL_Color SECRET_START = {180, 150, 0, 255};
    static const SDL_Color SECRET_END = {255, 140, 0, 255};

    // STATE3 - APROVADE
    static const SDL_Color PASS_START = {144, 238, 144, 255};
    static const SDL_Color PASS_END = {0, 255, 128, 255};

    // -- --------- -- ------ -- ----- -- //

    float colorPercent = 0.0f;

    if (mGrade < 40.f) {
        colorPercent = mGrade / 40.f;
        gradeComp->SetGradientColors(BG_COLOR, FAIL_START, FAIL_END);
        gradeComp->SetEffect(ProgressBarComponent::VisualEffect::Pulsing);
    } else if (mGrade < 60.f) {
        colorPercent = (mGrade - 40.f) / 20.f;
        gradeComp->SetGradientColors(BG_COLOR, SECRET_START, SECRET_END);
        gradeComp->SetEffect(ProgressBarComponent::VisualEffect::None);
    } else {
        colorPercent = (mGrade - 60.f) / 40.f;
        gradeComp->SetGradientColors(BG_COLOR, PASS_START, PASS_END);
        gradeComp->SetEffect(ProgressBarComponent::VisualEffect::None);
    }

    gradeComp->SetColorPercent(colorPercent);
    GradeTextUpdate();

}
void Battle::GradeTextUpdate() {

    // DUAS CASAS, E NAO UMA. Com uma casa o teto de 99,99 apareceria como
    // "100.0": quem levou dano demais leria nota cheia na tela, nao veria a
    // comemoracao que acompanha o 100 de verdade, e teria toda a razao de achar
    // que o jogo quebrou. A segunda casa e o que torna o teto visivel.
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << mGrade;

    const auto texto = mGradeTextActor->GetComponent<DrawTextComponent>();
    texto->SetText(ss.str());

    // O dourado SEGUE A NOTA, em vez de ser ligado uma vez e ficar aceso: quem
    // chega a 100 e leva um acerto perde a nota cheia de verdade, e a tela tem de
    // contar isso. Acender e nunca apagar faria o dourado mentir no momento em
    // que ele mais importa.
    const bool cheia = Nota::ECheia(mGrade);
    texto->SetColor(cheia ? Color::Gold : Color::White);

    // O ANUNCIO, AO CONTRARIO, NAO VOLTA ATRAS. Ele marca que o teste final
    // comecou - e ter comecado nao deixa de ser verdade quando a nota cai. Se
    // ele piscasse junto com o dourado, viraria um alarme a cada acerto.
    if (cheia && mTesteFinalActor) {
        // A PLACA ACENDE JUNTO. Sao dois componentes no mesmo ator, e deixar um
        // para tras e o jeito classico de a placa ficar acesa sozinha - uma
        // mancha escura no meio da barra, sem texto nenhum.
        mTesteFinalActor->GetComponent<DrawCaixaComponent>()->SetIsVisible(true);
        mTesteFinalActor->GetComponent<DrawTextComponent>()->SetIsVisible(true);
    }

    // O aviso de fase limpa some sozinho. Ele informa, nao comemora: deixar na
    // tela a batalha toda transformaria em enfeite o que precisa ser lido como
    // "isto acabou de acontecer".
    if (mFaseLimpaActor) {
        const bool acesa = mAvisoDeFaseLimpa > 0.0f;
        mFaseLimpaActor->GetComponent<DrawCaixaComponent>()->SetIsVisible(acesa);
        mFaseLimpaActor->GetComponent<DrawTextComponent>()->SetIsVisible(acesa);
    }

}
//endregion UpdateFunctions


void Battle::OnProcessInput(const Uint8* keyState) {
    // NAO chame ProcessInput dos jogadores aqui.
    // Scene::ProcessInput ja percorre mActors e entrega o input a todos os
    // atores Active - e o Player e um deles. A chamada extra que existia aqui
    // fazia Player::OnProcessInput rodar DUAS vezes por frame. Hoje isso passa
    // despercebido porque MoveInput e idempotente e Shoot/Special sao
    // protegidos por timers, mas qualquer acao nova sem guarda dispararia em
    // dobro.
    // Este gancho continua sendo o lugar certo para input da BATALHA em si
    // (pausa, debug), que nao pertence a nenhum ator especifico.
}
void Battle::CheckCollisions() {
    if (mJogadores.empty() || !mBoss) return;

    const auto *bossCollider = mBoss->GetComponent<CircleColliderComponent>();
    if (!bossCollider) return;

    const auto &bossProjectiles = mProjectileManager->GetBossProjectiles();
    const auto &playerProjectiles = mProjectileManager->GetPlayerProjectiles();

    // AS QUATRO REGRAS CONTINUAM NESTA ORDEM, e por isso sao quatro lacos em vez
    // de um. Juntar as tres regras de jogador num laco so seria mais curto, e
    // poria a regra 2 - que paga ganho - depois das regras 1 e 3, que cobram
    // dano. A ordem PODE mudar a nota: acima de 60 o ganho passa pela curva, e
    // Nota::Somar le a nota de agora e quantos acertos ja foram sofridos.
    //
    // MEDI, E NAO CONSEGUI VER DIFERENCA. Rodei a batalha do Salles inteira com
    // as regras reordenadas de tres jeitos e o despejo da nota saiu igual,
    // quadro a quadro, nos tres. O motivo: abaixo de 60 o ganho e cheio e os
    // dois caminhos dao o mesmo numero, e acima de 60 a diferenca so aparece
    // num quadro que tenha ganho E dano ao mesmo tempo - o que nao aconteceu
    // uma vez sequer em 2400 quadros com dez acertos sofridos acima de 60.
    //
    // Mantive a ordem assim mesmo. Ela custa tres lacos em vez de um e tira a
    // duvida de um refatoramento cujo contrato e nao mudar nada - e o custo de
    // estar errado aqui e uma nota diferente que ninguem ia notar.

    // --- REGRA 1 - Projéteis do Boss vs Player --- //
    for (Player* jogador : mJogadores) {
        const auto *playerCollider = ColisorDe(jogador);
        if (!playerCollider) continue;

        for (const auto &bossProj: bossProjectiles) {
            if (bossProj->GetState() != ActorState::Active) continue;
            if (const auto *bossProjCollider = bossProj->GetComponent<CircleColliderComponent>()) {
                if (playerCollider->Intersect(*bossProjCollider) && !jogador->GetIsInvincible()) {
                    jogador->SetIsInvincible(true);
                    jogador->OnCollision(bossProj.get());
                    bossProj->OnCollision(jogador);
                    GradeDown();
                }
            }
        }
    }

    // --- REGRA 2 - Projéteis do Player vs Boss
    //
    // ESTA NAO E DE NINGUEM. Os tiros dos dois jogadores saem do mesmo pool e
    // nao guardam de quem sao - e nao precisam: a nota e da dupla
    // (requisitos-coop, D2), entao um acerto paga a mesma barra venha de quem vier.
    //
    // E NAO E DE NENHUM CORPO TAMBEM. Na fase que se divide ha dois chefes em
    // campo, e acertar qualquer um dos dois paga a mesma barra: o laco percorre
    // Corpos() em vez de perguntar a mBoss. Com um corpo so a lista tem um
    // elemento e isto e exatamente o que era antes - sem "if (dividido?)".
    for (const auto &playerProj: playerProjectiles) {
        if (playerProj->GetState() != ActorState::Active) continue;

        const auto *playerProjCollider = playerProj->GetComponent<CircleColliderComponent>();
        if (!playerProjCollider) continue;

        for (Boss* corpo : mCorpos) {
            const auto* colisorDoCorpo = corpo ? corpo->GetComponent<CircleColliderComponent>()
                                               : nullptr;
            if (!colisorDoCorpo) continue;

            if (colisorDoCorpo->Intersect(*playerProjCollider)) {
                corpo->OnCollision(playerProj.get());
                playerProj->OnCollision(corpo);

                // Acerto de UM jogador: repartido (requisitos-coop, 2.1).
                GradeUp(PorJogador(GRADE_CHANGE_UP));

                // UM TIRO ACERTA UM CORPO SO. Sem este corte, um projetil que
                // passasse na sobreposicao dos dois pagaria a barra duas vezes.
                break;
            }
        }
    }

    // --- REGRA 3 - Player vs Boss  (Contato Direto)
    //
    // Os dois lacos sao por LISTA: jogadores por fora, corpos por dentro.
    // Encostar em qualquer um dos dois chefes custa o mesmo.
    for (Player* jogador : mJogadores) {
        const auto *playerCollider = ColisorDe(jogador);
        if (!playerCollider) continue;

        for (Boss* corpo : mCorpos) {
            const auto* colisorDoCorpo = corpo ? corpo->GetComponent<CircleColliderComponent>()
                                               : nullptr;
            if (!colisorDoCorpo) continue;

            if (playerCollider->Intersect(*colisorDoCorpo) && !jogador->GetIsInvincible()) {
                jogador->SetIsInvincible(true);
                jogador->OnCollision(corpo);
                corpo->OnCollision(jogador);
                GradeDown();
            }
        }
    }

    // --- REGRA 4 - Player vs ExtraPoints
    //
    // O item e de QUEM ENCOSTOU (requisitos-coop, D8). O laco do jogador e o de
    // fora: assim, se os dois tocarem o mesmo item no mesmo quadro, o primeiro
    // o destroi e o segundo ja o encontra fora de Active.
    //
    // O som fica fora dos dois lacos, porque e um por quadro e nao um por item.
    bool playedPowerUpSound = false;

    for (Player* jogador : mJogadores) {
        const auto *playerCollider = ColisorDe(jogador);
        if (!playerCollider) continue;

        for (const auto item : mExtraPoints) {
            if (item->GetState() != ActorState::Active) continue;

            if (const auto itemCollider = item->GetComponent<CircleColliderComponent>(); itemCollider && playerCollider->Intersect(*itemCollider)) {

                // 1. Efeito da Coleta
                jogador->AddExtraPoint();

                // 2. Toca o som
                if (!playedPowerUpSound) {
                    // Toca só se ainda não tocou neste frame
                    mGame->GetAudio()->PlaySound("powerup.wav", false);
                    playedPowerUpSound = true;
                }

                // 3. Destroi o item
                item->SetState(ActorState::Destroy);
            }
        }
    }

    // A nota NAO e mais escrita aqui. Gravar durante a batalha fazia as regras
    // de desbloqueio oscilarem a cada dano, e podia re-trancar uma materia
    // aprovada. Quem grava e FinishBattle, uma vez, no fim.

}

Player* Battle::JogadorEm(const size_t posicao) const {
    return posicao < mJogadores.size() ? mJogadores[posicao] : nullptr;
}

Player* Battle::Alvo() const {

    // SEM REVEZAR AQUI DENTRO: o laser pergunta duas vezes no mesmo disparo (o
    // nulo e a posicao) e trocaria de alvo entre uma e outra. Quem passa a vez
    // e RevezarAlvo, chamado por quem sabe que um disparo comecou.
    if (mJogadores.empty()) return nullptr;
    return mVezDoAlvo < mJogadores.size() ? mJogadores[mVezDoAlvo] : mJogadores.front();
}

void Battle::RevezarAlvo() {

    // O "% tamanho" e o que faz isto nao dar nada com um jogador: 0 + 1 = 1, e
    // 1 % 1 = 0. Nao ha um caminho para um jogador e outro para dois.
    if (mJogadores.empty()) { mVezDoAlvo = 0; return; }
    mVezDoAlvo = (mVezDoAlvo + 1) % mJogadores.size();
}

float Battle::PorJogador(const float bruto) const {
    return Nota::PorJogador(bruto, mJogadores.size());
}

void Battle::GradeUp(const float amount) {

    // A aritmetica inteira vive em Nota: a curva acima de 60, o teto de quem
    // levou dano e os limites de 0 a 100. Aqui so entra o que a batalha sabe e
    // Nota nao: a nota de agora e quantas vezes a DUPLA foi atingida.
    mGrade = Nota::Somar(mGrade, amount, mAcertosSofridos);
}
void Battle::GradeDown() {

    // A contagem mora AQUI, no instante do acerto, e nao num balanco no fim da
    // batalha: assim o proximo ganho ja ve o teto novo. Entre esta linha e a
    // seguinte a ordem nao importa - Subtrair nao le a contagem - e nao vale
    // comentar uma dependencia que nao existe.
    //
    // A CONTAGEM NAO E REPARTIDA, so a perda. Ela conta acertos, e nao
    // quantidade de nota: com dois em campo, a dupla perde o direito a nota
    // cheia no dobro da velocidade - e isso e de proposito (requisitos-coop,
    // 2.1, o segundo dos tres desafios a mais).
    ++mAcertosSofridos;
    ++mAcertosNaFase;
    // PerdaPorJogador, e nao PorJogador: alem da divisao, a perda em grupo leva
    // um alivio (Nota::kAlivioDaPerdaEmGrupo). Em dupla um acerto custa 2,4
    // brutos contra os 6,0 do solo. O motivo esta no cabecalho da funcao, e sai
    // da medicao: o total ja era favoravel a dupla, mas a barra caia quase no
    // dobro da frequencia - e e a frequencia que o jogador sente.
    mGrade = Nota::Subtrair(mGrade, PerdaPorJogador(GRADE_CHANGE_DOWN));
}

float Battle::PerdaPorJogador(const float bruto) const {
    return Nota::PerdaPorJogador(bruto, mJogadores.size());
}

bool Battle::PerdeuACheia() const {
    return Nota::PerdeuACheia(mAcertosSofridos);
}

/// Segundos que o aviso de fase limpa fica na tela. Curto: ele informa, nao
/// comemora - e o jogador esta no meio de uma batalha.
static constexpr float kDuracaoDoAvisoDeFaseLimpa = 2.5f;

void Battle::OnFaseTrocada(const float novaDuracao) {

    // A ORDEM IMPORTA. Fechar a fase ANTES de reiniciar a barra: quem acabou de
    // terminar uma fase limpa tem de receber por ELA, e nao pela que comeca
    // agora. Invertido, o contador zeraria antes de ser lido e o bonus sairia
    // sempre - inclusive para quem levou dano a fase inteira.
    EncerrarFase();

    if (mHUD) {
        mHUD->ResetTimeBar(novaDuracao);
    }
}

void Battle::EncerrarFase() {

    // DEPOIS QUE A BATALHA ACABOU, NAO HA MAIS FASE A FECHAR. A maquina de
    // estados do chefe continua trocando de fase depois do FinishBattle - a
    // StateFinal aponta de volta para a StateOne - e sem esta guarda o callback
    // pagava mais bonus com a batalha ja encerrada, mexendo na nota que aparece
    // na tela depois de ela ja ter ido para o disco.
    if (mIsEnding) return;

    // A PRIMEIRA CHAMADA NAO PAGA NADA. O callback da maquina de estados dispara
    // tambem ao ENTRAR na primeira fase, quando nao ha fase anterior para ter
    // sido limpa. mPrimeiraFase cobre esse caso.
    if (mPrimeiraFase) {
        mPrimeiraFase = false;
        mAcertosNaFase = 0;
        return;
    }

    if (mAcertosNaFase == 0) {
        // DA FASE, e nao de um jogador: pago inteiro. E ele ja ficou mais
        // dificil no co-op, porque mAcertosNaFase conta os dois - a fase so e
        // limpa se NENHUM dos dois foi atingido.
        GradeUp(Nota::kBonusDeFaseLimpa);

        // O aviso existe para a nota nao subir por motivo invisivel. Um salto sem
        // explicacao e lido como defeito, e o jogador nunca descobre que desviar
        // paga - que e justamente o que este bonus veio ensinar.
        mAvisoDeFaseLimpa = kDuracaoDoAvisoDeFaseLimpa;
    }

    mAcertosNaFase = 0;
}

SDL_FRect Battle::GetPlayfieldBounds() const
{
    // O CAMPO ACABA ONDE A FAIXA DA NOTA COMECA, e esse numero e o da barra -
    // nao uma copia dele.
    //
    // Eram dois 25 com um comentario pedindo para manter os dois iguais, e
    // pedido em comentario nao segura nada: quando a barra ganhou moldura e
    // recuo, a faixa ficou para tras e o aluno continuou podendo parar EM CIMA
    // da barra, com a personagem por cima da propria nota.
    //
    // A conta tambem decide onde ficam as linhas da tabela do Thiago (ver
    // ConsultaAttack): a ultima linha agora termina onde o jogador termina, em
    // vez de descer por baixo do HUD - uma linha que so existia para ser
    // varrida, nunca para ser habitada.
    return {
        0.0f, // x
        0.0f, // y
        static_cast<float>(mGame->GetWindowWidth()), // w
        static_cast<float>(mGame->GetWindowHeight() - kBarraDoFundo) // h
    };
}

void Battle::DividirChefe(const std::string& faseDoClone) {

    if (!mBoss || mCorpos.size() > 1) return;   // ja dividido, ou sem original

    IBossFactory* fabrica = mGame->GetFactory(static_cast<size_t>(mStage));
    if (!fabrica) {
        SDL_Log("BATALHA: a fase pediu divisao, mas esta materia nao tem fabrica de chefe.");
        return;
    }

    // O CLONE E UM CHEFE INTEIRO DA MESMA FABRICA, e e isso que o deixa barato:
    // ele ganha o mesmo sprite, o mesmo colisor, as mesmas fabricas de projetil
    // e as mesmas fases, sem uma linha de montagem propria. O que o diferencia
    // e a MARCA e o estado inicial.
    auto clone = fabrica->CreateBoss(this);
    if (!clone) return;

    clone->MarcarComoClone();
    clone->SetInitialState(faseDoClone);

    // NASCE AO LADO DO ORIGINAL, e nao em cima: dois corpos no mesmo pixel
    // seriam um so para quem olha, e a fase inteira perderia o sentido.
    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const Vector2 ondeEstava = mBoss->GetPosition();
    clone->SetPosition(Vector2(largura - ondeEstava.x, ondeEstava.y));

    Boss* bruto = dynamic_cast<Boss*>(this->AddActor(std::move(clone)));
    if (!bruto) return;

    // Start() e o que dispara a FSM e os primeiros ataques. Sem ele o corpo
    // aparece e fica parado - o mesmo cuidado que LoadBoss ja tomava.
    bruto->Start();
    mCorpos.push_back(bruto);

    SDL_Log("BATALHA: o chefe se dividiu. O segundo corpo usa os ataques de \"%s\".",
            faseDoClone.c_str());
}

void Battle::JuntarCorpos() {

    // O ORIGINAL FICA, os extras saem. Percorre de tras para frente porque a
    // lista encolhe no caminho.
    for (size_t i = mCorpos.size(); i-- > 1; ) {
        if (mCorpos[i]) mCorpos[i]->SetState(ActorState::Destroy);
        mCorpos.pop_back();
    }
}

Player* Battle::OutroJogador(const Player* quem) const {

    // Com um jogador so nao ha outro, e quem pergunta trata o nulo - mesma
    // ideia do "% tamanho" em RevezarAlvo: um caminho so, sem "if (co-op)".
    if (mJogadores.size() < 2) return nullptr;

    for (size_t i = 0; i < mJogadores.size(); ++i) {
        if (mJogadores[i] == quem) return mJogadores[(i + 1) % mJogadores.size()];
    }
    return nullptr;
}

void Battle::OnPlayerUsedExtraPoint(Player* quemGastou) {

    // O GANHO E O MESMO NOS DOIS MODOS. Gastar um Ponto Extra e acao de UM
    // jogador, e sao dois estoques: sem repartir, a dupla poria 20 brutos na
    // mesma barra.
    GradeUp(PorJogador(kGanhoDoPontoExtra));

    Player* oOutro = OutroJogador(quemGastou);

    if (!oOutro) {
        // SOZINHO, O PONTO EXTRA LIMPA A TELA, como sempre fez. Nao ha parceiro
        // a quem dar imunidade, e trocar a limpeza por "proteja a si mesmo"
        // deixaria o solo com um efeito mais fraco do que ele tem hoje - por
        // causa de uma regra que so existe para o modo que ele nao esta jogando.
        if (mProjectileManager) mProjectileManager->ClearBossProjectiles();
        // TODO - Audio - efeito sonoro de limpeza de campo
        return;
    }

    // EM DUPLA, O PONTO EXTRA E DO OUTRO, e nao limpa a tela.
    //
    // O PRECO E O MESMO TEMPO, DOS DOIS LADOS: quem gasta passa
    // OVERLOAD_DURATION sem atirar, e o parceiro passa OVERLOAD_DURATION
    // atravessando bala. Um numero so (Player::OVERLOAD_DURATION), entao
    // mexer na sobrecarga mexe no presente junto.
    //
    // E NAO AS DUAS COISAS. Limpar a tela E proteger daria, por metade do
    // estoque da dupla, o dobro do efeito de um Ponto Extra de solo - e tiraria
    // o que torna esta versao interessante: ela nao salva quem aperta. Gastar
    // e ficar desarmado tres segundos no meio do que ja estava voando, para
    // que o outro atravesse.
    oOutro->Proteger(Player::OVERLOAD_DURATION);
}
void Battle::SpawnExtraPoint(Vector2 position) {
    auto item = std::make_unique<ExtraPointItem>(this);
    item->SetPosition(position);

    mExtraPoints.emplace_back(item.get());

    AddActor(std::move(item));
}

void Battle::FinishBattle(bool approved) {
    if (mIsEnding) return; // Já está acabando, ignora chamadas duplicadas

    // A ULTIMA FASE TAMBEM PAGA. Ela nao troca para fase nenhuma, entao o
    // callback da maquina de estados nunca dispara por ela - sem esta chamada,
    // terminar a batalha com uma fase final limpa nao valeria nada, e o jogador
    // leria isso como o bonus falhando justamente quando mais importa.
    //
    // ANTES DE mIsEnding, e nao depois: EncerrarFase se recusa a pagar com a
    // batalha ja encerrada (e tem de se recusar - a maquina de estados continua
    // trocando de fase depois daqui). Chamar depois da marcacao faria esta
    // ultima fase ser a unica que nunca paga.
    //
    // A chamada e segura contra repeticao porque o proprio FinishBattle sai
    // cedo quando ja esta encerrando, logo acima.
    EncerrarFase();

    mIsEnding = true;

    // O unico ponto em que a nota vai para o armazenamento persistente. Antes
    // isto nao acontecia em lugar nenhum: a nota chegava la por efeito colateral
    // do laco de colisoes, entao o +10 de um ponto extra usado no ultimo
    // instante era perdido.
    if (mGame) {
        mGame->RegistrarNota(mStage, mGrade);
    }
    for (auto& actor : mActors) {
        actor->SetState(ActorState::Paused);
    }
    mIsApproved = approved;
    mEndTimer = 0.0f;

    // 1. Limpa projéteis para limpar a poluição visual
    if (mProjectileManager) {
        mProjectileManager->ClearBossProjectiles();
        // Se tiver função para limpar projéteis do player, chame aqui também
    }

    // 2. Torna o Player Invencível (para não morrer durante a tela de vitória)
    for (Player* jogador : mJogadores) {
        if (jogador) jogador->SetIsInvincible(true);
    }

    // 3. Configura e Mostra o Texto
    if (mEndTextActor) {
        auto dc = mEndTextActor->GetComponent<DrawTextComponent>();
        dc->SetIsVisible(true);

        if (approved) {
            dc->SetText("APROVADE!");
        } else {
            dc->SetText("REPROVADE");
        }
    }
}
