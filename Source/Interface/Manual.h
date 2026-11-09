//
// O manual do jogo: as paginas da tela "Como Jogar".
//

#pragma once

#include <string>
#include <vector>

/// O texto mora em Assets/como-jogar.json, e nao no C++.
namespace Manual {

    struct Pagina {
        std::string titulo;
        std::vector<std::string> linhas;
        /// Linhas que so aparecem fora do gabinete: as que falam de teclas.
        std::vector<std::string> noComputador;
    };

    struct Lido {
        std::vector<Pagina> paginas;
        /// Uma frase por problema, para o log: o arquivo nunca derruba o jogo.
        std::vector<std::string> problemas;
    };

    /// Le o texto de como-jogar.json. Pagina sem titulo ou sem linhas e descartada, com o motivo em
    /// problemas.
    Lido LerManual(const std::string& textoJson);

    /// As linhas da pagina nesta maquina. No gabinete, so as de sempre: o painel nao tem teclado, e
    /// nenhum texto anuncia tecla que ele nao tem.
    std::vector<std::string> Linhas(const Pagina& p, bool arcade);

    /// Quantas linhas uma pagina mostra no maximo: e quantas a tela reserva.
    constexpr int kLinhasNoMaximo = 8;

    /// A pagina 'passo' depois da 'atual', parando nas pontas: o manual e uma sequencia, e dar a
    /// volta da ultima para a primeira desorienta.
    int Virar(int atual, int passo, int quantas);

    /// "3 de 6": onde o leitor esta.
    std::string Indicador(int atual, int quantas);
}
