//
// A prova da semana: uma materia por semana, com a mesma luta para todo mundo.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../Curso/Materias.h"

/// Toda semana uma materia do curso e a prova da semana: quem a joga naquela semana enfrenta a
/// mesma semente (Battle::SementeDaBatalha) - a mesma ordem, os mesmos numeros, os mesmos tempos - e
/// a nota entra num ranking que so vale ate domingo. Na segunda a materia e a semente trocam.
/// Aqui moram a conta da semana, o arquivo em texto e a ordem; nao conhece disco, tela nem SDL.
namespace ProvaDaSemana {

    /// A semana de um instante: comeca na segunda-feira.
    struct Semana {
        int indice = -1;            ///< as semanas contadas desde a segunda 1970-01-05; -1: data ilegivel
        std::string segunda;        ///< "2026-11-09": o nome dela no arquivo
        int faltam = 0;             ///< dias ate trocar, contando hoje (7 na segunda, 1 no domingo)

        [[nodiscard]] bool Valida() const { return indice >= 0; }
    };

    /// A semana de "2026-11-10 14:30:00" (Relogio::Agora). So a data conta.
    Semana Da(const std::string& quando);

    /// "09/11", para a tela.
    std::string DiaDaSegunda(const Semana& s);

    /// A semente da luta dessa semana.
    std::uint32_t Semente(const Semana& s);

    /// A materia da semana: as obrigatorias que tem batalha e nao pedem orientador (o TCC e as
    /// optativas ficam de fora), em rodizio na ordem do curso.
    /// -1 se nao ha nenhuma, ou se a semana nao vale.
    int MateriaDa(const Semana& s, const Materias::Lista& materias);

    /// Uma nota na prova da semana. Na dupla, uma entrada por aluno, cada uma com o parceiro.
    struct Entrada {
        std::string semana;         ///< Semana::segunda
        std::string materia;        ///< o codigo
        bool dupla = false;
        bool dificil = false;
        std::string matricula;
        std::string parceiro;
        float nota = 0.0f;
        std::string quando;
    };

    /// O arquivo: uma linha por entrada, separada por ';', com cabecalho.
    std::string Cabecalho();
    std::string Linha(const Entrada& e);
    /// Le o arquivo inteiro; a linha que nao serve e pulada.
    std::vector<Entrada> Ler(const std::string& texto);

    /// Uma linha do ranking: a melhor nota de cada um na semana.
    struct Colocado {
        int posicao = 1;            ///< empate repete a posicao
        std::string quem;           ///< a matricula; na dupla, "a + b", a menor primeiro
        float nota = 0.0f;
    };

    /// Quem uma entrada representa no ranking (Colocado::quem).
    std::string Quem(const Entrada& e);

    /// O ranking de uma semana, num modo e numa dificuldade, da maior nota para a menor.
    std::vector<Colocado> Ranking(const std::vector<Entrada>& entradas, const Semana& s, bool dupla, bool dificil);

    /// A posicao de 'quem' no ranking; 0 se nao esta.
    int PosicaoDe(const std::vector<Colocado>& ranking, const std::string& quem);

    /// Quantos primeiros lugares fazem o podio.
    constexpr int kPodio = 3;
}
