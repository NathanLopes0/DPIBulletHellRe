//
// O progresso academico do jogador: o recorde por materia e o ponto de retomada.
//

#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Nota.h"

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

    /// SOZINHO OU EM DUPLA: cada modo tem o proprio recorde, e o proprio
    /// ranking (requisitos-coop, D9).
    enum class Modo { Solo, Dupla };

    /**
     * @brief Um recorde: a nota, e - na dupla - com quem e quando foi feito.
     *
     * O PARCEIRO VAI JUNTO porque a nota da dupla e da DUPLA, e o ranking de
     * duplas precisa saber com quem: guarda o parceiro DO RECORDE - se o aluno
     * fizer melhor com outra pessoa, o nome muda junto com a nota. No solo os
     * dois ficam vazios.
     */
    struct Marca {
        float nota = 0.0f;
        std::string parceiro;
        /// Quando a DUPLA fez o recorde: e ele que data a linha do ranking de
        /// duplas. No solo fica vazio - "quando jogou" e Entrada::quando.
        std::string quando;
    };

    /**
     * @brief Registra o resultado de uma batalha jogada SOZINHO.
     *
     * Faz duas coisas distintas, que antes eram a mesma:
     *   - o PONTO DE RETOMADA passa a ser esta nota, sempre;
     *   - o RECORDE do nivel passa a ser esta nota apenas se for maior.
     *
     * A diferenca importa: antes, jogar mal apagava o recorde e podia
     * RE-TRANCAR uma materia que o jogador ja tinha passado.
     *
     * O NIVEL NAO TEM VALOR PADRAO, de proposito: esquecer de passa-lo poria
     * uma nota do Dificil no ranking do Normal, e o compilador e quem lembra.
     *
     * @param quando o instante, como texto (ver Relogio.h). Vazio = nao sabemos,
     *        que e o caso dos saves gravados antes de a data existir.
     */
    void RegistrarNota(int materia, float nota, Dificuldade::Nivel nivel, const std::string& quando = "");

    /**
     * @brief Registra o resultado de uma batalha jogada EM DUPLA.
     *
     * Funcao propria, e nao um booleano em RegistrarNota, porque esquecer o
     * booleano poria uma nota de co-op no recorde de solo - exatamente o que
     * os rankings existem para separar (requisitos-coop, D9).
     *
     * Visitante nao tem matricula, entao um parceiro vazio e normal: o recorde
     * vale para o progresso e para o desbloqueio, e fica de fora do ranking de
     * duplas (RF6).
     */
    void RegistrarNotaDeDupla(int materia, float nota, Dificuldade::Nivel nivel,
                              const std::string& parceiro, const std::string& quando = "");

    /**
     * @brief A maior nota que o jogador ja tirou nesta materia, EM QUALQUER
     * MODO E NIVEL.
     *
     * Zero quando nunca jogou. E const e usa busca, nao operator[]: o getter
     * antigo INSERIA uma entrada zero no mapa a cada leitura, o que e mutacao
     * dentro de uma consulta e fazia o mapa crescer com materias nunca jogadas.
     *
     * A MAIOR DE TODAS, e e disso que o mapa do curso precisa: passar e passar,
     * tenha sido com quem for e no nivel que for (requisitos-coop, D10). Quem
     * precisa separa-las e o ranking, e para isso ha Recorde.
     */
    [[nodiscard]] float MelhorNota(int materia) const;

    /// @brief O recorde de um modo e nivel. Nota zero quando nunca jogou assim.
    [[nodiscard]] const Marca& Recorde(int materia, Modo modo, Dificuldade::Nivel nivel) const;

    /// @brief O jogador passou nesta materia? Olha o RECORDE, nao a ultima nota.
    [[nodiscard]] bool Aprovado(int materia) const;

    /// @brief Quantas das materias listadas estao aprovadas.
    [[nodiscard]] int QuantasAprovadas(const std::vector<int>& materias) const;

    /// @brief Quantas materias tem registro. So para diagnostico e teste.
    [[nodiscard]] size_t QuantasRegistradas() const { return mPorMateria.size(); }

    /**
     * @brief TUDO O QUE SE SABE DE UMA MATERIA, numa linha so: o que se grava
     * e o que se le de volta.
     *
     * A ficha do aluno precisa guardar a retomada E os recordes: so os
     * recordes perderiam o ponto de retomada, e so a retomada perderia a
     * aprovacao.
     *
     * OS QUATRO RECORDES NUM ARRANJO, e nao em doze campos soltos. Eram seis
     * (recorde, recordeDeDupla, parceiroDoRecorde, quandoDaDupla...), e a
     * dificuldade dobraria isso: doze campos copiados um a um entre o registro,
     * a ficha e a planilha sao doze chances de um caminho esquecer um.
     */
    struct Entrada {
        int materia = 0;

        /// A nota da ultima tentativa, em qualquer modo e nivel. NAO HA UMA
        /// POR MODO: a retomada e informacao para o professor sobre a ultima
        /// tentativa, e toda batalha comeca em kNotaInicial de qualquer jeito.
        float retomada = 0.0f;

        /// Quando o aluno jogou esta materia pela ULTIMA vez. Vazio quando o save
        /// e anterior a este campo existir.
        ///
        /// E a ultima vez, e nao quando o recorde foi feito: a pergunta que isto
        /// responde e "quem jogou esta semana", nao "quando cada um foi melhor".
        std::string quando;

        std::array<Marca, 4> recordes{};

        [[nodiscard]] Marca& De(Modo modo, Dificuldade::Nivel nivel) { return recordes[Indice(modo, nivel)]; }
        [[nodiscard]] const Marca& De(Modo modo, Dificuldade::Nivel nivel) const {
            return recordes[Indice(modo, nivel)];
        }

        /// O maior dos quatro recordes.
        [[nodiscard]] float MaiorRecorde() const;
    };

    /**
     * @brief Tudo que esta guardado, em ordem de materia.
     *
     * Existe para a persistencia e para a planilha. Uma copia: quem grava nao
     * mexe no progresso.
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
    /// A posicao de um modo e nivel em Entrada::recordes.
    static constexpr size_t Indice(const Modo modo, const Dificuldade::Nivel nivel) {
        return (modo == Modo::Dupla ? 2u : 0u) + (nivel == Dificuldade::Nivel::Dificil ? 1u : 0u);
    }

    /// O que as duas formas de registrar tem em comum.
    void Registrar(int materia, float nota, Modo modo, Dificuldade::Nivel nivel,
                   const std::string& parceiro, const std::string& quando);

    /**
     * UM REGISTRO POR MATERIA, e nao mapas paralelos. Eram tres mapas -
     * recorde, retomada e data -, e o co-op pediria mais tres: seis chances de
     * um caminho escrever em cinco e esquecer o sexto. Com um registro so, ou a
     * materia existe inteira ou nao existe. E o registro e a propria Entrada:
     * gravar e ler nao precisam copiar campo a campo.
     */
    std::map<int, Entrada> mPorMateria;

    /// O registro de uma materia, ou um vazio. Nao insere - o getter antigo
    /// usava operator[] e fazia o mapa crescer a cada leitura.
    [[nodiscard]] const Entrada& Ver(int materia) const;
};
