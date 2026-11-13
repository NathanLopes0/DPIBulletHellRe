//
// A fase 2 do Goulart: a memoria paginada.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "Fase.h"

/// A fase 2 do Goulart e a paginacao - e o aluno e a referencia. O chao do campo sao as kPaginas
/// paginas virtuais, uma coluna cada, com a entrada da tabela de paginas embaixo: a coluna em que o
/// aluno esta e a pagina que o processo le. A memoria fisica e um relogio de molduras no alto:
///   - a pagina que esta numa moldura e um acerto: a moldura acende (o bit R), e nada cai;
///   - a que nao esta e uma falta de pagina: o ponteiro do relogio varre as molduras - quem tem R
///     ligado ganha a segunda chance (o R apaga), e a primeira com R apagado e a vitima. A pagina
///     vitima e despejada: a coluna dela desaba, uma cortina de cima a baixo.
/// Quem fica em poucas colunas (a localidade) quase nao tem falta; quem anda por todas faz o
/// thrashing - e as cortinas caem em volta. A memoria encolhe a cada etapa (5, 4 e 3 molduras), e o
/// placar mostra a conta da MMU de cada referencia.
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
        /// A memoria encolhe para 'molduras': as de sobra saem, e as paginas delas sao despejadas.
        std::vector<int> Encolher(int molduras);

    private:
        std::vector<Moldura> mMolduras;
        int mPonteiro = 0;
    };

    /// Quanto o ponteiro leva de uma moldura a seguinte, na varredura que a tela mostra - e a coluna
    /// da vitima fica marcada esse tempo antes de desabar.
    constexpr float kPassoDoPonteiro = 0.1f;

    // ---- O chao ----

    /// A pagina da coluna em que esta o x, num chao que comeca em 'x' e tem 'largura'; e onde a
    /// coluna da pagina comeca e quanto mede.
    int PaginaDoX(float px, float x, float largura);
    float InicioDaPagina(int p, float x, float largura);
    float LarguraDaPagina(float largura);

    /// O aluno le a pagina dele a cada kEntreLeituras (e na hora em que muda de coluna).
    constexpr float kEntreLeituras = 0.45f;

    /// A cortina da pagina despejada: quanto dura, e de quanto em quanto cai um pedaco.
    constexpr float kDespejo = 0.9f;
    constexpr float kEntrePedacos = 0.05f;
    constexpr float kVelocidadeDoDespejo = 330.0f;

    /// As molduras da memoria em cada etapa: ela encolhe.
    int MoldurasNaEtapa(int etapa, Dificuldade::Nivel n);

    /// Quantas referencias o placar do thrashing conta.
    constexpr int kJanelaDoThrashing = 10;

    /// O professor tambem usa a memoria:
    ///   - le a pagina do aluno com um leque de tres, mirado, de tanto em tanto - que o tira do lugar (o
    ///     leque e o fases.json);
    ///   - e le as paginas dele, pelo mesmo relogio: a falta dele despeja uma vitima, que desaba onde
    ///     estiver. A pagina que o aluno usa tem o R aceso, e o relogio a poupa.
    struct Degrau {
        float entreAcessos = 1.1f;
        float leituraDoProfessor = 2.2f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// De quanto em quanto o professor le uma pagina na 'etapa': mais depressa a cada uma.
    float EntreLeiturasDoProfessor(const Degrau& d, int etapa);

    /// A pagina que o professor le: qualquer uma, menos a do aluno ('sorteio' de 0 a kPaginas - 2).
    int PaginaDoProfessor(int sorteio, int doAluno);
}
