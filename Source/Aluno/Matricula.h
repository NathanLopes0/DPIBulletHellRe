//
// A matricula do aluno: o que e uma matricula valida e como ela e digitada.
//

#pragma once

#include <string>

/// Nao conhece Game, Scene, SDL nem arquivo: so transforma texto. A matricula so identifica.
namespace Matricula {

    /// A matricula da UFV tem no maximo seis digitos. Pode ter menos: 89384 tem
    /// cinco, 115849 tem seis.
    inline constexpr int kMaximoDeDigitos = 6;

    /// Por que uma matricula foi recusada. Enum em vez de bool porque a tela precisa dizer o que
    /// esta errado.
    enum class Erro {
        Nenhum,
        Vazia,          ///< nao digitou nada
        NaoSoNumeros,   ///< tem letra, pontuacao ou espaco no meio
        LongaDemais,    ///< mais de seis digitos
        Zero            ///< so zeros; ver a nota em Validar
    };

    struct Resultado {
        bool valida = false;

        /// A matricula sem zeros a esquerda. E por ela que a ficha e procurada no disco, entao
        /// "089384" e "89384" sao o mesmo aluno e nao dois.
        std::string canonica;

        Erro erro = Erro::Vazia;
    };

    /// Diz se o texto serve como matricula e devolve a forma canonica.
    Resultado Validar(const std::string& digitada);

    /// A frase que a tela mostra para cada erro. Vazia quando nao ha erro.
    std::string MensagemDeErro(Erro erro);

    /// Aplica uma tecla ao texto que esta sendo digitado. Toda a regra de digitacao mora aqui, e
    /// nao na tela: so digito entra, e nada entra depois do sexto.
    std::string Digitar(const std::string& atual, char tecla);

    /// Apaga o ultimo caractere. Texto vazio continua vazio.
    std::string Apagar(const std::string& atual);

    /// Como a matricula aparece na tela e nos arquivos. Hoje e a propria canonica.
    std::string ParaExibir(const std::string& canonica);

    /// O rotulo de quem escolheu jogar sem se identificar.
    inline const char* kVisitante = "visitante";
}
