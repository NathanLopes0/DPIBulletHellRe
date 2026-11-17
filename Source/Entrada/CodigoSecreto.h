//
// O codigo secreto do menu inicial: cima, cima, baixo, baixo, esquerda, direita, esquerda, direita,
// B, A.
//

#pragma once

#include <array>
#include <deque>

/// Quem digita o codigo no menu inicial abre a materia secreta mais cedo: ela aparece assim que o
/// aluno passa na materia do "atalho" dela (Materias::Materia::atalho). Aqui so a sequencia: a cena
/// diz cada toque, e a sequencia diz quando o codigo inteiro acabou de entrar.
namespace CodigoSecreto {

    enum class Toque { Cima, Baixo, Esquerda, Direita, B, A };

    constexpr std::array<Toque, 10> kCodigo = {
        Toque::Cima, Toque::Cima, Toque::Baixo, Toque::Baixo, Toque::Esquerda,
        Toque::Direita, Toque::Esquerda, Toque::Direita, Toque::B, Toque::A};

    /// Lembra os ultimos toques. Errar nao obriga a esperar: os toques certos do fim ainda contam
    /// (quem aperta cima tres vezes ainda esta no comeco do codigo).
    class Sequencia {
    public:
        /// True no toque que fecha o codigo; depois, a sequencia recomeca do zero.
        bool Tocar(Toque toque);
        /// Quantos toques do codigo ja entraram, em ordem, ate agora (0 a 9).
        [[nodiscard]] int Certos() const;
        void Esquecer() { mToques.clear(); }

    private:
        std::deque<Toque> mToques;
    };
}
