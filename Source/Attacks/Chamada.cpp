//
// O sorteio do Daniel: no comeco da aula, um aluno explica a aula passada.
//
// As perguntas e as opcoes estao tambem em Ferramentas/gerar_daniel.py, que escreve as cartas: mudou
// uma, muda a outra.
//

#include "Chamada.h"

#include <algorithm>
#include <cmath>

namespace Chamada {

    namespace {
        // O MSVC nao le acento em literal: os acentos vao em UTF-8 escapado.
        const std::array<std::array<Pergunta, kPerguntasPorAula>, kAulas> kRevisoes = {{
        // aula 0
        {{
            {"O que a padroniza\xc3\xa7" "\xc3\xa3" "o (z-score) faz com os dados?",
             {"m\xc3\xa9" "dia 0 e desvio 1", "tudo entre 0 e 1", "apaga os nulos"}},
            {"E o dado que veio faltando, o nulo?",
             {"imputa: vira a m\xc3\xa9" "dia", "vira um outlier", "fica como est\xc3\xa1" ""}},
            {"Como se acha um outlier?",
             {"o z-score passa de 2", "\xc3\xa9" " o dado mais novo", "\xc3\xa9" " o que veio da API"}},
        }},
        // aula 1
        {{
            {"No k-m\xc3\xa9" "dias, para onde vai o centr\xc3\xb3" "ide?",
             {"para a m\xc3\xa9" "dia do grupo", "para o ponto mais longe", "para o canto do campo"}},
            {"A PCA projeta os dados na dire\xc3\xa7" "\xc3\xa3" "o...",
             {"de maior vari\xc3\xa2" "ncia", "de menor vari\xc3\xa2" "ncia", "do professor"}},
            {"Como se escolhe o k?",
             {"pelo cotovelo da in\xc3\xa9" "rcia", "\xc3\xa9" " sempre 3", "pelo n\xc3\xba" "mero de alunos"}},
        }},
        // aula 2
        {{
            {"Num comit\xc3\xaa" " de \xc3\xa1" "rvores, quem decide?",
             {"a maioria dos votos", "a \xc3\xa1" "rvore mais funda", "a \xc3\xba" "ltima plantada"}},
            {"Aprender com r\xc3\xb3" "tulo \xc3\xa9" " o aprendizado...",
             {"supervisionado", "n\xc3\xa3" "o supervisionado", "por sorteio"}},
            {"O perceptron corrige a reta com...",
             {"os erros", "os acertos", "a m\xc3\xa9" "dia dos r\xc3\xb3" "tulos"}},
        }},
        }};
    }

    const std::array<Pergunta, kPerguntasPorAula>& Revisao(const int aula) {
        return kRevisoes[static_cast<size_t>(std::clamp(aula, 0, kAulas - 1))];
    }

    std::string Cara(const int aula, const int q, const int o) {
        return "R" + std::to_string(aula) + "Q" + std::to_string(q) + "O" + std::to_string(o);
    }

    int Sortear(Variacao::Sorteio& s, const int alunos) {
        if (alunos <= 1) return s.Inteiro(0, 1) == 0 ? 0 : -1;
        const int r = s.Inteiro(0, 2);
        return r == 2 ? -1 : r;
    }

    std::string Colega(const int k) {
        static constexpr std::array<const char*, kColegas> kNomes = {"Aluno 7", "Aluna 12", "Aluno 23",
                                                                     "Aluna 31", "Aluno 42", "Aluna 5"};
        return kNomes[static_cast<size_t>(((k % kColegas) + kColegas) % kColegas)];
    }

    int NomeNaRoleta(const float t, const int nomes) {
        if (nomes <= 0) return 0;
        // A roleta desacelera: o nome troca a cada 0,06 s no comeco e a cada 0,3 s no fim.
        const float u = std::clamp(t / kRoleta, 0.0f, 1.0f);
        const float trocas = 6.0f * u + 12.0f * (1.0f - (1.0f - u) * (1.0f - u));
        return static_cast<int>(std::floor(trocas)) % nomes;
    }
}
