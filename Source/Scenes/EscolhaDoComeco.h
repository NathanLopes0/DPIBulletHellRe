//
// "Personalizar personagem" ou "jogar agora": a escolha de quem ainda nao passou na INF 110.
//

#pragma once

#include <memory>

#include "Scene.h"
#include "PainelDeMenu.h"
#include "../Entrada/Borda.h"
#include "../Entrada/Painel.h"

class Font;

/// Logo depois da matricula, para cada aluno que ainda nao passou na primeira materia do curso
/// (Materias::Lista::Primeira) - o novo, o visitante e quem ainda esta no comeco. Montar a
/// personagem e importante para uns e demora para outros: "Jogar agora" sorteia uma aparencia
/// (Personagens::Catalogo::Aleatoria) para quem ainda nao tem nenhuma - quem ja tem fica com a dele - e
/// segue a entrada; "Personalizar personagem" abre a criacao de sempre. O Botao 2 volta a matricula.
class EscolhaDoComeco : public Scene {
public:
    explicit EscolhaDoComeco(Game* game);
    ~EscolhaDoComeco() override;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

private:
    /// O painel de quem esta entrando: o jogador 2 escolhe no manche dele.
    [[nodiscard]] Painel::Jogador PainelDaVez() const;

    void JogarAgora();

    std::unique_ptr<Font> mFonte;
    std::unique_ptr<PainelDeMenu> mPainel;
    int mSelecionada = 0;
    size_t mQuem = 0;

    Borda mBordaCima;
    Borda mBordaBaixo;
    Borda mBordaConfirmar{true};
    Borda mBordaVoltar{true};
};
