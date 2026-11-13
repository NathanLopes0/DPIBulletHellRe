//
// As horas complementares: o que o aluno fez nas provas alem de passar.
//

#pragma once

#include <set>
#include <string>
#include <vector>

#include "../Curso/Materias.h"

/// As atividades complementares do curso: feitos de uma prova so (uma fase limpa, uma virada), cada
/// um valendo horas, uma vez. Umas contam por materia - e levam o nome dela, "Fase limpa em INF 213"
/// -, outras uma vez no curso. Com kMeta horas o aluno cumpriu a carga. Nao conhece batalha, disco
/// nem SDL: recebe o que a prova contou (Feito) e devolve as chaves cumpridas, que a ficha guarda.
namespace Horas {

    enum class Atividade {
        FaseLimpa,       ///< por materia: uma fase sem levar acerto
        PoucosAcertos,   ///< por materia: aprovado com ate kPoucosAcertos acertos sofridos
        Veterano,        ///< por materia: kAprovacoesDoVeterano aprovacoes nela
        Virada,          ///< aprovado depois de a nota cair abaixo de kNotaDaVirada
        DeRaspao,        ///< kRaspoes raspoes numa prova
        PontoExtra,      ///< kPontosExtras pontos extras usados numa prova
        Cobertura,       ///< kCoberturas coberturas ao parceiro numa prova em dupla
        Poder,           ///< kPoderes poderes do orientador soltos num TCC
        ProvaDaSemana,   ///< aprovado na prova da semana
        Podio            ///< no podio da prova da semana, na hora em que a fez
    };

    constexpr int kMeta = 100;
    constexpr int kPoucosAcertos = 10;
    constexpr int kAprovacoesDoVeterano = 3;
    constexpr float kNotaDaVirada = 25.0f;
    constexpr int kRaspoes = 15;
    constexpr int kPontosExtras = 2;
    constexpr int kCoberturas = 10;
    constexpr int kPoderes = 3;

    struct Dados {
        Atividade atividade = Atividade::FaseLimpa;
        const char* chave = "";         ///< o nome na ficha: "fase-limpa"
        bool porMateria = false;        ///< uma vez em cada materia ("fase-limpa@INF213"), e nao uma no curso
        int horas = 0;
        const char* titulo = "";        ///< "Fase limpa"
        const char* descricao = "";     ///< "uma fase sem levar acerto"
    };

    /// Todas, na ordem em que a tela as lista.
    const std::vector<Dados>& Todas();

    /// O que uma prova contou. So quem foi aprovado cumpre atividade.
    struct Feito {
        std::string materia;            ///< o codigo: "INF213"
        bool aprovado = false;
        bool dupla = false;
        int acertosSofridos = 0;
        int fasesLimpas = 0;
        int aprovacoesNaMateria = 0;    ///< contando esta
        float menorNota = 100.0f;       ///< a menor que a nota chegou durante a prova
        int raspoes = 0;
        int pontosExtras = 0;
        int coberturas = 0;
        int poderes = 0;
        bool provaDaSemana = false;
        int posicaoNaSemana = 0;        ///< 0: fora do ranking (o visitante)
    };

    /// A chave de uma atividade na ficha; a por materia leva o codigo.
    std::string Chave(Atividade a, const std::string& materia);

    /// As chaves que esta prova cumpriu (as ja cumpridas tambem: quem registra descarta as repetidas).
    std::vector<std::string> Cumpridas(const Feito& f);

    /// As horas de um conjunto de chaves. A chave que nao existe (mais) vale zero.
    int De(const std::set<std::string>& chaves);

    /// As horas que o curso inteiro oferece.
    int Possiveis(const Materias::Lista& materias);

    // ---- As horas como moeda: as optativas se compram com elas ----
    //
    // A carga (kMeta) conta todas as horas ganhas, e nunca desce. O saldo e o que sobra para gastar:
    // as ganhas menos o preco das optativas compradas (Materias::ChaveDaCompra).

    /// As horas gastas: a soma dos precos das optativas que estas chaves compraram.
    int Gastas(const std::set<std::string>& chaves, const Materias::Lista& materias);
    /// As horas que o aluno ainda pode gastar (nunca menos que zero).
    int Saldo(const std::set<std::string>& chaves, const Materias::Lista& materias);

    /// Se este aluno pode comprar esta materia agora, e se nao, por que.
    enum class Compra {
        NaoSeVende,    ///< sem preco: abre so com a regra
        JaComprou,
        FaltaARegra,   ///< as aprovacoes que ela pede ainda nao vieram
        FaltaHora,     ///< a regra esta cumprida, mas o saldo nao chega no preco
        Pode
    };
    Compra PodeComprar(const Materias::Lista& materias, int indice, const Progresso& progresso);

    /// "Fase limpa em INF 213", "Virada": o nome de uma chave para a tela, com o nome da materia.
    /// Vazio para a chave que nao existe.
    std::string Nome(const std::string& chave, const Materias::Lista& materias);

    /// "Horas complementares: +5 h (Fase limpa em INF 213, Virada)"; vazio sem nenhuma.
    std::string FraseDasNovas(const std::vector<std::string>& novas, const Materias::Lista& materias);

    /// Uma linha do quadro de atividades: quantas vezes foi cumprida, de quantas da.
    struct Linha {
        Dados dados;
        int cumpridas = 0;
        int possiveis = 1;
    };
    std::vector<Linha> Quadro(const std::set<std::string>& chaves, const Materias::Lista& materias);
}
