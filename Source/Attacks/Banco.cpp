//
// A tabela "alunos" do Thiago: o que esta escrito no campo, e as consultas.
//

#include "Banco.h"

#include <algorithm>
#include <numeric>
#include <random>

namespace Banco {

    namespace {
        /// Um numero de 0 a n - 1, direto do mt19937: a sequencia do gerador e
        /// a mesma em todo compilador, e as distribuicoes da biblioteca nao sao.
        int Ate(std::mt19937& g, const int n) {
            return n <= 0 ? 0 : static_cast<int>(g() % static_cast<std::uint32_t>(n));
        }

        /// Um valor de [de, ate] que ainda nao esta em 'usados'.
        int Novo(std::mt19937& g, const int de, const int ate, std::vector<int>& usados) {
            for (;;) {
                const int v = de + Ate(g, ate - de + 1);
                if (std::find(usados.begin(), usados.end(), v) == usados.end()) {
                    usados.push_back(v);
                    return v;
                }
            }
        }

        /// Todas as condicoes que a tabela sabe escrever.
        std::vector<Condicao> Candidatas(const Registros& regs) {
            std::vector<Condicao> c;
            for (int t = 5; t <= 95; t += 5) {
                c.push_back({Coluna::Nota, Op::Menor, t});
                c.push_back({Coluna::Nota, Op::MaiorOuIgual, t});
            }
            for (int k = 0; k < static_cast<int>(kCursos.size()); ++k) c.push_back({Coluna::Curso, Op::Igual, k});
            for (int a = 1; a <= kMaiorAno; ++a) c.push_back({Coluna::Ano, Op::Igual, a});
            for (int a = 2; a <= kMaiorAno; ++a) c.push_back({Coluna::Ano, Op::Menor, a});
            for (int m = 20; m <= 90; m += 10) c.push_back({Coluna::Mat, Op::Menor, m});
            for (const Registro& r : regs) c.push_back({Coluna::Nome, Op::Igual, r.nome});
            return c;
        }

        /// Uma condicao que pega de 'menos' a 'mais' linhas. Escolhe primeiro a
        /// coluna - a nota vale por duas, e a que mais se le -, depois a condicao.
        Condicao Escolher(const Registros& regs, std::mt19937& g, const int menos, const int mais) {
            const std::array<Coluna, 6> colunas = {Coluna::Nota, Coluna::Nota, Coluna::Curso,
                                                   Coluna::Ano, Coluna::Mat, Coluna::Nome};
            const int inicio = Ate(g, static_cast<int>(colunas.size()));
            const auto todas = Candidatas(regs);
            for (size_t k = 0; k < colunas.size(); ++k) {
                const Coluna col = colunas[(static_cast<size_t>(inicio) + k) % colunas.size()];
                std::vector<Condicao> boas;
                for (const Condicao& c : todas) {
                    if (c.coluna != col) continue;
                    const int n = static_cast<int>(Linhas(regs, c).size());
                    if (n >= menos && n <= mais) boas.push_back(c);
                }
                if (!boas.empty()) return boas[static_cast<size_t>(Ate(g, static_cast<int>(boas.size())))];
            }
            return {Coluna::Nota, Op::Menor, 50};
        }
    }

    Registros Sortear(const std::uint32_t sorteio, const int n) {
        std::mt19937 g(sorteio);
        const int quantos = std::clamp(n, 0, static_cast<int>(kNomes.size()));
        std::vector<int> mats, nomes, notas;
        Registros regs;
        for (int i = 0; i < quantos; ++i) {
            Registro r;
            r.mat = Novo(g, 10, 99, mats);
            r.nome = Novo(g, 0, static_cast<int>(kNomes.size()) - 1, nomes);
            r.curso = Ate(g, static_cast<int>(kCursos.size()));
            r.nota = Novo(g, 0, 99, notas);
            r.ano = 1 + Ate(g, kMaiorAno);
            regs.push_back(r);
        }
        return regs;
    }

    int Valor(const Registro& r, const Coluna c) {
        switch (c) {
            case Coluna::Mat:   return r.mat;
            case Coluna::Nome:  return r.nome;
            case Coluna::Curso: return r.curso;
            case Coluna::Nota:  return r.nota;
            case Coluna::Ano:   return r.ano;
        }
        return 0;
    }

    std::string Texto(const Registro& r, const Coluna c) {
        if (c == Coluna::Nome) return kNomes[static_cast<size_t>(std::clamp(r.nome, 0, 7))];
        if (c == Coluna::Curso) return kCursos[static_cast<size_t>(std::clamp(r.curso, 0, 2))];
        return std::to_string(Valor(r, c));
    }

    std::string Cara(const Registro& r, const Coluna c) {
        if (c == Coluna::Nome || c == Coluna::Curso) return Texto(r, c);
        return "V" + std::to_string(std::clamp(Valor(r, c), 0, 99));
    }

    std::string CaraDoCabecalho(const Coluna c) {
        return std::string("C") + kNomesDasColunas[static_cast<size_t>(c)];
    }

    bool Satisfaz(const Registro& r, const Condicao& c) {
        const int v = Valor(r, c.coluna);
        switch (c.op) {
            case Op::Menor:        return v < c.valor;
            case Op::MaiorOuIgual: return v >= c.valor;
            case Op::Igual:        return v == c.valor;
        }
        return false;
    }

    std::vector<int> Linhas(const Registros& regs, const Condicao& c) {
        std::vector<int> linhas;
        for (size_t i = 0; i < regs.size(); ++i) {
            if (Satisfaz(regs[i], c)) linhas.push_back(static_cast<int>(i));
        }
        return linhas;
    }

    std::string TextoDaCondicao(const Condicao& c) {
        const std::string coluna = kNomesDasColunas[static_cast<size_t>(c.coluna)];
        if (c.coluna == Coluna::Nome) return coluna + " = '" + kNomes[static_cast<size_t>(std::clamp(c.valor, 0, 7))] + "'";
        if (c.coluna == Coluna::Curso) return coluna + " = '" + kCursos[static_cast<size_t>(std::clamp(c.valor, 0, 2))] + "'";
        const char* op = c.op == Op::Menor ? " < " : c.op == Op::MaiorOuIgual ? " >= " : " = ";
        return coluna + op + std::to_string(c.valor);
    }

    std::vector<Casa> Casas(const Registros& regs, const Consulta& c) {
        std::vector<Casa> casas;
        const int n = static_cast<int>(regs.size());
        switch (c.tipo) {
            case Tipo::Selecao:
                for (const int l : Linhas(regs, c.onde)) {
                    for (int k = 0; k < kColunas; ++k) casas.push_back({l, k});
                }
                break;
            case Tipo::Projecao:
                for (int l = 0; l < n; ++l) casas.push_back({l, static_cast<int>(c.coluna)});
                break;
            case Tipo::Ambas:
                for (const int l : Linhas(regs, c.onde)) casas.push_back({l, static_cast<int>(c.coluna)});
                break;
            case Tipo::Ordenacao:
                break;
        }
        return casas;
    }

    Consulta Proxima(const Registros& regs, const std::uint32_t sorteio, const int vez) {
        static constexpr std::array<Tipo, 6> kCiclo = {Tipo::Selecao, Tipo::Projecao, Tipo::Ambas,
                                                       Tipo::Selecao, Tipo::Projecao, Tipo::Ordenacao};
        std::mt19937 g(sorteio);
        Consulta c;
        c.tipo = kCiclo[static_cast<size_t>((vez % 6 + 6) % 6)];
        const int n = static_cast<int>(regs.size());
        switch (c.tipo) {
            case Tipo::Projecao:
                c.coluna = static_cast<Coluna>(Ate(g, kColunas));
                break;
            case Tipo::Selecao:
                c.onde = Escolher(regs, g, 1, std::max(1, n - 1));
                break;
            case Tipo::Ambas:
                c.coluna = static_cast<Coluna>(Ate(g, kColunas));
                c.onde = Escolher(regs, g, 1, std::min(2, std::max(1, n)));
                break;
            case Tipo::Ordenacao: {
                // UMA ORDEM QUE MUDA ALGUMA COISA: ordenar pelo que ja esta em
                // ordem nao mexeria em linha nenhuma.
                const std::array<Coluna, 4> ordens = {Coluna::Nota, Coluna::Mat, Coluna::Nome, Coluna::Ano};
                const int inicio = Ate(g, 4);
                c.coluna = ordens[static_cast<size_t>(inicio)];
                for (int k = 0; k < 4; ++k) {
                    const Coluna col = ordens[static_cast<size_t>((inicio + k) % 4)];
                    const bool emOrdem = std::is_sorted(regs.begin(), regs.end(), [col](const Registro& a, const Registro& b) {
                        return Valor(a, col) < Valor(b, col);
                    });
                    if (!emOrdem) { c.coluna = col; break; }
                }
                break;
            }
        }
        return c;
    }

    std::string Sql(const Consulta& c) {
        const std::string coluna = kNomesDasColunas[static_cast<size_t>(c.coluna)];
        switch (c.tipo) {
            case Tipo::Selecao:   return "SELECT * WHERE " + TextoDaCondicao(c.onde);
            case Tipo::Projecao:  return "SELECT " + coluna;
            case Tipo::Ambas:     return "SELECT " + coluna + " WHERE " + TextoDaCondicao(c.onde);
            case Tipo::Ordenacao: return "ORDER BY " + coluna;
        }
        return "";
    }

    float XDoProfessor(const int coluna, const float largura) {
        const float faixa = largura / static_cast<float>(kColunas);
        const float meio = (static_cast<float>(coluna) + 0.5f) * faixa;
        return coluna >= kColunas - 1 ? meio - kAoLado : meio + kAoLado;
    }

    std::vector<Casa> Produto(const Plano plano) {
        std::vector<Casa> casas;
        switch (plano) {
            case Plano::PorLinha:
                for (int l = 0; l < kLinhas; ++l) for (int c = 0; c < kColunas; ++c) casas.push_back({l, c});
                break;
            case Plano::PorColuna:
                for (int c = 0; c < kColunas; ++c) for (int l = 0; l < kLinhas; ++l) casas.push_back({l, c});
                break;
            case Plano::DeTrasParaFrente:
                for (int l = kLinhas - 1; l >= 0; --l) for (int c = kColunas - 1; c >= 0; --c) casas.push_back({l, c});
                break;
            case Plano::ZigueZague:
                for (int l = 0; l < kLinhas; ++l) {
                    for (int k = 0; k < kColunas; ++k) casas.push_back({l, l % 2 == 0 ? k : kColunas - 1 - k});
                }
                break;
        }
        return casas;
    }

    Plano PlanoDaVolta(const int vez) {
        return static_cast<Plano>(((vez % kPlanos) + kPlanos) % kPlanos);
    }

    std::string PlacarDoProduto(const Plano plano) {
        switch (plano) {
            case Plano::PorLinha:         return "JOIN sem WHERE: por linha";
            case Plano::PorColuna:        return "JOIN sem WHERE: por coluna";
            case Plano::DeTrasParaFrente: return "JOIN sem WHERE: de tr\xC3\xA1s pra frente";
            case Plano::ZigueZague:       return "JOIN sem WHERE: em zigue-zague";
        }
        return "";
    }

    float PassoDoProduto(const int vez) {
        return std::max(kPassoMinimoDoProduto, kPassoDoProduto - kAceleracaoDoProduto * static_cast<float>(std::max(0, vez)));
    }

    std::vector<int> Ordenar(Registros& regs, const Coluna c) {
        std::vector<int> antes(regs.size());
        std::iota(antes.begin(), antes.end(), 0);
        std::stable_sort(antes.begin(), antes.end(), [&regs, c](const int a, const int b) {
            return Valor(regs[static_cast<size_t>(a)], c) < Valor(regs[static_cast<size_t>(b)], c);
        });
        Registros novos;
        for (const int i : antes) novos.push_back(regs[static_cast<size_t>(i)]);
        regs = novos;
        return antes;
    }
}
