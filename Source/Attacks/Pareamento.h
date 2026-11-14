//
// A fase 2 da Sabrina: o codigo - as bases, o pareamento, a transcricao e a traducao.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A fase 2 da Sabrina e a aula do codigo, e nela o aluno CARREGA UMA BASE (o selo ao lado dele):
/// todo tiro da fase e uma base, e a que pareia com a do aluno (A com T, C com G; no RNA, A com U)
/// nao machuca - encaixa nele, pelas pontes de hidrogenio, e ainda vale um pouco de nota. As outras
/// machucam. As capsulas de nucleotideo livre giram embaixo; encostar numa troca a base do aluno:
///   - no pareamento, a professora anuncia a base da proxima onda, e a onda e quase toda dela: quem
///     pega a base que pareia atravessa a onda e a absorve;
///   - na transcricao, a polimerase le a fita molde de DNA no alto, base a base, e solta o RNA: o
///     complementar, com U no lugar de T (o selo de T vira U). Quem le a fita molde a frente da
///     polimerase sabe o que vem - e a base que pareia com o RNA e a propria base do molde;
///   - na traducao, o ribossomo le o RNA de tres em tres: cada codon e um aminoacido, e a familia
///     dele decide como as tres bases voam (hidrofobico vem reto e rapido, polar abre devagar,
///     positivo persegue, negativo espalha em anel). O codon de parada solta a cadeia, que cai.
namespace Pareamento {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateTwo";
    constexpr std::array<const char*, 3> kEtapas = {"Pareamento", "Transcricao", "Traducao"};
    constexpr std::array<float, 3> kDuracoes = {6.0f, 7.0f, 7.0f};
    constexpr const char* kGatilhoDaOnda = "onda";
    constexpr const char* kGatilhoDoRna = "rna";
    constexpr const char* kGatilhoDoCodon = "codon";
    constexpr const char* kGatilhoDaCadeia = "cadeia";
    constexpr const char* kGatilhoDaCapsula = "capsula";
    constexpr const char* kGatilhoDoSelo = "selo";
    constexpr const char* kGatilhoDaLetra = "letra";
    constexpr const char* kGatilhoDaMaquina = "maquina";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- As bases ----

    enum class Base { A, C, G, T, U };
    constexpr std::array<Base, 4> kDna = {Base::A, Base::C, Base::G, Base::T};
    constexpr std::array<Base, 4> kRna = {Base::A, Base::C, Base::G, Base::U};

    char Letra(Base b);
    /// O nome da animacao da base (a letra): "A", "C", "G", "T" ou "U".
    const char* Cara(Base b);
    /// As pontes de hidrogenio do par: duas no A-T (e no A-U), tres no C-G.
    int Pontes(Base b);
    /// A base que pareia com 'b': no DNA, A com T; no RNA, A com U; sempre C com G.
    Base Complementar(Base b, bool rna);
    bool Pareiam(Base a, Base b);
    /// O DNA tem T, o RNA tem U: a mesma base, no alfabeto de cada um.
    Base NoAlfabeto(Base b, bool rna);

    /// A classe do aluno que carrega 'b' (Player::DefinirClasse): T e U sao a mesma (0 a 3).
    int Classe(Base b);
    /// A classe do tiro que e a base 'b': a da base que pareia com ele. O tiro da classe do aluno
    /// passa por ele (Colisoes) - e o aluno o absorve.
    int ClasseDoTiro(Base b);
    /// A base da classe 'c' (0 a 3), no alfabeto do DNA ou do RNA.
    Base DaClasse(int c, bool rna);

    /// Quanto vale cada base absorvida.
    constexpr float kPremioDoPar = 0.3f;

    // ---- As sequencias ----

    /// Uma base qualquer do alfabeto, diferente de 'menos'.
    Base Outra(Variacao::Sorteio& s, Base menos, bool rna);
    /// 'n' bases, cada uma repetindo a anterior com chance 'repetir' (as corridas, como GGG, sao o
    /// que da para absorver de uma vez).
    std::vector<Base> Sequencia(Variacao::Sorteio& s, int n, float repetir, bool rna);
    /// Uma onda de 'n' bases: 'fracao' delas sao a 'dominante', o resto outras, embaralhadas.
    std::vector<Base> Onda(Variacao::Sorteio& s, Base dominante, int n, float fracao, bool rna);

    // ---- A transcricao ----

    /// O RNA que a polimerase escreve lendo a base 'molde' do DNA: A vira U, T vira A, C vira G e G
    /// vira C.
    Base Transcrever(Base molde);
    /// A fita molde de DNA que se transcreve em 'rna'.
    std::vector<Base> Molde(const std::vector<Base>& rna);

    // ---- A traducao ----

    /// O aminoacido do codon (o codigo genetico padrao), pela letra: 'M' e a metionina, '*' a
    /// parada. Aceita T no lugar de U.
    char Traduzir(Base b1, Base b2, Base b3);
    enum class Familia { Hidrofobico, Polar, Positivo, Negativo, Parada };
    Familia FamiliaDe(char aminoacido);
    /// O nome de tres letras: "Met", "Ala"... e "Pare" para a parada.
    const char* NomeDo(char aminoacido);
    const char* NomeDa(Familia f);
    /// Um gene de 'codons' codons em RNA: AUG, os do meio (nenhum de parada) e uma parada no fim.
    std::vector<Base> Gene(Variacao::Sorteio& s, int codons);

    // ---- As capsulas ----

    /// Onde a capsula 'k' de 'n' esta no instante 't': na elipse de 'raios' em volta de 'centro',
    /// todas igualmente espacadas, girando 'rapidez' radianos por segundo.
    Vector2 NaOrbita(int k, int n, float t, Vector2 centro, Vector2 raios, float rapidez);

    // ---- O ritmo ----

    struct Degrau {
        // As capsulas
        float rapidezDaOrbita = 0.42f;
        float toqueDaCapsula = 28.0f;    ///< o raio do toque que troca a base
        float voltaDaCapsula = 1.0f;     ///< o tempo ate ela voltar, depois de pega
        // O pareamento
        float entreOndas = 1.6f;
        float avisoDaOnda = 1.2f;
        int linhasDaOnda = 2;
        float entreLinhas = 30.0f;
        int basesPorLinha = 20;
        float fracaoDominante = 0.8f;
        float velocidadeDaOnda = 150.0f;
        // A transcricao
        int basesDoMolde = 24;
        float porBase = 0.28f;
        int tirosPorBase = 2;
        float aberturaPorBase = 26.0f;
        float velocidadeDoRna = 190.0f;
        float repetir = 0.45f;
        // A traducao
        float porCodon = 0.62f;
        int tirosPorBaseNaTraducao = 2;
        float velocidadeHidrofobica = 270.0f;
        float velocidadePolar = 145.0f;
        float aberturaPolar = 70.0f;
        float velocidadePositiva = 170.0f;
        float giroPositivo = 1.9f;       ///< radianos por segundo: o teleguiado
        int anelNegativo = 9;
        float velocidadeNegativa = 160.0f;
        float quedaDaCadeia = 210.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
