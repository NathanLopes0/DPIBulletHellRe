//
// A matricula do aluno: o que e uma matricula valida e como ela e digitada.
//

#pragma once

#include <string>

/**
 * CAMADA PURA (mesma disciplina de Progresso, Caminhos, PathAim e os leitores de
 * dados). Nao conhece Game, Scene, SDL nem arquivo: so transforma texto.
 *
 * A MATRICULA SO IDENTIFICA. Nao ha senha, nao ha lista de alunos para conferir e
 * nao ha como provar que quem digitou 89384 e mesmo o dono daquela matricula.
 * Isso e deliberado: o jogo roda numa maquina do departamento, o objetivo e o
 * professor saber quais alunos jogaram e como foram, e uma senha so criaria um
 * jeito de o aluno nao conseguir entrar no dia da apresentacao.
 *
 * O que esta camada faz, entao, e menos do que "autenticar": ela diz se o texto
 * TEM CARA de matricula e devolve a forma canonica, para que o mesmo aluno caia
 * sempre na mesma ficha.
 */
namespace Matricula {

    /// A matricula da UFV tem no maximo seis digitos. Pode ter menos: 89384 tem
    /// cinco, 115849 tem seis.
    inline constexpr int kMaximoDeDigitos = 6;

    /**
     * @brief Por que uma matricula foi recusada.
     *
     * Enum em vez de bool porque a tela precisa dizer O QUE esta errado. "Invalida"
     * sozinho faz o aluno tentar de novo o mesmo erro.
     */
    enum class Erro {
        Nenhum,
        Vazia,          ///< nao digitou nada
        NaoSoNumeros,   ///< tem letra, pontuacao ou espaco no meio
        LongaDemais,    ///< mais de seis digitos
        Zero            ///< so zeros; ver a nota em Validar
    };

    struct Resultado {
        bool valida = false;

        /// A matricula SEM zeros a esquerda. E por ela que a ficha e procurada no
        /// disco, entao "089384" e "89384" sao o mesmo aluno e nao dois.
        std::string canonica;

        Erro erro = Erro::Vazia;
    };

    /**
     * @brief Diz se o texto serve como matricula e devolve a forma canonica.
     *
     * Aceita espaco ANTES e DEPOIS (quem digita rapido sobra espaco), mas nao no
     * meio: "89 384" e recusado com a mesma mensagem de uma letra, porque do ponto
     * de vista de quem digitou os dois sao "tem coisa que nao e numero ai".
     *
     * SO ZEROS E RECUSADO, e esta e uma decisao minha e nao uma regra da UFV: a
     * forma canonica de "000000" seria "0", e matricula zero nao identifica
     * ninguem - aceitar faria "apertei zero sem querer" virar uma ficha de aluno
     * indistinguivel de uma de verdade. Se existir matricula 0 por algum motivo,
     * basta tirar o teste; ele esta num lugar so.
     */
    Resultado Validar(const std::string& digitada);

    /// @brief A frase que a tela mostra para cada erro. Vazia quando nao ha erro.
    std::string MensagemDeErro(Erro erro);

    /**
     * @brief Aplica uma tecla ao texto que esta sendo digitado.
     *
     * TODA a regra de digitacao mora aqui, e nao na tela: so digito entra, e nada
     * entra depois do sexto. Assim o aluno nao consegue digitar uma matricula
     * invalida em primeiro lugar, e a tela fica sendo so um desenho - a mesma
     * divisao que separa os leitores de dados das pontes.
     *
     * Devolve o texto novo. Tecla que nao serve devolve o texto sem mudanca, em
     * vez de um bool que a tela teria de interpretar.
     */
    std::string Digitar(const std::string& atual, char tecla);

    /// @brief Apaga o ultimo caractere. Texto vazio continua vazio.
    std::string Apagar(const std::string& atual);

    /**
     * @brief Como a matricula aparece na tela e nos arquivos.
     *
     * Hoje e a propria canonica. Existe como funcao para que mudar a aparencia
     * (zeros a esquerda, um prefixo) nao exija cacar concatenacao espalhada.
     */
    std::string ParaExibir(const std::string& canonica);

    /// @brief O rotulo de quem escolheu jogar sem se identificar.
    ///
    /// Nao e uma matricula valida de proposito: assim ele nunca colide com um
    /// aluno de verdade nem cria ficha no disco.
    inline const char* kVisitante = "visitante";
}
