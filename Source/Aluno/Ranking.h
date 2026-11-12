//
// A ordem das melhores notas de uma materia.
//

#pragma once

#include <string>
#include <vector>

#include "Exportacao.h"
#include "Progresso.h"
#include "../Regras/Dificuldade.h"

/// So a ordenacao; nao conhece disco, tela nem SDL.
namespace Ranking {

    struct Linha {
        int posicao = 1;            ///< 1 para o primeiro. Empate repete a posicao.
        std::string matricula;
        float nota = 0.0f;

        /// Quantas materias do curso o aluno jogou.
        int materiasJogadas = 1;
    };

    /// As melhores notas de uma materia, da maior para a menor. Quem nunca jogou a materia nao
    /// entra.
    std::vector<Linha> DaMateria(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                 int materia, Dificuldade::Nivel nivel, size_t quantas = 0);

    /// A media do aluno sobre o curso inteiro, da maior para a menor. A media divide pelo numero
    /// total de materias do curso, e nao pelas que o aluno jogou.
    std::vector<Linha> Geral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                             int quantasMaterias, Dificuldade::Nivel nivel, size_t quantas = 0);

    /// A posicao de uma matricula no ranking geral. Zero se ela nao jogou nada.
    int PosicaoNoGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                       int quantasMaterias, Dificuldade::Nivel nivel, const std::string& matricula);

    /// Em que posicao esta uma matricula, na lista completa da materia. Devolve 0 quando ela nao
    /// jogou.
    int PosicaoDe(const std::vector<Exportacao::FichaDeAluno>& fichas,
                  int materia, Dificuldade::Nivel nivel, const std::string& matricula);

    // ---- O ranking de duplas (requisitos-coop, D9) ----

    /// Uma linha do ranking de duplas.
    struct LinhaDeDupla {
        int posicao = 1;        ///< 1 para a primeira. Empate repete a posicao.
        std::string um;         ///< a matricula que vem antes
        std::string dois;       ///< a outra
        float nota = 0.0f;

        /// Quantas materias esta dupla jogou junta. So tem sentido no geral,
        /// pelo mesmo motivo de Linha::materiasJogadas.
        int materiasJogadas = 1;
    };

    /// As melhores notas de dupla numa materia, da maior para a menor. Sai das mesmas fichas.
    std::vector<LinhaDeDupla> DuplasDaMateria(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                              int materia, Dificuldade::Nivel nivel, size_t quantas = 0);

    /// A media das duplas sobre o curso inteiro.
    std::vector<LinhaDeDupla> DuplasGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                          int quantasMaterias, Dificuldade::Nivel nivel, size_t quantas = 0);

    /// Em que posicao esta uma dupla, na lista completa da materia. Devolve 0 quando os dois nunca
    /// jogaram esta materia juntos.
    int PosicaoDaDupla(const std::vector<Exportacao::FichaDeAluno>& fichas, int materia,
                       Dificuldade::Nivel nivel, const std::string& um, const std::string& dois);

    /// A posicao de uma dupla no geral. Zero se eles nunca jogaram juntos.
    int PosicaoDaDuplaNoGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                              int quantasMaterias, Dificuldade::Nivel nivel,
                              const std::string& um, const std::string& dois);

    // ---- As quatro listas ----

    /// Uma das quatro listas: o modo e o nivel em que as notas foram feitas. Quatro, e nao duas.
    struct Lista {
        Progresso::Modo modo;
        Dificuldade::Nivel nivel;
    };

    /// As listas, na ordem das abas da tela: primeiro os dois niveis do solo,
    /// depois os da dupla.
    constexpr Lista kListas[] = {
        {Progresso::Modo::Solo,  Dificuldade::Nivel::Normal},
        {Progresso::Modo::Solo,  Dificuldade::Nivel::Dificil},
        {Progresso::Modo::Dupla, Dificuldade::Nivel::Normal},
        {Progresso::Modo::Dupla, Dificuldade::Nivel::Dificil},
    };
    constexpr int kQuantasListas = static_cast<int>(sizeof(kListas) / sizeof(kListas[0]));

    /// "Normal", "Dificil", "Normal em dupla", "Dificil em dupla" - com o i
    /// agudo, em UTF-8.
    std::string NomeDaLista(Lista l);

    /// A posicao da lista de um modo e nivel em kListas.
    int IndiceDaLista(Progresso::Modo modo, Dificuldade::Nivel nivel);

    /// A lista 'passo' abas depois da 'atual', dando a volta nas pontas.
    int MoverLista(int atual, int passo);
}
