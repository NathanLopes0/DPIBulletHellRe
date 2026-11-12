//
// O placar: o quadro-negro no canto de cima, com o que o professor anuncia na fase.
//

#pragma once

#include <string>

class Actor;
class Font;
class Scene;

/// O quadro da sala ("anuncio" em fases.json): verde de quadro-negro, moldura de giz e o texto da
/// fase. Sem anuncio, ele some.
class PlacarDaFase {
public:
    /// 'esquerda': onde comeca a area do aluno - o quadro fica depois do trilho da esquerda, e nao
    /// colado na borda da tela.
    PlacarDaFase(Scene& cena, Font* fonte, float esquerda);

    /// O que o quadro diz agora. So reescreve (e apaga o giz) quando o texto muda: e o texto, e nao a
    /// fase, que se compara - dentro de uma fase com Intercambio o placar troca de texto no aviso.
    void Mostrar(const std::string& anuncio);

private:
    Actor* mAtor = nullptr;
    std::string mTexto;

    static constexpr int kLargura = 320;
    static constexpr int kAltura = 46;
    static constexpr int kMargem = 16;
    static constexpr int kTamanho = 30;
    /// Atras dos projeteis (88 a 92) e do aluno (100).
    static constexpr int kOrdem = 60;
};
