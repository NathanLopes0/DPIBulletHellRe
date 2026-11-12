//
// A tabela hash da fase 2 do Salles.
//

#pragma once

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

/// A fase 2 do Salles e uma tabela hash com encadeamento, desenhada no alto do campo.
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

    /// H(k) = k mod m.
    int Balde(int chave, int m);

    /// O tamanho seguinte da tabela; no ultimo, fica.
    int ProximoTamanho(int m);

    /// A tabela precisa crescer? Mais chaves que baldes.
    bool PrecisaDeRehash(int chaves, int m);

    /// O x do meio do balde i, numa tabela de m baldes de lado a lado.
    float CentroDoBalde(int i, int m, float largura);

    /// O y da chave na posicao p da lista (0 e a primeira, colada no balde).
    float AlturaNaLista(int p);

    enum class Tipo { Inserir, Buscar };

    struct Operacao {
        Tipo tipo = Tipo::Inserir;
        int chave = 0;
    };

    /// Os placares: "h(23) = 23 mod 7 = 2", "busca(23): achou", "busca(41): NULL", "rehash: m = 7 →
    /// 11".
    std::string PlacarDaInsercao(int chave, int m);
    std::string PlacarDaBusca(int chave, bool achou);
    std::string PlacarDoRehash(int de, int para);

    /// As caras: a chave k e "K23", o balde i e "B2".
    std::string CaraDaChave(int chave);
    std::string CaraDoBalde(int balde);

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// A tabela desta luta: com quantos baldes ela abre (7 ou 11 - o rehash segue a lista de
    /// kTamanhos dali) e a chance de uma insercao (de 0,45 a 0,65.
    struct Plano {
        int tamanhoInicial = kTamanhos[0];
        float chanceDeInserir = kChanceDeInserir;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// A proxima operacao. Com a tabela vazia, insere.
    Operacao Proxima(const std::vector<int>& naTabela, std::uint32_t sorteio, const Plano& p = Plano{});

    /// O que o Dificil muda: uma operacao a cada 0,55 s, e o leque do
    /// percurso e o NULL andam 25% mais depressa.
    struct Degrau {
        float passo = kPasso;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
