//
// Os selos das mencoes de uma materia, numa fileira.
//

#pragma once

#include <array>

#include "../Aluno/Mencoes.h"
#include "../Motor/Math.h"

class DrawCaixaComponent;
class DrawTextComponent;
class Font;
class Scene;

/// Uma fileira com um selo por mencao (Mencoes.h): aceso na cor dela quando o aluno a tem, apagado
/// quando ainda falta, e ausente quando a materia nao a tem. O boletim, a selecao de materias e a
/// tela do orientador desenham as mesmas.
class SelosDeMencao {
public:
    /// 'esquerda': o meio da borda esquerda da fileira.
    SelosDeMencao(Scene& cena, Font* fonte, const Vector2& esquerda, int ordem = 200);

    /// Acende, apaga e esconde os selos conforme as mencoes da materia.
    void Mostrar(const Mencoes::DaMateria& mencoes);
    void Esconder();

    static constexpr float kLargura = 28.0f;
    static constexpr float kAltura = 20.0f;
    static constexpr float kEntre = 4.0f;
    /// A fileira inteira, com todas as mencoes.
    static constexpr float kLarguraDaFileira = Mencoes::kQuantas * (kLargura + kEntre) - kEntre;

private:
    struct Selo {
        DrawCaixaComponent* caixa = nullptr;
        DrawTextComponent* texto = nullptr;
    };
    std::array<Selo, Mencoes::kQuantas> mSelos{};
};
