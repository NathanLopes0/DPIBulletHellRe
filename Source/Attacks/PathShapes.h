//
// Formas de caminho prontas para o PathBehavior.
//

#pragma once

#include <vector>
#include <memory>
#include <map>
#include <string>
#include "../Motor/Math.h"

/// Geradores de caminhos para o PathBehavior. Espaço de caminho.
namespace PathShapes {

    /// Forma compartilhada entre projeteis (Flyweight).
    using Path = std::shared_ptr<const std::vector<Vector2>>;

    /// Arco: avança enquanto faz uma barriga para um dos lados e volta ao eixo no fim. Lê como um
    /// tiro que "contorna" alguma coisa.
    Path Reta(float distance = 1200.f);

    Path Arc(float forward, float lateral, int segments = 10);

    /// Laço: o projétil dá uma volta completa e retoma o rumo original.
    Path Loop(float radius, float exitDistance = 400.f, int segments = 12);

    /// Ziguezague: avança alternando para os dois lados.
    Path Zigzag(float step, float amplitude, int legs = 5);

    /// Quantas folhas tem uma arvore binaria com este numero de divisoes.
    int FolhasDeArvore(int divisoes);

    /// O caminho de uma folha de uma arvore binaria, em espaco de caminho.
    std::vector<Vector2> ArvoreBinaria(int divisoes, int folha,
                                       float tronco = 120.f,
                                       float passo = 90.f,
                                       float espacamento = 60.f,
                                       float saida = 560.f);

    /// Resultado da leitura de um arquivo de formas.
    struct FormasLidas {
        std::map<std::string, std::vector<Vector2>> formas;

        /// Familias: um nome que vale por varias formas, uma por projetil. Existe porque uma arvore
        /// de 3 divisoes sao 8 caminhos que so fazem sentido juntos.
        std::map<std::string, std::vector<std::vector<Vector2>>> familias;

        std::vector<std::string> problemas;
    };

    /// Le formas a partir do texto de um arquivo JSON. Funcao pura: nao abre arquivo, nao escreve
    /// log.
    FormasLidas LerFormas(const std::string& textoJson);

    /// Devolve, pelo nome, uma forma definida em Assets/Paths/formas.json.
    Path DoArquivo(const std::string& nome);

    /// A forma numero 'indice' de uma familia declarada em formas.json.
    Path DaFamilia(const std::string& nome, int indice);
}
