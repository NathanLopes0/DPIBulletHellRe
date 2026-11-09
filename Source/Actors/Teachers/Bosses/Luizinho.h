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
#include "../../../Attacks/Coercao.h"
#include "../../../Attacks/Complexidade.h"
#include "../../../Attacks/Excecao.h"
#include "../../../Attacks/Polimorfismo.h"

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
    Polimorfismo::Plano mPlanoDasFormas;
    std::vector<int> mOrdemDosTipos;
    Complexidade::Plano mPlanoDaComplexidade;
    Variacao::Sorteio mSorteioDasRodadas{0u};

    /// O degrau do modo, fase por fase (o do Normal e o de sempre).
    Polimorfismo::Degrau mDegrauDasFormas;
    Coercao::Degrau mDegrauDosTipos;
    Complexidade::Degrau mDegrauDaComplexidade;
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

    /// As linhas do campo - a referencia da chamada, o leque, o grafico, a conta -, limpas a cada
    /// quadro e redesenhadas por quem estiver rodando.
    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 80;

    // ---- A fase 1: o polimorfismo ----

    void AtualizarFormas();
    void DesmontarFormas();
    /// Os pontos que ja partiram: cada um some no fim da vida dele ou da fase, e no Dificil gira em
    /// volta do objeto que o desenhou.
    void AtualizarTracos();
    struct TracoNoAr {
        BossProjectile* p = nullptr;
        Vector2 centro;
        float vidaAoPartir = 0.0f;
        float giro = 0.0f;   ///< graus por segundo; zero no Normal
    };
    std::vector<TracoNoAr> mTracosNoAr;
    void ComecarChamada(float agora);
    void DesenharFormas(float agora);

    bool mNasFormas = false;
    float mProximaChamada = 0.0f;
    float mInicioDaChamada = 0.0f;
    bool mChamadaEmCurso = false;
    Polimorfismo::Chamada mChamada;

    /// Um objeto da chamada em campo: o rotulo, o lugar, e o contorno que ele desenha.
    struct Objeto {
        EtiquetaDeCodigo* rotulo = nullptr;
        Polimorfismo::Classe classe = Polimorfismo::Classe::Circulo;
        Vector2 lugar;
        float invoca = 0.0f;       ///< f.desenhar(): a referencia aponta para ele
        float traca = 0.0f;        ///< o contorno comeca a ser desenhado
        bool revelado = false;
        bool partiu = false;
        float partiuEm = 0.0f;
        float giro = 0.0f;
        int tracados = 0;
        std::vector<Vector2> contorno;
        std::vector<BossProjectile*> pontos;
    };
    std::vector<Objeto> mObjetos;

    /// Os rotulos dos objetos, criados uma vez e reaproveitados.
    std::vector<EtiquetaDeCodigo*> mRotulos;
    static constexpr int kLarguraDoRotulo = 210;
    static constexpr int kOrdemDosRotulos = 57;
    static constexpr float kYDoProfessorNasFormas = 120.0f;

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

    // ---- A fase 3: a complexidade ----

    void AtualizarComplexidade();
    void DesmontarComplexidade();
    void ComecarRodadaDaComplexidade(float agora);
    /// Uma operacao na casa do leque; a que 'acha' o aluno mira nele, pelo gatilho do acesso.
    void Operar(int casa, bool acha);
    /// A cara do custo da ordem, e o tamanho do degrau.
    void VestirOperacao(BossProjectile* p) const;
    void Dobrar();
    void DesenharLeque(float agora);
    void DesenharGrafico();

    bool mNaComplexidade = false;
    int mVezDaComplexidade = 0;
    int mOrdemAnterior = -1;
    float mProximaRodadaDaComplexidade = 0.0f;
    bool mRodadaEmCurso = false;
    Complexidade::Rodada mRodadaDaComplexidade;

    /// As leituras de uma passada pelo leque: quando, e qual casa.
    struct Leitura { float quando = 0.0f; int casa = 0; };
    std::vector<Leitura> mLeituras;
    size_t mProximaLeitura = 0;

    /// A busca binaria: o intervalo que sobra, e quando le o proximo meio.
    Complexidade::Intervalo mBusca;
    float mProximoMeio = 0.0f;
    int mMeiosLidos = 0;

    /// Os subconjuntos: cada galho e um tiro que, a cada dobra, vira dois.
    struct Galho { BossProjectile* p = nullptr; float angulo = 90.0f; };
    std::vector<Galho> mGalhos;
    int mDobras = 0;
    float mProximaDobra = 0.0f;

    /// Quando cada casa do leque foi lida pela ultima vez: ela acende por um instante.
    std::vector<float> mLidaEm;

    /// O grafico das ordens, no canto de cima a direita.
    PainelDeCodigo* mQuadroDoGrafico = nullptr;
    EtiquetaDeCodigo* mNomeDoGrafico = nullptr;
    static constexpr float kYDoProfessorNaComplexidade = 160.0f;
    /// O leque das casas do vetor: abaixo do cartao do professor, e de onde sai cada operacao.
    static constexpr float kRaioDoLeque = 92.0f;

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
