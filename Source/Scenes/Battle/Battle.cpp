//
// Created by nslop on 16/09/2024.
//

#include "SDL.h"
#include "Battle.h"

#include "../../Progresso.h"
#include "../../Nota.h"

#include <locale>
#include <random>
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
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Components/DrawComponents/ProgressBarComponent.h"
#include "../../Components/DrawComponents/DrawBarraDaNotaComponent.h"
#include "../../Components/DrawComponents/DrawMolduraDoCampoComponent.h"
#include "../../Components/DrawComponents/DrawQuadroComponent.h"
#include "../../Components/DrawComponents/DrawRelogioDaFaseComponent.h"
#include "../../Actors/Teachers/ExtraPointItem.h"
#include "../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../PontoExtra.h"
#include "../../Destrutivel.h"
#include "../../Actors/Orientador/Orientador.h"
#include "../../MateriasArquivo.h"
#include "../../Painel.h"

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

    // O MODO E A SEMENTE ANTES DO CHEFE: a fabrica e o chefe podem perguntar
    // pelos dois.
    //
    // A SEMENTE NAO SAI DO Random DO JOGO: tirar um numero de la mudaria tudo
    // o que ele sorteia depois (a capivara do Salles, a porta do semaforo do
    // Vitor) e a mesma DPI_SEMENTE do arnes daria outra luta. Ela vem do
    // random_device, como a do proprio Random (Random::Init); no arnes, de
    // DPI_VARIACAO.
    mNivel = mGame->NivelDeDificuldade();
    mSemente = static_cast<std::uint32_t>(std::random_device{}());
    SDL_Log("BATALHA: semente %u, modo %s", mSemente, Dificuldade::Chave(mNivel));

    // 2. Criar os atores principais
    LoadPlayer();
    LoadBoss();
    LoadOrientador();
    LoadHUD();
    LoadEndScreen();
    LoadPausa();

    // A MUSICA NAO COMECA AQUI: so quando a batalha estiver carregada e o veu
    // da tela de carregamento tiver sumido (FinishLoadingProjectilePools).
    // Tocando desde o Load, ela comecava por cima de uma tela parada.

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
void Battle::LoadOrientador() {

    mOrientador = nullptr;

    // SO NA MATERIA QUE PEDE ORIENTACAO. O Game ja zera o orientador a cada
    // entrada pela selecao de fases; conferir aqui tambem e o que impede um
    // orientador perdido de aparecer numa batalha que nao o pediu.
    const Materias::Lista& materias = Materias::Carregadas();
    const Materias::Materia* aqui = materias.Por(mStage);
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

void Battle::LoadHUD() {

    //Cria a HUD para segurar todas as infos
    mHUD = std::make_unique<BattleHUD>(this);
    LoadMoldura();
    LoadTimeBar();
    LoadGradeBar();
    LoadPlacar();
    LoadIntercambio();

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

    // A CAPSULA DO RELOGIO DA FASE (DrawRelogioDaFaseComponent): pequena e
    // colada no chefe como antes, agora com moldura, cor que anda do verde ao
    // vermelho, um dente a cada 5 s e o pulso dos ultimos segundos.
    auto barCompPtr = barActor->AddComponent<DrawRelogioDaFaseComponent>(
        12, static_cast<int>(static_cast<float>(bossHeight) / 1.4f), 200);

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

    // A BARRA DESENHADA POR CONTA PROPRIA (DrawBarraDaNotaComponent): volume,
    // as marcas do 40 e do 60, o rastro do acerto e o clarao do ganho. A
    // ProgressBarComponent continua sendo a do relogio do chefe.
    auto gradeBarComp = gradeBarActor->AddComponent<DrawBarraDaNotaComponent>(barraLargura, kBarraAltura, 200);
    gradeBarComp->Comecar(mGrade);

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
        kPlacaLargura, kPlacaAltura - 4, kMolduraDaPlacaDaNota, kFundoDaPlacaDaNota, 1, kOrdemDaPlaca);

    // A NOTA DE PARTIDA desde o primeiro quadro. Era "omg", um texto de teste
    // que ficava na faixa ate a primeira atualizacao da barra.
    std::stringstream inicial;
    inicial << std::fixed << std::setprecision(2) << mGrade;
    auto gradeText = gradeBarTextActor->AddComponent<DrawTextComponent>(
        inicial.str(), mGradeBarFont.get(), 200, tamanhoDaNota + 8, tamanhoDaNota, 304);
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

    // O SELO DO DIFICIL, na ponta direita da faixa: entre a placa de FASE LIMPA
    // (que vai ate 3/4 da largura mais meia placa) e o fim da barra. Aceso a
    // luta inteira, sem piscar - e um aviso de modo, e nao um acontecimento.
    if (mNivel == Dificuldade::Nivel::Dificil) {
        auto selo = std::make_unique<Actor>(this);
        selo->AddComponent<DrawCaixaComponent>(
            kPlacaLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca);
        auto texto = selo->AddComponent<DrawTextComponent>(
            Dificuldade::Nome(mNivel), mGradeBarFont.get(), kPlacaLargura, tamanhoDaNota + 8,
            tamanhoDaNota, 304);
        texto->SetAjustarAoTexto(true);
        texto->SetColor(Vector3(Dificuldade::kCorDoDificil[0], Dificuldade::kCorDoDificil[1],
                                Dificuldade::kCorDoDificil[2]));
        selo->SetPosition(Vector2(static_cast<float>(windowWidth - kBarraMargemLateral - kPlacaLargura / 2 - 8),
                                  textPosY));
        mSeloDoNivel = selo.get();
        this->AddActor(std::move(selo));
    }
}
void Battle::LoadPlacar() {

    // O QUADRO DA SALA, no canto de cima a esquerda. E o canto que os chefes
    // menos visitam - a orbita e a varredura do Vladimir ficam longe dele - e o
    // relogio de fase ja anda colado no chefe, entao o canto estava livre.
    //
    // NASCE APAGADO. So acende numa fase que tenha "anuncio": um chefe que nao
    // anuncia nada nao ganha um quadro vazio no canto.
    mFonteDoPlacar = std::make_unique<Font>();
    mFonteDoPlacar->Load(Caminhos::Asset("Fonts/Carlito-Regular.ttf"));

    auto placar = std::make_unique<Actor>(this);

    // Verde de quadro-negro e moldura de giz. Translucido como as placas da
    // faixa, para o campo continuar aparecendo atras.
    // UM QUADRO DE VERDADE (DrawQuadroComponent): moldura de madeira, lousa,
    // canaleta com giz, e o apagador que passa quando o texto troca.
    auto quadro = placar->AddComponent<DrawQuadroComponent>(kPlacarLargura, kPlacarAltura, kOrdemDoPlacar);
    quadro->SetIsVisible(false);

    // A caixa do texto e MENOR que o quadro, e o ajuste ao texto so encolhe:
    // uma expressao comprida diminui para caber em vez de vazar pela moldura.
    auto texto = placar->AddComponent<DrawTextComponent>(
        " ", mFonteDoPlacar.get(), kPlacarLargura - 24, kPlacarAltura - 6,
        kTamanhoDoPlacar, kOrdemDoPlacar + 1);
    texto->SetAjustarAoTexto(true);
    texto->SetIsVisible(false);

    // DEPOIS DO TRILHO da esquerda, e nao colado na borda da tela: o trilho e
    // da moldura, e a madeira do quadro ficaria por baixo dele.
    placar->SetPosition(Vector2(mAreaDoAluno.esquerda + static_cast<float>(kPlacarMargem + kPlacarLargura / 2) - 4.0f,
                                static_cast<float>(kPlacarMargem + kPlacarAltura / 2)));

    mPlacarActor = placar.get();
    this->AddActor(std::move(placar));
}

void Battle::AtualizarPlacar() {
    if (!mPlacarActor || !mBoss) return;

    // FaseAtual, e nao o nome que a maquina de estados acabou de trocar: e a
    // fase em que o chefe esta agindo neste quadro, a mesma que escolhe o
    // movimento e os ataques. O placar e os tiros mudam juntos.
    //
    // DURANTE O AVISO DO INTERCAMBIO o placar avisa em vez de anunciar: e o
    // unico texto da tela que o aluno ja aprendeu a ler antes de cada rajada.
    const std::string anuncio = Intercambio::Avisando(mRelogioDoIntercambio)
                                    ? mBoss->AvisoDoIntercambio()
                                    : mBoss->AnuncioDaFase(mBoss->FaseAtual());
    if (anuncio == mTextoDoPlacar) return;
    mTextoDoPlacar = anuncio;

    const bool aceso = !anuncio.empty();

    // So escreve quando ha o que escrever: um texto vazio nao gera textura, e
    // a ultima expressao ficaria guardada esperando a placa acender de novo.
    auto* texto = mPlacarActor->GetComponent<DrawTextComponent>();
    if (aceso) texto->SetText(anuncio);
    texto->SetIsVisible(aceso);
    auto* quadro = mPlacarActor->GetComponent<DrawQuadroComponent>();
    quadro->SetIsVisible(aceso);
    if (aceso) quadro->Apagar();
}

void Battle::LoadIntercambio() {

    // UM MARCADOR POR ALUNO, do tamanho dele: e o contorno de onde ele vai
    // estar quando a troca acontecer. Nascem apagados - so acendem durante o
    // aviso, e so numa fase com "intercambio".
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
    if (!mBoss) return;

    // O RELOGIO E DA FASE. Entrar numa fase recomeca a contagem do zero, e numa
    // fase sem intercambio ele fica invalido - e um relogio invalido nunca
    // avisa nem troca.
    const std::string& fase = mBoss->FaseAtual();
    if (fase != mFaseDoIntercambio) {
        mFaseDoIntercambio = fase;
        mRelogioDoIntercambio = Intercambio::Relogio{};
        if (const auto ic = mBoss->IntercambioDaFase(fase)) {
            mRelogioDoIntercambio.aCada = ic->first;
            mRelogioDoIntercambio.aviso = ic->second;
        }
    }

    const bool trocou = Intercambio::Avancar(mRelogioDoIntercambio, deltaTime);

    // O DESTINO E RECALCULADO TODO QUADRO, e nao congelado quando o aviso
    // acende. Sozinho, o aluno escolhe onde vai cair andando - o espelho anda
    // junto. Em dupla, cada um cai onde o parceiro esta AGORA. Nos dois casos
    // o marcador diz a verdade ate o ultimo quadro.
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
        // O CHEFE FICA SABENDO, depois que os alunos ja estao no lugar novo: o
        // Vladimir troca as letras em voo no mesmo quadro (Alfabeto.h).
        mBoss->OnIntercambio();
    }

    const bool aceso = Intercambio::Avisando(mRelogioDoIntercambio);
    for (size_t i = 0; i < mMarcadoresDoIntercambio.size(); ++i) {
        Actor* marcador = mMarcadoresDoIntercambio[i];
        if (!marcador) continue;
        if (i < destinos.size()) marcador->SetPosition(destinos[i]);
        marcador->GetComponent<DrawCaixaComponent>()->SetIsVisible(aceso && i < destinos.size());
    }
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

    // SEM NADA PARA AQUECER (nao deveria acontecer com um chefe valido), a
    // tela passa do mesmo jeito: a barra ja nasce com tudo feito.
    mIsLoading = true;
    mCarregamento = Carregamento::Estado{};

    // Pausa o Player durante o carregamento: Actor::Update() e
    // Scene::ProcessInput() já respeitam ActorState::Active/Inactive, então
    // isso já impede movimento/disparo do jogador sem precisar de nenhum
    // mecanismo novo. Revertido em FinishLoadingProjectilePools().
    for (Player* jogador : mJogadores) {
        if (jogador) jogador->SetState(ActorState::Paused);
    }

    CriarTelaDeCarregamento();
}

void Battle::CriarTelaDeCarregamento() {
    const auto w = static_cast<float>(mGame->GetWindowWidth());
    const auto h = static_cast<float>(mGame->GetWindowHeight());

    // ACIMA DE TUDO, ate da pausa (900): enquanto ela esta na tela, a batalha
    // nao existe para quem olha.
    constexpr int kOrdemDoVeu = 1000;
    constexpr SDL_Color kFundo{5, 6, 11, 255};

    auto veu = std::make_unique<Actor>(this);
    veu->SetPosition(Vector2(w / 2.0f, h / 2.0f));
    veu->AddComponent<DrawCaixaComponent>(static_cast<int>(w) + 4, static_cast<int>(h) + 4,
                                          kSemMoldura, kFundo, 0, kOrdemDoVeu);
    mVeuDoCarregamento = AddActor(std::move(veu));

    // O QUE VEM AI: o codigo da materia, o nome dela e o professor.
    const Materias::Materia* materia = Materias::Carregadas().Por(mStage);
    const auto texto = [&](const std::string& t, Font* fonte, const float y, const int tamanho, const Vector3& cor) {
        if (t.empty() || !fonte) return;
        auto ator = std::make_unique<Actor>(this);
        ator->SetPosition(Vector2(w / 2.0f, y));
        auto* dc = ator->AddComponent<DrawTextComponent>(t, fonte, 1000, tamanho + 10, tamanho, kOrdemDoVeu + 2);
        dc->SetAjustarAoTexto(true);
        dc->SetColor(cor);
        mTextosDoCarregamento.push_back(AddActor(std::move(ator)));
    };
    if (materia) {
        texto(materia->nome, mGradeBarFont.get(), h * 0.36f, 52, Color::White);
        texto(materia->nomeCompleto, mFonteDoPlacar.get(), h * 0.44f, 28, Vector3(0.75f, 0.78f, 0.9f));
        if (!materia->professor.empty()) {
            texto("Prof. " + materia->professor, mFonteDoPlacar.get(), h * 0.50f, 22, Vector3(0.55f, 0.58f, 0.7f));
        }
    }
    if (mNivel == Dificuldade::Nivel::Dificil) {
        texto(Dificuldade::Nome(mNivel), mGradeBarFont.get(), h * 0.57f, 24,
              Vector3(Dificuldade::kCorDoDificil[0], Dificuldade::kCorDoDificil[1], Dificuldade::kCorDoDificil[2]));
    }
    texto("preparando a prova", mFonteDoPlacar.get(), h * 0.66f, 20, Vector3(0.45f, 0.48f, 0.6f));

    // A BARRA, com o acabamento da barra de relogio do chefe: a cor anda do
    // azul ao verde conforme enche.
    constexpr int kLargura = 520, kAltura = 14;
    auto barra = std::make_unique<Actor>(this);
    barra->SetPosition(Vector2((w - static_cast<float>(kLargura)) / 2.0f, h * 0.70f));
    auto* pb = barra->AddComponent<ProgressBarComponent>(kOrdemDoVeu + 1);
    pb->SetOrientation(ProgressBarComponent::Orientation::Horizontal);
    pb->SetSize(kLargura, kAltura);
    pb->SetAcabamento(true);
    pb->SetGradientColors(SDL_Color{22, 24, 36, 255}, SDL_Color{80, 120, 230, 255}, SDL_Color{110, 230, 150, 255});
    pb->SetFillPercent(0.0f);
    mBarraDoCarregamento = AddActor(std::move(barra));
}

void Battle::UpdateLoadingStep(const float deltaTime) {

    // Processa UM tipo de projétil inteiro nesta chamada (ver justificativa
    // em Battle.h, próximo a mIsLoading).
    if (!mLoadingPendingFactoryNames.empty()) {
        const std::string nextName = mLoadingPendingFactoryNames.back();
        mLoadingPendingFactoryNames.pop_back();
        if (ProjectileFactory* factory = mBoss ? mBoss->GetProjectileFactory(nextName) : nullptr) {
            factory->Prewarm(this, mBoss, kProjectilePrewarmCountPerType);
        }
    }

    // A BARRA ANDA NO RITMO DELA atras do que ja foi feito (Carregamento.h).
    // O primeiro quadro carrega o Load inteiro no deltaTime; limitado, ele nao
    // enche a barra de uma vez.
    const float feito = mLoadingTotalTypes > 0
        ? static_cast<float>(mLoadingTotalTypes - static_cast<int>(mLoadingPendingFactoryNames.size())) /
          static_cast<float>(mLoadingTotalTypes)
        : 1.0f;
    Carregamento::Avancar(mCarregamento, feito, std::min(deltaTime, 0.05f));
    if (mBarraDoCarregamento) {
        if (auto* pb = mBarraDoCarregamento->GetComponent<ProgressBarComponent>()) {
            pb->SetFillPercent(mCarregamento.mostrado);
            pb->SetColorPercent(mCarregamento.mostrado);
        }
    }

    // PRONTA, a tela some: primeiro o texto e a barra, depois o veu esmaece
    // sobre a batalha parada.
    if (Carregamento::Pronto(mCarregamento)) {
        for (Actor* t : mTextosDoCarregamento) {
            if (auto* dc = t->GetComponent<DrawTextComponent>()) dc->SetIsVisible(false);
        }
        if (mBarraDoCarregamento) {
            if (auto* pb = mBarraDoCarregamento->GetComponent<ProgressBarComponent>()) pb->SetIsVisible(false);
        }
        if (mVeuDoCarregamento) {
            if (auto* veu = mVeuDoCarregamento->GetComponent<DrawCaixaComponent>()) {
                veu->SetFundo(SDL_Color{5, 6, 11, static_cast<Uint8>(255.0f * Carregamento::Opacidade(mCarregamento))});
            }
        }
    }

    if (Carregamento::Acabou(mCarregamento)) FinishLoadingProjectilePools();
}

void Battle::FinishLoadingProjectilePools() {

    mIsLoading = false;

    // A TELA DE CARREGAMENTO SAI DE CENA de vez.
    for (Actor* t : mTextosDoCarregamento) t->SetState(ActorState::Destroy);
    mTextosDoCarregamento.clear();
    if (mVeuDoCarregamento) mVeuDoCarregamento->SetState(ActorState::Destroy);
    if (mBarraDoCarregamento) mBarraDoCarregamento->SetState(ActorState::Destroy);
    mVeuDoCarregamento = mBarraDoCarregamento = nullptr;

    // E SO AGORA A MUSICA: a batalha esta carregada e a vista.
    if (mGame->GetAudio() && !mMusicHandle.IsValid()) {
        mMusicHandle = mGame->GetAudio()->PlaySound("backgroundmusic.ogg", true);
        mGame->GetAudio()->SetSoundVolume(mMusicHandle, 32);
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
        UpdateLoadingStep(deltaTime);
        return;
    }

    // -- A PAUSA: a cena ja nao atualizou os atores (Scene::mAtoresCongelados),
    // e aqui param os projeteis, as colisoes e os relogios da batalha. Nada
    // anda, e nada conta tempo.
    if (mPausa.aberto) return;

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
    AtualizarDesenhoSobreOsAlunos(deltaTime);
    AtualizarMoldura();
    AtualizarEfeitos(deltaTime);

    if (mHUD) mHUD->OnUpdate(deltaTime);

    TimeBarUpdate();
    GradeBarUpdate();

    // O INTERCAMBIO ANTES DO PLACAR: e ele que decide se o aviso esta aceso
    // neste quadro, e o placar so le a resposta.
    AtualizarIntercambio(deltaTime);
    AtualizarPlacar();


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

    mHUDClockActor->SetPosition(mHUDTimeBarActor->GetPosition() + Vector2(6, static_cast<float>(bossHeight) / 1.4f + 4.0f));

    // O RELOGIO FICA VERMELHO junto com a capsula, nos ultimos segundos.
    if (auto* relogio = mHUDClockActor->GetComponent<DrawAnimatedComponent>(); relogio && mHUD) {
        const bool urgente = Hud::Urgente(mHUD->Restante());
        const bool aceso = urgente && std::fmod(mHUD->Restante(), 0.5f) > 0.25f;
        if (aceso) relogio->SetColor(255, 110, 100);
        else relogio->SetColor(255, 255, 255);
    }
}
void Battle::GradeBarUpdate() {
    auto gradeComp = mGradeBarActor->GetComponent<DrawBarraDaNotaComponent>();
    if (!gradeComp) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "GradeBarActor not found");
        return;
    }

    // -- DEFINIÇÃO DA PALETA DE CORES -- //

    // O SULCO E A MOLDURA sao da barra (DrawBarraDaNotaComponent); aqui so a
    // cor do preenchimento, que muda com a faixa da nota.

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

    SDL_Color de = PASS_START, ate = PASS_END;
    if (mGrade < 40.f) {
        colorPercent = mGrade / 40.f;
        de = FAIL_START; ate = FAIL_END;
    } else if (mGrade < 60.f) {
        colorPercent = (mGrade - 40.f) / 20.f;
        de = SECRET_START; ate = SECRET_END;
    } else {
        colorPercent = (mGrade - 60.f) / 40.f;
    }
    const auto lerp = [colorPercent](const Uint8 a, const Uint8 b) {
        return static_cast<Uint8>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * colorPercent);
    };
    const SDL_Color cor{lerp(de.r, ate.r), lerp(de.g, ate.g), lerp(de.b, ate.b), 255};

    // ABAIXO DO EXAME A MOLDURA PULSA em vermelho (era o preenchimento que
    // piscava, e piscando ele sumia metade do tempo); na nota cheia, ouro.
    gradeComp->SetNota(mGrade, cor, mGrade < 40.f, Nota::ECheia(mGrade));
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

    // O ESC E BORDA, e nao estado: segurado, ele abriria e fecharia a pausa a
    // cada quadro. NAO E O Ctrl+Esc do operador - esse so existe no gabinete,
    // e no gabinete a pausa nao abre (Pausa::PodePausar).
    const bool esc = keyState[SDL_SCANCODE_ESCAPE] != 0;
    const bool escNovo = esc && !mEscAnterior;
    mEscAnterior = esc;

    if (mPausa.aberto) {
        ProcessarPausa(keyState, escNovo);
        return;
    }
    if (escNovo && Pausa::PodePausar(mGame->NoGabinete(), EmCombate())) AbrirPausa();
}

//region Pausa
void Battle::LoadPausa() {
    const auto w = static_cast<float>(mGame->GetWindowWidth());
    const auto h = static_cast<float>(mGame->GetWindowHeight());
    const Vector2 meio(w / 2.0f, h / 2.0f);

    // O VEU escurece a batalha inteira: ela continua a vista, congelada, para
    // quem volta saber onde estava cada tiro.
    auto veu = std::make_unique<Actor>(this);
    veu->SetPosition(meio);
    veu->AddComponent<DrawCaixaComponent>(static_cast<int>(w), static_cast<int>(h), kSemMoldura,
                                          SDL_Color{6, 8, 16, 165}, 0, kOrdemDaPausa);
    mVeuDaPausa = AddActor(std::move(veu));

    auto placa = std::make_unique<Actor>(this);
    placa->SetPosition(meio);
    placa->AddComponent<DrawCaixaComponent>(kLarguraDaPlacaDaPausa, kAlturaDaPlacaDaPausa,
                                            SDL_Color{120, 140, 180, 230}, SDL_Color{12, 13, 17, 225},
                                            3, kOrdemDaPausa + 1);
    mPlacaDaPausa = AddActor(std::move(placa));

    auto titulo = std::make_unique<Actor>(this);
    titulo->SetPosition(meio + Vector2(0.0f, -128.0f));
    auto* dcTitulo = titulo->AddComponent<DrawTextComponent>("PAUSA", mGradeBarFont.get(),
                                                             400, 64, 52, kOrdemDaPausa + 3);
    dcTitulo->SetAjustarAoTexto(true);
    mTituloDaPausa = AddActor(std::move(titulo));

    // A moldura da opcao em foco entra antes dos textos e com ordem menor, para
    // ficar atras deles - como no menu principal.
    auto selecao = std::make_unique<Actor>(this);
    selecao->SetPosition(meio);
    selecao->AddComponent<DrawCaixaComponent>(kLarguraDaSelecaoDaPausa, kAlturaDaSelecaoDaPausa,
                                              SDL_Color{235, 235, 235, 255}, SDL_Color{34, 38, 48, 150},
                                              3, kOrdemDaPausa + 2);
    mSelecaoDaPausa = AddActor(std::move(selecao));

    for (int i = 0; i < Pausa::kQuantas; ++i) {
        auto opcao = std::make_unique<Actor>(this);
        opcao->SetPosition(meio + Vector2(0.0f, -40.0f + static_cast<float>(i) * kPassoDaPausa));
        auto* dc = opcao->AddComponent<DrawTextComponent>(Pausa::Texto(Pausa::kOpcoes[i]), mGradeBarFont.get(),
                                                          kLarguraDaSelecaoDaPausa - 40, 42, 34,
                                                          kOrdemDaPausa + 3);
        dc->SetLarguraDeQuebra(static_cast<unsigned>(kLarguraDaSelecaoDaPausa));
        dc->SetAjustarAoTexto(true);
        mOpcoesDaPausa.push_back(AddActor(std::move(opcao)));
    }

    // O RODAPE SAI DO PAINEL, como o de toda tela (Painel.h). O ESC entra
    // escrito so aqui: a pausa so existe no computador, e e ele que a fecha.
    const std::string rodape = Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher"),
                                               {Painel::Jogador::Um, Painel::Botao::Um, "confirmar"},
                                               {std::string("ESC"), std::string(), std::string("continuar")}},
                                              mGame->JeitoDoRodape());
    auto pe = std::make_unique<Actor>(this);
    pe->SetPosition(meio + Vector2(0.0f, 150.0f));
    auto* dcPe = pe->AddComponent<DrawTextComponent>(rodape, mGradeBarFont.get(),
                                                     kLarguraDaPlacaDaPausa - 40, 28, 18, kOrdemDaPausa + 3);
    dcPe->SetLarguraDeQuebra(static_cast<unsigned>(kLarguraDaPlacaDaPausa - 40));
    dcPe->SetAjustarAoTexto(true);
    mRodapeDaPausa = AddActor(std::move(pe));

    MostrarPausa(false);
}

void Battle::MostrarPausa(const bool visivel) {
    for (Actor* a : {mVeuDaPausa, mPlacaDaPausa, mSelecaoDaPausa}) {
        if (a) if (auto* c = a->GetComponent<DrawCaixaComponent>()) c->SetIsVisible(visivel);
    }
    for (Actor* a : {mTituloDaPausa, mRodapeDaPausa}) {
        if (a) if (auto* t = a->GetComponent<DrawTextComponent>()) t->SetIsVisible(visivel);
    }
    for (Actor* a : mOpcoesDaPausa) {
        if (a) if (auto* t = a->GetComponent<DrawTextComponent>()) t->SetIsVisible(visivel);
    }
}

void Battle::PosicionarSelecaoDaPausa() {
    if (!mSelecaoDaPausa) return;
    const auto w = static_cast<float>(mGame->GetWindowWidth());
    const auto h = static_cast<float>(mGame->GetWindowHeight());
    mSelecaoDaPausa->SetPosition(Vector2(w / 2.0f, h / 2.0f - 40.0f
                                                   + static_cast<float>(mPausa.selecionada) * kPassoDaPausa));
}

void Battle::AbrirPausa() {
    Pausa::Abrir(mPausa);
    mAtoresCongelados = true;

    // Quem estava segurando o manche ou o tiro quando apertou ESC nao anda no
    // menu por isso: so conta a batida seguinte.
    mCimaNaPausa = mBaixoNaPausa = mConfirmarNaPausa = true;

    // O ALUNO PARA ONDE ESTA. Congelado, o Update dele nao roda - mas a
    // velocidade do ultimo quadro ficaria guardada no corpo dele, e e o
    // primeiro quadro depois da pausa que a usaria. Zerar aqui faz a volta
    // comecar parada, como comeca qualquer movimento.
    for (Player* jogador : mJogadores) {
        if (jogador) if (auto* rb = jogador->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
    if (mGame->GetAudio()) mGame->GetAudio()->PauseSound(mMusicHandle);

    PosicionarSelecaoDaPausa();
    MostrarPausa(true);
}

void Battle::FecharPausa() {
    Pausa::Fechar(mPausa);
    mAtoresCongelados = false;
    if (mGame->GetAudio()) mGame->GetAudio()->ResumeSound(mMusicHandle);
    MostrarPausa(false);
}

void Battle::ProcessarPausa(const Uint8* keyState, const bool escNovo) {
    if (escNovo) {
        FecharPausa();
        return;
    }

    // QUALQUER UM DOS DOIS mexe no menu: a pausa e da dupla, e quem esta mais
    // perto do teclado nao precisa pedir ao outro. O ENTER fica como atalho,
    // sem aparecer escrito - como no menu principal.
    bool cima = false, baixo = false, confirmar = false;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        cima = cima || Painel::Apertado(keyState, quem, Painel::Direcao::Cima);
        baixo = baixo || Painel::Apertado(keyState, quem, Painel::Direcao::Baixo);
        confirmar = confirmar || Painel::Apertado(keyState, quem, Painel::Botao::Um);
    }
    confirmar = confirmar || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];

    if (cima && !mCimaNaPausa) {
        Pausa::Mover(mPausa, -1);
        PosicionarSelecaoDaPausa();
    }
    if (baixo && !mBaixoNaPausa) {
        Pausa::Mover(mPausa, 1);
        PosicionarSelecaoDaPausa();
    }
    const bool escolheu = confirmar && !mConfirmarNaPausa;
    mCimaNaPausa = cima;
    mBaixoNaPausa = baixo;
    mConfirmarNaPausa = confirmar;
    if (escolheu) EscolherNaPausa();
}

void Battle::EscolherNaPausa() {
    switch (Pausa::Escolhida(mPausa)) {
        case Pausa::Opcao::Continuar:
            FecharPausa();
            break;
        case Pausa::Opcao::Recomecar:
            // A MESMA MATERIA, de novo e do zero: a troca de cena refaz a
            // batalha com o que o Game guarda - a materia e o orientador.
            SairDaBatalha(SceneType::Battle);
            break;
        case Pausa::Opcao::Sair:
            SairDaBatalha(SceneType::StageSelect);
            break;
    }
}

void Battle::SairDaBatalha(const SceneType destino) {
    if (mProjectileManager) {
        mProjectileManager->ClearBossProjectiles();
        mProjectileManager->Cleanup();
    }
    for (Boss* corpo : Corpos()) if (corpo) corpo->AoTerminarABatalha();
    mGame->RequestSceneChange(destino);
}
//endregion Pausa
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

    // --- REGRA 0 - O ABRIGO come os projeteis do chefe
    //
    // ANTES DA REGRA 1, de proposito: um projetil que encosta no abrigo e no
    // aluno no mesmo quadro some no abrigo - quem esta embaixo da capivara esta
    // protegido de verdade, e nao "quase sempre".
    //
    // SO COME O QUE ESTA SOLTO. Enfeite nao e tiro, outro abrigo nao se come, e
    // um projetil num posto e parte de uma estrutura do professor - a pilha
    // pendurada nele, uma chave na tabela -, que a capivara nao desmonta.
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

    // --- REGRA 0,5 - O ESCUDO DO PONTO EXTRA come os projeteis do chefe
    //
    // Em dupla, quem recebeu o Ponto Extra do parceiro fica com um escudo
    // pelo tempo da imunidade (PontoExtra.h): o que encosta nele some. Mesmo
    // filtro do abrigo - enfeite nao e tiro, e o que esta num posto e parte de
    // uma estrutura do professor, que o escudo nao desmonta.
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

    // --- REGRA 1 - Projéteis do Boss vs Player --- //
    for (Player* jogador : mJogadores) {
        const auto *playerCollider = ColisorDe(jogador);
        if (!playerCollider) continue;

        for (const auto &bossProj: bossProjectiles) {
            if (bossProj->GetState() != ActorState::Active) continue;
            if (bossProj->Enfeite()) continue;   // o cursor da sprint e a mao, nao um tiro
            if (bossProj->Abrigo()) continue;    // a capivara nao machuca ninguem
            if (const auto *bossProjCollider = bossProj->GetComponent<CircleColliderComponent>()) {
                if (playerCollider->Intersect(*bossProjCollider) && !jogador->GetIsInvincible()) {
                    jogador->SetIsInvincible(true);
                    jogador->OnCollision(bossProj.get());
                    bossProj->OnCollision(jogador);

                    // NEM TODO ACERTO E A MESMA COISA. As correntes do Vitor
                    // quase nao cobram nota e em troca prendem o aluno - e e a
                    // TROCA que faz o jogador ler "fiquei preso" em vez de
                    // "tomei dano". Os outros projeteis deixam estes campos no
                    // padrao e caem no mesmo GradeDown() de sempre.
                    if (bossProj->Acorrenta()) {
                        jogador->Acorrentar(bossProj->SegundosDeCorrente(),
                                            bossProj->FatorDaCorrente());
                    }
                    GradeDown(bossProj->MultiplicadorDaPerda());

                    // O ACERTO SOFRIDO SE VE: o anel vermelho no aluno e o
                    // campo piscando em vermelho pelas bordas.
                    Faiscar(playerCollider->GetCenter(), Hud::Faisca::Dano);
                    if (mMoldura) mMoldura->Acender(SDL_Color{255, 60, 50, 255}, 0.5f);
                }
            }
        }
    }

    // --- REGRA 1b - Projéteis do Player vs Projéteis DERRUBAVEIS do Boss
    //
    // ANTES DA REGRA 2, e de proposito: um tiro que derruba um processo se gasta
    // nele e nao chega ao chefe. E a troca da fase do Elixir - limpar o caminho
    // custa os acertos que iriam para a nota.
    //
    // QUEM E AVISADO E O CORPO ORIGINAL (mBoss), e nao o dono do projetil: o
    // supervisor e um so, e o dono pode ser um clone que ja deixou de existir.
    //
    // O BLINDADO CAI NA MESMA REGRA, pela mesma razao de ordem, e so o tiro
    // some: e a historia que o cursor da Maria Lucia ainda arrasta.
    for (const auto &playerProj: playerProjectiles) {
        if (playerProj->GetState() != ActorState::Active) continue;
        const auto *tiro = playerProj->GetComponent<CircleColliderComponent>();
        if (!tiro) continue;

        for (const auto &bossProj: bossProjectiles) {
            if (bossProj->GetState() != ActorState::Active) continue;
            // O ABRIGO TAMBEM COME O TIRO: quem se esconde embaixo da capivara
            // nao acerta o professor atraves dela.
            if (!bossProj->Derrubavel() && !bossProj->Blindado() && !bossProj->Abrigo()) continue;
            if (bossProj->Enfeite()) continue;   // o tiro passa pelo cursor
            const auto *alvo = bossProj->GetComponent<CircleColliderComponent>();
            if (!alvo || !alvo->Intersect(*tiro)) continue;

            playerProj->OnCollision(bossProj.get());
            // A BLINDAGEM VENCE: uma historia que o cursor ainda arrasta e
            // derrubavel (e do tipo), mas na mao dele o tiro so some.
            if (Destrutivel::Pode(bossProj->Derrubavel(), bossProj->Blindado(), bossProj->Abrigo(), bossProj->Enfeite())) {
                // A RESISTENCIA: so o ultimo golpe derruba. Os outros so comem o
                // tiro - a janela modal que pede mais de um clique.
                if (bossProj->Golpear()) {
                    Faiscar(bossProj->GetPosition(), Hud::Faisca::Derrubada);
                    bossProj->OnCollision(playerProj.get());
                    if (mBoss) {
                        mBoss->OnProjetilDerrubado();
                        mBoss->OnDerrubou(bossProj.get());
                    }
                } else if (mBoss) {
                    Faiscar(playerProj->GetPosition(), Hud::Faisca::Acerto);
                    mBoss->OnProjetilBarrado();
                }
            }
            break;   // um tiro derruba um processo so, ou some numa historia so
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

                // Acerto de UM jogador: repartido (requisitos-coop, 2.1). O
                // CHEFE BLINDADO come o tiro sem pagar a nota (Boss::AceitaDano).
                if (corpo->AceitaDano()) {
                    GradeUp(PorJogador(Nota::kGanhoPorAcerto));
                    Faiscar(playerProj->GetPosition(), Hud::Faisca::Acerto);
                }

                // TODO ACERTO CARREGA O PODER DO ORIENTADOR, blindado ou nao:
                // e a recompensa de quem continua atacando.
                if (mOrientador) mOrientador->Carregar();


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
                Faiscar(playerCollider->GetCenter(), Hud::Faisca::Coleta);
                if (mMoldura) mMoldura->Acender(SDL_Color{255, 215, 80, 255}, 0.3f);

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

void Battle::DarAVez(const size_t posicao) {
    if (posicao < mJogadores.size()) mVezDoAlvo = posicao;
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
void Battle::GradeDown(const float multiplicadorDaPerda) {

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
    // O MULTIPLICADOR NAO MEXE NA CONTAGEM, so na nota - e por isso ele entra
    // aqui embaixo e nao nos dois ++ acima. Uma corrente E um acerto sofrido:
    // ela conta para o direito a nota cheia e para a fase limpa, porque o
    // jogador de fato foi acertado. O que ela nao faz e custar os 6 pontos.
    mGrade = Nota::Subtrair(mGrade, PerdaPorJogador(GRADE_CHANGE_DOWN) * multiplicadorDaPerda);
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
    // A FASE NOVA passa pelas bordas do campo, em lilas.
    if (mMoldura) mMoldura->Acender(SDL_Color{196, 170, 230, 255}, 0.45f);
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
        if (mCorpos[i]) {

            // ANTES DE DESTRUIR O CORPO, TIRA OS TIROS DELE DE CENA. O corpo
            // leva junto as fabricas de projetil dele, e um tiro que sobreviva
            // ao dono tenta voltar a um pool que nao existe mais. Ver
            // ProjectileManager::DescartarProjeteisDe - foi um crash de
            // verdade, na passagem da fase 3 do Vitor para o exame.
            if (mProjectileManager) {
                mProjectileManager->DescartarProjeteisDe(mCorpos[i]);
            }
            mCorpos[i]->SetState(ActorState::Destroy);
        }
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
    if (mMoldura) mMoldura->Acender(SDL_Color{255, 215, 80, 255}, 0.7f);

    Player* oOutro = OutroJogador(quemGastou);

    if (!oOutro) {
        // SOZINHO, O PONTO EXTRA LIMPA A TELA, como sempre fez. Nao ha parceiro
        // a quem dar imunidade, e trocar a limpeza por "proteja a si mesmo"
        // deixaria o solo com um efeito mais fraco do que ele tem hoje - por
        // causa de uma regra que so existe para o modo que ele nao esta jogando.
        // O CENARIO FICA (ClearAtaquesDoChefe): a tabela do Thiago, o vetor do
        // Andre, as caixas da cascata. So o que machuca sai.
        if (mProjectileManager) mProjectileManager->ClearAtaquesDoChefe();
        // TODO - Audio - efeito sonoro de limpeza de campo
        return;
    }

    // EM DUPLA, O PONTO EXTRA E DO OUTRO, e nao limpa a tela.
    //
    // O PARCEIRO PASSA OVERLOAD_DURATION IMUNE E COM O ESCUDO, que come os
    // projeteis do chefe que encostam nele (a regra 0,5 de CheckCollisions).
    // Quem gastou CONTINUA ATIRANDO: a sobrecarga deixava a dupla com um
    // atirador so justamente quando o outro podia avancar. Um numero so
    // (Player::OVERLOAD_DURATION) para o escudo e para a espera de quem gastou.
    //
    // E NAO LIMPA A TELA. Limpar a tela E proteger daria, por metade do
    // estoque da dupla, o dobro do efeito de um Ponto Extra de solo - e o
    // escudo ja limpa o que chega perto de quem mais precisa.
    //
    // E SE VE: o feixe corre de quem gastou ate o parceiro (PontoExtra.h).
    oOutro->Proteger(Player::OVERLOAD_DURATION);
    mFeixes.push_back(FeixeDoPontoExtra{quemGastou, oOutro, 0.0f});
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

    // OS FEIXES: a cabeca corre ate o parceiro, e depois a linha se apaga.
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
            // A CABECA TEM CORPO: tres tracos lado a lado, o do meio mais
            // claro. Uma linha de 2 px sumia no meio da tela cheia.
            Vector2 lado(ate.y - de.y, de.x - ate.x);
            if (lado.LengthSq() > 0.0f) lado.Normalize();
            mDesenhoSobreOsAlunos->Acrescentar(feixe.cabecaDe + lado * 2.0f, feixe.cabecaAte + lado * 2.0f, SDL_Color{150, 210, 255, 200});
            mDesenhoSobreOsAlunos->Acrescentar(feixe.cabecaDe - lado * 2.0f, feixe.cabecaAte - lado * 2.0f, SDL_Color{150, 210, 255, 200});
            mDesenhoSobreOsAlunos->Acrescentar(feixe.cabecaDe, feixe.cabecaAte, SDL_Color{235, 250, 255, 255});
        }
    }

    // OS ESCUDOS: um anel azul pulsando em volta de quem esta protegido, que
    // aperta nos ultimos instantes - o aviso de que vai acabar.
    mPulsoDoEscudo += deltaTime;
    for (const Player* p : mJogadores) {
        if (!p || p->TempoDeImunidade() <= 0.0f) continue;
        const float pulso = 0.5f + 0.5f * Math::Sin(mPulsoDoEscudo * (p->TempoDeImunidade() < 0.8f ? 22.0f : 8.0f));
        const auto alfa = static_cast<Uint8>(120.0f + 120.0f * pulso);
        // DOIS ANEIS, o de dentro mais apagado: a borda do escudo tem espessura.
        for (const auto& [a, b] : PontoExtra::Anel(centroDe(p), PontoExtra::kRaioDoEscudo)) {
            mDesenhoSobreOsAlunos->Acrescentar(a, b, SDL_Color{120, 190, 255, alfa});
        }
        for (const auto& [a, b] : PontoExtra::Anel(centroDe(p), PontoExtra::kRaioDoEscudo - 3.0f)) {
            mDesenhoSobreOsAlunos->Acrescentar(a, b, SDL_Color{120, 190, 255, static_cast<Uint8>(alfa / 2)});
        }
    }

    // A HITBOX (opcao da tela de Opcoes): o circulo de colisao de verdade, e
    // um ponto no meio. E menor que o sprite - o que se aprende vendo isto e
    // que o tiro que passa no cabelo nao acerta.
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


// ---------------------------------------------------------------------------
// A MOLDURA DO CAMPO E OS EFEITOS
// ---------------------------------------------------------------------------

void Battle::LoadMoldura() {
    // A AREA DO ALUNO sai do MESMO tamanho que o Player::BorderLimitCheck usa
    // - o do desenho dele -, e nao de um numero copiado: a moldura fica onde
    // ele para, e mudar o sprite muda os dois juntos.
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
}

void Battle::Faiscar(const Vector2& onde, const Hud::Faisca tipo) {
    if (mFaiscas.size() >= kMaximoDeFaiscas) mFaiscas.erase(mFaiscas.begin());
    mFaiscas.push_back({onde, tipo, 0.0f});
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
        // O GIRO DE CADA UMA sai de onde ela nasceu: duas lado a lado nao sao
        // a mesma estrela repetida.
        const float giro = f.onde.x * 0.37f + f.onde.y * 0.11f;
        switch (f.tipo) {
            case Hud::Faisca::Acerto:
                // MIUDA E BRANCA: sao dez por segundo, e nao podem cobrir o chefe.
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
        }
    }
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
    // E O QUE O CHEFE DESENHOU NO CAMPO SEM SER PROJETIL (Boss.h).
    for (Boss* corpo : Corpos()) if (corpo) corpo->AoTerminarABatalha();

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
