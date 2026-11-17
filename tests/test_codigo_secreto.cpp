// O codigo secreto do menu inicial.

#include "doctest.h"

#include "../Source/Entrada/CodigoSecreto.h"

using namespace CodigoSecreto;

namespace {
    /// Toca a lista toda; devolve em que toque (contando de 1) o codigo fechou, ou 0.
    int TocarTudo(Sequencia& s, std::initializer_list<Toque> toques) {
        int k = 0;
        for (const Toque t : toques) {
            ++k;
            if (s.Tocar(t)) return k;
        }
        return 0;
    }
}

TEST_CASE("CodigoSecreto: cima, cima, baixo, baixo, esquerda, direita, esquerda, direita, B, A") {
    Sequencia s;
    for (size_t k = 0; k + 1 < kCodigo.size(); ++k) {
        CHECK_FALSE(s.Tocar(kCodigo[k]));
        CHECK(s.Certos() == static_cast<int>(k) + 1);
    }
    CHECK(s.Tocar(kCodigo.back()));
    // Depois do codigo, recomeca do zero: um A sozinho nao fecha de novo.
    CHECK(s.Certos() == 0);
    CHECK_FALSE(s.Tocar(Toque::A));
}

TEST_CASE("CodigoSecreto: errar no meio volta para o comeco, mas o toque certo do fim conta") {
    using T = Toque;
    Sequencia s;
    // Um erro no meio: o codigo nao fecha.
    CHECK(TocarTudo(s, {T::Cima, T::Cima, T::Baixo, T::Esquerda, T::Direita, T::Esquerda, T::Direita, T::B, T::A}) == 0);
    CHECK(s.Certos() == 0);
    // Cima tres vezes: os dois ultimos ainda sao o comeco do codigo.
    s.Esquecer();
    CHECK(TocarTudo(s, {T::Cima, T::Cima, T::Cima}) == 0);
    CHECK(s.Certos() == 2);
    CHECK(TocarTudo(s, {T::Baixo, T::Baixo, T::Esquerda, T::Direita, T::Esquerda, T::Direita, T::B, T::A}) == 8);
    // Andar no menu antes nao atrapalha.
    s.Esquecer();
    CHECK(TocarTudo(s, {T::Baixo, T::Baixo, T::A, T::Cima, T::Cima, T::Baixo, T::Baixo, T::Esquerda,
                        T::Direita, T::Esquerda, T::Direita, T::B, T::A}) == 13);
    // A ordem importa: A antes do B nao vale.
    s.Esquecer();
    CHECK(TocarTudo(s, {T::Cima, T::Cima, T::Baixo, T::Baixo, T::Esquerda, T::Direita, T::Esquerda, T::Direita, T::A, T::B}) == 0);
}
