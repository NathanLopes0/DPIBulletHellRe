//
// O progresso academico do jogador: o recorde por materia e o ponto de retomada.
//

#include "Progresso.h"

#include <algorithm>

const Progresso::Entrada& Progresso::Ver(const int materia) const {

    // Um registro vazio compartilhado, para a consulta nao inserir nada.
    static const Entrada kVazio{};

    const auto it = mPorMateria.find(materia);
    return it == mPorMateria.end() ? kVazio : it->second;
}

float Progresso::Entrada::MaiorRecorde() const {
    float maior = 0.0f;
    for (const Marca& m : recordes) maior = std::max(maior, m.nota);
    return maior;
}

void Progresso::Registrar(const int materia, const float nota, const Modo modo,
                          const Dificuldade::Nivel nivel, const std::string& parceiro,
                          const std::string& quando) {

    Entrada& r = mPorMateria[materia];
    r.materia = materia;

    // A retomada, a data e a contagem sao da materia, e nao do modo nem do nivel.
    r.retomada = nota;
    ++r.tentativas;
    if (nota < kNotaDeAprovacao) ++r.reprovacoes;

    if (!quando.empty()) r.quando = quando;

    // O recorde so sobe.
    Marca& recorde = r.De(modo, nivel);
    if (nota <= recorde.nota) return;
    recorde.nota = nota;

    // Na dupla, o parceiro e a data acompanham a nota.
    if (modo == Modo::Dupla) {
        recorde.parceiro = parceiro;
        recorde.quando = quando;
    }
}

void Progresso::RegistrarNota(const int materia, const float nota, const Dificuldade::Nivel nivel,
                              const std::string& quando) {
    Registrar(materia, nota, Modo::Solo, nivel, "", quando);
}

void Progresso::RegistrarNotaDeDupla(const int materia, const float nota, const Dificuldade::Nivel nivel,
                                     const std::string& parceiro, const std::string& quando) {
    Registrar(materia, nota, Modo::Dupla, nivel, parceiro, quando);
}

float Progresso::MelhorNota(const int materia) const {
    // A maior de todas.
    return Ver(materia).MaiorRecorde();
}

const Progresso::Marca& Progresso::Recorde(const int materia, const Modo modo,
                                           const Dificuldade::Nivel nivel) const {
    return Ver(materia).De(modo, nivel);
}

int Progresso::Tentativas(const int materia) const {
    return Ver(materia).tentativas;
}

int Progresso::Reprovacoes(const int materia) const {
    return Ver(materia).reprovacoes;
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
    // O mapa ja e por materia, entao a ordem sai pronta.
    std::vector<Entrada> saida;
    saida.reserve(mPorMateria.size());
    for (const auto& par : mPorMateria) saida.push_back(par.second);
    return saida;
}

void Progresso::Restaurar(const std::vector<Entrada>& entradas) {

    mPorMateria.clear();
    for (const auto& e : entradas) mPorMateria[e.materia] = e;
}
