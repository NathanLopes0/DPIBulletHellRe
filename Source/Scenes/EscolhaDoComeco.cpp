//
// "Personalizar personagem" ou "jogar agora": a escolha de quem ainda nao passou na INF 110.
//

#include "EscolhaDoComeco.h"

#include <chrono>
#include <random>
#include <string>

#include <SDL_scancode.h>

#include "FundoDeCena.h"
#include "../Actors/Actor.h"
#include "../Motor/Game.h"
#include "../Motor/Font.h"
#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Fontes.h"
#include "../Personagem/PersonagensArquivo.h"

namespace {
    /// As opcoes, a mais rapida primeiro: o foco ja nasce nela.
    enum Opcao { kJogarAgora = 0, kPersonalizar = 1 };
    constexpr int kQuantas = 2;
    constexpr int kLinhas = 2;
    constexpr int kLargura = 760;
    constexpr int kOrdem = 200;
}

EscolhaDoComeco::EscolhaDoComeco(Game* game)
    : Scene(game, SceneType::EscolhaDoComeco),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset(Fontes::kJogo));
}

EscolhaDoComeco::~EscolhaDoComeco() = default;

Painel::Jogador EscolhaDoComeco::PainelDaVez() const {
    return mQuem == 1 ? Painel::Jogador::Dois : Painel::Jogador::Um;
}

void EscolhaDoComeco::Load() {

    CriarFundoDoCorredor(*this);
    mQuem = mGame->PosicaoEntrando();

    const std::string rodape = Painel::Rodape({Painel::Manche(PainelDaVez(), "escolher"),
                                               {PainelDaVez(), Painel::Botao::Um, "confirmar"},
                                               {PainelDaVez(), Painel::Botao::Dois, "voltar"}},
                                              mGame->JeitoDoRodape());
    PainelDeMenu::Medidas medidas;
    medidas.larguraDaPlaca = kLargura;
    medidas.alturaDaPlaca = PainelDeMenu::AlturaPara(kLinhas);
    medidas.ordem = kOrdem;
    mPainel = std::make_unique<PainelDeMenu>(*this, mFonte.get(), " ",
                                             std::vector<std::string>{"Jogar agora", "Personalizar personagem"},
                                             rodape, kLinhas, medidas);
    const bool emDupla = mGame->QuantosJogadores() > 1;
    mPainel->SetTitulo(emDupla ? "JOGADOR " + std::to_string(mQuem + 1) + ": COMO COMEÇAR?"
                               : std::string("COMO VOCÊ QUER COMEÇAR?"),
                       Vector3(1.0f, 0.86f, 0.42f));
    mPainel->SetLinhas({"Jogar agora: uma personagem sorteada, sem montar nada.",
                        "Personalizar: monte a sua peça por peça (dá para trocar depois)."});
    mPainel->Selecionar(mSelecionada);
    mPainel->Mostrar(true);
}

void EscolhaDoComeco::JogarAgora() {
    // Quem ja tem uma aparencia (uma ficha antiga) fica com ela; quem nao tem ganha uma sorteada,
    // gravada na ficha como se tivesse montado.
    if (mGame->AparenciaDe(mQuem).Vazia()) {
        const auto agora = static_cast<std::uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count());
        const std::uint32_t semente = std::random_device{}() ^ agora ^ static_cast<std::uint32_t>(mQuem * 7919u);
        mGame->DefinirAparencia(Personagens::Carregado().Aleatoria(semente));
    }
    mGame->AvancarEntrada();
}

void EscolhaDoComeco::OnProcessInput(const Uint8* keyState) {

    const Painel::Jogador quem = PainelDaVez();
    const bool voltar = Painel::Apertado(keyState, quem, Painel::Botao::Dois) || keyState[SDL_SCANCODE_ESCAPE];
    if (mBordaVoltar.Bateu(voltar)) {
        // De volta a matricula deste aluno: nada foi gravado.
        mGame->RequestSceneChange(SceneType::Identificacao);
        return;
    }

    int passo = 0;
    if (mBordaCima.Bateu(Painel::Apertado(keyState, quem, Painel::Direcao::Cima))) passo -= 1;
    if (mBordaBaixo.Bateu(Painel::Apertado(keyState, quem, Painel::Direcao::Baixo))) passo += 1;
    if (passo != 0 && mPainel) {
        mSelecionada = (mSelecionada + passo + kQuantas) % kQuantas;
        mPainel->Selecionar(mSelecionada);
    }

    const bool confirmar = Painel::Apertado(keyState, quem, Painel::Botao::Um)
                        || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (mBordaConfirmar.Bateu(confirmar)) {
        if (mSelecionada == kJogarAgora) JogarAgora();
        else mGame->RequestSceneChange(SceneType::CriacaoDePersonagem);
    }
}
