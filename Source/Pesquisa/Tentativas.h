//
// O registro de cada tentativa de batalha: os dados para a analise do TCC.
//

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

/// A ficha guarda o recorde; isto guarda a historia.
namespace Tentativas {

    /// A versao do formato, gravada em toda linha: mudar uma coluna e mudar isto. Coluna nova vai
    /// sempre no fim (TrocarCabecalho).
    constexpr int kVersaoDoFormato = 4;

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
        int acertosDePerto = 0;      ///< da dupla, nesta fase
        int raspoes = 0;
        int coberturas = 0;
    };

    /// O que cada aluno fez nesta tentativa.
    struct Jogador {
        int posicao = 1;             ///< 1 ou 2, a posicao no painel
        std::string matricula;       ///< ou "visitante"
        bool visitante = false;
        std::string parceiro;        ///< na dupla, a matricula do outro
        int tiros = 0;               ///< Duvidas disparadas
        int acertosNoProfessor = 0;  ///< tiros dele que acertaram um corpo do chefe
        int acertosSofridos = 0;     ///< Vezes que ele foi acertado.
        int pontosExtrasPegos = 0;
        int pontosExtrasUsados = 0;

        // Desde o formato 2. Contam pelo que o aluno fez, mesmo quando a condicao do experimento
        // nao paga a mecanica (Experimento.h).
        int acertosDePerto = 0;      ///< acertos no professor de perto: o "!!" (Nota::kFatorDaRespostaDupla)
        int raspoes = 0;             ///< projeteis que passaram rente a ele (Raspao.h)
        int coberturas = 0;          ///< na dupla, projeteis que ele derrubou indo no parceiro (Cobertura.h)
        int oportunidadesDeCobertura = 0;  ///< na dupla, projeteis destrutiveis que foram no parceiro
        float tempoNaMira = 0.0f;    ///< na dupla, segundos com a mira presa nele (Papeis.h)
        int sinais = 0;              ///< na dupla, os sinais que ele deu ao parceiro (Botao 3; formato 4)
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
        std::string orientador;      ///< So no TCC.
        int acertosDoOrientador = 0;
        int poderesUsados = 0;
        std::uint32_t semente = 0;   ///< a variacao sorteada da luta
        std::string condicao = "normal";  ///< a do experimento (Experimento::Condicao::Nome)
        /// A mira podia prender num aluno: sem os papeis (Experimento.h), tempo_na_mira fica vazio.
        bool comPapeis = true;
        float distanciaMedia = 0.0f; ///< na dupla, a distancia media entre os dois alunos, em pixels
        int sincronias = 0;          ///< na dupla, as vezes que os dois gastaram o Ponto Extra juntos (formato 3)
        std::vector<Jogador> jogadores;
        std::vector<Fase> fases;
    };

    // ---- O ID E a data ----

    /// O id de uma tentativa: "20261028-140322-0f3a9c21".
    std::string NovoId(const std::string& inicio, std::uint32_t sal);

    /// O dia da semana de "AAAA-MM-DD..." ("domingo", ..., "sabado"), ou "" se a data nao se le.
    std::string DiaDaSemana(const std::string& data);

    // ---- As linhas ----

    std::string CabecalhoDasTentativas();
    /// Uma linha por jogador. 'vezes[i]' e a vez do jogador i nesta materia (1 na primeira
    /// tentativa); 0 para visitante.
    std::vector<std::string> LinhasDaTentativa(const Tentativa& t, const std::vector<int>& vezes);

    std::string CabecalhoDasFases();
    std::vector<std::string> LinhasDasFases(const Tentativa& t);

    /// O conteudo de um CSV com o cabecalho 'novo' no lugar do que ele tem. So se o antigo for o
    /// comeco do novo - colunas se acrescentam no fim -, e as linhas antigas ficam como estavam: mais
    /// curtas, e com o formato delas na primeira coluna. Sem isso, nada: o arquivo e de outra coisa.
    std::optional<std::string> TrocarCabecalho(const std::string& csv, const std::string& novo);

    // ---- A leitura de volta, para o resumo ----

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

    /// As linhas de um tentativas.csv (o cabecalho e o que nao se le sao pulados).
    std::vector<Linha> Ler(const std::string& csv);

    /// A proxima vez de 'matricula' em 'materia': 1 + as que ja houve.
    int ProximaVez(const std::vector<Linha>& linhas, const std::string& matricula, const std::string& materia);

    /// O resumo: uma linha por aluno, materia, dificuldade e modo.
    std::string Resumo(const std::vector<Linha>& linhas);

    /// Os campos de uma linha CSV (aspas e aspas dobradas respeitadas).
    std::vector<std::string> Campos(const std::string& linha);
}
