//
// Quem esta jogando, de cada lado do painel.
//

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Personagens.h"
#include "Progresso.h"

/**
 * CAMADA PURA (mesma disciplina de Progresso, Nota e Painel).
 *
 * Nao conhece Game, Scene nem SDL: guarda o que pertence a UM jogador e
 * responde as perguntas que surgem quando ha dois.
 *
 * POR QUE ISTO EXISTE. A matricula, o progresso e a aparencia eram tres membros
 * soltos do Game, no singular. Com duas posicoes no painel eles viram tres
 * vetores soltos - e tres vetores soltos e o comeco de um estado inconsistente:
 * basta um caminho acrescentar em dois deles e esquecer o terceiro. Agrupados,
 * ou existe a sessao inteira ou nao existe nenhuma.
 *
 * Ver Documentacao/requisitos-coop.md, RNF1.
 */
namespace Sessao {

/**
 * @brief O que pertence a UM jogador, e nao a partida.
 *
 * O que NAO esta aqui e tao importante quanto o que esta: a materia escolhida,
 * a nota da batalha em curso e o modo de entrada sao da PARTIDA, e continuam no
 * Game. A nota da batalha, em especial, e da dupla (requisitos-coop, D2).
 */
struct Dados {
    /// A forma canonica (ver Matricula.h), ou Matricula::kVisitante.
    std::string matricula;

    Progresso progresso;

    /// Vazia quer dizer "a padrao". Cada jogador tem a sua (D6).
    Personagens::Aparencia aparencia;
};

/// Quantos jogam esta partida. Um e o caso normal, e tudo tem de valer nele.
enum class Modo { Um, Dois };

[[nodiscard]] size_t Quantos(Modo modo);

/**
 * @brief A matricula ja esta em uso por OUTRA posicao?
 *
 * Duas sessoes do mesmo aluno gravariam uma por cima da outra: as duas apontam
 * para o mesmo arquivo em Saves/, e a ultima a fechar venceria. O aluno perderia
 * a batalha inteira sem nenhum aviso. Por isso a segunda tela de matricula
 * recusa a matricula da primeira (requisitos-coop, RF2).
 *
 * VISITANTE NAO CONTA, e os dois podem ser (D7): visitante nao grava arquivo
 * nenhum, entao nao ha o que um sobrescreva do outro.
 *
 * @param posicao A posicao que esta digitando agora, e que portanto NAO conta
 *                contra si mesma - voltar e redigitar a propria matricula tem
 *                de continuar valendo.
 */
[[nodiscard]] bool JaEstaEmUso(const std::vector<Dados>& sessoes, size_t posicao,
                               const std::string& matricula);

/**
 * @brief A matricula de quem jogou do OUTRO lado, para o registro de dupla.
 *
 * Devolve VAZIO quando nao ha parceiro de quem falar, e sao tres casos:
 *
 *   - jogou SOZINHO: uma sessao so, e nao ha outro lado;
 *
 *   - o parceiro era VISITANTE, que nao tem matricula. A nota continua valendo
 *     para o progresso de quem se identificou (RF6) e entra como nota de DUPLA
 *     - a batalha foi em dupla -, so sem nome de parceiro. O ranking de duplas
 *     descarta exatamente as linhas sem nome, porque "89384 + " nao diz quem
 *     jogou. As duas metades combinam aqui, e e de proposito;
 *
 *   - TRES POSICOES NAO TEM "O PARCEIRO": a palavra so faz sentido numa dupla,
 *     e nomear um dos dois seria dizer que o outro nao jogou. Hoje Modo so vai
 *     ate Dois; se um dia crescer, este e o lugar que avisa.
 */
[[nodiscard]] std::string ParceiroDe(const std::vector<Dados>& sessoes, size_t posicao);

/// O que vem depois de uma posicao terminar de entrar.
enum class Passo {
    OutroJogador,   ///< ainda falta alguem se identificar
    Jogar           ///< todos entraram
};

/**
 * @brief Quem acabou de entrar foi o ultimo?
 *
 * A REGRA MORA AQUI, e nao nas duas telas que terminam a entrada. A matricula e
 * a criacao de personagem as duas mandavam para a selecao de fases, cada uma
 * com a sua linha; com dois jogadores as duas precisariam da mesma condicao
 * nova, e bastaria uma esquecer para o jogador 2 nunca ser chamado.
 */
[[nodiscard]] Passo Proximo(Modo modo, size_t posicaoQueTerminou);

} // namespace Sessao
