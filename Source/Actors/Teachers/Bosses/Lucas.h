//
// O chefe da INF 321 - Programacao Web.
//

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Dom.h"
#include "../../../Attacks/Elixir.h"
#include "../../../Attacks/Flex.h"

class BossProjectile;

/**
 * @class Lucas
 * @brief O professor de Programacao Web (INF 321).
 *
 * A IDEIA DO CHEFE: ele da a materia na ordem classica - HTML, CSS, JavaScript
 * - e no fim ensina o que ele gosta de verdade, que e Elixir no back-end. A
 * batalha conta a mesma historia. As duas primeiras fases sao o front; a
 * terceira, a que decide, e o Elixir; a final volta ao front, agora assincrono.
 *
 *   fase 1  HTML        um documento escrito tag por tag: cada elemento aberto
 *                       e uma caixa, os filhos dentro do pai, e cada tag
 *                       fechada vira a caixa do avesso (Dom.h)
 *   fase 2  CSS         o flexbox: fileiras que caem e colunas que atravessam
 *                       de lado, cada uma se rearranjando no justify-content
 *                       do placar (Flex.h)
 *   fase 3  Elixir      processos que perseguem e mandam mensagens, e que o
 *                       supervisor reinicia (Elixir.h)
 *   final   JavaScript  promessas que resolvem ou sao rejeitadas
 *
 * O BALANCEAMENTO VIVE EM DADOS, como o dos outros: Assets/Attacks/lucas/. O
 * que mora aqui e o que dados nao sabem fazer: o relogio do documento, a
 * direcao de cada disparo do flexbox, o rodizio das mensagens e o supervisor.
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

    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override { return mAlvo; }

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A LUTA SORTEADA ----------------------------------------------------
    //
    // Antes do primeiro quadro (Variacao.h): de onde saem os documentos da fase
    // 1 e a sequencia da fase 2 - a regra de cada fileira (Flex::Proxima) e o
    // lado de cada coluna (Flex::DirecaoDoDisparo) -, e o degrau do modo.

    void SortearALuta();

    bool mLutaSorteada = false;
    Variacao::Sorteio mSorteioDoDom{0u};
    Variacao::Sorteio mSorteioDoFlex{0u};
    Dom::Degrau mDegrauDoDom;
    Elixir::Degrau mDegrauDoElixir;

    std::optional<Vector2> mAlvo;

    /// Dispara 'gatilho' de 'onde', no rumo 'rumo' - ou mirado no aluno, sem
    /// rumo. Devolve o que nasceu. 'escala' encolhe a caixa da CaixaAttack.
    std::vector<BossProjectile*> DispararDe(const char* gatilho, Vector2 onde, std::optional<float> rumo,
                                            float escala = 1.0f);
    [[nodiscard]] Vector2 PosicaoDoAluno() const;

    bool mDisparandoDe = false;
    Vector2 mOndeDisparar;
    float mRumoDoDisparo = 90.0f;
    float mEscalaDoDisparo = 1.0f;
    std::vector<BossProjectile*> mDisparados;

    /// A caixa da CaixaAttack como o arquivo a escreve, guardada no primeiro
    /// disparo: params e o mesmo objeto a cada disparo (Boss.h), e encolher
    /// sobre o anterior encolheria para sempre.
    float mMeiaAlturaDoArquivo = -1.0f;
    int mTagsDoArquivo = -1;

    // ---- O DOCUMENTO DA FASE 1 ------------------------------------------------
    //
    // Uma tag a cada Dom::Degrau::porEvento. Abrir dispara a caixa do elemento
    // do lugar dele (o gatilho "abrir"), menor a cada geracao; fechar vira do
    // avesso a caixa que ele abriu - as tags que ainda estao em campo voltam,
    // passam pelo centro e saem do outro lado; o vazio e um anel ("vazio"). O
    // professor vai ate o proximo elemento.

    void AtualizarDom();

    struct Caixa {
        std::vector<BossProjectile*> tags;
        float aberta = 0.0f;   ///< no relogio da fase
    };

    bool mNoDom = false;
    std::string mAnuncioDoDom;
    Dom::Documento mDocumento;
    std::vector<Dom::Evento> mEventos;
    size_t mProximo = 0;
    float mProximoEvento = 0.0f;
    std::vector<Caixa> mCaixas;   ///< por elemento do documento

    // ---- O FLEXBOX DA FASE 2 ----------------------------------------------------

    static constexpr const char* kFaseDoFlex = "StateTwo";

    /**
     * Para onde a ultima fileira do flexbox se rearranjou: a seguinte sai dali.
     * A primeira sai do "de" do arquivo, guardado no primeiro disparo -
     * GUARDADO, e nao relido: o params e o mesmo objeto a cada disparo, e
     * depois da primeira troca ele ja nao diz o que o arquivo disse (Boss.h).
     */
    bool mGuardouOFlex = false;
    Flex::Justificar mUltimoArranjo = Flex::Justificar::Centro;
    void AtualizarFlex();
    bool mNoFlex = false;
    float mXDoComecoDaVarredura = 0.0f;
    float mLarguraDaFileira = 0.0f;

    int mItensDaFileira = 8;

    int mDisparosDoFlex = 0;

    // ---- O ELIXIR DA FASE 3 -------------------------------------------------------

    void AtualizarElixir();

    bool mNoElixir = false;
    std::string mAnuncioDoElixir;
    float mProximaMensagem = 0.0f;
    int mMensagens = 0;

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
