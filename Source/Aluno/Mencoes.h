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
        Orientou,    ///< o professor da materia orientou um TCC aprovado do aluno
        // ---- Os desafios: aprovado jogando de um jeito diferente (Desafios) ----
        // Os nomes dos tres de baixo ficaram da primeira versao, e continuam nas chaves da ficha
        // (desafio-pacifista, -mira, -de-perto): o aluno ve os de Nome.
        Pacifista,   ///< "quietinho da turma": aprovado sem fazer nenhuma pergunta
        Intocavel,   ///< aprovado sem ser acertado nenhuma vez
        Mira,        ///< "prestando atencao": kPrecisaoDaMira das duvidas acertando o professor
        DePerto      ///< "sentar na frente": kFracaoDePerto dos acertos feitos de perto (o "!!")
    };
    constexpr int kQuantas = 11;
    constexpr std::array<Mencao, kQuantas> kTodas{Mencao::Bronze,   Mencao::Prata,     Mencao::Ouro,
                                                  Mencao::NotaCheia, Mencao::Dificil,  Mencao::Dupla,
                                                  Mencao::Orientou,  Mencao::Pacifista, Mencao::Intocavel,
                                                  Mencao::Mira,      Mencao::DePerto};

    /// Os desafios, na ordem dos selos.
    constexpr std::array<Mencao, 4> kDesafios{Mencao::Pacifista, Mencao::Intocavel, Mencao::Mira, Mencao::DePerto};
    /// Mira certeira: a fracao dos tiros que acertou o professor, com pelo menos kTirosDaMira tiros.
    constexpr float kPrecisaoDaMira = 0.70f;
    constexpr int kTirosDaMira = 50;
    /// Na cara do professor: a fracao dos acertos feitos de perto, com pelo menos kAcertosDePerto acertos.
    constexpr float kFracaoDePerto = 0.5f;
    constexpr int kAcertosDePerto = 50;

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

    // ---- Os desafios ----
    // As outras mencoes saem dos recordes; os desafios, de como a prova foi jogada - por isso a batalha
    // conta e a ficha guarda, como uma atividade (Progresso::RegistrarAtividade): "desafio-pacifista@INF213".

    /// O que a prova contou, da dupla inteira: o que os desafios perguntam.
    struct Prova {
        bool aprovado = false;
        int tiros = 0;               ///< os tiros dos alunos (os do orientador nao contam)
        int acertosNoProfessor = 0;  ///< os tiros dos alunos que acertaram o professor
        int acertosDePerto = 0;      ///< deles, os feitos de perto (o "!!")
        int acertosSofridos = 0;
    };
    /// A chave do desafio na ficha.
    std::string Chave(Mencao desafio, const std::string& materia);
    /// As chaves dos desafios que esta prova cumpriu, na materia de codigo 'materia'.
    std::vector<std::string> Desafios(const Prova& p, const std::string& materia);

    /// O que vai escrito no selo: "B", "P", "O", "100", "D", "2", "T", "Q", "I", "A", "F".
    const char* Selo(Mencao m);

    /// O nome, para a frase do resultado e o manual: "bronze", "prata", ..., "quietinho da turma",
    /// "intocavel", "prestando atencao", "sentar na frente".
    const char* Nome(Mencao m);

    /// Como se ganha, numa frase curta, para a pagina das mencoes do manual.
    const char* ComoGanhar(Mencao m);

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
