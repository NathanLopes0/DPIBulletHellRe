//
// O registro de cada tentativa de batalha: os dados para a analise do TCC.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * CAMADA PURA. Coberto por test_tentativas.cpp.
 *
 * A FICHA GUARDA O RECORDE; ISTO GUARDA A HISTORIA. A ficha do aluno (Ficha.h)
 * sabe a melhor nota de cada materia e a ultima data, e mais nada - e o que o
 * jogo precisa. Para estudar como os alunos jogam, e preciso cada TENTATIVA:
 * quando, quanto durou, como terminou, onde ele errou. E isto.
 *
 * TRES ARQUIVOS, todos CSV - abrem no Excel, no LibreOffice, no R e no pandas
 * sem conversao (TentativasArquivo grava em <base>/Saves):
 *
 *   tentativas.csv         UMA LINHA POR ALUNO POR TENTATIVA. So cresce: cada
 *                          batalha acrescenta as suas linhas ao fim, e nada e
 *                          reescrito. Numa dupla, duas linhas com o mesmo id.
 *   tentativas_fases.csv   uma linha por FASE de cada tentativa: onde a nota
 *                          subiu, onde caiu, quanto tempo cada fase levou.
 *   tentativas_resumo.csv  uma linha por aluno, materia, dificuldade e modo:
 *                          quantas vezes jogou, passou, reprovou, desistiu.
 *                          DERIVADO - e refeito de tentativas.csv a cada
 *                          batalha; apaga-lo nao perde nada.
 *
 * O significado de cada coluna esta em Documentacao/dados-das-tentativas.md.
 *
 * A HORA E A LOCAL (Relogio.h), no formato que ordena alfabeticamente na ordem
 * do tempo. Os numeros usam PONTO como separador decimal, sempre - um CSV com
 * virgula decimal quebra as colunas.
 */
namespace Tentativas {

    /// A versao do formato, gravada em toda linha: mudar uma coluna e mudar isto.
    constexpr int kVersaoDoFormato = 1;

    enum class Resultado { Aprovado, Reprovado, Abandonou };
    const char* Nome(Resultado r);

    /// Uma fase jogada nesta tentativa.
    struct Fase {
        int ordem = 0;               ///< 1 para a primeira fase jogada
        std::string nome;            ///< "StateOne", ..., "StateFinal"
        std::string anuncio;         ///< o nome que o placar deu a ela
        float inicio = 0.0f;         ///< segundos de batalha
        float fim = 0.0f;
        float notaNoInicio = 0.0f;
        float notaNoFim = 0.0f;
        int acertosSofridos = 0;     ///< da dupla, nesta fase
        bool completa = false;       ///< terminou pelo relogio (e nao pelo fim da batalha)
        bool limpa = false;          ///< pagou o bonus de fase limpa (Nota::kBonusDeFaseLimpa)
    };

    /// O que cada aluno fez nesta tentativa.
    struct Jogador {
        int posicao = 1;             ///< 1 ou 2, a posicao no painel
        std::string matricula;       ///< ou "visitante"
        bool visitante = false;
        std::string parceiro;        ///< na dupla, a matricula do outro
        int tiros = 0;               ///< Duvidas disparadas
        int acertosNoProfessor = 0;  ///< tiros dele que acertaram um corpo do chefe
        int acertosSofridos = 0;     ///< vezes que ELE foi acertado
        int pontosExtrasPegos = 0;
        int pontosExtrasUsados = 0;
    };

    struct Tentativa {
        std::string id;              ///< o mesmo para os dois alunos de uma dupla
        std::string versaoDoJogo;
        std::string inicio;          ///< "2026-10-28 14:03:22"
        std::string fim;
        float duracao = 0.0f;        ///< segundos de batalha: sem carregamento e sem pausa
        float tempoEmPausa = 0.0f;
        std::string materia;         ///< o codigo estavel: "INF213"
        std::string nomeDaMateria;
        std::string professor;
        std::string dificuldade;     ///< "normal" ou "dificil"
        std::string modo;            ///< "solo" ou "dupla"
        Resultado resultado = Resultado::Abandonou;
        float notaFinal = 0.0f;
        float notaMaxima = 0.0f;
        float notaMinima = 0.0f;
        bool notaCheia = false;      ///< chegou a 100 em algum momento
        bool fezExame = false;       ///< chegou a fase final (StateFinal)
        int fasesLimpas = 0;
        int acertosSofridos = 0;     ///< da dupla inteira
        std::string orientador;      ///< so no TCC
        int acertosDoOrientador = 0;
        int poderesUsados = 0;
        std::uint32_t semente = 0;   ///< a variacao sorteada da luta
        std::vector<Jogador> jogadores;
        std::vector<Fase> fases;
    };

    // ---- O ID E A DATA ---------------------------------------------------------------

    /// @brief O id de uma tentativa: "20261028-140322-0f3a9c21". O comeco ordena
    /// no tempo; o fim ('sal', a semente da luta) separa duas que comecem no
    /// mesmo segundo.
    std::string NovoId(const std::string& inicio, std::uint32_t sal);

    /// @brief O dia da semana de "AAAA-MM-DD..." ("domingo", ..., "sabado"), ou
    /// "" se a data nao se le.
    std::string DiaDaSemana(const std::string& data);

    // ---- AS LINHAS ------------------------------------------------------------------

    std::string CabecalhoDasTentativas();
    /// @brief Uma linha por jogador. 'vezes[i]' e a vez do jogador i nesta
    /// materia (1 na primeira tentativa); 0 para visitante.
    std::vector<std::string> LinhasDaTentativa(const Tentativa& t, const std::vector<int>& vezes);

    std::string CabecalhoDasFases();
    std::vector<std::string> LinhasDasFases(const Tentativa& t);

    // ---- A LEITURA DE VOLTA, PARA O RESUMO -----------------------------------------

    /// O que o resumo precisa de cada linha de tentativas.csv.
    struct Linha {
        std::string matricula;
        bool visitante = false;
        std::string materia;
        std::string nomeDaMateria;
        std::string dificuldade;
        std::string modo;
        std::string resultado;
        std::string inicio;
        float nota = 0.0f;
        float duracao = 0.0f;
        [[nodiscard]] bool Aprovou() const { return resultado == "aprovado"; }
    };

    /// @brief As linhas de um tentativas.csv (o cabecalho e o que nao se le sao
    /// pulados). Um arquivo de formato mais novo que este nao e lido errado:
    /// as colunas sao achadas pelo NOME no cabecalho.
    std::vector<Linha> Ler(const std::string& csv);

    /// @brief A proxima vez de 'matricula' em 'materia': 1 + as que ja houve.
    int ProximaVez(const std::vector<Linha>& linhas, const std::string& matricula, const std::string& materia);

    /// @brief O resumo: uma linha por aluno, materia, dificuldade e modo.
    std::string Resumo(const std::vector<Linha>& linhas);

    /// @brief Os campos de uma linha CSV (aspas e aspas dobradas respeitadas).
    std::vector<std::string> Campos(const std::string& linha);
}
