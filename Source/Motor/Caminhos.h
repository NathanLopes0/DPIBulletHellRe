//
// Montagem e busca de caminhos de arquivo.
//

#pragma once

#include <functional>
#include <string>

/// Nao toca em disco, nao conhece SDL.
namespace Caminhos {

    /// Junta uma base e um caminho relativo com uma barra entre eles. Normaliza '\' para '/'.
    std::string Juntar(const std::string& base, const std::string& relativo);

    /// O diretorio um nivel acima. Devolve vazio quando nao ha acima.
    std::string DiretorioPai(const std::string& caminho);

    /// Responde se um diretorio candidato e a base procurada. Injetada de fora
    /// para que a busca possa ser testada sem disco nenhum.
    using Predicado = std::function<bool(const std::string&)>;

    /// Sobe a partir de 'partida' ate achar um diretorio que sirva. Pura.
    std::string ProcurarSubindo(const std::string& partida, int maxNiveis,
                               const Predicado& serve);

}
