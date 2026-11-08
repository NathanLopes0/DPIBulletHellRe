//
// O manual do jogo: as paginas da tela "Como Jogar".
//

#pragma once

#include <string>
#include <vector>

/**
 * CAMADA PURA. Coberto por test_manual.cpp; o arquivo de verdade, por
 * test_arquivos_de_dados.cpp.
 *
 * O TEXTO MORA EM Assets/como-jogar.json, e nao no C++. Sao dezenas de frases
 * com acento, e no C++ cada acento vira um \xc3\xa9 (o MSVC nao le acento em
 * literal): um texto que ninguem consegue revisar lendo. No JSON ele e texto
 * de verdade, e quem ajusta uma frase nao precisa recompilar.
 *
 * Quem desenha e a TelaComoJogar; aqui ficam a leitura do arquivo, as linhas
 * que valem em cada maquina e a navegacao entre as paginas.
 */
namespace Manual {

    struct Pagina {
        std::string titulo;
        std::vector<std::string> linhas;
        /// Linhas que so aparecem FORA do gabinete: as que falam de teclas.
        std::vector<std::string> noComputador;
    };

    struct Lido {
        std::vector<Pagina> paginas;
        /// Uma frase por problema, para o log: o arquivo nunca derruba o jogo.
        std::vector<std::string> problemas;
    };

    /// @brief Le o texto de como-jogar.json. Pagina sem titulo ou sem linhas e
    /// descartada, com o motivo em problemas.
    Lido LerManual(const std::string& textoJson);

    /**
     * @brief As linhas da pagina nesta maquina.
     *
     * No gabinete, so as de sempre: o painel nao tem teclado, e nenhum texto
     * anuncia tecla que ele nao tem (Documentacao/gabinete.md). No computador,
     * as de sempre e depois as de teclado.
     */
    std::vector<std::string> Linhas(const Pagina& p, bool arcade);

    /// Quantas linhas uma pagina mostra no maximo: e quantas a tela reserva.
    constexpr int kLinhasNoMaximo = 8;

    /// @brief A pagina 'passo' depois da 'atual', PARANDO nas pontas: o manual
    /// e uma sequencia, e dar a volta da ultima para a primeira desorienta.
    int Virar(int atual, int passo, int quantas);

    /// @brief "3 de 6": onde o leitor esta. "de", e nao a barra: a Zelda.ttf
    /// nao tem o glifo "/", e desenharia um quadrado.
    std::string Indicador(int atual, int quantas);
}
