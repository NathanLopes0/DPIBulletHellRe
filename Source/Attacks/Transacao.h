//
// As transacoes da fase 3 do Thiago.
//

#pragma once

#include "Banco.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

/// Tudo ou nada: cada transacao prepara alguns comandos das fases 1 e 2, que ficam na tabela como
/// rascunho, e no COMMIT todos acontecem juntos. O aluno que derruba o selo da transacao antes
/// disso provoca o ROLLBACK, e nada do que ela preparou acontece.
namespace Transacao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um. A varredura do COMMIT usa
    /// os gatilhos do SELECT (Banco).
    constexpr const char* kFaseDaTransacao = "StateThree";
    constexpr const char* kGatilhoDoSelo = "selo";
    constexpr const char* kGatilhoDoRascunho = "rascunho";

    enum class Tipo { SelectLinha, SelectColuna, Update, Delete, Insert };

    struct Operacao {
        Tipo tipo = Tipo::Update;
        int mat = 0;              ///< a linha (SelectLinha, Update, Delete), pela matricula: as linhas sobem num DELETE
        int coluna = 0;           ///< SelectColuna
        int nota = 0;             ///< Update: a nota nova
        Banco::Registro novo;     ///< Insert
    };

    /// O peso de cada comando nesta luta: a chance de a operacao ser um SELECT (o resto e escrita).
    struct Plano {
        float chanceDeSelect = 0.35f;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// A proxima operacao de uma transacao, sobre a tabela de agora e o que as transacoes abertas
    /// ja prepararam ('pendentes', inclusive as desta). Uma linha preparada por alguem fica travada
    /// ate o COMMIT ou o ROLLBACK; o DELETE deixa ao menos kMenosLinhas linhas, contando os
    /// pendentes; o INSERT precisa de linha vazia, e so um fica pendente por vez; e as varreduras
    /// pendentes nunca passam de kVarredurasDeLinha linhas e kVarredurasDeColuna colunas - sempre
    /// sobra lugar. Devolve false se nenhuma operacao cabe.
    bool Proxima(const Banco::Registros& regs, const std::vector<Operacao>& pendentes, std::uint32_t sorteio,
                 Operacao& saida, const Plano& plano = Plano{});

    /// A linha da matricula na tabela de agora, ou -1.
    int LinhaDe(const Banco::Registros& regs, int mat);

    /// As casas que o rascunho da operacao cobre na tabela de agora. O INSERT cobre 'linhaDoInsert',
    /// a linha vazia em que ele vai cair.
    std::vector<Banco::Casa> Casas(const Banco::Registros& regs, const Operacao& op, int linhaDoInsert);

    /// Se, com todas as operacoes acontecendo juntas, sobra casa que nenhuma varredura atravessa.
    bool SobraLugar(const Banco::Registros& regs, const std::vector<Operacao>& ops);

    /// Os selos vao de T1 a T9: depois do T9 vem o T1 de novo.
    constexpr int kIds = 9;

    /// As caras na folha, pela cor da transacao (0 ou 1: as duas abertas nunca tem a mesma): o selo
    /// "Ciano3" ("CianoPressa3" quando o COMMIT esta perto) e o rascunho "Magenta"
    /// ("MagentaRiscado" no DELETE).
    std::string CaraDoSelo(int cor, int id, bool pressa);
    std::string CaraDoRascunho(int cor, bool riscado);

    /// "BEGIN T3", "T3: UPDATE SET nota = 12 WHERE mat = 42", "T3: SELECT nota", "T3: COMMIT",
    /// "T3: ROLLBACK".
    std::string PlacarDoBegin(int id);
    std::string PlacarDaOperacao(int id, const Operacao& op);
    std::string PlacarDoCommit(int id);
    std::string PlacarDoRollback(int id);

    /// O relogio: a primeira transacao em kPrimeira, uma nova a cada kNovaTransacao (ate kMaximo
    /// abertas), a primeira operacao kPrimeiraOperacao depois do BEGIN, uma a cada kEntreOperacoes,
    /// e o COMMIT kAteOCommit depois da ultima.
    constexpr float kPrimeira = 1.0f;
    constexpr float kNovaTransacao = 2.2f;
    constexpr int kMaximo = 2;
    constexpr int kOperacoes = 2;
    constexpr float kPrimeiraOperacao = 0.45f;
    constexpr float kEntreOperacoes = 0.55f;
    constexpr float kAteOCommit = 1.8f;

    /// A varredura do COMMIT ainda para na borda, mas pouco: o rascunho ja avisou.
    constexpr float kAvisoNoCommit = 0.3f;

    constexpr int kMenosLinhas = 2;
    constexpr int kVarredurasDeLinha = 2;
    constexpr int kVarredurasDeColuna = 2;

    /// Onde o selo de uma transacao pode ficar, em fracao da largura do campo, na altura do
    /// professor: sorteado a cada BEGIN, para quem fica parado num lugar nao derrubar todos.
    constexpr std::array<float, 4> kPosicoesDoSelo = {0.13f, 0.33f, 0.67f, 0.87f};
    constexpr float kYDoSelo = 150.0f;

    /// Uma posicao para o selo novo que nenhuma transacao aberta ocupa, sorteada.
    int PosicaoDoSelo(const std::vector<int>& ocupadas, std::uint32_t sorteio);

    /// O que o Dificil muda: transacoes mais seguidas, com tres operacoes e menos tempo ate o COMMIT.
    struct Degrau {
        float novaTransacao = kNovaTransacao;
        int operacoes = kOperacoes;
        float entreOperacoes = kEntreOperacoes;
        float ateOCommit = kAteOCommit;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
