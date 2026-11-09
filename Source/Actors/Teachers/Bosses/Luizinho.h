//
// O chefe da INF 340 - Linguagens de Programacao.
//

#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <vector>

#include <SDL_rect.h>

#include "../Boss.h"
#include "../../../Attacks/Amarracao.h"
#include "../../../Attacks/Coercao.h"
#include "../../../Attacks/Excecao.h"
#include "../../../Attacks/Termina.h"

class BossProjectile;
class DrawLinhasComponent;
class EtiquetaDeCodigo;
class PainelDeCodigo;

/// O professor de Linguagens de Programacao (INF 340). A ideia do chefe: ele e conhecido por saber
/// de onde vem cada expressao, quase sempre do latim.
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
    // ---- A luta sorteada ----

    void SortearALuta();

    bool mLutaSorteada = false;
    Amarracao::Programa mPrograma;
    bool mFAEsquerda = true;
    std::vector<int> mOrdemDosTipos;
    Variacao::Sorteio mSorteioDasRodadas{0u};

    /// O degrau do modo, fase por fase (o do Normal e o de sempre).
    Amarracao::Degrau mDegrauDasAmarracoes;
    Coercao::Degrau mDegrauDosTipos;
    Termina::Degrau mDegrauDoTermina;
    Excecao::Degrau mDegrauDasExcecoes;

    // ---- O que as fases dividem ----

    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram.
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, int quantos = 0,
                                            std::optional<float> centro = std::nullopt,
                                            std::optional<float> abertura = std::nullopt,
                                            float rapidez = 1.0f);

    bool mDisparandoEm = false;
    Vector2 mOndeDisparar;
    int mQuantos = 0;
    std::optional<float> mCentroEm;
    std::optional<float> mAberturaEm;
    std::vector<BossProjectile*> mCriados;

    /// O angulo, em graus, de 'de' ate o aluno.
    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;

    /// Para onde o movimento "Conduzido" leva o professor.
    std::optional<Vector2> mAlvo;

    /// As linhas do campo - a busca do nome, a chamada de f, a conta -, limpas a cada quadro e
    /// redesenhadas por quem estiver rodando.
    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 80;

    // ---- A fase 1: as amarracoes ----

    void AtualizarAmarracoes();
    void MontarAmarracoes();
    void DesmontarAmarracoes();
    void ComecarComando(float agora);
    /// O dono do x responde: o leque com o valor dele, mirado no aluno.
    void Responder();
    void DesenharBusca();
    [[nodiscard]] Vector2 OndeFicaOCanto(Amarracao::Bloco b) const;
    void PintarCanto(Amarracao::Bloco b, SDL_Color moldura);

    bool mNasAmarracoes = false;
    bool mComandoEmCurso = false;
    float mInicioDoComando = 0.0f;
    float mProximoComando = 0.0f;
    Amarracao::Comando mComando;
    Amarracao::Bloco mBlocoAnterior = Amarracao::Bloco::Global;
    std::vector<Amarracao::Bloco> mBuscaDoComando;
    size_t mSaltos = 0;
    int mRajadas = 0;
    int mChamadas = 0;   ///< quantas das duas chamadas de f por g ja rodaram

    /// Um painel por bloco (o global nao tem: e o campo) e um canto por bloco, com a declaracao.
    std::array<PainelDeCodigo*, Amarracao::kBlocos> mPaineis{};
    std::array<EtiquetaDeCodigo*, Amarracao::kBlocos> mCantos{};
    std::array<float, Amarracao::kBlocos> mLarguraDosCantos{};
    static constexpr int kOrdemDosPaineis = 52;
    static constexpr int kOrdemDoInterno = 54;

    // ---- A fase 2: os sistemas de tipos ----

    void AtualizarTipos();
    void DesmontarTipos();
    void ComecarConta(int k);
    void LancarOperandos();
    void Compilar();
    void FazerAConta();
    void LimparConta();

    bool mNosTipos = false;
    int mContaDaVez = -1;
    std::vector<float> mHorariosDosTipos;   ///< quando cada conta comeca (Coercao::Horarios)
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

    // ---- A fase 3: o programa Termina ----

    void AtualizarTermina();
    void MontarTermina();
    void DesmontarTermina();
    void ComecarRodadaDoTermina(float agora);
    void ComecarParadoxo(float agora);
    /// Uma volta do laco: um anel saindo do professor, girado em relacao ao da volta anterior.
    void DarVolta();
    void MostrarVeredito(std::optional<bool> veredito);
    void EstourarOTermina();
    void EscreverContador(const std::string& texto, SDL_Color moldura);

    bool mNoTermina = false;
    bool mRodadaDoTerminaEmCurso = false;
    float mInicioDaRodadaDoTermina = 0.0f;
    float mProximaRodadaDoTermina = 0.0f;
    Termina::Rodada mRodadaDoTermina;
    int mProgramaAnterior = -1;
    std::optional<bool> mVeredito;
    std::vector<int> mValoresDoLaco;   ///< o n de cada volta, ate parar (ou kMaisVoltas)
    int mVoltasDadas = 0;
    float mCtrlC = 0.0f;               ///< quanto o laco infinito roda ate o ^C
    float mFimDaRodadaDoTermina = -1.0f;
    float mGiroDaEspiral = 90.0f;

    /// O Paradoxo: comecou, em qual virada esta, e se o Termina ja estourou.
    bool mParadoxoFeito = false;
    bool mNoParadoxo = false;
    int mVirada = -1;
    bool mEstourou = false;
    float mProximaVolta = 0.0f;

    /// A caixa do Termina no canto de cima, com a pergunta e a resposta, e o contador do laco
    /// embaixo do professor.
    PainelDeCodigo* mCaixaDoTermina = nullptr;
    EtiquetaDeCodigo* mPerguntaDoTermina = nullptr;
    EtiquetaDeCodigo* mRespostaDoTermina = nullptr;
    EtiquetaDeCodigo* mContador = nullptr;
    static constexpr float kYDoProfessorNoTermina = 150.0f;

    // ---- A final: as excecoes ----

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
