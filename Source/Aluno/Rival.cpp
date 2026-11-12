//
// O rival da turma.
//

#include "Rival.h"

#include <algorithm>
#include <cstdio>

namespace Rival {

namespace {

    std::string Numero(const float v) {
        char b[32];
        std::snprintf(b, sizeof(b), "%.2f", static_cast<double>(v));
        return b;
    }

    // \xc2\xba e o o do ordinal.
    std::string Lugar(const int p) { return std::to_string(p) + "\xc2\xba"; }

}

int PosicaoCom(const std::vector<Colega>& colegas, const float nota) {
    int acima = 0;
    for (const Colega& c : colegas) if (c.nota > nota) ++acima;
    return acima + 1;
}

Situacao Da(const std::vector<Colega>& colegas, const float nota) {
    Situacao s;
    std::vector<Colega> jogaram;
    for (const Colega& c : colegas) if (c.nota > 0.0f) jogaram.push_back(c);
    if (jogaram.empty()) return s;

    if (nota <= 0.0f) {
        // O ultimo da turma; no empate, o que vem primeiro na lista.
        const auto ultimo = std::min_element(jogaram.begin(), jogaram.end(),
                                             [](const Colega& a, const Colega& b) { return a.nota < b.nota; });
        s.tipo = Tipo::Estreante;
        s.alvo = *ultimo;
        s.posicaoDoAlvo = PosicaoCom(jogaram, ultimo->nota);
        return s;
    }

    const Colega* acima = nullptr;
    const Colega* abaixo = nullptr;
    for (const Colega& c : jogaram) {
        if (c.nota > nota) {
            if (!acima || c.nota < acima->nota) acima = &c;
        } else if (!abaixo || c.nota > abaixo->nota) {
            abaixo = &c;
        }
    }
    if (acima) {
        s.tipo = Tipo::Atras;
        s.alvo = *acima;
        s.posicaoDoAlvo = PosicaoCom(jogaram, acima->nota);
        s.distancia = acima->nota - nota;
    } else {
        s.tipo = Tipo::Lider;
        s.alvo = *abaixo;
        s.posicaoDoAlvo = 2;
        s.distancia = nota - abaixo->nota;
    }
    return s;
}

std::vector<Colega> Passados(const std::vector<Colega>& colegas, const float antes, const float depois) {
    std::vector<Colega> out;
    for (const Colega& c : colegas) {
        if (c.nota > antes && c.nota < depois) out.push_back(c);
    }
    std::stable_sort(out.begin(), out.end(), [](const Colega& a, const Colega& b) { return a.nota > b.nota; });
    return out;
}

std::string Frase(const Situacao& s) {
    // \xc2\xb7 e o ponto do meio, \xc3\xa9 o e agudo, \xc3\xba o u agudo.
    switch (s.tipo) {
        case Tipo::Sozinho:
            return "Ningu\xc3\xa9m mais jogou: o ranking \xc3\xa9 seu";
        case Tipo::Estreante:
            return "\xc3\x9altimo da turma: " + s.alvo.quem + " com " + Numero(s.alvo.nota);
        case Tipo::Atras:
            return "Rival: " + s.alvo.quem + " (" + Lugar(s.posicaoDoAlvo) + ") \xc2\xb7 faltam " + Numero(s.distancia);
        case Tipo::Lider:
            if (s.distancia <= 0.0f) return "1\xc2\xba da turma, empatado com " + s.alvo.quem;
            return "1\xc2\xba da turma \xc2\xb7 " + s.alvo.quem + " vem " + Numero(s.distancia) + " atr\xc3\xa1s";
    }
    return {};
}

std::string FraseDoResultado(const std::vector<Colega>& colegas, const float antes, const float depois,
                             const bool entraNoRanking) {
    const std::vector<Colega> passados = Passados(colegas, antes, depois);
    if (!passados.empty()) {
        std::string quem = passados.front().quem;
        if (passados.size() == 2) quem += " e " + passados[1].quem;
        else if (passados.size() > 2) quem += " e mais " + std::to_string(passados.size() - 1);
        // \xc3\xa9 o e agudo.
        const std::string lugar = Lugar(PosicaoCom(colegas, depois));
        if (!entraNoRanking) return "Com essa nota voc\xc3\xaa passaria " + quem + " e seria o " + lugar;
        return "Voc\xc3\xaa passou " + quem + ": agora \xc3\xa9 o " + lugar + " da turma";
    }
    // Sem passar ninguem: so fala do rival se a nota desta luta chegou perto dele.
    const Situacao s = Da(colegas, depois);
    if (s.tipo == Tipo::Atras && s.distancia <= kPerto) {
        return "Faltaram " + Numero(s.distancia) + " para passar " + s.alvo.quem;
    }
    return {};
}

Turma DaTurma(const std::vector<Exportacao::FichaDeAluno>& fichas, const int materia,
              const Dificuldade::Nivel nivel, const std::vector<std::string>& alunos, const Progresso& progresso) {
    Turma t;
    if (alunos.empty()) return t;
    if (alunos.size() == 1) {
        t.colegas = DoRanking(Ranking::DaMateria(fichas, materia, nivel), alunos[0]);
        t.nota = progresso.Recorde(materia, Progresso::Modo::Solo, nivel).nota;
        return t;
    }
    const std::vector<Ranking::LinhaDeDupla> linhas = Ranking::DuplasDaMateria(fichas, materia, nivel);
    t.colegas = DoRanking(linhas, alunos[0], alunos[1]);
    for (const auto& l : linhas) {
        const bool nos = (l.um == alunos[0] && l.dois == alunos[1]) || (l.um == alunos[1] && l.dois == alunos[0]);
        if (nos) t.nota = std::max(t.nota, l.nota);
    }
    // A dupla de visitantes nao esta no disco: vale o recorde da sessao, se foi com este parceiro.
    const Progresso::Marca& m = progresso.Recorde(materia, Progresso::Modo::Dupla, nivel);
    if (m.parceiro == alunos[1]) t.nota = std::max(t.nota, m.nota);
    return t;
}

std::vector<Colega> DoRanking(const std::vector<Ranking::Linha>& linhas, const std::string& eu) {
    std::vector<Colega> out;
    for (const auto& l : linhas) if (l.matricula != eu) out.push_back({l.matricula, l.nota});
    return out;
}

std::vector<Colega> DoRanking(const std::vector<Ranking::LinhaDeDupla>& linhas, const std::string& um,
                              const std::string& dois) {
    std::vector<Colega> out;
    for (const auto& l : linhas) {
        const bool nos = (l.um == um && l.dois == dois) || (l.um == dois && l.dois == um);
        if (!nos) out.push_back({l.um + " + " + l.dois, l.nota});
    }
    return out;
}

}
