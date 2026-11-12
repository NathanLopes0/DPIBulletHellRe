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
#include "../../../Attacks/Dml.h"
#include "../../../Attacks/Transacao.h"

class BossProjectile;

/// O chefe que trata o campo de jogo como uma tabela e o jogador como uma linha dela. A ideia da
/// batalha.
class Thiago : public Boss {

public:
    explicit Thiago(Scene* scene);
    ~Thiago() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;
    void AoTerminarABatalha() override;
    void OnDerrubou(BossProjectile* qual) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A luta sorteada ----

    void SortearALuta();

    bool mLutaSorteada = false;
    std::vector<int> mOrdemDasConsultas;
    std::vector<int> mOrdemDosPlanos;
    Banco::Degrau mDegrau;
    Dml::Degrau mDegrauDaEscrita;
    Transacao::Plano mPlanoDasTransacoes;
    Transacao::Degrau mDegrauDasTransacoes;

    // ---- A tabela visivel ----

    /// Monta os cabecalhos e as casas, uma vez: a tabela fica a batalha inteira.
    void CriarTabela();

    /// Escreve um registro na linha 'linha' da tabela: um valor por coluna,
    /// nascendo em 'de' (e indo para o lugar dele).
    void EscreverRegistro(size_t linha, Vector2 de);

    /// Cada casa no lugar dela - e a que trocou de linha num order by, a caminho da nova.
    void AtualizarTabela(float deltaTime);

    [[nodiscard]] Vector2 CentroDaCasa(int linha, int coluna) const;
    [[nodiscard]] Vector2 LugarDoDado(int linha, int coluna) const;

    Banco::Registros mRegistros;
    std::vector<std::array<BossProjectile*, Banco::kColunas>> mCasas;   ///< por linha
    std::array<BossProjectile*, Banco::kColunas> mCabecalhos{};
    bool mTabelaCriada = false;

    /// A grade da tabela, desenhada no campo: as linhas e as colunas.
    class DrawLinhasComponent* mGrade = nullptr;
    void DesenharGrade();

    // ---- O disparo num lugar ----

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

    // ---- A fase 1: o select ----

    void AtualizarSelecao(float deltaTime);

    /// Acende o marcador de cada casa ate 'ate'.
    void Marcar(const std::vector<Banco::Casa>& casas, float ate);

    /// Acende o marcador de cada casa da linha (ou da coluna) varrida, ate a
    /// varredura passar por ela.
    void MarcarVarredura(const std::vector<Banco::Casa>& casas, bool porLinha, float agora, float aviso,
                         bool invertida = false);

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

    // ---- Os registros que voam (fases 2 e 3) ----

    /// O relogio dos voos: corre em toda fase, e o que esta no ar termina a viagem mesmo que a
    /// fase acabe.
    float mRelogio = 0.0f;

    struct Voo {
        enum class Tipo { Arco, Queda, Caco };
        BossProjectile* p = nullptr;
        Tipo tipo = Tipo::Arco;
        Vector2 de;                ///< Arco: de onde sai
        Vector2 para;              ///< Arco: onde chega, se a linha sumiu
        float altura = 0.0f;       ///< Arco: o quanto ele sobe; zero e a reta
        float inicio = 0.0f;       ///< Arco: quando sai, no relogio dos voos
        float duracao = 1.0f;
        int mat = -1;              ///< Arco: a linha em que o valor fica, pela matricula
        int coluna = -1;
    };
    std::vector<Voo> mVoos;

    void AtualizarVoos(float deltaTime);

    /// O INSERT: os valores do registro da linha 'linha' aparecem em fila em 'de' e, depois de
    /// 'espera', voam em arco ate as casas da linha.
    void Arremessar(int linha, Vector2 de, float espera);

    /// O DELETE: os valores da linha caem e se espatifam no chao. A linha sai de mCasas; o
    /// registro, quem chama tira.
    void Derrubar(int linha);

    /// Os cacos de um valor que bateu no chao.
    void Espatifar(Vector2 onde);

    /// O UPDATE: o valor velho da nota da linha e chutado no aluno.
    void Chutar(int linha);

    /// O valor novo da nota da linha, voando de 'de' ate a casa.
    void EscreverNota(int linha, Vector2 de);

    /// A linha que o DELETE vai apagar: vermelha e riscada ate o comando acontecer.
    void Riscar(int linha);
    void ApagarRiscos();
    std::vector<BossProjectile*> mRiscos;

    /// Onde o que cai se espatifa.
    [[nodiscard]] float Chao() const;

    // ---- A fase 2: o INSERT, o UPDATE e o DELETE ----

    void AtualizarEscrita();

    bool mNaEscrita = false;
    float mProximaEscrita = 0.0f;
    std::string mAnuncioDaEscrita;

    /// O DELETE e o UPDATE acontecem depois do aviso.
    struct Execucao { Banco::Escrita escrita; float quando = 0.0f; };
    std::vector<Execucao> mExecucoes;

    // ---- A fase 3: as transacoes ----

    void AtualizarTransacoes();

    /// O rascunho de uma operacao numa casa: a 'casa'-esima de Transacao::Casas da operacao 'op'.
    struct Rascunho { BossProjectile* p = nullptr; int op = 0; int casa = 0; };

    struct Aberta {
        int id = 0;
        int cor = 0;                             ///< a cor da transacao: as duas abertas nunca tem a mesma
        int posicao = 0;                         ///< onde fica o selo (Transacao::kPosicoesDoSelo)
        BossProjectile* selo = nullptr;
        std::vector<Transacao::Operacao> ops;
        std::vector<Rascunho> rascunhos;
        std::vector<BossProjectile*> sujos;      ///< o valor sujo de cada UPDATE (nullptr nas outras)
        float proxima = 0.0f;                    ///< a proxima operacao, ou o COMMIT
        bool preparada = false;                  ///< ja tem todas as operacoes: o proximo e o COMMIT
    };

    /// A proxima operacao da transacao: o placar a escreve, e o rascunho dela aparece na tabela.
    void Preparar(Aberta& tx, float agora);

    /// O COMMIT: tudo o que a transacao preparou acontece de uma vez.
    void Confirmar(Aberta& tx);

    /// O fim sem COMMIT: os rascunhos e os valores sujos somem, e a tabela fica como estava.
    void Desfazer(Aberta& tx);

    /// Os rascunhos e os valores sujos acompanham as linhas, que sobem quando um DELETE acontece.
    void Acompanhar();

    [[nodiscard]] std::vector<Transacao::Operacao> Pendentes() const;
    [[nodiscard]] Vector2 OndeFicaOSelo(const Aberta& tx) const;

    bool mNasTransacoes = false;
    float mProximaTransacao = 0.0f;
    int mProximoId = 1;
    std::string mAnuncioDasTransacoes;
    std::vector<Aberta> mAbertas;

    // ---- A repescagem: o produto cartesiano ----

    void AtualizarProduto();

    bool mNoProduto = false;
    int mVolta = 0;
    float mProximaVolta = 0.0f;
    std::string mAnuncioDoProduto;

    /// A casa da volta em curso, e a hora em que ela e marcada.
    struct Agendada { Banco::Casa casa; float marca = 0.0f; };
    std::vector<Agendada> mAgendadas;

    std::optional<Vector2> mAlvo;

    /// Quao depressa uma casa chega ao lugar dela (Ancora::Aproximar): devagar, para o order by ser
    /// visto.
    static constexpr float kRapidezDaCasa = 5.0f;
    /// O registro do INSERT, escrito em fila embaixo do professor antes de voar.
    static constexpr float kEntreValoresEscritos = 62.0f;
    static constexpr float kAbaixoDoProfessor = 78.0f;
    /// A grade da tabela: atras do quadro (60) e de tudo o mais, e apagada -
    /// e chao, e nao aviso.
    static constexpr int kOrdemDaGrade = 50;
    static inline const SDL_Color kCorDaGrade{110, 130, 190, 38};
    static inline const SDL_Color kCorDoCabecalho{150, 170, 230, 80};
};
