//
// O progresso academico do jogador: o recorde por materia e o ponto de retomada.
//

#include "Progresso.h"

#include <algorithm>

const Progresso::Registro& Progresso::Ver(const int materia) const {

    // Um registro vazio compartilhado, para a consulta nao inserir nada. O
    // getter antigo usava operator[] e fazia o mapa crescer com materias nunca
    // jogadas - mutacao dentro de uma consulta.
    static const Registro kVazio{};

    const auto it = mPorMateria.find(materia);
    return it == mPorMateria.end() ? kVazio : it->second;
}

void Progresso::RegistrarNota(const int materia, const float nota, const std::string& quando) {

    Registro& r = mPorMateria[materia];

    // O ponto de retomada e sempre a nota desta batalha.
    r.retomada = nota;

    // A data tambem: ela marca a ULTIMA vez que esta materia foi jogada. Vazio nao
    // apaga o que havia - um save antigo sem data que recebe uma nota sem data
    // continua sem data, mas quem ja tinha data nao a perde.
    if (!quando.empty()) r.quando = quando;

    // O recorde so sobe. Sem isto, uma tentativa ruim apagaria o recorde - e,
    // porque as regras de desbloqueio leem o recorde, poderia re-trancar uma
    // materia que o jogador ja tinha passado.
    r.recorde = std::max(r.recorde, nota);
}

void Progresso::RegistrarNotaDeDupla(const int materia, const float nota,
                                     const std::string& parceiro, const std::string& quando) {

    Registro& r = mPorMateria[materia];

    // A RETOMADA E A DATA SAO DA MATERIA, e nao do modo: "quando este aluno
    // jogou esta materia pela ultima vez" e a mesma pergunta tenha ele jogado
    // sozinho ou acompanhado, e o professor quer a resposta das duas.
    r.retomada = nota;
    if (!quando.empty()) r.quando = quando;

    // O RECORDE DE DUPLA SO SOBE, como o de solo. E o parceiro acompanha a
    // NOTA: se o aluno fizer melhor com outra pessoa, o nome muda junto. Um
    // parceiro que ficasse para tras poria no ranking uma dupla que nunca fez
    // aquele numero.
    if (nota <= r.recordeDeDupla) return;

    r.recordeDeDupla = nota;
    r.parceiroDoRecorde = parceiro;
    r.quandoDaDupla = quando;
}

float Progresso::MelhorNota(const int materia) const {
    const Registro& r = Ver(materia);

    // A MAIOR DAS DUAS. Passar e passar, tenha sido com quem for
    // (requisitos-coop, D10); quem precisa separa-las e o ranking.
    return std::max(r.recorde, r.recordeDeDupla);
}

float Progresso::MelhorNotaSolo(const int materia) const {
    return Ver(materia).recorde;
}

float Progresso::MelhorNotaDeDupla(const int materia) const {
    return Ver(materia).recordeDeDupla;
}

const std::string& Progresso::ParceiroDoRecorde(const int materia) const {
    return Ver(materia).parceiroDoRecorde;
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

    // UM MAPA SO, entao basta percorre-lo. Eram tres mapas paralelos e esta
    // funcao precisava varrer a UNIAO das chaves para nao perder uma materia
    // que estivesse num e nao no outro - uma precaucao que so existia porque a
    // estrutura permitia o descompasso.
    std::vector<Entrada> saida;
    saida.reserve(mPorMateria.size());

    for (const auto& [materia, r] : mPorMateria) {
        Entrada e;
        e.materia = materia;
        e.recorde = r.recorde;
        e.retomada = r.retomada;
        e.quando = r.quando;
        e.recordeDeDupla = r.recordeDeDupla;
        e.parceiroDoRecorde = r.parceiroDoRecorde;
        e.quandoDaDupla = r.quandoDaDupla;
        saida.push_back(std::move(e));
    }
    return saida;
}

void Progresso::Restaurar(const std::vector<Entrada>& entradas) {

    // SUBSTITUI, nao acumula: carregar a ficha do aluno B depois da do aluno A
    // nao pode deixar as notas de A penduradas.
    mPorMateria.clear();

    for (const auto& e : entradas) {
        Registro& r = mPorMateria[e.materia];
        r.recorde = e.recorde;
        r.retomada = e.retomada;
        r.quando = e.quando;
        r.recordeDeDupla = e.recordeDeDupla;
        r.parceiroDoRecorde = e.parceiroDoRecorde;
        r.quandoDaDupla = e.quandoDaDupla;
    }
}
