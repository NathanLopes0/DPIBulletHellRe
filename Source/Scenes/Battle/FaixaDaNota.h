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

    /// Onde a barra acaba com a nota em 'nota': para onde voam as moedas do Ponto Extra (solta, a
    /// barra leva a ponta junto).
    [[nodiscard]] Vector2 Ponta(float nota) const;

    // ---- A barra solta ----
    //
    // O Lucas Ferreira arranca a barra da HUD e a usa de golpe. Solta, ela vai aonde Pousar mandar,
    // com o numero no meio dela, e o lugar dela no rodape fica vazio: so o contorno do encaixe. Ela
    // continua sendo a nota - enche e esvazia como no rodape.

    void Soltar();
    void Devolver();
    [[nodiscard]] bool Solta() const { return mSolta; }
    /// O centro da barra no rodape, e o comprimento dela la.
    [[nodiscard]] Vector2 CentroDoEncaixe() const;
    [[nodiscard]] float ComprimentoNoEncaixe() const;
    /// A barra solta em 'centro', com 'comprimento', girada 'angulo' graus (horario, como o SDL), e
    /// espelhada - enchendo do outro fim.
    void Pousar(Vector2 centro, float comprimento, float angulo, bool espelhada);
    /// A nota que a barra mostra agora (a que persegue a de verdade).
    [[nodiscard]] float Mostrada() const;
    /// O numero da placa some (o Lucas Ferreira o tirou dali para reescreve-lo), e a placa fica vazia.
    void EsconderNumero(bool esconder);
    /// A barra se quebrou (o Ponto Extra do aluno no golpe do Lucas Ferreira): ela some, e o rodape
    /// fica so com o lugar vazio dela e o numero.
    void Quebrar();
    /// A cor do cheio da barra agora.
    [[nodiscard]] SDL_Color CorDoCheio() const;
    /// Onde o numero fica: o meio da placa.
    [[nodiscard]] Vector2 LugarDoNumero() const;
    /// A espessura da barra.
    static constexpr int kEspessura = 28;

private:
    void AtualizarTexto(float nota, bool faseLimpa);

    int mLarguraDaTela;
    int mAlturaDaTela;
    Actor* mBarra = nullptr;
    Actor* mTexto = nullptr;
    /// O "TESTE FINAL": escondido ate a nota chegar a 100, e dali em diante fica.
    Actor* mTesteFinal = nullptr;
    Actor* mFaseLimpa = nullptr;
    /// O contorno do lugar vazio, aceso enquanto a barra esta solta.
    Actor* mEncaixe = nullptr;
    bool mSolta = false;
    Vector2 mCentroSolto;
    float mComprimentoSolto = 0.0f;
    float mAnguloSolto = 0.0f;
    bool mEspelhadaSolta = false;

    /// A altura da barra.
    static constexpr int kAltura = kEspessura;
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
    /// O lugar vazio da barra: o contorno e um fundo quase nada.
    static inline const SDL_Color kMolduraDoEncaixe{235, 240, 255, 90};
    static inline const SDL_Color kFundoDoEncaixe{6, 8, 16, 120};
};
