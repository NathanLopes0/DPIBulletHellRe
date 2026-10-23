//
// Thiago - professor de Banco de Dados (INF 220)
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include <SDL_rect.h>

#include "../Boss.h"
#include "../../../Attacks/Banco.h"

class BossProjectile;

/**
 * @class Thiago
 * @brief O chefe que trata o campo de jogo como uma TABELA e o jogador como uma
 *        linha dela.
 *
 * A IDEIA DA BATALHA. Os outros chefes miram no jogador; o Thiago CONSULTA. Cada
 * ataque e uma leitura de uma parte da tabela, anunciada antes: as casas que vao
 * ser lidas acendem. O jogador nao desvia de projetil - ele sai das casas que a
 * consulta le. A pergunta que a batalha faz nao e "voce consegue passar entre
 * as balas?", e sim "voce consegue ler a consulta a tempo?".
 *
 * A TABELA ESTA ESCRITA NO CAMPO (Banco.h): os cabecalhos das cinco colunas no
 * alto e os quatro registros, um por linha, cada valor na sua casa. Ela nasce na
 * primeira fase e fica ate o fim da batalha. Aqui mora a tabela em campo.
 *
 * A FASE 1 E O SELECT: a selecao varre as linhas que satisfazem o WHERE, a
 * projecao varre a coluna, as duas juntas so as casas do cruzamento, e o ORDER
 * BY troca as linhas de lugar. O placar e a consulta, e o professor vai para
 * cima da coluna que ela le.
 *
 * QUEM ESCOLHE A FAIXA DAS FASES ANTIGAS E ESTE ARQUIVO, nao a estrategia: so o
 * chefe sabe onde o jogador esta, e a ConsultaAttack nao deve saber. Ver
 * CustomizeAttackParams.
 */
class Thiago : public Boss {

public:
    explicit Thiago(Scene* scene);
    ~Thiago() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    /// Em que faixa o jogador esta AGORA, no eixo que a consulta pede.
    [[nodiscard]] int FaixaDoJogador(const struct ConsultaAttackParams& consulta);

    /// Contador da varredura das fases antigas. Vive aqui, e nao na
    /// estrategia: a estrategia e sem memoria de proposito.
    int mProximaFaixa = 0;

    // ---- A TABELA VISIVEL ---------------------------------------------------

    /// Monta os cabecalhos e as casas, uma vez: a tabela fica a batalha inteira.
    void CriarTabela();

    /// Escreve um registro na linha 'linha' da tabela: um valor por coluna,
    /// nascendo em 'de' (e indo para o lugar dele).
    void EscreverRegistro(size_t linha, Vector2 de);

    /// Cada casa no lugar dela - e a que trocou de linha num ORDER BY, a
    /// caminho da nova.
    void AtualizarTabela(float deltaTime);

    /// O retangulo do campo, e o meio de uma casa e o lugar do valor dentro dela.
    [[nodiscard]] SDL_FRect Campo() const;
    [[nodiscard]] Vector2 CentroDaCasa(int linha, int coluna) const;
    [[nodiscard]] Vector2 LugarDoDado(int linha, int coluna) const;

    Banco::Registros mRegistros;
    std::vector<std::array<BossProjectile*, Banco::kColunas>> mCasas;   ///< por linha
    std::array<BossProjectile*, Banco::kColunas> mCabecalhos{};
    bool mTabelaCriada = false;

    // ---- O DISPARO NUM LUGAR ------------------------------------------------
    //
    // Todo ataque do Thiago nasce onde ele diz: o valor na casa, o marcador em
    // cima dela, o clarao dentro dela, a varredura na faixa dela.

    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram.
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde);

    /// Varre a linha ou a coluna 'indice' (gatilho da selecao ou da projecao).
    void Varrer(const char* gatilho, int indice, float aviso, float velocidade);

    bool mDisparandoEm = false;
    Vector2 mOndeDisparar;
    int mIndiceDaConsulta = 0;
    float mAvisoDaConsulta = 1.0f;
    float mVelocidadeDaConsulta = 700.0f;
    std::vector<BossProjectile*> mCriados;

    // ---- A FASE 1: O SELECT -------------------------------------------------

    void AtualizarSelecao(float deltaTime);

    /// Acende o marcador de cada casa ate 'ate'.
    void Marcar(const std::vector<Banco::Casa>& casas, float ate);

    /// Acende o marcador de cada casa da linha (ou da coluna) varrida, ate a
    /// varredura passar por ela.
    void MarcarVarredura(const std::vector<Banco::Casa>& casas, bool porLinha, float agora, float aviso);

    bool mNaSelecao = false;
    float mProximaConsulta = 0.0f;
    int mVez = 0;
    std::string mAnuncioDaSelecao;

    struct Marca { BossProjectile* p = nullptr; float ate = 0.0f; };
    std::vector<Marca> mMarcas;

    /// As casas que vao acender (as duas operacoes juntas), e quando.
    struct Pendente { std::vector<Banco::Casa> casas; float quando = 0.0f; };
    std::vector<Pendente> mPendentes;

    /// Os pedacos de cada casa que acendeu, e ate quando ficam.
    struct Clarao { BossProjectile* p = nullptr; float ate = 0.0f; };
    std::vector<Clarao> mClaroes;

    /// As casas que acenderam ou vao acender, de qualquer fase: o que tem hora
    /// para acabar acaba, e o fim da fase apaga tudo.
    void AtualizarCasas(float agora);
    void ApagarCasas();

    // ---- A FASE 2: O INSERT, O UPDATE E O DELETE ----------------------------

    void AtualizarEscrita();

    /// O DELETE apaga a linha de verdade: os valores dela deixam de ser
    /// enfeite e caem pelo campo, e as linhas de baixo sobem.
    void Apagar(int linha);

    bool mNaEscrita = false;
    float mProximaEscrita = 0.0f;
    std::string mAnuncioDaEscrita;

    /// O comando que ja foi anunciado e ainda vai acontecer (o UPDATE quando a
    /// casa acende, o DELETE quando a varredura passa).
    struct Execucao { Banco::Escrita escrita; float quando = 0.0f; };
    std::vector<Execucao> mExecucoes;

    // ---- A REPESCAGEM: O PRODUTO CARTESIANO ---------------------------------

    void AtualizarProduto();

    bool mNoProduto = false;
    int mVolta = 0;
    float mProximaVolta = 0.0f;
    std::string mAnuncioDoProduto;

    /// A casa da volta em curso, e a hora em que ela e marcada.
    struct Agendada { Banco::Casa casa; float marca = 0.0f; };
    std::vector<Agendada> mAgendadas;

    std::optional<Vector2> mAlvo;

    /// Quao depressa uma casa chega ao lugar dela (Ancora::Aproximar): devagar,
    /// para o ORDER BY ser visto.
    static constexpr float kRapidezDaCasa = 5.0f;
};
