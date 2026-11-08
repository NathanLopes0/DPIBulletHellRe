//
// Quem esta jogando, de cada lado do painel.
//

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../Personagem/Personagens.h"
#include "Progresso.h"

/// Nao conhece Game, Scene nem SDL: guarda o que pertence a um jogador e responde as perguntas que
/// surgem quando ha dois.
namespace Sessao {

/// O que pertence a um jogador, e nao a partida.
struct Dados {
    /// A forma canonica, ou Matricula::kVisitante.
    std::string matricula;

    Progresso progresso;

    /// Vazia quer dizer "a padrao". Cada jogador tem a sua.
    Personagens::Aparencia aparencia;
};

/// Quantos jogam esta partida. Um e o caso normal, e tudo tem de valer nele.
enum class Modo { Um, Dois };

[[nodiscard]] size_t Quantos(Modo modo);

/// A matricula ja esta em uso por outra posicao?
[[nodiscard]] bool JaEstaEmUso(const std::vector<Dados>& sessoes, size_t posicao,
                               const std::string& matricula);

/// A matricula de quem jogou do outro lado, para o registro de dupla.
[[nodiscard]] std::string ParceiroDe(const std::vector<Dados>& sessoes, size_t posicao);

/// O que vem depois de uma posicao terminar de entrar.
enum class Passo {
    OutroJogador,   ///< ainda falta alguem se identificar
    Jogar           ///< todos entraram
};

/// Quem acabou de entrar foi o ultimo? A regra mora aqui, e nao nas duas telas que terminam a
/// entrada.
[[nodiscard]] Passo Proximo(Modo modo, size_t posicaoQueTerminou);

} // namespace Sessao
