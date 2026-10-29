//
// A tabela hash da fase 2 do Salles.
//

#pragma once

#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

/**
 * CAMADA PURA.
 *
 * A FASE 2 DO SALLES E UMA TABELA HASH COM ENCADEAMENTO, desenhada no alto do
 * campo: m baldes lado a lado, e de cada balde pende a lista das chaves que
 * cairam nele. A funcao e a da disciplina, h(k) = k mod m, e o placar a escreve
 * por extenso a cada operacao - "h(23) = 23 mod 7 = 2" -, entao o aluno sabe
 * em que balde cada coisa vai acontecer.
 *
 * INSERIR: a chave sai do professor e vai para o fim da lista do balde dela.
 * Duas chaves no mesmo balde sao uma COLISAO, e a lista cresce para baixo.
 *
 * BUSCAR: o professor percorre a lista do balde, no por no, e cada no lido
 * atira um leque no aluno - uma lista longa, de muitas colisoes, e uma
 * rajada longa. Se ACHA a chave, ela sai da tabela atirada no aluno; se NAO ACHA,
 * o fim da lista - o NULL - cai pela coluna inteira.
 *
 * REHASH: com mais chaves que baldes (fator de carga acima de 1), a tabela
 * cresce para o proximo tamanho de kTamanhos, e TODA chave muda de balde: h(k)
 * agora e k mod m', e as listas se refazem na frente do aluno.
 *
 * O ALUNO REMOVE A TIRO: a chave pendurada e derrubavel, e o tiro que sobe
 * acerta primeiro o fim da lista.
 *
 * Quem desenha, move e atira e o Salles; aqui ficam a funcao, os tamanhos, a
 * proxima operacao, o lugar de cada chave e os textos. Coberto por
 * test_hash.cpp.
 */
namespace Hash {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoHash = "StateTwo";
    constexpr const char* kPostoDosBaldes = "baldes";
    constexpr const char* kPostoDasChaves = "chaves";
    constexpr const char* kGatilhoDosBaldes = "baldes";
    constexpr const char* kGatilhoDaChave = "chave";
    constexpr const char* kGatilhoDoPercurso = "percorrer";
    constexpr const char* kGatilhoDoNulo = "nulo";

    /// Os tamanhos da tabela, primos, na ordem em que o rehash os usa.
    constexpr std::array<int, 4> kTamanhos = {7, 11, 13, 17};

    /// O relogio: a primeira operacao, uma a cada kPasso, e o intervalo entre
    /// dois nos lidos numa busca.
    constexpr float kPrimeira = 1.2f;
    constexpr float kPasso = 0.7f;
    constexpr float kEntreNos = 0.16f;

    /// A chance de uma operacao ser insercao; e, numa busca, de a chave buscada
    /// estar na tabela.
    constexpr float kChanceDeInserir = 0.55f;
    constexpr float kChanceDeAchar = 0.5f;

    /// Onde fica a tabela: a fileira dos baldes, e a lista que pende de cada um.
    constexpr float kYDosBaldes = 205.0f;
    constexpr float kPrimeiraChave = 48.0f;
    constexpr float kEntreChaves = 42.0f;

    /// As chaves vao de 0 a kMaiorChave.
    constexpr int kMaiorChave = 99;

    /// @brief h(k) = k mod m.
    int Balde(int chave, int m);

    /// @brief O tamanho seguinte da tabela; no ultimo, fica.
    int ProximoTamanho(int m);

    /// @brief A tabela precisa crescer? Mais chaves que baldes.
    bool PrecisaDeRehash(int chaves, int m);

    /// @brief O x do meio do balde i, numa tabela de m baldes de lado a lado.
    float CentroDoBalde(int i, int m, float largura);

    /// @brief O y da chave na posicao p da lista (0 e a primeira, colada no balde).
    float AlturaNaLista(int p);

    enum class Tipo { Inserir, Buscar };

    struct Operacao {
        Tipo tipo = Tipo::Inserir;
        int chave = 0;
    };

    /// @brief Os placares: "h(23) = 23 mod 7 = 2", "busca(23): achou",
    /// "busca(41): NULL", "rehash: m = 7 → 11".
    std::string PlacarDaInsercao(int chave, int m);
    std::string PlacarDaBusca(int chave, bool achou);
    std::string PlacarDoRehash(int de, int para);

    /// @brief As caras: a chave k e "K23", o balde i e "B2".
    std::string CaraDaChave(int chave);
    std::string CaraDoBalde(int balde);

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA, E O DEGRAU -------------------

    /// A tabela desta luta: com quantos baldes ela abre (7 ou 11 - o rehash
    /// segue a lista de kTamanhos dali) e a chance de uma insercao (de 0,45 a
    /// 0,65: com menos, ha mais busca; com mais, o rehash chega antes).
    struct Plano {
        int tamanhoInicial = kTamanhos[0];
        float chanceDeInserir = kChanceDeInserir;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// @brief A proxima operacao. Com a tabela vazia, insere. Numa busca, com
    /// chance kChanceDeAchar a chave buscada e uma de 'naTabela'; senao, uma que
    /// nao esta. Toda chave inserida e nova: nao ha duas iguais na tabela.
    Operacao Proxima(const std::vector<int>& naTabela, std::uint32_t sorteio, const Plano& p = Plano{});

    /// O que o Dificil muda: uma operacao a cada 0,55 s, e o leque do
    /// percurso e o NULL andam 25% mais depressa.
    struct Degrau {
        float passo = kPasso;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
