//
// A tela onde o aluno escolhe a personagem dele.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"

class Font;
class Actor;

/**
 * @class CriacaoDePersonagem
 * @brief Escolhe a aparencia que vai ficar gravada no perfil da matricula.
 *
 * ESTA TELA NAO SABE COMPOR NADA. Quais camadas existem, em que ordem e com que
 * cor e assunto de Source/Personagens (puro, testado) e de
 * Source/ComporPersonagem (a ponte). Aqui so ha navegacao e desenho.
 *
 * POR ORA ESCOLHE ENTRE COMBINACOES PRONTAS, e nao camada por camada. A segunda
 * forma esta descrita em Documentacao/requisitos-perfil-e-personagem.md, RF3, e
 * - por construcao - nao mexe em save nem em composicao: o que muda e so esta
 * tela, porque o que vai para o disco ja e o conjunto de escolhas.
 *
 * UM ATOR POR COMBINACAO, com visibilidade alternada, em vez de um ator que
 * troca de textura: DrawAnimatedComponent::LoadSpriteSheet ACRESCENTA quadros
 * em vez de substituir, entao recarregar a mesma componente duplicaria o atlas.
 * Com meia duzia de combinacoes, compor todas na entrada e barato - e ainda
 * deixa a troca instantanea. A versao camada por camada vai precisar recompor,
 * e ai esse caminho tem de ser arrumado.
 */
class CriacaoDePersonagem : public Scene {
public:
    explicit CriacaoDePersonagem(Game* game);
    ~CriacaoDePersonagem() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    /// Cria um ator de texto centrado horizontalmente, no tamanho natural dele.
    Actor* Texto(const std::string& conteudo, float y, int tamanho, int larguraMaxima);

    /// Deixa visivel so a personagem em foco, e atualiza o nome e o contador.
    void Mostrar() const;

    std::unique_ptr<Font> mFonte;

    /// Um ator por combinacao, na ordem do catalogo.
    std::vector<Actor*> mPersonagens;

    Actor* mNomeAtor{};
    Actor* mContadorAtor{};

    int mEscolhida = 0;

    bool mEsquerdaAnterior{};
    bool mDireitaAnterior{};

    /// Comeca em TRUE: chega-se aqui apertando ENTER na tela de matricula, e sem
    /// a guarda a primeira combinacao seria confirmada sozinha no mesmo toque.
    bool mConfirmarAnterior = true;
    bool mVoltarAnterior = true;
};
