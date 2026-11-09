//
// Created by nslop on 16/09/2024.
//

#include "Battle.h"

#include "../../Aluno/Progresso.h"
#include "../../Regras/Nota.h"

#include <random>
#include <sstream>
#include <iomanip>

#include "BattleHUD.h"
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
#include "../../Components/DrawComponents/DrawBarraDaNotaComponent.h"
#include "../../Components/DrawComponents/DrawFundoDoCampoComponent.h"
#include "../../Components/DrawComponents/DrawMolduraDoCampoComponent.h"
#include "../../Components/DrawComponents/DrawQuadroComponent.h"
#include "../../Components/DrawComponents/DrawRelogioDaFaseComponent.h"
#include "../../Actors/Teachers/ExtraPointItem.h"
#include "../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../Regras/PontoExtra.h"
#include "../../Regras/Destrutivel.h"
#include "../../Actors/Orientador/Orientador.h"
#include "../../Curso/MateriasArquivo.h"
#include "../../Entrada/Painel.h"

#include "../../Motor/CaminhosArquivo.h"
#include "../../Motor/Fontes.h"
#include "RegistroDaTentativa.h"
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
    , mNota(40.0f)
    , mMateria(selectedStage)
{
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

    // Toda batalha comeca em 40, mesmo que o aluno ja tenha tirado mais nesta materia.
    mNota = Progresso::kNotaInicial;

    // Quem foi atingido ontem nao perde a nota cheia de hoje.
    mAcertosSofridos = 0;
    mAcertosNaFase = 0;
    mPrimeiraFase = true;
    mFasesLimpas = 0;
    mAvisoDeFaseLimpa = 0.0f;

    // O modo e a semente antes do chefe: a fabrica e o chefe podem perguntar pelos dois.
    mNivel = mGame->NivelDeDificuldade();
    mSemente = static_cast<std::uint32_t>(std::random_device{}());
    SDL_Log("BATALHA: semente %u, modo %s", mSemente, Dificuldade::Chave(mNivel));

    // 2.
    CriarJogadores();
    CriarChefe();
    CriarOrientador();
    CriarHUD();
    CriarResultado();
    CriarPausa();

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
    CriarBarraDaNota();
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
void Battle::CriarBarraDaNota() {
    auto gradeBarActor = std::make_unique<Actor>(this);

    const int windowWidth = mGame->GetWindowWidth();
    const int windowHeight = mGame->GetWindowHeight();

    // A barra recuada, e nao de ponta a ponta.
    const int barraX = kBarraMargemLateral;
    const int barraY = windowHeight - kBarraDoFundo;
    const int barraLargura = windowWidth - 2 * kBarraMargemLateral;

    gradeBarActor->SetPosition(Vector2(static_cast<float>(barraX), static_cast<float>(barraY)));

    // A barra desenhada por conta propria (DrawBarraDaNotaComponent).
    auto gradeBarComp = gradeBarActor->AddComponent<DrawBarraDaNotaComponent>(barraLargura, kBarraAltura, 200);
    gradeBarComp->Comecar(mNota);

    mBarraDaNotaAtor = gradeBarActor.get();
    this->AddActor(std::move(gradeBarActor));

    // Configurar Actor e fonte que vai ter a nota no meio da barra.
    mFonteDaNota = std::make_unique<Font>();
    mFonteDaNota->Load(Caminhos::Asset(Fontes::kJogo));
    auto gradeBarTextActor = std::make_unique<Actor>(this);

    // A caixa e 200x32, e nao 36x25.
    constexpr int tamanhoDaNota = 24;
    // A placa primeiro, com ordem menor.
    gradeBarTextActor->AddComponent<DrawCaixaComponent>(
        kPlacaLargura, kPlacaAltura - 4, kMolduraDaPlacaDaNota, kFundoDaPlacaDaNota, 1, kOrdemDaPlaca);

    // A nota de partida desde o primeiro quadro.
    std::stringstream inicial;
    inicial << std::fixed << std::setprecision(2) << mNota;
    auto gradeText = gradeBarTextActor->AddComponent<DrawTextComponent>(
        inicial.str(), mFonteDaNota.get(), 200, tamanhoDaNota + 8, tamanhoDaNota, 304);
    gradeText->SetAjustarAoTexto(true);

    const auto textPosX = static_cast<float>(windowWidth) / 2.f;
    const auto textPosY = static_cast<float>(barraY + kBarraAltura / 2);
    gradeBarTextActor->SetPosition(Vector2(textPosX, textPosY));

    mTextoDaNotaAtor = gradeBarTextActor.get();
    this->AddActor(std::move(gradeBarTextActor));

    // O anuncio do teste final, escondido ate a nota chegar a 100.
    auto avisoActor = std::make_unique<Actor>(this);
    auto placaDoTeste = avisoActor->AddComponent<DrawCaixaComponent>(
        kPlacaDoAvisoLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca);
    placaDoTeste->SetIsVisible(false);

    auto aviso = avisoActor->AddComponent<DrawTextComponent>(
        "TESTE FINAL", mFonteDaNota.get(), 260, tamanhoDaNota + 8, tamanhoDaNota, 304);
    aviso->SetAjustarAoTexto(true);
    aviso->SetColor(Color::Gold);
    aviso->SetIsVisible(false);

    avisoActor->SetPosition(Vector2(static_cast<float>(windowWidth) / 4.f, textPosY));

    mTesteFinalAtor = avisoActor.get();
    this->AddActor(std::move(avisoActor));

    // O aviso de fase limpa, na direita da faixa.
    auto limpaActor = std::make_unique<Actor>(this);
    auto placaDaLimpa = limpaActor->AddComponent<DrawCaixaComponent>(
        kPlacaDoAvisoLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca);
    placaDaLimpa->SetIsVisible(false);

    auto limpa = limpaActor->AddComponent<DrawTextComponent>(
        "FASE LIMPA", mFonteDaNota.get(), 260, tamanhoDaNota + 8, tamanhoDaNota, 304);
    limpa->SetAjustarAoTexto(true);
    limpa->SetColor(Color::LightGreen);
    limpa->SetIsVisible(false);

    limpaActor->SetPosition(Vector2(static_cast<float>(windowWidth) * 3.f / 4.f, textPosY));

    mFaseLimpaAtor = limpaActor.get();
    this->AddActor(std::move(limpaActor));

    // O selo do Dificil, na ponta direita da faixa.
    if (mNivel == Dificuldade::Nivel::Dificil) {
        auto selo = std::make_unique<Actor>(this);
        selo->AddComponent<DrawCaixaComponent>(
            kPlacaLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca);
        auto texto = selo->AddComponent<DrawTextComponent>(
            Dificuldade::Nome(mNivel), mFonteDaNota.get(), kPlacaLargura, tamanhoDaNota + 8,
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

void Battle::CriarResultado() {
    std::vector<std::string> opcoes;
    for (const Resultado::Opcao o : Resultado::kOpcoes) opcoes.emplace_back(Resultado::Texto(o));

    // Sem o ESC no rodape: a luta acabou, e quem escolhe e o painel.
    const std::string rodape = Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher"),
                                               {Painel::Jogador::Um, Painel::Botao::Um, "confirmar"}},
                                              mGame->JeitoDoRodape());

    // Mais alta que a da pausa: cabem as linhas do resumo entre o titulo e as opcoes.
    PainelDeMenu::Medidas medidas;
    medidas.alturaDaPlaca = 480;
    medidas.ordem = kOrdemDosMenus;
    mPainelDoResultado = std::make_unique<PainelDeMenu>(*this, mFonteDaNota.get(), " ", opcoes, rodape,
                                                        Resultado::kLinhasNoMaximo, medidas);
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

        if (Resultado::VoltaSozinha(mGame->NoGabinete(), mTempoNoFim)) {
            mGame->RequestSceneChange(SceneType::StageSelect);
        }

        return;
    }

    // A cena base já atualiza todos os atores.
    Avisar([&](OuvinteDaBatalha& o) { o.AoPassarOTempo(*this, deltaTime, false); });

    // Atualiza os sistemas principais apenas da Battle (os atores são atualizados pela Scene)
    if (mProjeteis) mProjeteis->Update(deltaTime);

    // Orquestra a checagem de colisões.
    VerificarColisoes();
    AtualizarDesenhoSobreOsAlunos(deltaTime);
    AtualizarMoldura();
    AtualizarOndas(deltaTime);
    AtualizarEfeitos(deltaTime);

    if (mHUD) mHUD->OnUpdate(deltaTime, mChefe && mChefe->Congelado());
    if (mRede > 0.0f) mRede = std::max(0.0f, mRede - deltaTime);

    AtualizarBarraDeTempo();
    AtualizarBarraDaNota();

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
    if (mNota <= 0) {
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
void Battle::AtualizarBarraDaNota() {
    auto gradeComp = mBarraDaNotaAtor->GetComponent<DrawBarraDaNotaComponent>();
    if (!gradeComp) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "GradeBarActor not found");
        return;
    }

    // ---- Definição da paleta de cores -- // ----

    // O sulco e a moldura sao da barra (DrawBarraDaNotaComponent).

    // STATE1 - reprovade.
    static const SDL_Color FAIL_START = {100, 0, 0, 255};
    static const SDL_Color FAIL_END = {255, 0, 0, 255};

    // STATE2 - final.
    static const SDL_Color SECRET_START = {180, 150, 0, 255};
    static const SDL_Color SECRET_END = {255, 140, 0, 255};

    // STATE3 - aprovade.
    static const SDL_Color PASS_START = {144, 238, 144, 255};
    static const SDL_Color PASS_END = {0, 255, 128, 255};

    // ---- --------- -- ------ -- ----- -- // ----

    float colorPercent = 0.0f;

    SDL_Color de = PASS_START, ate = PASS_END;
    if (mNota < 40.f) {
        colorPercent = mNota / 40.f;
        de = FAIL_START; ate = FAIL_END;
    } else if (mNota < 60.f) {
        colorPercent = (mNota - 40.f) / 20.f;
        de = SECRET_START; ate = SECRET_END;
    } else {
        colorPercent = (mNota - 60.f) / 40.f;
    }
    const auto lerp = [colorPercent](const Uint8 a, const Uint8 b) {
        return static_cast<Uint8>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * colorPercent);
    };
    const SDL_Color cor{lerp(de.r, ate.r), lerp(de.g, ate.g), lerp(de.b, ate.b), 255};

    gradeComp->SetNota(mNota, cor, mNota < 40.f, Nota::ECheia(mNota));
    AtualizarTextoDaNota();

}
void Battle::AtualizarTextoDaNota() {

    // Duas casas, e nao uma.
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << mNota;

    const auto texto = mTextoDaNotaAtor->GetComponent<DrawTextComponent>();
    texto->SetText(ss.str());

    // O dourado segue a nota, em vez de ser ligado uma vez e ficar aceso.
    const bool cheia = Nota::ECheia(mNota);
    texto->SetColor(cheia ? Color::Gold : Color::White);

    // O anuncio, ao contrario, nao volta atras.
    if (cheia && mTesteFinalAtor) {
        // A placa acende junto.
        mTesteFinalAtor->GetComponent<DrawCaixaComponent>()->SetIsVisible(true);
        mTesteFinalAtor->GetComponent<DrawTextComponent>()->SetIsVisible(true);
    }

    // O aviso de fase limpa some sozinho.
    if (mFaseLimpaAtor) {
        const bool acesa = mAvisoDeFaseLimpa > 0.0f;
        mFaseLimpaAtor->GetComponent<DrawCaixaComponent>()->SetIsVisible(acesa);
        mFaseLimpaAtor->GetComponent<DrawTextComponent>()->SetIsVisible(acesa);
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
    mPainelDoResultado->SetTitulo(Resultado::Titulo(resumo.aprovado),
                                  resumo.aprovado ? Color::LightGreen : Color::LightPink);
    mPainelDoResultado->SetLinhas(Resultado::Linhas(resumo), Resultado::LinhaEmDestaque(resumo));
    mPainelDoResultado->Selecionar(mResultadoSelecionado);
    mPainelDoResultado->Mostrar(true);
}

void Battle::ProcessarResultado(const Uint8* keyState) {
    // Lido mesmo na trava (Resultado::kTravaDoMenu).
    const BatidasDoMenu b = LerMenu(keyState);
    if (mTempoNoFim < Resultado::kTravaDoMenu) return;

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
                    DescerNota(bossProj->MultiplicadorDaPerda());

                    Faiscar(playerCollider->GetCenter(), Hud::Faisca::Dano);
                    Avisar([&](OuvinteDaBatalha& o) { o.AoSerAcertado(*this, *jogador, true); });
                }
            }
        }
    }
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
                        Faiscar(bossProj->GetPosition(), Hud::Faisca::Derrubada);
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
                    Faiscar(bossProj->GetPosition(), Hud::Faisca::Derrubada);
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
                Avisar([&](OuvinteDaBatalha& o) { o.AoAcertarOProfessor(*this, playerProj->Dono()); });

                if (ouviu) {
                    SubirNota(PorJogador(Nota::kGanhoPorAcerto));
                    // A duvida respondida: o "?" vira um "!" na cor de quem perguntou.
                    Faiscar(playerProj->GetPosition(), Hud::Faisca::Resposta,
                            PosicaoDoAluno(playerProj->Dono()));
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
                DescerNota();
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

    // Quem passa a vez e RevezarAlvo, chamado por quem sabe que um disparo comecou.
    if (mJogadores.empty()) return nullptr;
    return mVezDoAlvo < mJogadores.size() ? mJogadores[mVezDoAlvo] : mJogadores.front();
}

void Battle::DarAVez(const size_t posicao) {
    if (posicao < mJogadores.size()) mVezDoAlvo = posicao;
}

void Battle::RevezarAlvo() {

    // O "% tamanho" e o que faz isto nao dar nada com um jogador: 0 + 1 = 1, e 1 % 1 = 0.
    if (mJogadores.empty()) { mVezDoAlvo = 0; return; }
    mVezDoAlvo = (mVezDoAlvo + 1) % mJogadores.size();
}

float Battle::PorJogador(const float bruto) const {
    return Nota::PorJogador(bruto, mJogadores.size());
}

void Battle::SubirNota(const float amount) {

    // A aritmetica inteira vive em Nota.
    mNota = Nota::Somar(mNota, amount, mAcertosSofridos);
}
void Battle::DescerNota(const float multiplicadorDaPerda) {

    // A contagem mora aqui, no instante do acerto, e nao num balanco no fim da batalha.
    ++mAcertosSofridos;
    ++mAcertosNaFase;
    // PerdaPorJogador, e nao PorJogador.
    mNota = Nota::Subtrair(mNota, PerdaPorJogador(Nota::kPerdaPorAcerto) * multiplicadorDaPerda);
}

float Battle::PerdaPorJogador(const float bruto) const {
    return Nota::PerdaPorJogador(bruto, mJogadores.size());
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

    // A primeira chamada nao paga nada.
    if (mPrimeiraFase) {
        mPrimeiraFase = false;
        mAcertosNaFase = 0;
        return;
    }

    if (mAcertosNaFase == 0) {
        // Da fase, e nao de um jogador: pago inteiro.
        SubirNota(Nota::kBonusDeFaseLimpa);
        ++mFasesLimpas;
        Avisar([&](OuvinteDaBatalha& o) { o.AoLimparFase(*this); });

        // O aviso existe para a nota nao subir por motivo invisivel.
        mAvisoDeFaseLimpa = kDuracaoDoAvisoDeFaseLimpa;
    }

    mAcertosNaFase = 0;
}

SDL_FRect Battle::GetPlayfieldBounds() const
{
    // O campo acaba onde a faixa da nota comeca, e esse numero e o da barra - nao uma copia dele.
    return {
        0.0f, // x
        0.0f, // y
        static_cast<float>(mGame->GetWindowWidth()), // w
        static_cast<float>(mGame->GetWindowHeight() - kBarraDoFundo) // h
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
    mNota = PontoExtra::NotaDepois(mNota, mAcertosSofridos, PorJogador(Nota::kGanhoPorAcerto),
                                    PontoExtra::TirosNaSobrecarga());
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

Vector2 Battle::PontaDaBarra() const {
    const float largura = static_cast<float>(mGame->GetWindowWidth() - 2 * kBarraMargemLateral);
    return Vector2(static_cast<float>(kBarraMargemLateral) + largura * std::clamp(mNota, 0.0f, 100.0f) / 100.0f,
                   static_cast<float>(mGame->GetWindowHeight() - kBarraDoFundo + kBarraAltura / 2));
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
            case Hud::Faisca::Resposta: {
                // O "!" sobe e some, na cor do aluno, com o miolo claro e um contorno escuro.
                const Hud::Rgb c = Hud::CorDoAluno(f.quem < 0 ? 9 : static_cast<size_t>(f.quem));
                const SDL_Color cor{c.r, c.g, c.b, alfa(b)};
                const SDL_Color miolo{255, 255, 240, alfa(b)};
                const SDL_Color contorno{20, 22, 34, alfa(0.85f * b)};
                const Vector2 p = f.onde + Vector2(0.0f, -10.0f - r);
                const auto exclamacao = [&](const int largura, const float folga, const SDL_Color cor) {
                    for (int dx = -largura; dx <= largura; ++dx) {
                        const auto x = static_cast<float>(dx);
                        mEfeitos->Acrescentar(p + Vector2(x, -16.0f - folga), p + Vector2(x, -4.0f + folga), cor);
                        mEfeitos->Acrescentar(p + Vector2(x, -1.0f - folga), p + Vector2(x, 2.0f + folga), cor);
                    }
                };
                exclamacao(3, 1.0f, contorno);
                exclamacao(2, 0.0f, cor);
                exclamacao(0, -1.0f, miolo);
                break;
            }
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
    const Vector2 ponta = PontaDaBarra();
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

    // A ultima fase tambem paga.
    EncerrarFase();

    mAcabando = true;

    // O resumo da tela de resultado.
    Resultado::Resumo resumo;
    resumo.nota = mNota;
    resumo.aprovado = aprovado;
    resumo.temFicha = mGame->ProgressoEGravado(0);
    resumo.recordeAntes = mGame->RecordeDaPartida(mMateria);
    resumo.fasesLimpas = mFasesLimpas;
    resumo.acertosSofridos = mAcertosSofridos;

    // O unico ponto em que a nota vai para o armazenamento persistente.
    mGame->RegistrarNota(mMateria, mNota);
    // E a tentativa, com o resultado.
    Avisar([&](OuvinteDaBatalha& o) { o.AoFimDoQuadro(*this); });
    const auto resultado = aprovado ? Tentativas::Resultado::Aprovado : Tentativas::Resultado::Reprovado;
    Avisar([&](OuvinteDaBatalha& o) { o.AoTerminar(*this, resultado, mNota > 0.0f); });
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
