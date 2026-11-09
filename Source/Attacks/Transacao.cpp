//
// As transacoes da fase 3 do Thiago.
//

#include "Transacao.h"

#include <algorithm>
#include <random>
#include <set>

namespace Transacao {

namespace {
    int Ate(std::mt19937& g, const int n) {
        return static_cast<int>(g() % static_cast<std::uint32_t>(std::max(1, n)));
    }

    bool PorLinha(const Tipo t) {
        return t == Tipo::SelectLinha || t == Tipo::Update || t == Tipo::Delete;
    }

    int Contar(const std::vector<Operacao>& ops, const Tipo t) {
        return static_cast<int>(std::count_if(ops.begin(), ops.end(), [t](const Operacao& o) { return o.tipo == t; }));
    }
}

Plano Sortear(Variacao::Sorteio& s) {
    Plano p;
    p.chanceDeSelect = s.Real(0.25f, 0.45f);
    return p;
}

int LinhaDe(const Banco::Registros& regs, const int mat) {
    for (size_t l = 0; l < regs.size(); ++l) if (regs[l].mat == mat) return static_cast<int>(l);
    return -1;
}

bool Proxima(const Banco::Registros& regs, const std::vector<Operacao>& pendentes, const std::uint32_t sorteio,
             Operacao& saida, const Plano& plano) {
    std::mt19937 g(sorteio);

    // As linhas travadas, e as que sobram.
    std::set<int> travadas;
    std::set<int> colunas;
    for (const Operacao& o : pendentes) {
        if (PorLinha(o.tipo)) travadas.insert(o.mat);
        if (o.tipo == Tipo::SelectColuna) colunas.insert(o.coluna);
    }
    std::vector<int> livres;
    for (const Banco::Registro& r : regs) if (!travadas.count(r.mat)) livres.push_back(r.mat);

    const int linhas = static_cast<int>(regs.size());
    const int deletes = Contar(pendentes, Tipo::Delete);
    const int inserts = Contar(pendentes, Tipo::Insert);

    std::vector<Tipo> selects;
    if (!livres.empty() && Contar(pendentes, Tipo::SelectLinha) < kVarredurasDeLinha) selects.push_back(Tipo::SelectLinha);
    if (Contar(pendentes, Tipo::SelectColuna) < kVarredurasDeColuna
        && static_cast<int>(colunas.size()) < Banco::kColunas) selects.push_back(Tipo::SelectColuna);

    // As escritas, com o peso de cada uma.
    std::vector<std::pair<Tipo, int>> escritas;
    if (!livres.empty()) escritas.emplace_back(Tipo::Update, 45);
    if (!livres.empty() && linhas - deletes - 1 >= kMenosLinhas) escritas.emplace_back(Tipo::Delete, 35);
    if (inserts == 0 && linhas + inserts < Banco::kLinhas) escritas.emplace_back(Tipo::Insert, 30);

    if (selects.empty() && escritas.empty()) return false;

    const bool select = !selects.empty()
        && (escritas.empty() || static_cast<float>(Ate(g, 1000)) / 1000.0f < plano.chanceDeSelect);

    Operacao op;
    if (select) {
        op.tipo = selects[static_cast<size_t>(Ate(g, static_cast<int>(selects.size())))];
    } else {
        int total = 0;
        for (const auto& e : escritas) total += e.second;
        int sorte = Ate(g, total);
        op.tipo = escritas.back().first;
        for (const auto& e : escritas) {
            if (sorte < e.second) { op.tipo = e.first; break; }
            sorte -= e.second;
        }
    }

    switch (op.tipo) {
        case Tipo::SelectLinha:
        case Tipo::Delete:
            op.mat = livres[static_cast<size_t>(Ate(g, static_cast<int>(livres.size())))];
            break;
        case Tipo::Update: {
            op.mat = livres[static_cast<size_t>(Ate(g, static_cast<int>(livres.size())))];
            const int antiga = regs[static_cast<size_t>(LinhaDe(regs, op.mat))].nota;
            op.nota = (antiga + 1 + Ate(g, 99)) % 100;
            break;
        }
        case Tipo::SelectColuna: {
            std::vector<int> abertas;
            for (int c = 0; c < Banco::kColunas; ++c) if (!colunas.count(c)) abertas.push_back(c);
            op.coluna = abertas[static_cast<size_t>(Ate(g, static_cast<int>(abertas.size())))];
            break;
        }
        case Tipo::Insert:
            op.novo = Banco::NovoRegistro(regs, g());
            break;
    }
    saida = op;
    return true;
}

std::vector<Banco::Casa> Casas(const Banco::Registros& regs, const Operacao& op, const int linhaDoInsert) {
    std::vector<Banco::Casa> casas;
    const int nota = static_cast<int>(Banco::Coluna::Nota);
    switch (op.tipo) {
        case Tipo::SelectLinha:
        case Tipo::Delete: {
            const int l = LinhaDe(regs, op.mat);
            if (l >= 0) for (int c = 0; c < Banco::kColunas; ++c) casas.push_back({l, c});
            break;
        }
        case Tipo::SelectColuna:
            for (int l = 0; l < Banco::kLinhas; ++l) casas.push_back({l, op.coluna});
            break;
        case Tipo::Update: {
            const int l = LinhaDe(regs, op.mat);
            if (l >= 0) casas.push_back({l, nota});
            break;
        }
        case Tipo::Insert:
            if (linhaDoInsert >= 0 && linhaDoInsert < Banco::kLinhas) {
                for (int c = 0; c < Banco::kColunas; ++c) casas.push_back({linhaDoInsert, c});
            }
            break;
    }
    return casas;
}

bool SobraLugar(const Banco::Registros& regs, const std::vector<Operacao>& ops) {
    std::set<int> linhas, colunas;
    for (const Operacao& o : ops) {
        if (o.tipo == Tipo::SelectLinha) {
            const int l = LinhaDe(regs, o.mat);
            if (l >= 0) linhas.insert(l);
        }
        if (o.tipo == Tipo::SelectColuna) colunas.insert(o.coluna);
    }
    for (int l = 0; l < Banco::kLinhas; ++l) {
        for (int c = 0; c < Banco::kColunas; ++c) {
            if (!linhas.count(l) && !colunas.count(c)) return true;
        }
    }
    return false;
}

namespace {
    const char* Cor(const int cor) { return cor == 0 ? "Ciano" : "Magenta"; }
}

std::string CaraDoSelo(const int cor, const int id, const bool pressa) {
    return std::string(Cor(cor)) + (pressa ? "Pressa" : "") + std::to_string(std::clamp(id, 1, kIds));
}

std::string CaraDoRascunho(const int cor, const bool riscado) {
    return std::string(Cor(cor)) + (riscado ? "Riscado" : "");
}

int PosicaoDoSelo(const std::vector<int>& ocupadas, const std::uint32_t sorteio) {
    std::vector<int> livres;
    for (int i = 0; i < static_cast<int>(kPosicoesDoSelo.size()); ++i) {
        if (std::find(ocupadas.begin(), ocupadas.end(), i) == ocupadas.end()) livres.push_back(i);
    }
    if (livres.empty()) return 0;
    return livres[static_cast<size_t>(sorteio % static_cast<std::uint32_t>(livres.size()))];
}

std::string PlacarDoBegin(const int id) { return "BEGIN T" + std::to_string(id); }

std::string PlacarDaOperacao(const int id, const Operacao& op) {
    const std::string t = "T" + std::to_string(id) + ": ";
    const std::string onde = " WHERE mat = " + std::to_string(op.mat);
    switch (op.tipo) {
        case Tipo::SelectLinha: return t + "SELECT *" + onde;
        case Tipo::SelectColuna:
            return t + "SELECT " + Banco::kNomesDasColunas[static_cast<size_t>(std::clamp(op.coluna, 0, Banco::kColunas - 1))];
        case Tipo::Update: return t + "UPDATE SET nota = " + std::to_string(op.nota) + onde;
        case Tipo::Delete: return t + "DELETE" + onde;
        case Tipo::Insert:
            return t + "INSERT INTO alunos: " + Banco::kNomes[static_cast<size_t>(std::clamp(op.novo.nome, 0, 7))];
    }
    return t;
}

std::string PlacarDoCommit(const int id) { return "T" + std::to_string(id) + ": COMMIT"; }
std::string PlacarDoRollback(const int id) { return "T" + std::to_string(id) + ": ROLLBACK"; }

Degrau DegrauDo(const Dificuldade::Nivel n) {
    Degrau d;
    if (n != Dificuldade::Nivel::Dificil) return d;
    d.novaTransacao = 1.9f;
    d.operacoes = 3;
    d.entreOperacoes = 0.45f;
    d.ateOCommit = 1.4f;
    return d;
}

}
