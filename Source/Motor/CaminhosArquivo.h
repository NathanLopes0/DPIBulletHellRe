//
// A ponte entre a montagem de caminhos e o disco.
//

#pragma once

#include <optional>
#include <string>

/// Separado de Caminhos.cpp pelo mesmo motivo de PathShapesArquivo.cpp, RegrasDeAtaqueArquivo.cpp e
/// FasesDeAtaqueArquivo.cpp.
namespace Caminhos {

    /// Descobre a pasta base do jogo. Chame uma vez, no inicio.
    bool Inicializar();

    /// O caminho completo de um arquivo de Assets. Receba o caminho sem "Assets/" e sem "../":
    /// Caminhos::Asset("Teachers/DPIBHSalles.png")
    std::string Asset(const std::string& relativo);

    /// O conteudo inteiro de um arquivo de texto, ou vazio (nullopt) se ele nao abriu.
    std::optional<std::string> LerTexto(const std::string& caminho);

    /// O caminho completo de um arquivo de save, em <base>/Saves. Cria a pasta Saves se ela nao
    /// existir.
    std::string Save(const std::string& relativo);

}
