//
// O chefe da INF 340 - Linguagens de Programacao.
//

#pragma once

#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <SDL_rect.h>

#include "../Boss.h"
#include "../../../Attacks/Coercao.h"
#include "../../../Attacks/Escopo.h"
#include "../../../Attacks/Excecao.h"
#include "../../../Attacks/Memoria.h"

class BossProjectile;
class DrawLinhasComponent;
class EtiquetaDeCodigo;
class PainelDeCodigo;

/**
 * @class Luizinho
 * @brief O professor de Linguagens de Programacao (INF 340).
 *
 * A IDEIA DO CHEFE: ele e conhecido por saber de onde vem cada expressao, quase
 * sempre do latim. Cada fase e um assunto da ementa, e o placar abre com a
 * palavra latina por tras dele; depois, como nas fases do Andre, o placar
 * mostra o codigo que ele roda, e o campo e a execucao desse codigo.
 *
 *   fase 1  Amarracoes          ligare        atar
 *   fase 2  Sistemas de tipos   coercere      forcar
 *   fase 3  Gerencia de memoria memento mori  lembra-te de que morreras
 *   final   Sequenciadores      sequi         seguir
 *
 * A FASE 1 E O ESCOPO (Escopo.h): o campo e o texto do programa, um painel por
 * bloco com as declaracoes no canto. O professor vai ate o bloco do comando,
 * a busca do nome sobe de bloco em bloco - pelo texto no escopo estatico, por
 * quem chamou no dinamico -, e a variavel achada chove na regiao em que e
 * visivel: o bloco dela menos os de dentro que declaram outra com o mesmo
 * nome.
 *
 * A FASE 2 SAO OS SISTEMAS DE TIPOS (Coercao.h): a mesma conta em linguagens
 * diferentes. O operador aparece em cima do aluno, os operandos voam do
 * professor ate ele - e o convertido troca de chip no meio do voo -, e o
 * resultado ataca do jeito do tipo dele: o numero e um anel, o texto a
 * palavra, o TypeError estoura onde a conta roda e o erro de compilacao no
 * professor.
 *
 * A FASE 3 E A GERENCIA DE MEMORIA (Memoria.h), e a que decide: o campo e o
 * heap, e o alocador poe cada bloco no lugar livre mais perto do aluno. As
 * variaveis do professor tem uma etiqueta e uma linha ate o bloco delas. No
 * Java o que ninguem alcanca fica cinza ate o System.gc() parar o mundo e
 * derruba-lo; no C++ o delete deixa o ponteiro pendente, a escrita nele
 * estoura, e o laco que so aloca vaza ate o bad_alloc.
 *
 * A FINAL SAO OS SEQUENCIADORES (Excecao.h), e a repescagem e um catch: o
 * campo e a pilha de chamadas, uma faixa por chamada. O professor desce ate a
 * mais funda e lanca a excecao; ela sobe procurando quem a trate, o finally
 * roda no caminho, e cada faixa que nao a trata e desempilhada e cai em
 * destrocos sobre o aluno.
 *
 * O BALANCEAMENTO VIVE EM DADOS, como o dos outros: Assets/Attacks/luizinho/.
 */
class Luizinho : public Boss {
public:
    explicit Luizinho(Scene* scene);
    ~Luizinho() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;
    void AoTerminarABatalha() override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- O QUE AS FASES DIVIDEM ---------------------------------------------

    [[nodiscard]] SDL_FRect Campo() const;

    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram.
    /// 'quantos' > 0 troca o numero de projeteis do ataque, 'centro' o meio do
    /// leque e 'abertura' a abertura dele.
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, int quantos = 0,
                                            std::optional<float> centro = std::nullopt,
                                            std::optional<float> abertura = std::nullopt);

    bool mDisparandoEm = false;
    Vector2 mOndeDisparar;
    int mQuantos = 0;
    std::optional<float> mCentroEm;
    std::optional<float> mAberturaEm;
    std::vector<BossProjectile*> mCriados;

    [[nodiscard]] Vector2 PosicaoDoAluno() const;

    /// O angulo, em graus, de 'de' ate o aluno.
    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;

    /// Para onde o movimento "Conduzido" leva o professor.
    std::optional<Vector2> mAlvo;

    /// O placar: o que a fase diz no arquivo (o latim) e guardado na primeira
    /// vez, e volta quando a fase acaba.
    void Anunciar(const std::string& fase, const std::string& texto);
    void Restaurar(const std::string& fase);
    std::map<std::string, std::string> mAnunciosDoArquivo;

    /// As linhas do campo - a busca do nome, a chamada -, limpas a cada quadro
    /// e redesenhadas por quem estiver rodando.
    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 80;

    // ---- A FASE 1: O ESCOPO -------------------------------------------------

    void AtualizarEscopo();
    void MontarEscopo();
    void DesmontarEscopo();
    void ComecarComando(int k);
    void ChoverAvisos();
    void AcenderGotas();
    void SecarGotas();
    void DesenharEscopo(float u);

    bool mNoEscopo = false;
    int mComandoDoEscopo = -1;
    float mInicioDoComando = 0.0f;
    Escopo::Forma mForma = Escopo::Forma::XEmG;

    /// A chuva de um comando: espera a busca, nasce apagada (o aviso), acende
    /// e cai, e seca.
    enum class Chuva { Esperando, Aviso, Viva, Seca };
    Chuva mChuva = Chuva::Esperando;

    struct GotaEmCampo { BossProjectile* p = nullptr; float fim = 0.0f; };
    std::vector<GotaEmCampo> mGotas;

    /// Um painel e um canto por bloco (o global nao tem painel - e o campo -,
    /// e o cartao de f e so o canto). Os atores sao da cena; o professor os
    /// cria quando a fase abre e os destroi quando ela fecha.
    std::array<PainelDeCodigo*, Escopo::kBlocos> mPaineis{};
    std::array<EtiquetaDeCodigo*, Escopo::kBlocos> mCantos{};

    /// Os paineis atras do placar (60) e dos projeteis; o de dentro na frente
    /// do de fora.
    static constexpr int kOrdemDosPaineis = 52;
    static constexpr int kOrdemDoInterno = 54;
    static constexpr int kOrdemDosCantos = 57;

    // ---- A FASE 2: OS SISTEMAS DE TIPOS -------------------------------------

    void AtualizarTipos();
    void DesmontarTipos();
    void ComecarConta(int k);
    void LancarOperandos();
    void Compilar();
    void FazerAConta();
    void LimparConta();

    bool mNosTipos = false;
    int mContaDaVez = -1;
    float mInicioDaConta = 0.0f;
    Coercao::Comando mConta;
    Vector2 mOndeDaConta;
    Vector2 mBaseDoProfessor;   ///< o meio do balanco dele nesta conta

    /// A conta: espera a saida, os operandos voam (ou, na tipagem estatica,
    /// esperam o compilador no professor), e ela roda.
    enum class EtapaDaConta { Esperando, Voando, Checando, Feita };
    EtapaDaConta mEtapa = EtapaDaConta::Esperando;

    struct Operando {
        BossProjectile* p = nullptr;
        Vector2 de;
        Vector2 para;
        float lado = 0.0f;           ///< -1 o da esquerda, +1 o da direita: o arco do voo
        std::string depois;          ///< a cara depois da coercao; vazia se nao converte
        bool trocou = false;
    };
    std::array<Operando, 2> mOperandos{};
    BossProjectile* mOperador = nullptr;

    /// O arco do voo dos operandos, para os dois nao andarem na mesma linha.
    static constexpr float kArcoDoVoo = 46.0f;

    // ---- A FASE 3: A GERENCIA DE MEMORIA -------------------------------------

    void AtualizarMemoria();
    void DesmontarMemoria();
    void ComecarPasso(int i);
    void Executar(const Memoria::Evento& e);
    void Alocar(const Memoria::Evento& e);
    void Coletar();
    void LiberarBloco(const std::string& raiz);
    void EscreverNoPendente(const Memoria::Evento& e);
    void EstourarOsVazados();
    void LimparHeap();
    void PintarHeap();
    void DesenharHeap();

    /// Congela (ou descongela) TODOS os projeteis do professor - stop the world.
    void PararOMundo(bool parar) const;

    bool mNaMemoria = false;
    int mPassoDaVez = -1;
    std::vector<Memoria::Evento> mEventos;
    size_t mProximoEvento = 0;
    Vector2 mBaseDaMemoria;

    /// O heap de verdade (Memoria.h) e o que esta em campo: um bloco por
    /// alocacao, com as celulas dele. O que o delete ou a varredura levou sai
    /// daqui; o que vazou fica ate o bad_alloc.
    Memoria::Heap mHeap;
    struct BlocoEmCampo {
        int id = 0;
        Vector2 centro;
        int celulas = 0;
        bool java = false;
        bool no = false;              ///< um No do Java: a segunda celula e o prox
        float acendeEm = 0.0f;        ///< no relogio da fase
        bool aceso = false;
        std::vector<BossProjectile*> p;
    };
    std::vector<BlocoEmCampo> mBlocos;

    /// O mundo parado do System.gc(): o professor e os projeteis esperam.
    bool mMundoParado = false;

    /// Onde estava o bloco que o delete levou - para onde o ponteiro pendente
    /// continua apontando.
    std::optional<Memoria::Retangulo> mLiberado;
    int mCelulasDoLiberado = 0;

    /// As raizes: a etiqueta de cada variavel do professor ("a" no Java, "p"
    /// no C++), embaixo dele.
    std::map<std::string, EtiquetaDeCodigo*> mRaizes;
    static constexpr float kAbaixoDoProfessor = 74.0f;

    // ---- A FINAL: AS EXCECOES ------------------------------------------------

    void AtualizarExcecoes();
    void DesmontarExcecoes();
    void ComecarRodada(int k);
    void Empilhar(int i);
    void Desempilhar(int i, bool quebrando);
    void ExecutarPasso(const Excecao::Passo& p);
    void PintarFaixa(int i, SDL_Color moldura, SDL_Color fundo);
    [[nodiscard]] Vector2 MeioDaFaixa(int i) const;

    bool mNasExcecoes = false;
    int mRodadaDaVez = -1;
    Excecao::Rodada mRodada;
    std::vector<Excecao::Passo> mPassos;
    size_t mProximoPasso = 0;
    size_t mEmpilhados = 0;
    bool mLancou = false;
    bool mRetornou = false;
    int mCapturouEm = -1;
    float mXDaRodada = 0.0f;

    /// A excecao em campo: um enfeite com o nome do tipo, que sobe pela pilha.
    BossProjectile* mExcecao = nullptr;

    /// Uma faixa por chamada empilhada: o painel e o canto com o que ela faz
    /// com a excecao. Os atores sao da cena; o professor os cria e destroi.
    struct FaixaEmCampo {
        PainelDeCodigo* painel = nullptr;
        EtiquetaDeCodigo* canto = nullptr;
    };
    std::array<FaixaEmCampo, Excecao::kMaiorPilha> mFaixas{};
};
