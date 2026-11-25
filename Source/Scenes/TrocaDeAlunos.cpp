//
// A troca de aluno(s), do Botao 4 da selecao de fases.
//

#include "TrocaDeAlunos.h"

#include <SDL_scancode.h>

#include "FundoDeCena.h"
#include "../Entrada/Painel.h"
#include "../Aluno/Sessao.h"
#include "../Motor/Game.h"
#include "../Motor/Font.h"
#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Fontes.h"
#include "../Actors/Actor.h"

namespace {
    /// As opcoes, na ordem do menu principal: quem joga agora.
    constexpr Sessao::Modo kModos[] = {Sessao::Modo::Um, Sessao::Modo::Dois};
    constexpr const char* kNomes[] = {"Modo Solo", "Modo Dupla"};
    constexpr int kQuantas = 2;

    /// A placa: o titulo, uma linha dizendo o que acontece, as duas opcoes e o rodape.
    constexpr int kLinhas = 2;
    constexpr int kLargura = 720;
    constexpr int kOrdem = 200;
}

TrocaDeAlunos::TrocaDeAlunos(Game* game)
    : Scene(game, SceneType::TrocaDeAlunos),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset(Fontes::kJogo));
}

TrocaDeAlunos::~TrocaDeAlunos() = default;

void TrocaDeAlunos::Load() {

    CriarFundoDoCorredor(*this);

    const std::string rodape = Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher"),
                                               {Painel::Jogador::Um, Painel::Botao::Um, "confirmar"},
                                               {Painel::Jogador::Um, Painel::Botao::Dois, "voltar"}},
                                              mGame->JeitoDoRodape());
    PainelDeMenu::Medidas medidas;
    medidas.larguraDaPlaca = kLargura;
    medidas.alturaDaPlaca = PainelDeMenu::AlturaPara(kLinhas);
    medidas.ordem = kOrdem;
    mPainel = std::make_unique<PainelDeMenu>(*this, mFonte.get(), " ",
                                             std::vector<std::string>(kNomes, kNomes + kQuantas), rodape,
                                             kLinhas, medidas);
    mPainel->SetTitulo("TROCAR ALUNO(S)", Vector3(1.0f, 0.86f, 0.42f));
    mPainel->SetLinhas({"Quem joga agora? Escolha o modo e entre com a matrícula.",
                        "O progresso de quem estava jogando já está guardado."});
    mPainel->Selecionar(mSelecionada);
    mPainel->Mostrar(true);
}

void TrocaDeAlunos::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (mBordaVoltar.Bateu(voltar)) {
        // Ninguem trocou: as sessoes de quem jogava estao intactas.
        mGame->RequestSceneChange(SceneType::StageSelect);
        return;
    }

    const bool cima = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Cima);
    const bool baixo = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Baixo);
    int passo = 0;
    if (mBordaCima.Bateu(cima)) passo -= 1;
    if (mBordaBaixo.Bateu(baixo)) passo += 1;
    if (passo != 0 && mPainel) {
        mSelecionada = (mSelecionada + passo + kQuantas) % kQuantas;
        mPainel->Selecionar(mSelecionada);
    }

    const bool confirmar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Um)
                        || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (mBordaConfirmar.Bateu(confirmar)) {
        // Como no menu principal: as sessoes sao refeitas, e a entrada comeca do primeiro aluno.
        mGame->DefinirModoDeJogo(kModos[mSelecionada]);
        mGame->RequestSceneChange(SceneType::Identificacao);
    }
}
