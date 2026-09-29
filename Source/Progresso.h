//
// O progresso academico do jogador: o recorde por materia e o ponto de retomada.
//

#pragma once

#include <map>
#include <vector>

/**
 * CAMADA PURA (mesma disciplina de PathAim, RegrasDeAtaque e FasesDeAtaque)
 *
 * Nao conhece Game, Scene nem SDL: so guarda notas e responde perguntas sobre
 * elas. Toda a aritmetica de progresso mora aqui para poder ser testada sem
 * subir o jogo, e para que as duas coisas que o mapa de notas do Game confundia
 * passem a ter nomes distintos.
 *
 * AS MATERIAS SAO 'int' de proposito: Game::GameSubject e um enum aninhado em
 * Game, e incluir Game.h aqui arrastaria o SDL junto. Quem converte e o Game.
 */
class Progresso {
public:

    /// Nota com que uma batalha comeca quando a materia nunca foi jogada.
    static constexpr float kNotaInicial = 40.0f;

    /// A partir de quanto a materia conta como aprovada.
    static constexpr float kNotaDeAprovacao = 60.0f;

    /**
     * @brief Registra o resultado de uma batalha.
     *
     * Faz duas coisas distintas, que antes eram a mesma:
     *   - o PONTO DE RETOMADA passa a ser esta nota, sempre;
     *   - o RECORDE passa a ser esta nota apenas se for maior.
     *
     * A diferenca importa: antes, jogar mal apagava o recorde e podia
     * RE-TRANCAR uma materia que o jogador ja tinha passado.
     */
    void RegistrarNota(int materia, float nota);

    /**
     * @brief A maior nota que o jogador ja tirou nesta materia.
     *
     * Zero quando nunca jogou. E const e usa busca, nao operator[]: o getter
     * antigo INSERIA uma entrada zero no mapa a cada leitura, o que e mutacao
     * dentro de uma consulta e fazia o mapa crescer com materias nunca jogadas.
     */
    [[nodiscard]] float MelhorNota(int materia) const;

    /**
     * @brief A nota com que a proxima batalha desta materia comeca.
     *
     * Nunca abaixo de kNotaInicial. Preserva o comportamento atual: o jogador
     * retoma de onde parou, e nao do zero.
     */
    [[nodiscard]] float NotaDeRetomada(int materia) const;

    /// @brief O jogador passou nesta materia? Olha o RECORDE, nao a ultima nota.
    [[nodiscard]] bool Aprovado(int materia) const;

    /// @brief Quantas das materias listadas estao aprovadas.
    [[nodiscard]] int QuantasAprovadas(const std::vector<int>& materias) const;

    /// @brief Quantas materias tem registro. So para diagnostico e teste.
    [[nodiscard]] size_t QuantasRegistradas() const { return mRecordes.size(); }

private:
    /// O recorde por materia. So sobe.
    std::map<int, float> mRecordes;

    /// A ultima nota por materia, que e de onde a proxima batalha parte.
    std::map<int, float> mRetomadas;
};
