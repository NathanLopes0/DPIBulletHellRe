//
// O progresso academico do jogador: o recorde por materia e o ponto de retomada.
//

#include "Progresso.h"

#include <algorithm>

const Progresso::Entrada& Progresso::Ver(const int materia) const {

    // Um registro vazio compartilhado, para a consulta nao inserir nada. O
    // getter antigo usava operator[] e fazia o mapa crescer com materias nunca
    // jogadas - mutacao dentro de uma consulta.
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

    // A RETOMADA E A DATA SAO DA MATERIA, e nao do modo nem do nivel: "quando
    // este aluno jogou esta materia pela ultima vez" e a mesma pergunta tenha
    // ele jogado como for, e o professor quer a resposta de todas.
    r.retomada = nota;

    // Vazio nao apaga o que havia - um save antigo sem data que recebe uma nota
    // sem data continua sem data, mas quem ja tinha data nao a perde.
    if (!quando.empty()) r.quando = quando;

    // O RECORDE SO SOBE. Sem isto, uma tentativa ruim apagaria o recorde - e,
    // porque as regras de desbloqueio leem o recorde, poderia re-trancar uma
    // materia que o jogador ja tinha passado.
    Marca& recorde = r.De(modo, nivel);
    if (nota <= recorde.nota) return;
    recorde.nota = nota;

    // NA DUPLA, o parceiro e a data acompanham a NOTA: se o aluno fizer melhor
    // com outra pessoa, o nome muda junto. Um parceiro que ficasse para tras
    // poria no ranking uma dupla que nunca fez aquele numero.
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
    // A MAIOR DE TODAS. Passar e passar, tenha sido com quem for e no nivel
    // que for (requisitos-coop, D10); quem precisa separa-las e o ranking.
    return Ver(materia).MaiorRecorde();
}

const Progresso::Marca& Progresso::Recorde(const int materia, const Modo modo,
                                           const Dificuldade::Nivel nivel) const {
    return Ver(materia).De(modo, nivel);
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
    // O MAPA JA E POR MATERIA, entao a ordem sai pronta.
    std::vector<Entrada> saida;
    saida.reserve(mPorMateria.size());
    for (const auto& par : mPorMateria) saida.push_back(par.second);
    return saida;
}

void Progresso::Restaurar(const std::vector<Entrada>& entradas) {

    // SUBSTITUI, nao acumula: carregar a ficha do aluno B depois da do aluno A
    // nao pode deixar as notas de A penduradas.
    mPorMateria.clear();
    for (const auto& e : entradas) mPorMateria[e.materia] = e;
}
