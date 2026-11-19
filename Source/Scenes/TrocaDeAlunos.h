//
// A troca de aluno(s), do Botao 4 da selecao de fases.
//

#pragma once

#include <memory>

#include "Scene.h"
#include "PainelDeMenu.h"
#include "../Entrada/Borda.h"

class Font;

/// Num gabinete com fila, quem acabou de jogar passa a vez sem voltar ao menu principal: o Modo Solo
/// ou o Modo Dupla, e dali a matricula de sempre (Identificacao). O Botao 2 desiste e volta para as
/// materias de quem estava jogando, que nada perdeu.
class TrocaDeAlunos : public Scene {
public:
    explicit TrocaDeAlunos(Game* game);
    ~TrocaDeAlunos() override;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

private:
    std::unique_ptr<Font> mFonte;
    std::unique_ptr<PainelDeMenu> mPainel;
    int mSelecionada = 0;

    Borda mBordaCima;
    Borda mBordaBaixo;
    /// Os dois nascem apertados: o botao que abriu esta tela, ou o da tela anterior, nao escolhe nada.
    Borda mBordaConfirmar{true};
    Borda mBordaVoltar{true};
};
