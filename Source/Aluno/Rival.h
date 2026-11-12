//
// O rival da turma: quem esta logo acima no ranking da materia.
//
#pragma once

#include <string>
#include <vector>

#include "Exportacao.h"
#include "Progresso.h"
#include "Ranking.h"

/// Uma meta proxima puxa mais que o primeiro lugar: o cartao da materia mostra quem esta logo acima
/// do aluno, e quanto falta para passa-lo; a tela do resultado diz quem ele passou. Puro: as notas
/// dos colegas chegam prontas (Ranking.h), e nada aqui le disco.
namespace Rival {

    /// Um colega no ranking: a matricula (na dupla, "90112 + 88001") e o recorde dele.
    struct Colega {
        std::string quem;
        float nota = 0.0f;
    };

    enum class Tipo {
        Sozinho,    ///< ninguem mais jogou esta materia
        Estreante,  ///< o aluno nunca jogou: o alvo e o ultimo da turma
        Atras,      ///< ha alguem acima: o alvo e o mais perto dele
        Lider,      ///< ninguem acima: o alvo e quem vem logo atras
    };

    struct Situacao {
        Tipo tipo = Tipo::Sozinho;
        Colega alvo;
        int posicaoDoAlvo = 0;   ///< 1 para o primeiro; empate repete a posicao, como no Ranking
        float distancia = 0.0f;  ///< Atras: quanto falta; Lider: a vantagem; nos outros, 0
    };

    /// Onde o aluno, com 'nota' (0 = nunca jogou), esta em relacao aos 'colegas' - todos menos ele.
    Situacao Da(const std::vector<Colega>& colegas, float nota);

    /// Os colegas que uma nota nova passou: os que tinham mais que 'antes' e menos que 'depois',
    /// do maior para o menor. Empatar nao e passar.
    std::vector<Colega> Passados(const std::vector<Colega>& colegas, float antes, float depois);

    /// A posicao que a nota teria entre os colegas: 1 + quantos tem mais que ela.
    int PosicaoCom(const std::vector<Colega>& colegas, float nota);

    /// A linha do cartao da materia: "Rival: 90112 (2\xc2\xba) \xc2\xb7 faltam 2.41".
    std::string Frase(const Situacao& s);

    /// A linha da tela do resultado, com o recorde de 'antes' e o de 'depois' desta luta (o maior dos
    /// dois). Vazia quando nao ha o que dizer: ninguem passado, e o rival a mais de kPerto.
    /// Quem nao entra no ranking (o visitante) le no condicional: "passaria", "seria".
    std::string FraseDoResultado(const std::vector<Colega>& colegas, float antes, float depois,
                                 bool entraNoRanking = true);

    /// Os colegas de um ranking de solo, sem o proprio aluno.
    std::vector<Colega> DoRanking(const std::vector<Ranking::Linha>& linhas, const std::string& eu);

    /// Os colegas de um ranking de duplas, sem a dupla do aluno (em qualquer ordem).
    std::vector<Colega> DoRanking(const std::vector<Ranking::LinhaDeDupla>& linhas, const std::string& um,
                                  const std::string& dois);

    /// A turma de uma materia para quem esta jogando: os colegas (sem ele, ou sem a dupla dele) e o
    /// recorde dele, no modo e no nivel da lista. 'alunos' sao as matriculas em jogo (uma ou duas);
    /// 'progresso' e o do primeiro, o da sessao - do visitante tambem, que nao tem ficha.
    struct Turma {
        std::vector<Colega> colegas;
        float nota = 0.0f;
    };
    Turma DaTurma(const std::vector<Exportacao::FichaDeAluno>& fichas, int materia, Dificuldade::Nivel nivel,
                  const std::vector<std::string>& alunos, const Progresso& progresso);

    /// Ate quanto abaixo do alvo a frase do resultado ainda diz "faltaram": mais longe que isso, a
    /// nota nao chegou perto, e a tela nao fala do rival.
    constexpr float kPerto = 5.0f;
}
