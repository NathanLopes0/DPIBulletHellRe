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
     * @brief Registra o resultado de uma batalha jogada EM DUPLA.
     *
     * Funcao propria, e nao um booleano em RegistrarNota, porque as duas
     * escrevem em lugares diferentes e esquecer o booleano poria uma nota de
     * co-op no recorde de solo - exatamente o que os dois rankings existem
     * para separar (requisitos-coop, D9).
     *
     * O PARCEIRO VAI JUNTO porque a nota e da DUPLA, e o ranking de co-op e de
     * duplas: sem saber com quem, um recorde nao tem como entrar nele. Guarda
     * o parceiro DO RECORDE - se o aluno fizer melhor com outra pessoa, o nome
     * muda junto com a nota.
     *
     * Visitante nao tem matricula, entao um parceiro vazio e normal: o recorde
     * vale para o progresso e para o desbloqueio, e fica de fora do ranking de
     * duplas (RF6).
     */
    void RegistrarNotaDeDupla(int materia, float nota, const std::string& parceiro,
                              const std::string& quando = "");

    /**
     * @brief A maior nota que o jogador ja tirou nesta materia, SOZINHO OU EM
     * DUPLA.
     *
     * Zero quando nunca jogou. E const e usa busca, nao operator[]: o getter
     * antigo INSERIA uma entrada zero no mapa a cada leitura, o que e mutacao
     * dentro de uma consulta e fazia o mapa crescer com materias nunca jogadas.
     *
     * A MAIOR DAS DUAS, e e disso que o mapa do curso precisa: passar e passar,
     * tenha sido com quem for (requisitos-coop, D10). Quem precisa separa-las e
     * o ranking, e para isso ha MelhorNotaSolo e MelhorNotaDeDupla.
     */
    [[nodiscard]] float MelhorNota(int materia) const;

    /// @brief So o recorde de SOLO. Zero quando nunca jogou sozinho.
    [[nodiscard]] float MelhorNotaSolo(int materia) const;

    /// @brief So o recorde de DUPLA. Zero quando nunca jogou acompanhado.
    [[nodiscard]] float MelhorNotaDeDupla(int materia) const;

    /// @brief Com quem o recorde de dupla foi feito. Vazio quando nao ha
    /// recorde de dupla, ou quando o parceiro era visitante.
    [[nodiscard]] const std::string& ParceiroDoRecorde(int materia) const;

    /// @brief O jogador passou nesta materia? Olha o RECORDE, nao a ultima nota.
    [[nodiscard]] bool Aprovado(int materia) const;

    /// @brief Quantas das materias listadas estao aprovadas.
    [[nodiscard]] int QuantasAprovadas(const std::vector<int>& materias) const;

    /// @brief Quantas materias tem registro. So para diagnostico e teste.
    [[nodiscard]] size_t QuantasRegistradas() const { return mPorMateria.size(); }

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

        // ---- O que veio das batalhas em dupla -----------------------------
        //
        // NAO HA "retomadaCoop". A retomada e informacao para o professor sobre
        // a ultima tentativa, e toda batalha comeca em kNotaInicial de qualquer
        // jeito; dois campos onde um basta seria mais uma coisa para manter em
        // dia sem ninguem ler.

        float recordeDeDupla = 0.0f;

        /// A matricula do parceiro QUANDO o recorde de dupla foi feito.
        std::string parceiroDoRecorde;

        /// Quando a dupla fez esse recorde. Aqui e quando o RECORDE foi feito,
        /// e nao a ultima vez - e ele que data a linha do ranking de duplas.
        std::string quandoDaDupla;
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
    /**
     * @brief TUDO O QUE SE SABE DE UMA MATERIA, numa linha so.
     *
     * Eram TRES mapas paralelos - recorde, retomada e data -, e o co-op pediria
     * mais tres. Seis mapas paralelos sao seis chances de um caminho escrever
     * em cinco e esquecer o sexto, e Entradas() ja precisava percorrer a UNIAO
     * das chaves para nao perder uma materia que estivesse num e nao no outro.
     *
     * Com um registro so, ou a materia existe inteira ou nao existe.
     */
    struct Registro {
        float recorde = 0.0f;
        float retomada = 0.0f;
        std::string quando;

        float recordeDeDupla = 0.0f;
        std::string parceiroDoRecorde;
        std::string quandoDaDupla;
    };

    std::map<int, Registro> mPorMateria;

    /// O registro de uma materia, ou um vazio. Nao insere - o getter antigo
    /// usava operator[] e fazia o mapa crescer a cada leitura.
    [[nodiscard]] const Registro& Ver(int materia) const;
};
