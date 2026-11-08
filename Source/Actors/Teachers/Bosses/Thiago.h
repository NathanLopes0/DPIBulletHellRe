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

    /// O retangulo do campo, e o meio de uma casa e o lugar do valor dentro dela.
    [[nodiscard]] SDL_FRect Campo() const;
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

    // ---- A fase 2: o insert, o update e o delete ----

    void AtualizarEscrita();

    /// O delete apaga a linha de verdade: os valores dela deixam de ser enfeite e caem pelo campo,
    /// e as linhas de baixo sobem.
    void Apagar(int linha);

    bool mNaEscrita = false;
    float mProximaEscrita = 0.0f;
    std::string mAnuncioDaEscrita;

    struct Execucao { Banco::Escrita escrita; float quando = 0.0f; };
    std::vector<Execucao> mExecucoes;

    // ---- A fase 3: as transacoes ----

    void AtualizarTransacoes();

    /// O update da transacao na linha: o placar, o marcador, e o valor que vai ser escrito quando
    /// as escritas sairem.
    void Anunciar(int id, int linha, float agora);

    /// As escritas saem do professor para as casas da linha, e o valor da nota
    /// fica sujo.
    void Gravar(int id, int linha);

    /// Libera os locks.
    void Terminar(int id, bool desfaz, float agora);

    /// As escritas de uma transacao desfeita, esperando a varredura que desfaz
    /// partir para voltarem com ela.
    struct Volta { int tx = 0; float quando = 0.0f; };
    std::vector<Volta> mVoltas;

    enum class Etapa { MarcandoPrimeira, Segurando, Esperando, MarcandoSegunda, Fechando };

    struct TxEmCampo {
        int id = 0;
        Etapa etapa = Etapa::MarcandoPrimeira;
        float proximo = 0.0f;
        int linhaDaVez = -1;                              ///< A linha do update em curso.
        std::vector<std::pair<int, int>> antes;
        std::vector<std::pair<int, int>> novas;           ///< (linha, nota nova)
    };

    struct Gravacao {
        BossProjectile* p = nullptr;
        int tx = 0;
        Vector2 origem;
        Vector2 destino;
        bool parada = false;
        bool voltando = false;
    };

    bool mNasTransacoes = false;
    float mProximaTransacao = 0.0f;
    int mProximoId = 1;
    std::string mAnuncioDasTransacoes;
    Transacao::Txs mLocks;
    std::vector<TxEmCampo> mEmCampo;
    std::vector<Gravacao> mGravacoes;

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
    /// A grade da tabela: atras do quadro (60) e de tudo o mais, e apagada -
    /// e chao, e nao aviso.
    static constexpr int kOrdemDaGrade = 50;
    static inline const SDL_Color kCorDaGrade{110, 130, 190, 38};
    static inline const SDL_Color kCorDoCabecalho{150, 170, 230, 80};
};
