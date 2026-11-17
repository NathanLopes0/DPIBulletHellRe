//
// A tela onde o aluno monta a personagem dele, camada por camada.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Entrada/Borda.h"
#include "../Entrada/Painel.h"
#include "../Personagem/Personagens.h"

class Font;
class Actor;
class DrawCaixaComponent;

/// Monta a aparencia que vai ficar gravada no perfil da matricula. Esta tela nao sabe compor nada.
class CriacaoDePersonagem : public Scene {
public:
    explicit CriacaoDePersonagem(Game* game);
    ~CriacaoDePersonagem() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

private:

    /// Uma linha da tabela: uma categoria (cabelo, camisa...), com a peca na coluna do objeto e a cor
    /// na coluna do lado.
    struct Linha {
        std::string categoria;
        std::string rotulo;
        bool temPeca = false;     ///< mais de uma peca: a coluna do objeto troca
        bool temCor = false;      ///< mais de uma cor: a coluna da cor troca
    };

    /// As duas colunas da tabela.
    enum Coluna { kObjeto = 0, kCor = 1 };

    /// Se a celula troca alguma coisa.
    [[nodiscard]] bool Editavel(int linha, int coluna) const;

    /// Poe o foco numa coluna que troca alguma coisa nesta linha: a preferida, se der.
    void AjustarColuna();

    Actor* Texto(const std::string& conteudo, float x, float y, int tamanho, int larguraMaxima);

    /// Monta as linhas a partir do catalogo.
    void MontarLinhas();

    /// Anda uma posicao na celula em foco. `passo` e +1 ou -1.
    void Trocar(int passo);

    /// Recompoe a personagem e troca a textura em exibicao.
    void Recompor();

    /// Atualiza os rotulos, as amostras de cor e a moldura da celula em foco.
    void Redesenhar() const;

    std::unique_ptr<Font> mFonte;

    /// Qual posicao esta montando a personagem. Cada jogador monta a sua, e esta tela roda uma vez
    /// por jogador novo.
    size_t mQuem = 0;

    /// O painel da vez.
    [[nodiscard]] Painel::Jogador PainelDaVez() const {
        return mQuem == 1 ? Painel::Jogador::Dois : Painel::Jogador::Um;
    }

    std::vector<Linha> mLinhas;
    std::vector<Actor*> mObjetoAtores;
    std::vector<Actor*> mCorAtores;
    /// O quadradinho com a cor escolhida, ao lado do nome dela.
    std::vector<Actor*> mAmostras;

    Actor* mPersonagemAtor{};
    Actor* mSelecaoAtor{};

    /// A aparencia sendo montada.
    Personagens::Aparencia mAparencia;

    /// A chave da textura em exibicao. Guardada para poder destrui-la quando a
    /// proxima for composta - ver Game::EsquecerTextura.
    std::string mChaveEmUso;

    int mLinhaEmFoco = 0;
    int mColunaEmFoco = kObjeto;
    /// A coluna que o aluno escolheu com o Botao 3: ao mudar de linha, o foco fica nela quando ela
    /// troca alguma coisa (o tom de pele so tem cor).
    int mColunaPreferida = kObjeto;

    Borda mBordaCima;
    Borda mBordaBaixo;
    Borda mBordaEsquerda;
    Borda mBordaDireita;
    Borda mBordaColuna;

    /// Comeca em true: chega-se aqui apertando enter na tela de matricula, e sem a guarda a
    /// aparencia inicial seria confirmada sozinha no mesmo toque.
    Borda mBordaConfirmar{true};
    Borda mBordaVoltar{true};
};
