//
// A tabela "alunos" do Thiago: o que esta escrito no campo, e as consultas.
//

#pragma once

#include "../Dificuldade.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

/**
 * CAMADA PURA.
 *
 * O CAMPO DO THIAGO E UMA TABELA, e agora ela esta ESCRITA nele: cinco colunas
 * com cabecalho (mat, nome, curso, nota, ano) e quatro registros, um por linha,
 * cada valor na sua casa. As consultas leem o que esta escrito - e o aluno
 * tambem pode ler, e saber antes onde a consulta vai cair.
 *
 * AS TRES OPERACOES DA ALGEBRA RELACIONAL QUE O SELECT FAZ, cada uma uma forma
 * de ataque:
 *
 *   SELECAO (sigma)    "SELECT * WHERE nota < 50"   as LINHAS que satisfazem
 *                                                  a condicao sao varridas
 *   PROJECAO (pi)      "SELECT nota"                a COLUNA e varrida
 *   AS DUAS            "SELECT nome WHERE ano = 2"  so as CASAS do cruzamento
 *
 * E o ORDER BY reordena os registros: as linhas trocam de lugar na frente do
 * aluno, e a casa que era segura pode deixar de ser.
 *
 * Quem desenha a tabela, marca as casas, varre e move o professor e o Thiago;
 * aqui ficam os registros, as condicoes, as consultas e os textos. Coberto por
 * test_banco.cpp.
 */
namespace Banco {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kPostoDaTabela = "tabela";
    constexpr const char* kPostoDoAviso = "aviso";
    constexpr const char* kGatilhoDoCabecalho = "cabecalho";
    constexpr const char* kGatilhoDoDado = "registro";
    constexpr const char* kGatilhoDoAviso = "marcar";
    constexpr const char* kGatilhoDaSelecao = "selecionar";
    constexpr const char* kGatilhoDaProjecao = "projetar";
    constexpr const char* kGatilhoDaCasa = "celula";
    constexpr const char* kFaseDaSelecao = "StateOne";

    /// A forma da tabela: a mesma da Tabela.h que a ConsultaAttack usa.
    constexpr int kLinhas = 4;
    constexpr int kColunas = 5;

    enum class Coluna { Mat, Nome, Curso, Nota, Ano };
    constexpr std::array<const char*, 5> kNomesDasColunas = {"mat", "nome", "curso", "nota", "ano"};

    /// Os nomes e os cursos em ordem alfabetica: e por isso que o ORDER BY
    /// pode ordenar pelo indice deles.
    constexpr std::array<const char*, 8> kNomes = {"Ana", "Bia", "Caio", "Davi", "Eva", "Gil", "Iara", "Lia"};
    constexpr std::array<const char*, 3> kCursos = {"FIS", "INF", "MAT"};
    constexpr int kMaiorAno = 5;

    struct Registro {
        int mat = 10;     ///< 10 a 99, diferente em cada registro
        int nome = 0;     ///< indice em kNomes
        int curso = 0;    ///< indice em kCursos
        int nota = 0;     ///< 0 a 99, diferente em cada registro
        int ano = 1;      ///< 1 a kMaiorAno
    };
    using Registros = std::vector<Registro>;

    /// @brief n registros, com mat, nome e nota diferentes em cada um.
    Registros Sortear(std::uint32_t sorteio, int n = kLinhas);

    /// @brief O valor da coluna, como numero: nome e curso pelo indice.
    int Valor(const Registro& r, Coluna c);

    /// @brief O que esta escrito na casa: "42", "Ana", "INF".
    std::string Texto(const Registro& r, Coluna c);

    /// @brief As caras da folha DPIBHThiagoDados: "V42" (um numero), "Ana",
    /// "INF", e o cabecalho "Cnota".
    std::string Cara(const Registro& r, Coluna c);
    std::string CaraDoCabecalho(Coluna c);

    /// @brief A nota escrita por uma transacao que ainda nao confirmou: "S42",
    /// o mesmo numero em vermelho.
    std::string CaraSuja(int nota);

    enum class Op { Menor, MaiorOuIgual, Igual };

    struct Condicao {
        Coluna coluna = Coluna::Nota;
        Op op = Op::Menor;
        int valor = 0;
    };

    bool Satisfaz(const Registro& r, const Condicao& c);

    /// @brief As linhas (indices) cujos registros satisfazem a condicao.
    std::vector<int> Linhas(const Registros& regs, const Condicao& c);

    /// @brief "nota < 50", "nota >= 70", "curso = 'MAT'", "ano = 2", "mat < 40".
    std::string TextoDaCondicao(const Condicao& c);

    enum class Tipo { Selecao, Projecao, Ambas, Ordenacao };

    struct Consulta {
        Tipo tipo = Tipo::Selecao;
        Coluna coluna = Coluna::Nota;   ///< a projetada, ou a do ORDER BY
        Condicao onde;                  ///< a do WHERE (Selecao e Ambas)
    };

    struct Casa {
        int linha = 0;
        int coluna = 0;
        bool operator==(const Casa& o) const { return linha == o.linha && coluna == o.coluna; }
    };

    /// @brief As casas que a consulta le: as linhas inteiras da selecao, a
    /// coluna inteira da projecao, so os cruzamentos das duas. O ORDER BY nao
    /// le casa nenhuma.
    std::vector<Casa> Casas(const Registros& regs, const Consulta& c);

    /// @brief A consulta numero 'vez' da fase 1. Num ciclo de seis: selecao,
    /// projecao, as duas, selecao, projecao e ORDER BY. A selecao sempre pega
    /// de uma linha a todas menos uma - sempre ha uma linha que sobra -, e as
    /// duas pegam de uma a duas casas.
    Consulta Proxima(const Registros& regs, std::uint32_t sorteio, int vez);

    /// @brief A mesma, com a selecao pegando pelo menos 'linhasDaSelecao'
    /// linhas (o degrau do Dificil) - e ainda nunca todas.
    Consulta Proxima(const Registros& regs, std::uint32_t sorteio, int vez, int linhasDaSelecao);

    /// @brief "SELECT * WHERE nota < 50", "SELECT nota", "SELECT nome WHERE
    /// ano = 2", "ORDER BY nota".
    std::string Sql(const Consulta& c);

    /// @brief Ordena os registros pela coluna, crescente e estavel. Devolve de
    /// onde veio cada um: o registro que ficou na linha i estava na linha
    /// devolvido[i].
    std::vector<int> Ordenar(Registros& regs, Coluna c);

    /// O relogio da fase 1: a primeira consulta, uma a cada kPasso, o aviso de
    /// cada forma (a linha e mais estreita que a coluna, e a coluna mais larga,
    /// entao a coluna avisa antes), o tempo em que a casa fica perigosa, e a
    /// pausa do ORDER BY.
    constexpr float kPrimeira = 1.2f;
    constexpr float kPasso = 1.8f;
    constexpr float kAvisoDaLinha = 1.0f;
    constexpr float kAvisoDaColuna = 1.2f;
    constexpr float kAvisoDaCasa = 0.9f;
    constexpr float kDuracaoDaCasa = 0.35f;
    constexpr float kDepoisDeOrdenar = 1.2f;

    /// A velocidade da varredura de uma linha e de uma coluna. O marcador de
    /// cada casa apaga quando a varredura passa por ela - o que ja foi lido deixa
    /// de ser perigo.
    constexpr float kVelocidadeDaLinha = 700.0f;
    constexpr float kVelocidadeDaColuna = 640.0f;

    /// Onde a varredura nasce, a partir da borda (ConsultaAttack).
    constexpr float kMargemDaVarredura = 20.0f;

    /// Quantos pedacos cobrem uma casa que acende, em colunas e linhas: o
    /// ataque "celula" dispara exatamente kPedacosNaLargura x kPedacosNaAltura.
    constexpr int kPedacosNaLargura = 4;
    constexpr int kPedacosNaAltura = 3;

    /// Onde as coisas ficam: os cabecalhos logo abaixo do placar, e o valor
    /// dentro da casa (a fracao da altura da linha).
    constexpr float kYDoCabecalho = 78.0f;
    constexpr float kDadoNaLinha = 0.62f;

    /// O PROFESSOR FICA NA COLUNA QUE LE, ao lado do valor da primeira linha e
    /// abaixo do placar - nem o placar nem o que ele le somem atras dele: kAoLado
    /// a direita do meio da coluna (a esquerda, na ultima), na altura
    /// kYDoProfessor.
    constexpr float kYDoProfessor = 140.0f;
    constexpr float kAoLado = 90.0f;

    /// @brief O x do professor quando ele le a coluna c, numa tabela de
    /// 'largura'.
    float XDoProfessor(int coluna, float largura);

    // ---- A FASE 2: O INSERT, O UPDATE E O DELETE ----------------------------
    //
    // A tabela deixa de ser so lida e passa a ser ESCRITA. Cada comando muda o
    // que esta no campo, e o que ele muda e o ataque:
    //
    //   "DELETE WHERE nota < 30"           a linha e varrida e APAGADA: os
    //                                      valores dela caem pelo campo, e as
    //                                      linhas de baixo sobem para o lugar
    //   "UPDATE SET nota = 95 WHERE ..."   a casa da nota de cada linha do WHERE
    //                                      acende, e o valor muda
    //   "INSERT INTO alunos: Eva"          o registro novo sai do professor para
    //                                      a primeira linha vazia, e a linha e
    //                                      gravada (varrida)
    //
    // A tabela nunca fica vazia nem passa de kLinhas: com duas linhas ou menos
    // so ha INSERT, cheia nao ha INSERT, e o DELETE nunca apaga todas.

    constexpr const char* kFaseDaEscrita = "StateTwo";

    enum class Comando { Insert, Update, Delete };

    struct Escrita {
        Comando comando = Comando::Insert;
        Condicao onde;          ///< Update e Delete
        Registro novo;          ///< Insert
        int nota = 0;           ///< Update: a nota nova
    };

    /// @brief Um registro que nao repete a matricula, o nome nem a nota de
    /// ninguem da tabela.
    Registro NovoRegistro(const Registros& regs, std::uint32_t sorteio);

    /// @brief O proximo comando da fase 2, sobre a tabela como ela esta.
    Escrita ProximaEscrita(const Registros& regs, std::uint32_t sorteio);

    /// @brief "INSERT INTO alunos: Eva", "UPDATE SET nota = 95 WHERE ano = 2",
    /// "DELETE WHERE nota < 30".
    std::string Sql(const Escrita& e);

    /// @brief Executa o comando. Devolve as linhas que ele mexeu, contadas na
    /// tabela de ANTES: a linha nova do INSERT, as atualizadas, as apagadas.
    std::vector<int> Executar(Registros& regs, const Escrita& e);

    /// O relogio da fase 2, o aviso do UPDATE (o do DELETE e do INSERT e o da
    /// linha), e a velocidade com que o que foi apagado cai.
    constexpr float kPrimeiraEscrita = 1.2f;
    constexpr float kPassoDaEscrita = 1.7f;
    constexpr float kVelocidadeDoApagado = 260.0f;

    // ---- A REPESCAGEM: O PRODUTO CARTESIANO ---------------------------------
    //
    // "SELECT * FROM alunos, cursos" - o JOIN que esqueceu o WHERE, e por isso
    // junta TUDO com TUDO. O banco executa num laco aninhado, e e o laco que se
    // ve: uma casa por vez acende e se enche, ate a tabela inteira ter passado.
    // Nao ha casa que escape - quem fica parado e atingido a cada volta -, e o
    // lugar seguro e o que o laco ja leu. O placar diz o PLANO da volta: em que
    // ordem o laco anda.

    constexpr const char* kFaseDoProduto = "StateFinal";

    enum class Plano { PorLinha, PorColuna, DeTrasParaFrente, ZigueZague };
    constexpr int kPlanos = 4;

    /// @brief Todas as casas, na ordem em que o plano as le.
    std::vector<Casa> Produto(Plano plano);

    /// @brief O plano da volta 'vez': os quatro, um depois do outro.
    Plano PlanoDaVolta(int vez);

    /// @brief "JOIN sem WHERE: por linha", "... por coluna", "... de tras pra
    /// frente", "... em zigue-zague".
    std::string PlacarDoProduto(Plano plano);

    /// @brief De quanto em quanto tempo o laco passa para a casa seguinte, na
    /// volta 'vez': cada volta mais depressa, ate kPassoMinimoDoProduto.
    float PassoDoProduto(int vez);

    /// O relogio da repescagem: a primeira volta, quanto antes de acender a casa
    /// fica marcada (com 0,55s, so tres de cinco sementes aprovavam quem desvia;
    /// com 0,7s, cinco de seis), e a pausa entre uma volta e a seguinte.
    constexpr float kPrimeiroProduto = 1.0f;
    constexpr float kAvisoDoProduto = 0.7f;
    constexpr float kEntreVoltas = 0.8f;
    constexpr float kPassoDoProduto = 0.15f;
    constexpr float kPassoMinimoDoProduto = 0.10f;
    constexpr float kAceleracaoDoProduto = 0.01f;

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA, E O DEGRAU -------------------
    //
    // A TABELA JA E SORTEADA (Sortear), e as consultas tambem. O que passa a
    // ser da luta e a ORDEM: na fase 1 o ciclo de seis consultas e outro a cada
    // luta (Variacao::Ordem de kConsultasNoCiclo, com a selecao abrindo), e na
    // repescagem os quatro planos do laco tambem (o por linha abrindo). O Thiago
    // passa a vez por Variacao::NaOrdem.
    constexpr int kConsultasNoCiclo = 6;

    /// O que o Dificil muda: uma consulta a cada 1,5 s (e nao 1,8), menos
    /// aviso em cada forma, varreduras mais rapidas, o WHERE da selecao pegando
    /// pelo menos duas linhas (e ainda nunca todas), a escrita a cada 1,45 s, o
    /// que foi apagado cai mais depressa, e o produto avisa 0,6 s antes e anda
    /// 13% mais depressa. A regra de cada consulta e a mesma.
    struct Degrau {
        float passo = kPasso;
        int linhasDaSelecao = 1;         ///< o menor numero de linhas do WHERE
        float avisoDaLinha = kAvisoDaLinha;
        float avisoDaColuna = kAvisoDaColuna;
        float avisoDaCasa = kAvisoDaCasa;
        float velocidadeDaLinha = kVelocidadeDaLinha;
        float velocidadeDaColuna = kVelocidadeDaColuna;
        float passoDaEscrita = kPassoDaEscrita;
        float velocidadeDoApagado = kVelocidadeDoApagado;
        float avisoDoProduto = kAvisoDoProduto;
        float passoDoProduto = 1.0f;     ///< multiplica PassoDoProduto
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
