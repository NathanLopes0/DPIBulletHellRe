//
// O chefe da INF 213 - Estruturas de Dados.
//

#pragma once

#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Capivara.h"
#include "../../../Attacks/Pilha.h"
#include "../../../Attacks/Hash.h"
#include "../../../Attacks/Arvore.h"
#include "../../../Attacks/Ordenacao.h"

/**
 * @class Salles
 * @brief O professor de Estruturas de Dados (INF 213).
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/salles/fases.json e as regras no regras.json ao lado. O que
 * mora aqui e o que dados nao sabem fazer.
 *
 * A CAPIVARA (Capivara.h). Ele poe capivara em todo exercicio, e de vez em
 * quando ela atravessa o campo no meio da fase - em qualquer fase dele. Ela e
 * um abrigo: come os projeteis dele e os tiros do aluno. Aqui mora o sorteio
 * de cada fase e o aviso no placar.
 *
 * A FASE 1 E UMA PILHA (Pilha.h), pendurada nele: push poe um bloco numerado
 * no topo, que e a ponta de baixo, e pop o tira e o atira no aluno - e o bloco
 * estoura em tantas lascas quanto o numero dele. O tiro do aluno so alcanca o
 * topo. Aqui mora a pilha em campo e o programa dela; ele vai de coluna em
 * coluna (o movimento "Conduzido"), parando em cada uma.
 *
 * A FASE 2 E UMA TABELA HASH (Hash.h): os baldes no alto, a lista de cada um
 * pendurada embaixo dele, e o placar escrevendo h(k) = k mod m. Inserir
 * pendura a chave; buscar percorre a lista atirando de cada no, e o achado sai
 * atirado - ou o NULL cai pela coluna. Com mais chaves que baldes, o rehash
 * refaz as listas. Aqui mora a tabela em campo; ele vai ao balde da operacao.
 *
 * A FASE 3 E UMA ARVORE BINARIA DE BUSCA (Arvore.h), plantada embaixo dele: a
 * chave nova desce comparando ate virar folha, e cada no por onde ela passa
 * atira no aluno. O aluno derruba nos a tiro, e cada no leva a subarvore junto.
 * Aqui mora a arvore em campo e a descida em curso.
 *
 * A REPESCAGEM E UM BUBBLE SORT (Ordenacao.h): um vetor numerado no alto, o par
 * comparado sobe, a troca pula e atira capivaras, quem chega ao lugar solta a
 * lista circular, e o vetor ordenado e entregue caindo em rampa. Ele fica
 * parado na fronteira entre o que falta e o que ja esta ordenado. Aqui mora o
 * vetor em campo e o relogio do algoritmo.
 */
class Salles : public Boss {

public:
    explicit Salles(Scene* scene);
    ~Salles() override = default;
    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;

protected:
    void CustomizeAttackParams(AttackParams &params, const std::string &stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A CAPIVARA ---------------------------------------------------------
    //
    // A cada fase nova, um sorteio diz se ela vem, quando e de que lado. Na hora,
    // o Salles puxa o gatilho "capivara" e o placar avisa por um tempo.

    void AtualizarCapivara(float deltaTime);

    std::string mFaseDaCapivara;
    Capivara::Visita mVisita;
    bool mVeio = false;
    bool mDisparandoCapivara = false;

    /// O placar antes do aviso, e quanto falta para ele voltar.
    std::string mAnuncioAntesDaCapivara;
    float mAvisoDaCapivara = 0.0f;

    // ---- A PILHA DA FASE 1 --------------------------------------------------
    //
    // Os blocos em campo, do da base (colado no professor) ao topo, cada um com
    // o numero dele. A cada Pilha::kPasso, a operacao do programa; a cada
    // quadro, cada bloco vai para o lugar dele embaixo do professor, e o bloco
    // que saiu num pop estoura na hora dele.

    void AtualizarPilha(float deltaTime);

    /// Solta a pilha inteira para baixo, aberta em leque: o overflow, e o fim
    /// da fase.
    void DerrubarPilha();

    [[nodiscard]] std::vector<BossProjectile*> NoPosto(const char* posto) const;

    std::vector<BossProjectile*> mPilha;
    std::vector<int> mValores;

    struct Popado {
        BossProjectile* bloco = nullptr;
        float idade = 0.0f;
        int valor = 0;
    };
    std::vector<Popado> mPopados;

    bool mNaPilha = false;
    float mProximaOperacao = 0.0f;
    std::string mAnuncioDaPilha;

    /// Empurrando: o bloco que o push acabou de criar.
    bool mEmpurrando = false;
    BossProjectile* mEmpurrado = nullptr;

    /// Estourando um pop: de onde, e em quantas lascas.
    bool mEstourando = false;
    Vector2 mDisparoDoEstouro;
    int mLascas = 0;

    /// Para onde o professor vai: uma das colunas, e quando ele muda.
    std::optional<Vector2> mAlvo;
    float mProximaMudanca = 0.0f;
    int mColuna = 1;
    static constexpr std::array<float, 3> kColunasDaPilha = {260.0f, 600.0f, 940.0f};
    static constexpr float kAlturaDaPilha = 133.0f;
    static constexpr float kParadoNaColuna = 2.6f;

    /// Quao depressa um bloco chega ao lugar dele (Ancora::Aproximar).
    static constexpr float kRapidezDoBloco = 14.0f;

    // ---- A TABELA HASH DA FASE 2 --------------------------------------------
    //
    // As chaves em campo, na ordem em que entraram - e o balde e a posicao de
    // cada uma na lista dele. A cada Hash::kPasso, a operacao (ou o rehash que
    // ficou pendente); a cada quadro, cada chave vai para o lugar dela, e a
    // busca em curso le o proximo no.

    void AtualizarHash(float deltaTime);

    /// Desenha os baldes de uma tabela de m baldes, desfazendo os de antes.
    void FormarBaldes(int m);

    /// Refaz a posicao de cada chave na lista do balde dela, pela ordem de
    /// entrada.
    void RefazerListas();

    /// Dispara 'gatilho' de 'onde'. 'mirar': o leque sai mirado no aluno.
    void DispararDoHash(const char* gatilho, Vector2 onde, bool mirar = false);

    struct Chave {
        BossProjectile* p = nullptr;
        int valor = 0;
        int balde = 0;
        int posicao = 0;
    };
    std::vector<Chave> mChaves;
    std::vector<BossProjectile*> mBaldes;
    int mTamanho = Hash::kTamanhos[0];
    bool mRehashPendente = false;

    struct Busca {
        bool ativa = false;
        int chave = 0;
        bool achou = false;
        std::vector<BossProjectile*> nos;   ///< os que ela le, ate o achado
        size_t lidos = 0;
        float proximo = 0.0f;
        Vector2 fim;                        ///< de onde cai o NULL
    };
    Busca mBusca;

    bool mNoHash = false;
    float mProximaOperacaoDoHash = 0.0f;
    std::string mAnuncioDoHash;

    /// Disparando do hash: de onde, e o que o disparo criou.
    bool mDisparandoDoHash = false;
    bool mMirandoDoHash = false;
    Vector2 mDisparoDoHash;
    BossProjectile* mCriadaNoHash = nullptr;

    static constexpr float kAlturaNoHash = 110.0f;
    static constexpr float kRapidezDaChave = 9.0f;
    static constexpr float kVelocidadeDoAchado = 320.0f;

    // ---- A ARVORE DA FASE 3 -------------------------------------------------
    //
    // Os nos assentados, pelo indice de heap de cada um, e a descida em curso:
    // a chave que vai da raiz ao lugar dela, um no por Arvore::kEmCadaNo.

    void AtualizarArvore(float deltaTime);

    /// O no e tudo o que pende dele caem, e saem da arvore.
    void DerrubarSubarvore(int indice);

    /// Dispara 'gatilho' de 'onde'. 'mirar': o leque sai mirado no aluno.
    void DispararDaArvore(const char* gatilho, Vector2 onde, bool mirar);

    Arvore::Nos mNosDaArvore;
    std::map<int, BossProjectile*> mProjeteisDaArvore;

    /// Os pontos da aresta de cada no ate o pai, pelo indice do no.
    std::map<int, std::vector<BossProjectile*>> mArestas;
    void AtualizarArestas();

    struct Descida {
        bool ativa = false;
        Arvore::Operacao op;
        std::vector<int> caminho;
        size_t passo = 0;
        float proximo = 0.0f;
        BossProjectile* chave = nullptr;
    };
    Descida mDescida;

    bool mNaArvore = false;
    float mProximaOperacaoDaArvore = 0.0f;
    std::string mAnuncioDaArvore;

    bool mDisparandoDaArvore = false;
    bool mMirandoDaArvore = false;
    Vector2 mDisparoDaArvore;
    BossProjectile* mCriadaNaArvore = nullptr;

    /// Quanto acima do no a chave que desce para, para os dois se verem.
    static constexpr float kAcimaDoNo = 30.0f;
    static constexpr float kRapidezDaDescida = 16.0f;
    static constexpr float kVelocidadeDaQueda = 340.0f;

    // ---- O BUBBLE SORT DA REPESCAGEM ---------------------------------------
    //
    // Os elementos em campo, na ordem do vetor do algoritmo (mBolha.v), e o
    // relogio: um passo a cada Ordenacao::kPasso, e um vetor novo depois de
    // cada entrega.

    void AtualizarOrdenacao(float deltaTime);

    /// Solta o vetor para baixo: 'entregue', cada um na velocidade do valor
    /// dele (a rampa); senao, todos juntos (o fim da fase).
    void SoltarVetor(bool entregue);

    /// Dispara 'gatilho' de 'onde'. 'mirar': o tiro sai mirado no aluno.
    void DispararDaOrdenacao(const char* gatilho, Vector2 onde, bool mirar);

    std::vector<BossProjectile*> mVetor;
    Ordenacao::Bolha mBolha;

    /// Quantos lugares o vetor tinha ao nascer. Os lugares sao os desse
    /// tamanho a fileira inteira: quem o aluno tira deixa o vetor encostar a
    /// esquerda, e nao recentrar - recentrado, todo elemento andava meio lugar,
    /// os vaos viravam elementos, e o tiro que passava pelo vao derrubava o
    /// seguinte (seis de oito numa fase so, em cascata).
    int mLugares = Ordenacao::kTamanho;
    bool mNaOrdenacao = false;
    bool mVetorEmCampo = false;
    float mProximoPasso = 0.0f;
    float mProximoVetor = 0.0f;
    std::string mAnuncioDaOrdenacao;

    /// O par que esta sendo comparado (o da esquerda), ou -1.
    int mComparando = -1;

    /// O pulo da troca: quem vai para a direita sobe, quem vai para a esquerda
    /// desce, ate mFimDoArco.
    BossProjectile* mSobe = nullptr;
    BossProjectile* mDesce = nullptr;
    float mFimDoArco = 0.0f;

    bool mDisparandoDaOrdenacao = false;
    bool mMirandoDaOrdenacao = false;
    Vector2 mDisparoDaOrdenacao;
    BossProjectile* mCriadaNaOrdenacao = nullptr;

    static constexpr float kRapidezDoElemento = 12.0f;
};
