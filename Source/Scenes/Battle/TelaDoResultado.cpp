//
// A tela do fim da luta.
//

#include "TelaDoResultado.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

#include "../PainelDeMenu.h"
#include "../Scene.h"
#include "../TextoDeCena.h"
#include "../../Actors/Actor.h"
#include "../../Actors/Teachers/Boss.h"
#include "../../Actors/Teachers/BossFactory/BossProjectileFactory/FabricaDeProjetil.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../Entrada/Painel.h"
#include "../../Interface/Hud.h"
#include "../../Motor/AudioSystem.h"
#include "../../Motor/Game.h"
#include "../../Personagem/ComporPersonagem.h"
#include "../../Personagem/PersonagensArquivo.h"

TelaDoResultado::TelaDoResultado(Scene& cena, Game& jogo, Font* fonte, const std::size_t jogadores,
                                 const bool toque, std::function<void()> aoTocarAqui)
    : mCena(cena), mJogo(jogo), mFonte(fonte), mJogadores(jogadores), mToque(toque),
      mAoTocarAqui(std::move(aoTocarAqui)) {}

TelaDoResultado::~TelaDoResultado() = default;

void TelaDoResultado::CriarPainel(const int linhas, const bool comToque) {
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
    const std::string rodape = Painel::Rodape(acoes, mJogo.JeitoDoRodape());

    // Mais alta que a da pausa: cabem as linhas do resumo entre o titulo e as opcoes.
    PainelDeMenu::Medidas medidas;
    medidas.larguraDaPlaca = kLargura;
    medidas.alturaDaPlaca = PainelDeMenu::AlturaPara(linhas);
    medidas.ordem = kOrdem;
    mPainel = std::make_unique<PainelDeMenu>(mCena, mFonte, " ", opcoes, rodape, linhas, medidas);
}

void TelaDoResultado::Mostrar(const Resultado::Resumo& resumo, Boss* chefe) {
    mSelecionada = 0;
    // O tiro que estava apertado quando a luta acabou nao escolhe nada.
    mLeitor.Segurar();
    const std::vector<std::string> linhas = Resultado::Linhas(resumo);
    // A dupla aprovada comemora: os dois avatares no painel, e o toca aqui (Comunicacao.h).
    const bool duplaAprovada = mJogadores >= 2 && resumo.aprovado;
    mPodeTocar = duplaAprovada;
    CriarPainel(static_cast<int>(linhas.size()), duplaAprovada && mToque);
    // A formatura e dourada, o titulo e a linha de quem ganhou o capelo.
    const bool formou = Resultado::Formou(resumo);
    mPainel->SetTitulo(Resultado::Titulo(resumo),
                       formou ? Color::Gold : resumo.aprovado ? Color::LightGreen : Color::LightPink);
    mPainel->SetLinhas(linhas, Resultado::LinhaEmDestaque(resumo));
    if (formou) mPainel->ColorirLinha(Resultado::LinhaDaFormatura(resumo), Color::Gold);
    if (Resultado::LinhaDasMencoes(resumo) >= 0) mPainel->ColorirLinha(Resultado::LinhaDasMencoes(resumo), Color::Gold);
    if (formou || duplaAprovada) MostrarAlunos();
    // A justica: cada aluno na cor dele, com o titulo e os numeros.
    for (size_t i = 0; i < resumo.partes.size(); ++i) {
        const Hud::Rgb c = Hud::CorDoAluno(i);
        for (int j = 0; j < Resultado::LinhasPorParte(resumo); ++j) {
            mPainel->ColorirLinha(Resultado::LinhaDaParte(resumo, i) + j,
                                  Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f));
        }
    }
    mPainel->Selecionar(mSelecionada);
    mPainel->Mostrar(true);
    // A devolutiva: o desenho do que mais acertou, na linha dela.
    const int linhaDasCausas = Resultado::LinhaDasCausas(resumo);
    if (linhaDasCausas >= 0) MostrarCausas(resumo.causas, mPainel->OndeDaLinha(linhaDasCausas), chefe);
}

void TelaDoResultado::MostrarAlunos() {
    if (mJogadores == 0 || !mPainel) return;
    const Vector2 titulo = mPainel->OndeDoTitulo();
    const size_t ultimo = mJogadores - 1;
    // O primeiro aluno a esquerda, o ultimo a direita: sozinho, ele e os dois. Nao depende de onde o
    // aluno estava quando a luta acabou.
    const std::pair<size_t, float> lados[] = {{0, -1.0f}, {ultimo, 1.0f}};
    for (const auto& [quem, lado] : lados) {
        const Personagens::Composta c = Personagens::Compor(&mJogo, Personagens::Carregado(),
                                                            mJogo.AparenciaDe(quem), mJogo.Formado(quem));
        if (!c.ok) continue;
        auto formando = std::make_unique<Actor>(&mCena);
        formando->SetPosition(titulo + Vector2(lado * kAfastamentoDoFormando, 0.0f));
        formando->SetScale(kEscalaDoFormando);
        auto* desenho = formando->AddComponent<DrawAnimatedComponent>(c.chaveDaTextura, c.atlas, kOrdem + 4);
        desenho->AddAnimation("Andando", {0, 1, 2, 3});
        desenho->SetAnimFPS(6.0f);
        desenho->SetAnimation("Andando");
        const Vector2 base = formando->GetPosition();
        mAvatares.push_back(Avatar{mCena.AddActor(std::move(formando)), base});
    }
}

void TelaDoResultado::MostrarCausas(const std::vector<Devolutiva::Causa>& causas, const Vector2& onde, Boss* chefe) {
    // "Mais te acertou:" a esquerda; depois, cada causa: o desenho do projetil e quantas vezes.
    const int ordem = kOrdem + 4;
    CriarTexto(mCena, mFonte, "Mais te acertou:", onde + Vector2(-150.0f, 0.0f), 26, 230, 34, ordem);
    float x = onde.x - 10.0f;
    for (const Devolutiva::Causa& c : causas) {
        const std::string vezes = "\xc3\x97" + std::to_string(c.acertos);
        const auto* fabrica = chefe ? dynamic_cast<FabricaDeProjetil*>(chefe->GetProjectileFactory(c.causa)) : nullptr;
        if (c.causa == Devolutiva::kEsbarrao || !fabrica) {
            // O esbarrao nao tem desenho de projetil: o nome dele.
            CriarTexto(mCena, mFonte, "encostar " + vezes, Vector2(x + 50.0f, onde.y), 26, 150, 34, ordem);
            x += 170.0f;
            continue;
        }
        auto icone = std::make_unique<Actor>(&mCena);
        icone->SetPosition(Vector2(x, onde.y));
        auto* desenho = icone->AddComponent<DrawAnimatedComponent>(fabrica->CaminhoDoSprite(), fabrica->CaminhoDosDados(), ordem);
        for (const auto& anim : fabrica->Descricao().animacoes) desenho->AddAnimation(anim.nome, anim.quadros);
        desenho->SetAnimation(fabrica->Descricao().animacaoInicial);
        // Do tamanho de uma linha, por maior que seja o projetil.
        const float lado = static_cast<float>(std::max(desenho->GetSpriteWidth(), desenho->GetSpriteHeight()));
        // Pixel art cresce bem: o projetil miudo vai ate quatro vezes, o grande encolhe.
        icone->SetScale(lado > 0.0f ? std::min(4.0f, kLadoDoIcone / lado) : 1.0f);
        mCena.AddActor(std::move(icone));
        CriarTexto(mCena, mFonte, vezes, Vector2(x + 48.0f, onde.y), 26, 80, 34, ordem);
        x += 120.0f;
    }
}

void TelaDoResultado::Avancar(const float deltaTime) {
    mTempo += deltaTime;
    AtualizarToque(deltaTime);
    if (Resultado::VoltaSozinha(mJogo.NoGabinete(), mTempo)) {
        mJogo.RequestSceneChange(Scene::SceneType::StageSelect);
    }
}

void TelaDoResultado::Processar(const Uint8* keyState) {
    // Lido mesmo na trava (Resultado::kTravaDoMenu).
    const LeitorDoMenu::Batidas b = mLeitor.Ler(keyState);
    LerToque(keyState);
    if (mTempo < Resultado::kTravaDoMenu) return;

    if (!mPainel) return;
    if (b.cima) mSelecionada = Resultado::Mover(mSelecionada, -1);
    if (b.baixo) mSelecionada = Resultado::Mover(mSelecionada, 1);
    mPainel->Selecionar(mSelecionada);
    if (b.confirmar) Escolher();
}

void TelaDoResultado::Escolher() {
    // A nota ja esta na ficha e a tentativa ja esta nos dados (Battle::EncerrarBatalha).
    switch (Resultado::kOpcoes[mSelecionada]) {
        case Resultado::Opcao::TentarDeNovo:
            mJogo.RequestSceneChange(Scene::SceneType::Battle);
            break;
        case Resultado::Opcao::Voltar:
            mJogo.RequestSceneChange(Scene::SceneType::StageSelect);
            break;
    }
}

void TelaDoResultado::LerToque(const Uint8* keyState) {
    if (!mPodeTocar || mJogadores < 2) return;
    for (size_t i = 0; i < 2; ++i) {
        const auto quem = i == 0 ? Painel::Jogador::Um : Painel::Jogador::Dois;
        const bool apertado = Painel::Apertado(keyState, quem, Painel::Botao::Tres);
        // O botao que vinha segurado da luta (o sinal) nao conta: so o aperto depois da trava.
        if (apertado && !mTresAntes[i] && mTempo >= Resultado::kTravaDoMenu) mUltimoTres[i] = mTempo;
        mTresAntes[i] = apertado;
    }
    if (Comunicacao::Juntos(mUltimoTres[0], mUltimoTres[1]) && mTempo - mUltimoToque >= Comunicacao::kEsperaDoToque) {
        TocarAqui();
    }
}

void TelaDoResultado::TocarAqui() {
    // Cada par de apertos vale um toque so.
    mUltimoTres = {Comunicacao::kNuncaApertou, Comunicacao::kNuncaApertou};
    mUltimoToque = mTempo;
    // A medida conta sempre; o experimento sem o toque so nao pula, nao estoura e nao toca.
    if (mAoTocarAqui) mAoTocarAqui();
    if (!mToque) return;
    for (Avatar& a : mAvatares) a.pulo = 0.0f;
    mEstouro = 0.0f;
    if (!mDesenhoDoToque) {
        auto ator = std::make_unique<Actor>(&mCena);
        mDesenhoDoToque = ator->AddComponent<DrawLinhasComponent>(kOrdem + 5);
        mCena.AddActor(std::move(ator));
    }
    if (mJogo.GetAudio()) mJogo.GetAudio()->PlaySound("toque.wav", false);
}

void TelaDoResultado::AtualizarToque(const float deltaTime) {
    // O pulo dos dois, juntos.
    for (Avatar& a : mAvatares) {
        if (!a.ator) continue;
        a.pulo = std::min(a.pulo + deltaTime, Comunicacao::kDuracaoDoPulo);
        a.ator->SetPosition(a.base - Vector2(0.0f, Comunicacao::AlturaDoPulo(a.pulo)));
    }
    if (!mDesenhoDoToque) return;
    mDesenhoDoToque->Limpar();
    mEstouro = std::min(mEstouro + deltaTime, Comunicacao::kDuracaoDoEstouro);
    const float alfa = Comunicacao::AlfaDoEstouro(mEstouro);
    if (alfa <= 0.0f || !mPainel) return;

    // O estouro dourado entre os dois: raios que abrem do meio do titulo.
    const Vector2 meio = mPainel->OndeDoTitulo();
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
