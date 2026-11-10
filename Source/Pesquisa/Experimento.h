//
// A condicao de um experimento: as mecanicas que esta execucao desliga.
//

#pragma once

#include <string>
#include <vector>

/// Para comparar o jogo com e sem uma mecanica - os padroes de jogo como variavel do estudo, como
/// em Emmerich e Masuch -, o gabinete abre com --condicao, e cada tentativa grava a dela
/// (dados-das-tentativas.md). Sem --condicao, tudo ligado, como sempre. Desligar tira a mecanica do
/// jogo, e nao da medida: o que o aluno faz continua contado. Nao conhece Game nem SDL.
namespace Experimento {

    /// O que se pode desligar.
    enum class Mecanica {
        Brilho,      ///< o projetil que vai num aluno brilha na cor dele
        Cobertura,   ///< derrubar o que ia no parceiro paga inteiro - desligada, apaga o brilho junto
        Papeis,      ///< a mira vai em quem mais incomoda; desligada, os dois revezam
        Raspao,      ///< passar rente paga
        Perto,       ///< a duvida de perto vale mais
        Devolutiva,  ///< quem reprova ve onde a nota caiu e o que mais o acertou
        Sincronia,   ///< os dois Pontos Extras juntos soltam a onda
    };

    /// Todas, na ordem em que o nome da condicao as lista.
    constexpr Mecanica kTodas[] = {Mecanica::Brilho, Mecanica::Cobertura, Mecanica::Papeis,
                                   Mecanica::Raspao, Mecanica::Perto, Mecanica::Devolutiva,
                                   Mecanica::Sincronia};

    /// O nome na linha de comando, sem o "sem-": "brilho", "cobertura"...
    const char* Nome(Mecanica m);

    /// O que esta execucao desliga.
    class Condicao {
    public:
        void Desligar(Mecanica m) { mDesligadas |= Bit(m); }

        /// A mecanica vale nesta execucao? O brilho cai junto com a cobertura: ele prometeria o
        /// que nao paga.
        [[nodiscard]] bool Ligada(const Mecanica m) const {
            if (m == Mecanica::Brilho && (mDesligadas & Bit(Mecanica::Cobertura))) return false;
            return (mDesligadas & Bit(m)) == 0;
        }

        /// O que vai na coluna 'condicao': "sem-brilho+sem-papeis", na ordem de kTodas; "normal"
        /// com tudo ligado.
        [[nodiscard]] std::string Nome() const;

    private:
        static unsigned Bit(const Mecanica m) { return 1u << static_cast<unsigned>(m); }
        unsigned mDesligadas = 0;
    };

    /// Le o texto de --condicao: "normal", ou nomes com "sem-" separados por virgula ou "+"
    /// ("sem-brilho,sem-papeis"). O que nao se reconhece vai para 'problemas' e e ignorado.
    Condicao Ler(const std::string& texto, std::vector<std::string>* problemas = nullptr);
}
