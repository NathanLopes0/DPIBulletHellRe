//
// A pausa da batalha, no computador: o menu e a contagem da volta.
//

#pragma once

#include <memory>

#include <SDL.h>

#include "LeitorDoMenu.h"
#include "../../Entrada/Painel.h"
#include "../../Interface/Pausa.h"

class Actor;
class Font;
class PainelDeMenu;
class Scene;

/// O menu da pausa (Pausa.h) e o numero da contagem 3-2-1 no meio do campo. Quem congela e solta a
/// luta, e para e solta a musica, e a batalha: isto mostra o menu, le o que o aluno escolhe e conta.
class PausaDaBatalha {
public:
    /// O que o aluno pediu no menu, alem de continuar (que a pausa resolve sozinha).
    enum class Pedido { Nada, Recomecar, Sair };

    /// 'jeito': como o rodape mostra as teclas (Painel.h). 'campo': a contagem fica no meio dele.
    PausaDaBatalha(Scene& cena, Font* fonte, Painel::Jeito jeito, const SDL_FRect& campo);
    ~PausaDaBatalha();

    /// Abre o menu com o foco no Continuar. A contagem, se corria, para: ao fechar de novo ela
    /// recomeca do 3.
    void Abrir();

    /// Le o menu neste quadro. O ESC e o Continuar fecham o menu e comecam a contagem.
    Pedido Processar(const Uint8* keyState, bool escNovo);

    /// Anda a contagem; true no quadro em que ela acaba e a luta volta.
    bool Avancar(float deltaTime);

    [[nodiscard]] bool Aberta() const { return mMenu.aberto; }
    /// O menu aberto ou a contagem correndo: a luta esta parada.
    [[nodiscard]] bool Parada() const { return mMenu.aberto || Retomando(); }

private:
    [[nodiscard]] bool Retomando() const { return mRetomada > 0.0f; }
    /// Fecha o menu e comeca a contagem: a luta segue parada ate ela acabar.
    void Fechar();
    void MostrarNumero(bool visivel);

    Pausa::Menu mMenu;
    std::unique_ptr<PainelDeMenu> mPainel;
    LeitorDoMenu mLeitor;
    /// A contagem da volta (Pausa::kRetomada): segundos que faltam, e 0 fora dela.
    float mRetomada = 0.0f;
    Actor* mNumero = nullptr;

    /// Por cima de tudo: a faixa da nota vai ate 304.
    static constexpr int kOrdem = 900;
};
