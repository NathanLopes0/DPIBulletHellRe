//
// O chefe da INF 321 - Programacao Web.
//

#pragma once

#include "../Boss.h"
#include "../../../Attacks/Flex.h"

/**
 * @class Lucas
 * @brief O professor de Programacao Web (INF 321).
 *
 * A IDEIA DO CHEFE: ele da a materia na ordem classica - HTML, CSS, JavaScript
 * - e no fim ensina o que ele gosta de verdade, que e Elixir no back-end. A
 * batalha conta a mesma historia. As duas primeiras fases sao o front certinho
 * e geometrico; a terceira, a que decide, e o Elixir, onde o que o aluno
 * derruba volta sozinho ("let it crash"); a final volta ao front, agora
 * assincrono.
 *
 *   fase 1  HTML        caixas aninhadas, uma div dentro da outra
 *   fase 2  CSS         a fileira que se redistribui, como um flexbox
 *   fase 3  Elixir      processos que o supervisor reinicia
 *   final   JavaScript  promessas que resolvem ou sao rejeitadas
 *
 * O BALANCEAMENTO VIVE EM DADOS, como o dos outros: Assets/Attacks/lucas/.
 */
class Lucas : public Boss {
public:
    explicit Lucas(Scene* scene);
    ~Lucas() override = default;

    void OnUpdate(float deltaTime) override;

    /**
     * @brief O SUPERVISOR: um processo caiu, e ele vai voltar.
     *
     * "Let it crash" e a filosofia do Elixir: deixar um processo falhar e
     * barato, porque um supervisor o reinicia. Aqui, cada processo que o aluno
     * derruba entra na proxima rodada da fase 3 - o tiro abriu caminho por um
     * instante, e nao para sempre.
     */
    void OnProjetilDerrubado() override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

private:
    // ---- A LUTA SORTEADA ----------------------------------------------------
    //
    // Antes do primeiro quadro (Variacao.h): a sequencia da fase 2, de onde
    // cada fileira do flexbox tira a regra dela (Flex::Proxima). E uma funcao
    // da semente da luta e do numero da fileira, e nao do que o aluno fez.

    void SortearALuta();

    bool mLutaSorteada = false;
    Variacao::Sorteio mSorteioDoFlex{0u};



    static constexpr const char* kFaseDoFlex = "StateTwo";


    /**
     * Para onde a ultima fileira do flexbox se rearranjou: a seguinte sai dali.
     * A primeira sai do "de" do arquivo, guardado no primeiro disparo -
     * GUARDADO, e nao relido: o params e o mesmo objeto a cada disparo, e
     * depois da primeira troca ele ja nao diz o que o arquivo disse (Boss.h).
     */
    bool mGuardouOFlex = false;
    Flex::Justificar mUltimoArranjo = Flex::Justificar::Centro;




    /// Quantos processos cairam desde a ultima rodada da fase 3. Ver
    /// OnProjetilDerrubado.
    int mParaReiniciar = 0;

    /// Quantos processos a rodada da fase 3 tem pelo ARQUIVO, guardado no
    /// primeiro disparo pelo mesmo motivo do flexbox: depois do primeiro
    /// reinicio, params ja nao diz o que o arquivo disse.
    int mProcessosDoArquivo = -1;

    /// O MAXIMO QUE O SUPERVISOR DEVOLVE NUMA RODADA. Sem teto, um aluno que
    /// derrubasse doze processos num segundo receberia doze de uma vez - o
    /// supervisor viraria castigo, e o Elixir nao castiga: ele so insiste.
    static constexpr int kReiniciosPorRodada = 4;
};
