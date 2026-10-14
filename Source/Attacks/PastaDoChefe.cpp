//
// Onde moram os arquivos de ataque de um chefe.
//

#include "PastaDoChefe.h"

namespace PastaDoChefe {

std::string Arquivo(const std::string& chefe, const std::string& arquivo) {

    // Chefe ou arquivo em branco nao viram caminho. Materia sem batalha tem o
    // campo "chefe" vazio, e "Attacks//fases.json" poderia abrir alguma coisa
    // por acidente em vez de simplesmente nao existir.
    if (chefe.empty() || arquivo.empty()) return {};

    return "Attacks/" + chefe + "/" + arquivo;
}

} // namespace PastaDoChefe
