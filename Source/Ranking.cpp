//
// A ordem das melhores notas de uma materia.
//

#include "Ranking.h"

#include <algorithm>
#include <map>
#include <utility>

namespace Ranking {

namespace {

    /// Numera as posicoes de uma lista JA ORDENADA por nota decrescente.
    ///
    /// A REGRA DE EMPATE E UMA SO, E TODAS AS LISTAS PASSAM POR AQUI. Ela olha
    /// apenas .nota e .posicao, que Linha e LinhaDeDupla tem iguais, e e por
    /// isso que serve as duas sem saber qual e qual. Se cada lista numerasse a
    /// sua, o mesmo empate poderia sair "2o e 2o" numa tela e "2o e 3o" na
    /// outra - e seria a tela, e nao a nota, decidindo quem ganhou.
    ///
    /// A POSICAO E POR NOTA, E NAO POR INDICE: dois empatados dividem a
    /// posicao, e a seguinte pula. E como se le um placar.
    template <typename L>
    void Numerar(std::vector<L>& linhas) {
        for (size_t i = 0; i < linhas.size(); ++i) {
            linhas[i].posicao = (i > 0 && linhas[i].nota == linhas[i - 1].nota)
                                    ? linhas[i - 1].posicao
                                    : static_cast<int>(i) + 1;
        }
    }

    /// Ordena e numera as linhas de um aluno so.
    ///
    /// @param notaDe como tirar a nota de uma ficha. E a unica coisa que muda
    ///        entre o ranking de uma materia e o geral.
    template <typename NotaDe, typename JogadasDe>
    std::vector<Linha> Ordenar(const std::vector<Exportacao::FichaDeAluno>& fichas,
                               NotaDe notaDe, JogadasDe jogadasDe) {

        std::vector<Linha> linhas;
        linhas.reserve(fichas.size());

        for (const auto& f : fichas) {
            // Nao disputou: fora. Ver o comentario de DaMateria.
            if (const float nota = notaDe(f); nota > 0.0f) {
                linhas.push_back(Linha{1, f.matricula, nota, jogadasDe(f)});
            }
        }

        // Nota decrescente; empate em ordem de matricula, so para a ordem ser
        // sempre a mesma. stable_sort nao bastaria: a ordem de entrada vem da
        // listagem de arquivos, que nao e garantida.
        std::sort(linhas.begin(), linhas.end(), [](const Linha& a, const Linha& b) {
            if (a.nota != b.nota) return a.nota > b.nota;
            return a.matricula < b.matricula;
        });

        Numerar(linhas);
        return linhas;
    }

    /// O RANKING DE SOLO LE O RECORDE DE SOLO, e nao MelhorNota.
    ///
    /// MelhorNota devolve a maior das duas notas porque e ela que o mapa do
    /// curso le - passar e passar (requisitos-coop, D10). Aqui seria o
    /// contrario do que os dois rankings existem para fazer: uma nota feita
    /// acompanhado entraria na lista de quem jogou sozinho.
    std::vector<Linha> Completa(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                const int materia) {
        return Ordenar(fichas,
            [materia](const Exportacao::FichaDeAluno& f) { return f.progresso.MelhorNotaSolo(materia); },
            [](const Exportacao::FichaDeAluno&) { return 1; });
    }

    std::vector<Linha> CompletaGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                     const int quantasMaterias) {

        if (quantasMaterias <= 0) return {};

        // SO AS NOTAS DE SOLO, pelo mesmo motivo de Completa.
        auto media = [quantasMaterias](const Exportacao::FichaDeAluno& f) {
            float soma = 0.0f;
            for (int m = 0; m < quantasMaterias; ++m) {
                const float nota = f.progresso.MelhorNotaSolo(m);
                if (nota > 0.0f) soma += nota;    // materia nao jogada vale zero
            }
            return soma / static_cast<float>(quantasMaterias);
        };

        auto jogadas = [quantasMaterias](const Exportacao::FichaDeAluno& f) {
            int n = 0;
            for (int m = 0; m < quantasMaterias; ++m) if (f.progresso.MelhorNotaSolo(m) > 0.0f) ++n;
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

    // ---- as duplas -------------------------------------------------------

    /// A chave de uma dupla: as duas matriculas, A MENOR PRIMEIRO.
    ///
    /// E o que faz a dupla ser a MESMA dupla nas duas fichas. Sem ordenar, o
    /// par (A,B) gravado na ficha de A e o par (B,A) gravado na de B virariam
    /// duas linhas do ranking - a mesma dupla competindo contra si mesma, com a
    /// mesma nota, ocupando duas posicoes do topo.
    using Chave = std::pair<std::string, std::string>;

    Chave ChaveDa(const std::string& a, const std::string& b) {
        return a < b ? Chave{a, b} : Chave{b, a};
    }

    /// As duplas que jogaram uma materia, e a nota de cada uma.
    ///
    /// UM LUGAR SO decide o que e "a nota de uma dupla numa materia": a lista
    /// da materia e a geral chamam esta funcao, entao nao ha como as duas
    /// discordarem sobre quem jogou com quem.
    ///
    /// A MAIOR DAS DUAS FICHAS, quando a dupla aparece nas duas. No caminho
    /// normal os dois numeros sao iguais - e a MESMA batalha, gravada dos dois
    /// lados. Divergem quando uma das fichas nao chegou ao disco (queda de
    /// energia no gabinete entre uma gravacao e a outra), e ai a maior e a
    /// certa: a dupla fez aquela nota, e o outro lado so nao registrou.
    std::map<Chave, float> DuplasDe(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                    const int materia) {

        std::map<Chave, float> porDupla;

        for (const auto& f : fichas) {

            // Nao jogou acompanhado nesta materia: fora, pelo mesmo motivo de
            // Ordenar - uma linha zerada empurraria para fora quem disputou.
            const float nota = f.progresso.MelhorNotaDeDupla(materia);
            if (nota <= 0.0f) continue;

            // VISITANTE NAO ENTRA: sem matricula nao ha linha que diga quem
            // jogou. A nota continua valendo para o progresso de quem se
            // identificou (RF6); e a LINHA que precisa de dois nomes.
            const std::string& parceiro = f.progresso.ParceiroDoRecorde(materia);
            if (parceiro.empty()) continue;

            // Parceiro igual ao proprio aluno so sai de ficha editada a mao:
            // Sessao::JaEstaEmUso impede duas posicoes com a mesma matricula.
            // Sem este corte, a pasta do professor com um erro de digitacao
            // poria "89384 + 89384" no topo do ranking.
            if (parceiro == f.matricula) continue;

            const Chave chave = ChaveDa(f.matricula, parceiro);
            const auto [it, nova] = porDupla.try_emplace(chave, nota);
            if (!nova) it->second = std::max(it->second, nota);
        }

        return porDupla;
    }

    std::vector<LinhaDeDupla> OrdenarDuplas(std::vector<LinhaDeDupla> linhas) {

        // Nota decrescente; empate pelas matriculas, so para a ordem ser sempre
        // a mesma de uma abertura para a outra. Quem REPARTE a posicao entre
        // empatados e Numerar - este desempate e de exibicao, nao de
        // classificacao.
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
                             const int materia, const size_t quantas) {
    return Cortar(Completa(fichas, materia), quantas);
}

std::vector<Linha> Geral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                         const int quantasMaterias, const size_t quantas) {
    return Cortar(CompletaGeral(fichas, quantasMaterias), quantas);
}

int PosicaoDe(const std::vector<Exportacao::FichaDeAluno>& fichas,
              const int materia, const std::string& matricula) {

    for (const auto& l : Completa(fichas, materia)) {
        if (l.matricula == matricula) return l.posicao;
    }
    return 0;
}

int PosicaoNoGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                   const int quantasMaterias, const std::string& matricula) {

    for (const auto& l : CompletaGeral(fichas, quantasMaterias)) {
        if (l.matricula == matricula) return l.posicao;
    }
    return 0;
}

std::vector<LinhaDeDupla> DuplasDaMateria(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                          const int materia, const size_t quantas) {

    std::vector<LinhaDeDupla> linhas;

    for (const auto& [chave, nota] : DuplasDe(fichas, materia)) {
        linhas.push_back(LinhaDeDupla{1, chave.first, chave.second, nota, 1});
    }

    return Cortar(OrdenarDuplas(std::move(linhas)), quantas);
}

std::vector<LinhaDeDupla> DuplasGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                      const int quantasMaterias, const size_t quantas) {

    if (quantasMaterias <= 0) return {};

    // Soma e conta materia por materia. A DIVISAO E PELO CURSO, e nao pelas
    // materias que a dupla jogou junta - a mesma conta de Geral, e pelo mesmo
    // motivo: assim subir exige jogar mais materias COM A MESMA PESSOA, que e o
    // que uma lista de duplas deveria premiar. Dividir pelas jogadas faria uma
    // dupla de uma materia so passar na frente de uma que jogou o curso inteiro.
    struct Acumulado { float soma = 0.0f; int jogadas = 0; };
    std::map<Chave, Acumulado> porDupla;

    for (int m = 0; m < quantasMaterias; ++m) {
        for (const auto& [chave, nota] : DuplasDe(fichas, m)) {
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

}
