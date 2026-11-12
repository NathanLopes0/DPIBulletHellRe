//
// A ponte entre o registro das tentativas e o disco.
//

#pragma once

#include "Tentativas.h"

namespace TentativasArquivo {

    /// Acrescenta a tentativa a <base>/Saves/tentativas.csv e tentativas_fases.csv, e refaz
    /// tentativas_resumo.csv.
    bool Gravar(const Tentativas::Tentativa& tentativa);

/// Acrescenta um toca aqui da dupla aprovada a tentativas_toques.csv (Tentativas::LinhaDoToque).
bool GravarToque(const Tentativas::Tentativa& tentativa, int toque, float segundos);
}
