//
// A fase 2 do Goulart: a memoria paginada.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Fase.h"

/// A fase 2 do Goulart e a paginacao. O processo que roda referencia enderecos virtuais, e a MMU os
/// traduz: os bits de cima sao a pagina, os de baixo o deslocamento - o placar mostra a conta. A
/// tabela de paginas fica a esquerda (a entrada valida aponta a moldura), e a memoria fisica e um
/// relogio de molduras no meio do campo:
///   - a pagina que esta numa moldura e um acerto: a moldura acende (o bit R) e atira;
///   - a que nao esta e uma falta de pagina: o ponteiro do relogio varre as molduras - quem tem R
///     ligado ganha a segunda chance (o R apaga), e a primeira com R apagado e a vitima. A pagina
///     vitima e despejada no aluno, e a nova vem do disco para o lugar dela.
/// As etapas sao a localidade (o conjunto de trabalho cabe na memoria: quase so acerto), a mudanca
/// de fase do programa (o conjunto troca: uma rajada de faltas) e o thrashing (o conjunto nao cabe:
/// quase toda referencia e falta, e as vitimas enchem o campo).
namespace Paginacao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateTwo";
    constexpr std::array<const char*, 3> kEtapas = {"Localidade", "Mudanca", "Thrashing"};
    constexpr std::array<float, 3> kDuracoes = {8.0f, 5.0f, 7.0f};
    constexpr const char* kGatilhoDaMoldura = "moldura";
    constexpr const char* kGatilhoDaEntrada = "entrada";
    constexpr const char* kGatilhoDoAcesso = "acesso";
    constexpr const char* kGatilhoDaVitima = "vitima";
    constexpr const char* kGatilhoDaPagina = "pagina";
    constexpr const char* kGatilhoDaFalta = "falta";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- A traducao ----

    /// Paginas de 4 KiB (12 bits de deslocamento) e oito paginas virtuais: a tabela tem oito
    /// entradas, e o endereco virtual, 15 bits.
    constexpr int kBitsDoDeslocamento = 12;
    constexpr int kPaginas = 8;

    struct Endereco {
        int pagina = 0;
        int deslocamento = 0;
    };
    Endereco Traduzir(std::uint32_t virtual_);
    std::uint32_t Montar(int pagina, int deslocamento);

    /// O numero em hexadecimal, com 'digitos' algarismos: Hexa(0x3A7F, 4) e "0x3A7F".
    std::string Hexa(std::uint32_t valor, int digitos);

    /// O placar de cada referencia: "0x3A7F = página 3 + 0xA7F  ·  acerto na moldura 2",
    /// "0x5A07 = página 5 + 0xA07  ·  falta! sai a página 3" (ou "vai para a moldura 1", livre).
    std::string PlacarDoAcerto(std::uint32_t endereco, int moldura);
    std::string PlacarDaFalta(std::uint32_t endereco, int vitima, int moldura);

    /// O placar do thrashing: "Thrashing: 8 faltas nas últimas 10 referências".
    std::string PlacarDoThrashing(int faltas, int referencias);

    // ---- O relogio (a segunda chance) ----

    struct Moldura {
        int pagina = -1;          ///< -1: livre
        bool referenciada = false;   ///< o bit R
    };

    struct Referencia {
        bool acerto = false;
        int moldura = -1;               ///< onde a pagina ficou
        int vitima = -1;                ///< a pagina que saiu (-1: nenhuma)
        std::vector<int> segundaChance; ///< as molduras que o ponteiro poupou, na ordem da varredura
    };

    class Relogio {
    public:
        explicit Relogio(int molduras = 4);
        /// O processo le a 'pagina': acerto, ou falta (com moldura livre ou com vitima).
        Referencia Referenciar(int pagina);
        [[nodiscard]] const std::vector<Moldura>& Molduras() const { return mMolduras; }
        [[nodiscard]] int Ponteiro() const { return mPonteiro; }
        /// Em que moldura esta a 'pagina' (-1: em nenhuma - a entrada da tabela e invalida).
        [[nodiscard]] int MolduraDa(int pagina) const;

    private:
        std::vector<Moldura> mMolduras;
        int mPonteiro = 0;
    };

    // ---- As referencias ----

    /// O conjunto de trabalho de cada etapa, paginas vizinhas a partir de 'base' (sorteada por
    /// luta): na localidade e na mudanca ele cabe nas molduras (com uma sobrando), e a mudanca e
    /// outro pedaco do programa; no thrashing ele tem duas paginas a mais do que molduras.
    std::vector<int> ConjuntoDeTrabalho(int etapa, int molduras, int base);

    /// A pagina da proxima referencia: do conjunto de trabalho, menos uma vez em 'fora' - a
    /// localidade de referencia. 'emOrdem' percorre o conjunto em laco (o 'cursor' guarda onde
    /// parou): e o laco que nao cabe na memoria, o pior caso do relogio.
    int ProximaReferencia(Variacao::Sorteio& s, const std::vector<int>& conjunto, float fora, int& cursor,
                          bool emOrdem);

    /// Segundos entre duas referencias, por etapa: o programa acelera.
    float EntreReferencias(int etapa, Dificuldade::Nivel n);

    /// Quanto o ponteiro leva de uma moldura a seguinte, na varredura que a tela mostra.
    constexpr float kPassoDoPonteiro = 0.07f;

    /// Quantas referencias o placar do thrashing conta.
    constexpr int kJanelaDoThrashing = 10;

    /// O que o Dificil muda: uma moldura a menos e mais referencias fora do conjunto.
    struct Degrau {
        int molduras = 5;
        float fora = 0.10f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
