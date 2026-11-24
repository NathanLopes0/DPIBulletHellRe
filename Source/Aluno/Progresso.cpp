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
                          const std::string& quando, const float extra) {

    Entrada& r = mPorMateria[materia];
    r.materia = materia;
    const bool jaAprovada = r.MaiorRecorde() >= kNotaDeAprovacao;

    // A retomada, a data e a contagem sao da materia, e nao do modo nem do nivel.
    r.retomada = nota;
    ++r.tentativas;
    if (nota < kNotaDeAprovacao) ++r.reprovacoes;
    r.seguidas = nota < kNotaDeAprovacao ? r.seguidas + 1 : 0;

    if (!quando.empty()) r.quando = quando;

    // A primeira aprovacao fica datada, e nao muda mais: o recorde so sobe.
    if (!jaAprovada && nota >= kNotaDeAprovacao && r.aprovadaEm.empty()) r.aprovadaEm = quando;

    // O recorde so sobe. Entre dois 100, sobe a nota extra (Nota::Conta::Extra).
    Marca& recorde = r.De(modo, nivel);
    const float extraDaNota = Nota::ECheia(nota) ? std::max(0.0f, extra) : 0.0f;
    const bool melhor = nota > recorde.nota || (Nota::ECheia(nota) && nota >= recorde.nota && extraDaNota > recorde.extra);
    if (!melhor) return;
    recorde.nota = nota;
    recorde.extra = extraDaNota;

    // Na dupla, o parceiro e a data acompanham a nota.
    if (modo == Modo::Dupla) {
        recorde.parceiro = parceiro;
        recorde.quando = quando;
    }
}

void Progresso::RegistrarNota(const int materia, const float nota, const Dificuldade::Nivel nivel,
                              const std::string& quando, const float extra) {
    Registrar(materia, nota, Modo::Solo, nivel, "", quando, extra);
}

void Progresso::RegistrarNotaDeDupla(const int materia, const float nota, const Dificuldade::Nivel nivel,
                                     const std::string& parceiro, const std::string& quando,
                                     const float extra) {
    Registrar(materia, nota, Modo::Dupla, nivel, parceiro, quando, extra);
}

float Progresso::MelhorNota(const int materia) const {
    // A maior de todas.
    return Ver(materia).MaiorRecorde();
}

float Progresso::MelhorNotaNoNivel(const int materia, const Dificuldade::Nivel nivel) const {
    const Entrada& e = Ver(materia);
    return std::max(e.De(Modo::Solo, nivel).nota, e.De(Modo::Dupla, nivel).nota);
}

bool Progresso::CemNoDificil(const int materia) const {
    return Nota::ECheia(MelhorNotaNoNivel(materia, Dificuldade::Nivel::Dificil));
}

const Progresso::Marca& Progresso::Recorde(const int materia, const Modo modo,
                                           const Dificuldade::Nivel nivel) const {
    return Ver(materia).De(modo, nivel);
}

void Progresso::RegistrarOrientacao(const int materia) {
    const auto it = mPorMateria.find(materia);
    if (it == mPorMateria.end() || it->second.MaiorRecorde() < kNotaDeAprovacao) return;
    ++it->second.orientacoes;
}

int Progresso::Orientacoes(const int materia) const {
    return Ver(materia).orientacoes;
}

int Progresso::Tentativas(const int materia) const {
    return Ver(materia).tentativas;
}

int Progresso::Reprovacoes(const int materia) const {
    return Ver(materia).reprovacoes;
}

int Progresso::ReprovacoesSeguidas(const int materia) const {
    return Ver(materia).seguidas;
}

bool Progresso::Aprovado(const int materia) const {
    return MelhorNota(materia) >= kNotaDeAprovacao;
}

const std::string& Progresso::AprovadaEm(const int materia) const {
    return Ver(materia).aprovadaEm;
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
