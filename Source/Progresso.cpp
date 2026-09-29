//
// O progresso academico do jogador: o recorde por materia e o ponto de retomada.
//

#include "Progresso.h"

#include <algorithm>

void Progresso::RegistrarNota(const int materia, const float nota) {

    // O ponto de retomada e sempre a nota desta batalha.
    mRetomadas[materia] = nota;

    // O recorde so sobe. Sem isto, uma tentativa ruim apagaria o recorde - e,
    // porque as regras de desbloqueio leem o recorde, poderia re-trancar uma
    // materia que o jogador ja tinha passado.
    const auto it = mRecordes.find(materia);
    if (it == mRecordes.end()) {
        mRecordes.emplace(materia, nota);
    }
    else {
        it->second = std::max(it->second, nota);
    }
}

float Progresso::MelhorNota(const int materia) const {
    const auto it = mRecordes.find(materia);
    return it == mRecordes.end() ? 0.0f : it->second;
}

float Progresso::NotaDeRetomada(const int materia) const {
    const auto it = mRetomadas.find(materia);
    if (it == mRetomadas.end()) return kNotaInicial;
    return std::max(it->second, kNotaInicial);
}

bool Progresso::Aprovado(const int materia) const {
    return MelhorNota(materia) >= kNotaDeAprovacao;
}

int Progresso::QuantasAprovadas(const std::vector<int>& materias) const {
    int total = 0;
    for (const int m : materias) {
        if (Aprovado(m)) ++total;
    }
    return total;
}
