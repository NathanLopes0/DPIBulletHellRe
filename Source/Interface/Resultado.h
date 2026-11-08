//
// A tela de resultado da batalha: o resumo e o que fazer em seguida.
//

#pragma once

#include <string>
#include <vector>

/// A luta acabou: em vez de mostrar aprovade por quatro segundos e voltar sozinha para as materias,
/// a batalha mostra o resumo dela.
namespace Resultado {

    /// As opcoes, na ordem da tela. A primeira e a que abre em foco.
    enum class Opcao { TentarDeNovo, Voltar };
    constexpr Opcao kOpcoes[] = {Opcao::TentarDeNovo, Opcao::Voltar};
    constexpr int kQuantas = static_cast<int>(sizeof(kOpcoes) / sizeof(kOpcoes[0]));

    const char* Texto(Opcao o);

    /// A opcao 'passo' casas depois da 'atual', dando a volta.
    int Mover(int atual, int passo);

    /// O que a batalha sabe no fim dela.
    struct Resumo {
        float nota = 0.0f;
        bool aprovado = false;
        /// O aluno tem ficha gravada (o visitante nao tem recorde a bater).
        bool temFicha = false;
        float recordeAntes = 0.0f;
        int fasesLimpas = 0;
        int acertosSofridos = 0;
    };

    /// O titulo: "APROVADE!" ou "REPROVADE".
    const char* Titulo(bool aprovado);

    /// A nota desta luta passou o recorde? So para quem tem ficha, e por mais que o arredondamento
    /// da tela (0,01).
    bool NovoRecorde(const Resumo& r);

    /// As linhas do resumo, de cima para baixo: a nota, o recorde (ou o aviso de recorde novo), as
    /// fases limpas e os acertos sofridos.
    std::vector<std::string> Linhas(const Resumo& r);

    /// Quantas Linhas um resumo tem no maximo: e quantas o painel reserva.
    constexpr int kLinhasNoMaximo = 4;

    /// Qual das Linhas vai em destaque (dourada): a do recorde novo, ou -1 quando nao ha nenhuma.
    int LinhaEmDestaque(const Resumo& r);

    /// Segundos antes de o menu aceitar o botao. O botao 1 e o do tiro: quem estava atirando quando
    /// a luta acabou nao pode escolher sem querer.
    constexpr float kTravaDoMenu = 0.8f;

    /// No gabinete, o resultado volta sozinho para as materias.
    constexpr float kEsperaNoGabinete = 20.0f;
    bool VoltaSozinha(bool arcade, float segundosNaTela);
}
