//
// A tela onde o aluno monta a personagem dele, camada por camada.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Personagens.h"

class Font;
class Actor;
class DrawCaixaComponent;

/**
 * @class CriacaoDePersonagem
 * @brief Monta a aparencia que vai ficar gravada no perfil da matricula.
 *
 * ESTA TELA NAO SABE COMPOR NADA. Quais camadas existem, em que ordem e com que
 * cor e assunto de Source/Personagens (puro, testado) e de
 * Source/ComporPersonagem (a ponte). Aqui so ha navegacao e desenho.
 *
 * AS LINHAS SAO GERADAS DO CATALOGO, e nao escritas a mao: cada categoria rende
 * uma linha de tipo (quando ha mais de uma peca) e uma de cor (quando ha mais de
 * uma cor). Acrescentar uma categoria no arquivo acrescenta as linhas dela sem
 * tocar nesta classe - e e por isso que "Tom de pele" aparece so com a linha de
 * cor, sem a de tipo: o corpo e uma peca so.
 */
class CriacaoDePersonagem : public Scene {
public:
    explicit CriacaoDePersonagem(Game* game);
    ~CriacaoDePersonagem() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    /// Uma linha da lista: o que ela muda.
    struct Linha {
        std::string categoria;
        bool ehCor = false;       ///< false = troca a peca; true = troca a cor
        std::string rotulo;
    };

    Actor* Texto(const std::string& conteudo, float x, float y, int tamanho, int larguraMaxima);

    /// Monta as linhas a partir do catalogo.
    void MontarLinhas();

    /// Anda uma posicao na linha em foco. `passo` e +1 ou -1.
    void Trocar(int passo);

    /// Recompoe a personagem e troca a textura em exibicao.
    void Recompor();

    /// Atualiza os rotulos e a moldura da linha em foco.
    void Redesenhar() const;

    std::unique_ptr<Font> mFonte;

    std::vector<Linha> mLinhas;
    std::vector<Actor*> mLinhaAtores;

    Actor* mPersonagemAtor{};
    Actor* mSelecaoAtor{};

    /// A aparencia sendo montada.
    Personagens::Aparencia mAparencia;

    /// A chave da textura em exibicao. Guardada para poder destrui-la quando a
    /// proxima for composta - ver Game::EsquecerTextura.
    std::string mChaveEmUso;

    int mLinhaEmFoco = 0;

    bool mCimaAnterior{};
    bool mBaixoAnterior{};
    bool mEsquerdaAnterior{};
    bool mDireitaAnterior{};

    /// Comeca em TRUE: chega-se aqui apertando ENTER na tela de matricula, e sem
    /// a guarda a aparencia inicial seria confirmada sozinha no mesmo toque.
    bool mConfirmarAnterior = true;
    bool mVoltarAnterior = true;
};
