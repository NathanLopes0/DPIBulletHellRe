//
// Os selos das mencoes de uma materia, numa fileira.
//

#pragma once

#include <array>

#include "../Aluno/Mencoes.h"
#include "../Motor/Math.h"

class DrawAnimatedComponent;
class Scene;

/// Uma fileira com um selo por mencao (Mencoes.h): aceso na cor dela quando o aluno a tem, apagado
/// quando ainda falta, e ausente quando a materia nao a tem. O boletim e a selecao de materias
/// desenham as mesmas. Os selos sao pixel art de Ferramentas/gerar_interface.py (DPIBHSelos),
/// desenhada em 2x.
class SelosDeMencao {
public:
    /// 'esquerda': o meio da borda esquerda da fileira. 'compacta': selos menores, para a coluna do
    /// boletim.
    SelosDeMencao(Scene& cena, const Vector2& esquerda, int ordem = 200, bool compacta = false);

    /// Acende, apaga e esconde os selos conforme as mencoes da materia.
    void Mostrar(const Mencoes::DaMateria& mencoes);
    void Esconder();

    /// O selo da arte (13 x 10 pixels) em 2x.
    static constexpr float kEscala = 2.0f;
    static constexpr float kLargura = 13.0f * kEscala;
    static constexpr float kAltura = 10.0f * kEscala;
    static constexpr float kEntre = 4.0f;
    /// A fileira compacta so aperta o espaco entre os selos; e a folga a mais entre as mencoes e
    /// os desafios.
    static constexpr float kLarguraCompacta = kLargura;
    static constexpr float kEntreCompacta = 2.0f;
    static constexpr float kEntreOsGrupos = 8.0f;
    /// A fileira inteira, com todas as mencoes e a folga dos desafios.
    static constexpr float kLarguraDaFileira = Mencoes::kQuantas * (kLargura + kEntre) - kEntre + kEntreOsGrupos;
    static constexpr float kLarguraDaFileiraCompacta =
        Mencoes::kQuantas * (kLarguraCompacta + kEntreCompacta) - kEntreCompacta + kEntreOsGrupos;

private:
    std::array<DrawAnimatedComponent*, Mencoes::kQuantas> mSelos{};
};
