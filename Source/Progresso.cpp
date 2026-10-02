//
// O progresso academico do jogador: o recorde por materia e o ponto de retomada.
//

#include "Progresso.h"

#include <algorithm>

void Progresso::RegistrarNota(const int materia, const float nota, const std::string& quando) {

    // O ponto de retomada e sempre a nota desta batalha.
    mRetomadas[materia] = nota;

    // A data tambem: ela marca a ULTIMA vez que esta materia foi jogada. Vazio nao
    // apaga o que havia - um save antigo sem data que recebe uma nota sem data
    // continua sem data, mas quem ja tinha data nao a perde.
    if (!quando.empty()) mQuando[materia] = quando;

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

std::vector<Progresso::Entrada> Progresso::Entradas() const {

    // Percorre a UNIAO das duas chaves. Hoje RegistrarNota sempre escreve nos dois
    // mapas, entao eles tem o mesmo conjunto; mas Restaurar pode receber um
    // arquivo editado a mao, e perder uma materia por causa disso seria um bug
    // silencioso difícil de achar.
    std::map<int, Entrada> porMateria;

    for (const auto& r : mRecordes) {
        porMateria[r.first].materia = r.first;
        porMateria[r.first].recorde = r.second;
    }
    for (const auto& r : mRetomadas) {
        porMateria[r.first].materia = r.first;
        porMateria[r.first].retomada = r.second;
    }
    for (const auto& r : mQuando) {
        porMateria[r.first].materia = r.first;
        porMateria[r.first].quando = r.second;
    }

    std::vector<Entrada> saida;
    saida.reserve(porMateria.size());
    for (const auto& e : porMateria) saida.push_back(e.second);
    return saida;
}

void Progresso::Restaurar(const std::vector<Entrada>& entradas) {

    // SUBSTITUI, nao acumula: carregar a ficha do aluno B depois da do aluno A
    // nao pode deixar as notas de A penduradas.
    mRecordes.clear();
    mRetomadas.clear();
    mQuando.clear();

    for (const auto& e : entradas) {
        mRecordes[e.materia] = e.recorde;
        mRetomadas[e.materia] = e.retomada;
        if (!e.quando.empty()) mQuando[e.materia] = e.quando;
    }
}
