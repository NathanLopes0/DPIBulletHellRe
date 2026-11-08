//
// A ordem das melhores notas de uma materia.
//

#include "Ranking.h"

#include <algorithm>
#include <map>
#include <utility>

namespace Ranking {

namespace {

    /// Numera as posicoes de uma lista ja ordenada por nota decrescente. A regra de empate e uma
    /// so, e todas as listas passam por aqui.
    template <typename L>
    void Numerar(std::vector<L>& linhas) {
        for (size_t i = 0; i < linhas.size(); ++i) {
            linhas[i].posicao = (i > 0 && linhas[i].nota == linhas[i - 1].nota)
                                    ? linhas[i - 1].posicao
                                    : static_cast<int>(i) + 1;
        }
    }

    /// Ordena e numera as linhas de um aluno so.
    template <typename NotaDe, typename JogadasDe>
    std::vector<Linha> Ordenar(const std::vector<Exportacao::FichaDeAluno>& fichas,
                               NotaDe notaDe, JogadasDe jogadasDe) {

        std::vector<Linha> linhas;
        linhas.reserve(fichas.size());

        for (const auto& f : fichas) {
            // Nao disputou: fora.
            if (const float nota = notaDe(f); nota > 0.0f) {
                linhas.push_back(Linha{1, f.matricula, nota, jogadasDe(f)});
            }
        }

        std::sort(linhas.begin(), linhas.end(), [](const Linha& a, const Linha& b) {
            if (a.nota != b.nota) return a.nota > b.nota;
            return a.matricula < b.matricula;
        });

        Numerar(linhas);
        return linhas;
    }

    /// O ranking de solo le o recorde de solo do nivel, e nao MelhorNota.
    float NotaDeSolo(const Exportacao::FichaDeAluno& f, const int materia, const Dificuldade::Nivel nivel) {
        return f.progresso.Recorde(materia, Progresso::Modo::Solo, nivel).nota;
    }

    std::vector<Linha> Completa(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                const int materia, const Dificuldade::Nivel nivel) {
        return Ordenar(fichas,
            [materia, nivel](const Exportacao::FichaDeAluno& f) { return NotaDeSolo(f, materia, nivel); },
            [](const Exportacao::FichaDeAluno&) { return 1; });
    }

    std::vector<Linha> CompletaGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                     const int quantasMaterias, const Dificuldade::Nivel nivel) {

        if (quantasMaterias <= 0) return {};

        // So as notas de solo, pelo mesmo motivo de Completa.
        auto media = [quantasMaterias, nivel](const Exportacao::FichaDeAluno& f) {
            float soma = 0.0f;
            for (int m = 0; m < quantasMaterias; ++m) {
                soma += NotaDeSolo(f, m, nivel);    // materia nao jogada vale zero
            }
            return soma / static_cast<float>(quantasMaterias);
        };

        auto jogadas = [quantasMaterias, nivel](const Exportacao::FichaDeAluno& f) {
            int n = 0;
            for (int m = 0; m < quantasMaterias; ++m) if (NotaDeSolo(f, m, nivel) > 0.0f) ++n;
            return n;
        };

        return Ordenar(fichas, media, jogadas);
    }

    /// Template pelo mesmo motivo de Numerar: "zero devolve todas" e uma regra,
    /// e as duas listas a cumprem igual.
    template <typename L>
    std::vector<L> Cortar(std::vector<L> linhas, const size_t quantas) {
        if (quantas > 0 && linhas.size() > quantas) linhas.resize(quantas);
        return linhas;
    }

    // ---- As duplas ----

    /// A chave de uma dupla: as duas matriculas, a menor primeiro. E o que faz a dupla ser a mesma
    /// dupla nas duas fichas.
    using Chave = std::pair<std::string, std::string>;

    Chave ChaveDa(const std::string& a, const std::string& b) {
        return a < b ? Chave{a, b} : Chave{b, a};
    }

    /// As duplas que jogaram uma materia, e a nota de cada uma.
    std::map<Chave, float> DuplasDe(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                    const int materia, const Dificuldade::Nivel nivel) {

        std::map<Chave, float> porDupla;

        for (const auto& f : fichas) {

            // Nao jogou acompanhado nesta materia: fora, pelo mesmo motivo de Ordenar.
            const Progresso::Marca& recorde = f.progresso.Recorde(materia, Progresso::Modo::Dupla, nivel);
            const float nota = recorde.nota;
            if (nota <= 0.0f) continue;

            // Visitante nao entra: sem matricula nao ha linha que diga quem jogou.
            const std::string& parceiro = recorde.parceiro;
            if (parceiro.empty()) continue;

            // Parceiro igual ao proprio aluno so sai de ficha editada a mao.
            if (parceiro == f.matricula) continue;

            const Chave chave = ChaveDa(f.matricula, parceiro);
            const auto [it, nova] = porDupla.try_emplace(chave, nota);
            if (!nova) it->second = std::max(it->second, nota);
        }

        return porDupla;
    }

    std::vector<LinhaDeDupla> OrdenarDuplas(std::vector<LinhaDeDupla> linhas) {

        // Quem reparte a posicao entre empatados e Numerar.
        std::sort(linhas.begin(), linhas.end(), [](const LinhaDeDupla& a, const LinhaDeDupla& b) {
            if (a.nota != b.nota) return a.nota > b.nota;
            if (a.um != b.um) return a.um < b.um;
            return a.dois < b.dois;
        });

        Numerar(linhas);
        return linhas;
    }
}

std::vector<Linha> DaMateria(const std::vector<Exportacao::FichaDeAluno>& fichas,
                             const int materia, const Dificuldade::Nivel nivel, const size_t quantas) {
    return Cortar(Completa(fichas, materia, nivel), quantas);
}

std::vector<Linha> Geral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                         const int quantasMaterias, const Dificuldade::Nivel nivel, const size_t quantas) {
    return Cortar(CompletaGeral(fichas, quantasMaterias, nivel), quantas);
}

int PosicaoDe(const std::vector<Exportacao::FichaDeAluno>& fichas,
              const int materia, const Dificuldade::Nivel nivel, const std::string& matricula) {

    for (const auto& l : Completa(fichas, materia, nivel)) {
        if (l.matricula == matricula) return l.posicao;
    }
    return 0;
}

int PosicaoNoGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                   const int quantasMaterias, const Dificuldade::Nivel nivel, const std::string& matricula) {

    for (const auto& l : CompletaGeral(fichas, quantasMaterias, nivel)) {
        if (l.matricula == matricula) return l.posicao;
    }
    return 0;
}

std::vector<LinhaDeDupla> DuplasDaMateria(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                          const int materia, const Dificuldade::Nivel nivel,
                                          const size_t quantas) {

    std::vector<LinhaDeDupla> linhas;

    for (const auto& [chave, nota] : DuplasDe(fichas, materia, nivel)) {
        linhas.push_back(LinhaDeDupla{1, chave.first, chave.second, nota, 1});
    }

    return Cortar(OrdenarDuplas(std::move(linhas)), quantas);
}

namespace {

    /// Acha a dupla numa lista ja pronta. As duas buscas passam por aqui, pelo mesmo motivo de
    /// Numerar: "e esta a minha dupla?" tem uma resposta so.
    int Onde(const std::vector<LinhaDeDupla>& linhas, const std::string& um,
             const std::string& dois) {

        // Ordenada, como a chave: assim (A,B) e (B,A) acham a mesma linha.
        const Chave procurada = ChaveDa(um, dois);

        for (const auto& l : linhas) {
            if (l.um == procurada.first && l.dois == procurada.second) return l.posicao;
        }
        return 0;
    }
}

int PosicaoDaDupla(const std::vector<Exportacao::FichaDeAluno>& fichas, const int materia,
                   const Dificuldade::Nivel nivel, const std::string& um, const std::string& dois) {

    // Sem corte (quantas = 0): e a lista completa que responde "onde estamos".
    return Onde(DuplasDaMateria(fichas, materia, nivel), um, dois);
}

int PosicaoDaDuplaNoGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                          const int quantasMaterias, const Dificuldade::Nivel nivel,
                          const std::string& um, const std::string& dois) {
    return Onde(DuplasGeral(fichas, quantasMaterias, nivel), um, dois);
}

std::vector<LinhaDeDupla> DuplasGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                      const int quantasMaterias, const Dificuldade::Nivel nivel,
                                      const size_t quantas) {

    if (quantasMaterias <= 0) return {};

    // Soma e conta materia por materia.
    struct Acumulado { float soma = 0.0f; int jogadas = 0; };
    std::map<Chave, Acumulado> porDupla;

    for (int m = 0; m < quantasMaterias; ++m) {
        for (const auto& [chave, nota] : DuplasDe(fichas, m, nivel)) {
            Acumulado& a = porDupla[chave];
            a.soma += nota;
            ++a.jogadas;
        }
    }

    std::vector<LinhaDeDupla> linhas;
    linhas.reserve(porDupla.size());

    for (const auto& [chave, a] : porDupla) {
        linhas.push_back(LinhaDeDupla{1, chave.first, chave.second,
                                      a.soma / static_cast<float>(quantasMaterias),
                                      a.jogadas});
    }

    return Cortar(OrdenarDuplas(std::move(linhas)), quantas);
}

std::string NomeDaLista(const Lista l) {
    // \xc3\xad e o i agudo: o MSVC nao le acento em literal.
    const std::string nivel = l.nivel == Dificuldade::Nivel::Dificil ? "Dif\xc3\xad" "cil" : "Normal";
    return l.modo == Progresso::Modo::Dupla ? nivel + " em dupla" : nivel;
}

int IndiceDaLista(const Progresso::Modo modo, const Dificuldade::Nivel nivel) {
    for (int i = 0; i < kQuantasListas; ++i) {
        if (kListas[i].modo == modo && kListas[i].nivel == nivel) return i;
    }
    return 0;
}

int MoverLista(const int atual, const int passo) {
    const int r = (atual + passo) % kQuantasListas;
    return r < 0 ? r + kQuantasListas : r;
}

}
