//
// O progresso academico do jogador: o recorde por materia e o ponto de retomada.
//

#pragma once

#include <map>
#include <string>
#include <vector>

#include "Nota.h"

/**
 * CAMADA PURA (mesma disciplina de PathAim, RegrasDeAtaque e FasesDeAtaque)
 *
 * Nao conhece Game, Scene nem SDL: so guarda notas e responde perguntas sobre
 * elas. Toda a aritmetica de progresso mora aqui para poder ser testada sem
 * subir o jogo, e para que as duas coisas que o mapa de notas do Game confundia
 * passem a ter nomes distintos.
 *
 * AS MATERIAS SAO 'int' de proposito: e o indice na lista de Materias. Esta
 * camada nao inclui Materias.h nem Game.h - o indice e so um numero aqui, e
 * quem sabe o que ele significa e quem chama. Foi a escolha certa: por um tempo
 * o Game teve um enum GameSubject repetindo a lista, e quando ele foi aposentado
 * este arquivo nao precisou mudar nada.
 */
class Progresso {
public:

    /// Nota com que uma batalha comeca quando a materia nunca foi jogada.
    static constexpr float kNotaInicial = 40.0f;

    /// A partir de quanto a materia conta como aprovada.
    ///
    /// E A MESMA de Nota::kNotaAprovacao, e nao outra igual: eram dois 60
    /// soltos, um aqui e um la, e mexer na regra de aprovacao num deles
    /// deixaria o outro para tras sem nenhum aviso.
    static constexpr float kNotaDeAprovacao = Nota::kNotaAprovacao;

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
    /// @param quando o instante, como texto (ver Relogio.h). Vazio = nao sabemos,
    ///        que e o caso dos saves gravados antes de a data existir.
    void RegistrarNota(int materia, float nota, const std::string& quando = "");

    /**
     * @brief A maior nota que o jogador ja tirou nesta materia.
     *
     * Zero quando nunca jogou. E const e usa busca, nao operator[]: o getter
     * antigo INSERIA uma entrada zero no mapa a cada leitura, o que e mutacao
     * dentro de uma consulta e fazia o mapa crescer com materias nunca jogadas.
     */
    [[nodiscard]] float MelhorNota(int materia) const;

    /**
     * @brief A ULTIMA nota que o jogador tirou nesta materia, ou zero.
     *
     * Chamava-se NotaDeRetomada e tinha piso de kNotaInicial, porque a batalha
     * comecava daqui. Nao comeca mais: toda fase parte de kNotaInicial. O numero
     * continua guardado porque e informacao util para o professor (a planilha
     * exporta como "retomada"), mas o nome nao podia continuar prometendo o que
     * ele nao faz mais.
     */
    [[nodiscard]] float UltimaNota(int materia) const;

    /// @brief O jogador passou nesta materia? Olha o RECORDE, nao a ultima nota.
    [[nodiscard]] bool Aprovado(int materia) const;

    /// @brief Quantas das materias listadas estao aprovadas.
    [[nodiscard]] int QuantasAprovadas(const std::vector<int>& materias) const;

    /// @brief Quantas materias tem registro. So para diagnostico e teste.
    [[nodiscard]] size_t QuantasRegistradas() const { return mRecordes.size(); }

    /**
     * @brief Uma materia e os dois numeros dela, para gravar e ler de volta.
     *
     * A ficha do aluno precisa guardar os DOIS: so o recorde perderia o ponto de
     * retomada (o aluno voltaria a comecar do 40 em vez de onde parou), e so a
     * retomada perderia a aprovacao.
     */
    struct Entrada {
        int materia = 0;
        float recorde = 0.0f;
        float retomada = 0.0f;

        /// Quando o aluno jogou esta materia pela ULTIMA vez. Vazio quando o save
        /// e anterior a este campo existir.
        ///
        /// E a ultima vez, e nao quando o recorde foi feito: a pergunta que isto
        /// responde e "quem jogou esta semana", nao "quando cada um foi melhor".
        std::string quando;
    };

    /**
     * @brief Tudo que esta guardado, em ordem de materia.
     *
     * Existe para a persistencia. Nao devolve os mapas: quem grava nao precisa
     * saber que sao dois, e trocar a estrutura interna nao deve quebrar o
     * arquivo de save.
     */
    [[nodiscard]] std::vector<Entrada> Entradas() const;

    /**
     * @brief Repoe um estado lido de arquivo, SUBSTITUINDO o que havia.
     *
     * Nao da para fazer isto com RegistrarNota: ela acopla as duas coisas de
     * proposito (registrar sobe o recorde E move a retomada), entao reconstruir um
     * estado com recorde 80 e retomada 45 exigiria chamar na ordem certa e torcer.
     * Carregar um save nao e "jogar de novo"; e repor.
     */
    void Restaurar(const std::vector<Entrada>& entradas);

private:
    /// O recorde por materia. So sobe.
    std::map<int, float> mRecordes;

    /// A ultima nota por materia, que e de onde a proxima batalha parte.
    std::map<int, float> mRetomadas;

    /// Quando cada materia foi jogada pela ultima vez.
    std::map<int, std::string> mQuando;
};
