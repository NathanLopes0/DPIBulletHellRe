//
// A tabela "alunos" do Thiago: o que esta escrito no campo, e as consultas.
//

#pragma once

#include "../Regras/Dificuldade.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

/// As consultas leem o que esta escrito - e o aluno tambem pode ler, e saber antes onde a consulta
/// vai cair.
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

    /// Os nomes e os cursos em ordem alfabetica: e por isso que o order by pode ordenar pelo indice
    /// deles.
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

    /// N registros, com mat, nome e nota diferentes em cada um.
    Registros Sortear(std::uint32_t sorteio, int n = kLinhas);

    /// O valor da coluna, como numero: nome e curso pelo indice.
    int Valor(const Registro& r, Coluna c);

    /// O que esta escrito na casa: "42", "Ana", "INF".
    std::string Texto(const Registro& r, Coluna c);

    /// As caras da folha DPIBHThiagoDados: "V42" (um numero), "Ana", "INF", e o cabecalho "Cnota".
    std::string Cara(const Registro& r, Coluna c);
    std::string CaraDoCabecalho(Coluna c);

    /// A nota escrita por uma transacao que ainda nao confirmou: "S42", o mesmo numero em vermelho.
    std::string CaraSuja(int nota);

    enum class Op { Menor, MaiorOuIgual, Igual };

    struct Condicao {
        Coluna coluna = Coluna::Nota;
        Op op = Op::Menor;
        int valor = 0;
    };

    bool Satisfaz(const Registro& r, const Condicao& c);

    /// As linhas (indices) cujos registros satisfazem a condicao.
    std::vector<int> Linhas(const Registros& regs, const Condicao& c);

    /// "nota < 50", "nota >= 70", "curso = 'MAT'", "ano = 2", "mat < 40".
    std::string TextoDaCondicao(const Condicao& c);

    enum class Tipo { Selecao, Projecao, Ambas, Ordenacao };

    struct Consulta {
        Tipo tipo = Tipo::Selecao;
        Coluna coluna = Coluna::Nota;   ///< A projetada, ou a do order by.
        Condicao onde;                  ///< A do where (Selecao e Ambas)
    };

    struct Casa {
        int linha = 0;
        int coluna = 0;
        bool operator==(const Casa& o) const { return linha == o.linha && coluna == o.coluna; }
    };

    /// As casas que a consulta le: as linhas inteiras da selecao, a coluna inteira da projecao, so
    /// os cruzamentos das duas. O order by nao le casa nenhuma.
    std::vector<Casa> Casas(const Registros& regs, const Consulta& c);

    /// A consulta numero 'vez' da fase 1. Num ciclo de seis: selecao, projecao, as duas, selecao,
    /// projecao e order by.
    Consulta Proxima(const Registros& regs, std::uint32_t sorteio, int vez, int linhasDaSelecao = 1);

    /// "SELECT * WHERE nota < 50", "SELECT nota", "SELECT nome WHERE ano = 2", "ORDER BY nota".
    std::string Sql(const Consulta& c);

    /// Ordena os registros pela coluna, crescente e estavel. Devolve de onde veio cada um: o
    /// registro que ficou na linha i estava na linha devolvido[i].
    std::vector<int> Ordenar(Registros& regs, Coluna c);

    constexpr float kPrimeira = 1.2f;
    constexpr float kPasso = 1.8f;
    constexpr float kAvisoDaLinha = 1.0f;
    constexpr float kAvisoDaColuna = 1.2f;
    constexpr float kAvisoDaCasa = 0.9f;
    constexpr float kDuracaoDaCasa = 0.35f;
    constexpr float kDepoisDeOrdenar = 1.2f;

    /// A velocidade da varredura de uma linha e de uma coluna.
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

    /// O professor fica na coluna que le, ao lado do valor da primeira linha e abaixo do placar -
    /// nem o placar nem o que ele le somem atras dele.
    constexpr float kYDoProfessor = 140.0f;
    constexpr float kAoLado = 90.0f;

    /// O x do professor quando ele le a coluna c, numa tabela de 'largura'.
    float XDoProfessor(int coluna, float largura);

    // ---- A fase 2: o insert, o update e o delete ----

    constexpr const char* kFaseDaEscrita = "StateTwo";

    enum class Comando { Insert, Update, Delete };

    struct Escrita {
        Comando comando = Comando::Insert;
        Condicao onde;          ///< Update e Delete
        Registro novo;          ///< Insert
        int nota = 0;           ///< Update: a nota nova
    };

    /// Um registro que nao repete a matricula, o nome nem a nota de ninguem da tabela.
    Registro NovoRegistro(const Registros& regs, std::uint32_t sorteio);

    /// O proximo comando da fase 2, sobre a tabela como ela esta.
    Escrita ProximaEscrita(const Registros& regs, std::uint32_t sorteio);

    /// "INSERT INTO alunos: Eva", "UPDATE SET nota = 95 WHERE ano = 2", "DELETE WHERE nota < 30".
    std::string Sql(const Escrita& e);

    /// Executa o comando.
    std::vector<int> Executar(Registros& regs, const Escrita& e);

    /// O relogio da fase 2, o aviso do update (o do delete e do insert e o da linha), e a
    /// velocidade com que o que foi apagado cai.
    constexpr float kPrimeiraEscrita = 1.2f;
    constexpr float kPassoDaEscrita = 1.7f;
    constexpr float kVelocidadeDoApagado = 260.0f;

    // ---- A repescagem: o produto cartesiano ----

    constexpr const char* kFaseDoProduto = "StateFinal";

    enum class Plano { PorLinha, PorColuna, DeTrasParaFrente, ZigueZague };
    constexpr int kPlanos = 4;

    /// Todas as casas, na ordem em que o plano as le.
    std::vector<Casa> Produto(Plano plano);

    /// O plano da volta 'vez': os quatro, um depois do outro.
    Plano PlanoDaVolta(int vez);

    /// "JOIN sem WHERE: por linha", "... por coluna", "... de tras pra frente", "... em
    /// zigue-zague".
    std::string PlacarDoProduto(Plano plano);

    /// De quanto em quanto tempo o laco passa para a casa seguinte, na volta 'vez': cada volta mais
    /// depressa, ate kPassoMinimoDoProduto.
    float PassoDoProduto(int vez);

    /// O relogio da repescagem: a primeira volta, quanto antes de acender a casa fica marcada (com
    /// 0,55s, so tres de cinco sementes aprovavam quem desvia.
    constexpr float kPrimeiroProduto = 1.0f;
    constexpr float kAvisoDoProduto = 0.7f;
    constexpr float kEntreVoltas = 0.8f;
    constexpr float kPassoDoProduto = 0.15f;
    constexpr float kPassoMinimoDoProduto = 0.10f;
    constexpr float kAceleracaoDoProduto = 0.01f;

    // ---- O que muda de uma luta para a outra, e o degrau ----
    constexpr int kConsultasNoCiclo = 6;

    /// A regra de cada consulta e a mesma.
    struct Degrau {
        float passo = kPasso;
        int linhasDaSelecao = 1;         ///< O menor numero de linhas do where.
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
