//
// A faixa da nota, na base da tela: a barra, o numero e os avisos.
//

#pragma once

#include <SDL.h>

#include "../../Motor/Math.h"
#include "../../Regras/Dificuldade.h"

class Actor;
class Font;
class Scene;

/// A barra da nota (DrawBarraDaNotaComponent), o numero no meio dela, o "TESTE FINAL" (acende na
/// nota cheia e nao apaga mais), o "FASE LIMPA" (acende e apaga) e o selo do Dificil. So mostra:
/// quem conta a nota e Nota::Conta.
class FaixaDaNota {
public:
    /// Do fundo da tela ate o topo da barra. O campo acaba onde a faixa comeca
    /// (Battle::GetPlayfieldBounds).
    static constexpr int kDoFundo = 54;

    FaixaDaNota(Scene& cena, Font* fonte, int larguraDaTela, int alturaDaTela, float nota,
                Dificuldade::Nivel nivel);

    /// Um quadro com a nota em 'nota'. 'faseLimpa': o aviso de fase limpa esta aceso.
    void Atualizar(float nota, bool faseLimpa);

    /// Onde a barra acaba com a nota em 'nota': para onde voam as moedas do Ponto Extra.
    [[nodiscard]] Vector2 Ponta(float nota) const;

private:
    void AtualizarTexto(float nota, bool faseLimpa);

    int mLarguraDaTela;
    int mAlturaDaTela;
    Actor* mBarra = nullptr;
    Actor* mTexto = nullptr;
    /// O "TESTE FINAL": escondido ate a nota chegar a 100, e dali em diante fica.
    Actor* mTesteFinal = nullptr;
    Actor* mFaseLimpa = nullptr;

    /// A altura da barra.
    static constexpr int kAltura = 28;
    /// A sobra dos lados. A barra ia de ponta a ponta da tela e encostava nas bordas, o que a fazia
    /// parecer um risco no monitor em vez de uma peca.
    static constexpr int kMargemLateral = 40;

    static constexpr int kPlacaLargura = 112;
    /// A dos avisos e mais larga: "TESTE FINAL" tem onze letras contra as seis de "100.00".
    static constexpr int kPlacaDoAvisoLargura = 196;
    /// A mesma altura da barra, e nao um numero proprio: dois pixels de diferenca ja faziam a placa
    /// aparecer por cima da moldura.
    static constexpr int kPlacaAltura = kAltura;
    static constexpr int kOrdemDaPlaca = 303;   ///< o texto da faixa usa 304
    static inline const SDL_Color kSemMoldura{0, 0, 0, 0};
    static inline const SDL_Color kFundoDaPlaca{6, 8, 16, 135};
    /// A placa do numero da nota, mais fechada que a dos avisos e com um fio de moldura.
    static inline const SDL_Color kFundoDaPlacaDaNota{6, 8, 16, 200};
    static inline const SDL_Color kMolduraDaPlacaDaNota{255, 255, 255, 55};
};
