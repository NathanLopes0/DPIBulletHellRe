//
// Andre - professor de Programacao 1 (INF 110)
//

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <SDL_rect.h>

#include "../Boss.h"
#include "../../../Attacks/Condicional.h"
#include "../../../Attacks/Funcao.h"
#include "../../../Attacks/Laco.h"
#include "../../../Attacks/Prova.h"

class BossProjectile;
class DrawLinhasComponent;

/// O chefe da primeira materia do curso: o placar e o codigo que ele esta rodando, e o campo e a
/// execucao dele.
class Andre : public Boss {

public:
    explicit Andre(Scene* scene);
    ~Andre() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- O que as tres fases dividem ----

    enum class Mecanica { Nenhuma, Condicional, Laco, Funcao };

    /// 'numero' muda a cada sessao nova.
    struct Sessao {
        Mecanica qual = Mecanica::Nenhuma;
        std::string fase;
        float agora = 0.0f;
        int numero = 0;
        int primeiraVez = 0;
        int quantos = 0;      ///< quantos comandos ela roda; 0 = ate a fase acabar
        bool vetor = false;   ///< se o vetor do chao fica em campo
        /// As formas vem na ordem desta luta (Variacao::Ordem).
        bool sorteada = false;
    };
    Sessao SessaoAtual();
    std::string mChaveDaSessao;
    int mNumeroDaSessao = 0;

    [[nodiscard]] int CasaDoAluno() const;

    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram.
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, int quantos = 0,
                                            std::optional<float> angulo = std::nullopt,
                                            float rapidez = 1.0f);

    bool mDisparandoEm = false;
    Vector2 mOndeDisparar;
    int mQuantos = 0;
    std::optional<float> mAnguloEm;
    std::vector<BossProjectile*> mCriados;

    std::optional<Vector2> mAlvo;

    /// O professor vai para um x sorteado entre 'de' e 'ate', na altura 'y'.
    void MudarDeLugar(float de, float ate, float y);


    /// O relogio da batalha inteira - nao volta a zero na troca de fase. E nele
    /// que o vetor do chao conta os avisos e os cachos.
    float mRelogio = 0.0f;

    /// As linhas do campo - o fluxograma, as chamadas, o ponteiro -, limpas a
    /// cada quadro e redesenhadas por quem estiver rodando.
    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 80;

    // ---- O vetor do chao (fases 2 e 3) ----

    void AtualizarVetor(bool preciso);
    void CriarVetor();
    void ApagarVetor();

    /// O i passou pela casa 'casa' em 'quando' (no relogio da batalha): ela
    /// acende, e kAviso depois sobe o cacho. 'dono' diz de quem ele e.
    void AgendarCacho(int casa, float quando, int dono);
    void SoltarCacho(int casa, int dono);

    std::vector<BossProjectile*> mIndices;      ///< um por casa do vetor
    std::vector<float> mAcesoDoIndice;          ///< ate quando cada um fica aceso
    std::vector<float> mPuladoDoIndice;
    float mErroAte = 0.0f;

    struct Cacho { int casa = 0; float quando = 0.0f; int dono = 0; bool aceso = false; };
    std::vector<Cacho> mCachos;                 ///< os que ainda vao subir
    struct Subindo { BossProjectile* p = nullptr; int dono = 0; };
    std::vector<Subindo> mSubindo;              ///< os do while (1), que o juiz mata
    int mDonoQueOJuizMata = -1;

    // ---- A fase 1: o desvio condicional ----

    void AtualizarCondicional(float deltaTime, const Sessao& s);
    void NovoComando(float agora);
    void SoltarBalao();
    void AtualizarBaloes(float deltaTime, float agora);
    void Estourar(const Condicional::Comando& c, int ramo, int cor, float agora);
    [[nodiscard]] Vector2 Decisao() const;
    [[nodiscard]] std::vector<Vector2> Caminho(const Condicional::Comando& c, int ramo) const;
    [[nodiscard]] Vector2 CabecalhoDoRamo(const Condicional::Comando& c, int ramo) const;
    void DesenharFluxograma(float agora);
    void DesmontarCondicional();

    int mSessaoDoSe = -1;
    std::string mFaseDoSe;
    int mVez = 0;
    std::vector<int> mOrdemDoSe;      ///< vazia = a ordem de sempre
    Condicional::Degrau mDegrauDoSe;
    int mFeitosDoSe = 0;
    int mLimiteDoSe = 0;
    float mProximoComando = 0.0f;
    Condicional::Comando mComando;
    int mBaloesDoComando = 0;
    float mProximoBalao = 0.0f;

    std::vector<BossProjectile*> mRotulos;     ///< um cabecalho por ramo
    std::vector<float> mAcesoAte;              ///< ate quando cada um fica aceso

    enum class Etapa { Descendo, Decidindo, Voando, Saindo };
    struct Balao {
        BossProjectile* p = nullptr;
        Condicional::Comando c;
        int n = 0;
        int cor = 0;
        Etapa etapa = Etapa::Descendo;
        float ate = 0.0f;
        std::vector<int> ramos;           ///< os que executam, em ordem
        std::vector<Vector2> caminho;     ///< o que falta andar ate o cabecalho
    };
    std::vector<Balao> mBaloes;

    struct Queda { Condicional::Comando c; int ramo = 0; int cor = 0; float quando = 0.0f; };
    std::vector<Queda> mQuedas;               ///< os cases de baixo, no switch

    float mDecisaoAte = 0.0f;
    int mDecisaoRamo = -1;

    // ---- A fase 2: a repeticao ----

    void AtualizarLaco(const Sessao& s);
    void NovoLaco(float agora);
    void Visitar(const Laco::Visita& v);

    int mSessaoDoLaco = -1;
    std::string mFaseDoLaco;
    int mVezDoLaco = 0;
    std::vector<int> mOrdemDoLaco;
    /// O do cacho vale em toda fase que usa o vetor - a 2, a 3 e a prova.
    Laco::Degrau mDegrauDoLaco;
    int mFeitosDoLaco = 0;
    int mLimiteDoLaco = 0;
    int mDonoDoLaco = 0;
    bool mRodando = false;
    float mProximoLaco = 0.0f;
    float mInicioDoLaco = 0.0f;
    float mDuracaoDoLaco = 0.0f;
    bool mTempoEsgotado = false;
    Laco::Programa mPrograma;
    std::vector<Laco::Visita> mVisitas;
    size_t mProximaVisita = 0;

    // ---- A fase 3: as funcoes ----

    void AtualizarFuncao(float deltaTime, const Sessao& s);
    void NovaChamada(float agora);
    void Executar(float agora);
    void TerminarChamada(float agora);

    /// Os valores voam ate 'para' - ou ate o professor, que anda -, e
    /// devolve se todos ja chegaram.
    void Levar(const std::vector<int>& valores, Vector2 de, std::optional<Vector2> para);
    bool MoverValores(float deltaTime);

    BossProjectile* PorCartao(Funcao::Nome f, Vector2 onde);
    void ApagarCartoes();
    void DesmontarFuncao();

    int mSessaoDaFuncao = -1;
    std::string mFaseDaFuncao;
    int mVezDaFuncao = 0;
    std::vector<int> mOrdemDaFuncao;
    Funcao::Degrau mDegrauDaFuncao;
    int mFeitasDaFuncao = 0;
    int mLimiteDaFuncao = 0;
    float mProximaChamada = 0.0f;
    Funcao::Comando mChamada;
    std::vector<int> mArgumentos;

    enum class Passo { Parado, IdaDeDentro, NaDeDentro, Volta, IdaDeFora, NaDeFora, Seguindo };
    Passo mPasso = Passo::Parado;
    float mAteDoPasso = 0.0f;
    float mSeguindoAte = 0.0f;
    float mProximoSegue = 0.0f;
    float mForaAcesoAte = 0.0f;

    BossProjectile* mCartaoDeFora = nullptr;
    BossProjectile* mCartaoDeDentro = nullptr;
    Vector2 mOndeDeFora;
    Vector2 mOndeDeDentro;

    struct Valor { BossProjectile* p = nullptr; Vector2 desvio; std::optional<Vector2> para; };
    std::vector<Valor> mValores;

    // ---- A repescagem: a prova no boca ----

    void AtualizarProva();
    int mProblema = -2;   ///< o problema da prova em curso; -2 fora dela

    /// A prova desta luta (Prova::Embaralhada), sorteada ao entrar nela.
    Prova::Lista mProva = Prova::kProva;
    bool mProvaSorteada = false;
};
