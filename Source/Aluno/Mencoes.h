//
// As mencoes de cada materia: o que o aluno ja conquistou nela, alem de passar.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "Progresso.h"
#include "../Curso/Materias.h"

/// Passar na materia abre a seguinte; as mencoes sao o que ainda ha para buscar nela depois disso -
/// uma nota maior, o Dificil, a dupla, o professor dela orientando o TCC. Saem todas do que a
/// ficha ja guarda (Progresso): nao conhecem tela, disco nem SDL.
namespace Mencoes {

    enum class Mencao {
        Bronze,      ///< aprovado
        Prata,       ///< a melhor nota chegou a kNotaDaPrata
        Ouro,        ///< ...a kNotaDoOuro
        NotaCheia,   ///< ...a 100 (Nota::ECheia)
        Dificil,     ///< aprovado no Dificil, sozinho ou em dupla
        Dupla,       ///< aprovado em dupla, em qualquer dificuldade
        Orientou     ///< o professor da materia orientou um TCC aprovado do aluno
    };
    constexpr int kQuantas = 7;
    constexpr std::array<Mencao, kQuantas> kTodas{Mencao::Bronze, Mencao::Prata, Mencao::Ouro, Mencao::NotaCheia,
                                                  Mencao::Dificil, Mencao::Dupla, Mencao::Orientou};

    constexpr float kNotaDaPrata = 80.0f;
    constexpr float kNotaDoOuro = 90.0f;

    /// As mencoes de uma materia: as que existem nela e as que o aluno tem.
    struct DaMateria {
        std::array<bool, kQuantas> existe{};
        std::array<bool, kQuantas> tem{};

        [[nodiscard]] bool Existe(const Mencao m) const { return existe[static_cast<size_t>(m)]; }
        [[nodiscard]] bool Tem(const Mencao m) const { return tem[static_cast<size_t>(m)]; }
        [[nodiscard]] int Ganhas() const;
        [[nodiscard]] int Possiveis() const;
    };

    /// As mencoes da materia 'materia' para quem tem este progresso. A materia sem batalha nao tem
    /// nenhuma; a que pede orientador (o TCC) nao tem a de orientar.
    DaMateria Da(const Progresso& progresso, const Materias::Lista& materias, int materia);

    /// O curso inteiro: quantas o aluno tem, de quantas existem.
    struct Total {
        int ganhas = 0;
        int possiveis = 0;
    };
    Total DoCurso(const Progresso& progresso, const Materias::Lista& materias);

    /// As que 'depois' tem e 'antes' nao tinha: o que uma batalha acabou de dar.
    std::vector<Mencao> Novas(const DaMateria& antes, const DaMateria& depois);

    /// O que vai escrito no selo: "B", "P", "O", "100", "D", "2", "T".
    const char* Selo(Mencao m);

    /// O nome, para a frase do resultado: "bronze", "prata", "ouro", "nota cheia", "no Dificil",
    /// "em dupla", "orientou o TCC".
    const char* Nome(Mencao m);

    /// A legenda do boletim, numa linha.
    std::string Legenda();

    /// A cor do selo.
    struct Cor {
        unsigned char r = 255;
        unsigned char g = 255;
        unsigned char b = 255;
    };
    Cor CorDa(Mencao m);

    /// "Nova menção: prata" / "Novas menções: ouro, em dupla"; vazio sem nenhuma.
    std::string FraseDasNovas(const std::vector<Mencao>& novas);
}
