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

class BossProjectile;
class DrawLinhasComponent;

/**
 * @class Andre
 * @brief O chefe da primeira materia do curso: o placar e o codigo que ele esta
 *        rodando, e o campo e a execucao dele.
 *
 * Os BALOES DO BOCA ficam: no BOCA, o sistema em que a turma entrega os
 * exercicios, cada problema resolvido vale um balao da cor dele.
 *
 * A FASE 1 E O DESVIO CONDICIONAL (Condicional.h): o fluxograma desenhado no
 * campo - o losango embaixo do professor e um ramo por faixa, com o
 * cabecalho no alto -, e os baloes numerados que seguem pelo ramo que a
 * condicao escolhe e estouram em confete na faixa dele.
 *
 * A FASE 2 E A REPETICAO (Laco.h): o chao e um vetor de doze casas, cada laco
 * anda por ele com o i - a casa da vez acende -, e de cada casa visitada
 * sobe um cacho de baloes. O <= que passa do fim capota o programa, e o
 * while (1) e morto pelo juiz do BOCA.
 *
 * A FASE 3 SAO AS FUNCOES (Funcao.h): o professor e o main, cada funcao e um
 * cartao no meio do campo, os argumentos voam ate ela e o retorno volta; a
 * passagem por valor copia a casa do aluno, a por referencia o segue.
 *
 * A REPESCAGEM E A PROVA NO BOCA (Prova.h): quatro problemas, cada um rodado
 * pelo mesmo codigo da fase de onde vem, e o balao de cada um sobe no YES.
 *
 * UMA MECANICA POR VEZ. Cada fase - e cada problema da prova - abre uma
 * SESSAO de uma das tres mecanicas, com o relogio dela; a sessao que fecha
 * desmonta o que pos em campo.
 */
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
    // ---- O QUE AS TRES FASES DIVIDEM ----------------------------------------

    enum class Mecanica { Nenhuma, Condicional, Laco, Funcao };

    /// Qual mecanica roda agora, em que fase (para o placar), ha quanto tempo,
    /// e de onde ela comeca. 'numero' muda a cada sessao nova.
    struct Sessao {
        Mecanica qual = Mecanica::Nenhuma;
        std::string fase;
        float agora = 0.0f;
        int numero = 0;
        int primeiraVez = 0;
        int quantos = 0;      ///< quantos comandos ela roda; 0 = ate a fase acabar
        bool vetor = false;   ///< se o vetor do chao fica em campo
    };
    Sessao SessaoAtual();
    std::string mChaveDaSessao;
    int mNumeroDaSessao = 0;

    [[nodiscard]] SDL_FRect Campo() const;
    [[nodiscard]] int CasaDoAluno() const;
    [[nodiscard]] Vector2 PosicaoDoAluno() const;

    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram.
    /// 'quantos' > 0 troca o numero de projeteis do ataque, e 'angulo' o meio
    /// do leque dele.
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, int quantos = 0,
                                            std::optional<float> angulo = std::nullopt);

    bool mDisparandoEm = false;
    Vector2 mOndeDisparar;
    int mQuantos = 0;
    std::optional<float> mAnguloEm;
    std::vector<BossProjectile*> mCriados;

    std::optional<Vector2> mAlvo;

    /// O professor vai para um x sorteado entre 'de' e 'ate', na altura 'y'.
    void MudarDeLugar(float de, float ate, float y);

    /// O placar: o que a fase diz no arquivo e guardado na primeira vez, e
    /// volta quando a sessao fecha.
    void Anunciar(const std::string& fase, const std::string& texto);
    void Restaurar(const std::string& fase);
    std::map<std::string, std::string> mAnunciosDoArquivo;

    /// O relogio da batalha inteira - nao volta a zero na troca de fase. E nele
    /// que o vetor do chao conta os avisos e os cachos.
    float mRelogio = 0.0f;

    /// As linhas do campo - o fluxograma, as chamadas, o ponteiro -, limpas a
    /// cada quadro e redesenhadas por quem estiver rodando.
    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 80;

    // ---- O VETOR DO CHAO (fases 2 e 3) --------------------------------------

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

    // ---- A FASE 1: O DESVIO CONDICIONAL -------------------------------------

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

    // ---- A FASE 2: A REPETICAO ----------------------------------------------

    void AtualizarLaco(const Sessao& s);
    void NovoLaco(float agora);
    void Visitar(const Laco::Visita& v);

    int mSessaoDoLaco = -1;
    std::string mFaseDoLaco;
    int mVezDoLaco = 0;
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

    // ---- A FASE 3: AS FUNCOES -----------------------------------------------

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

    // ---- A REPESCAGEM: A PROVA NO BOCA --------------------------------------

    void AtualizarProva();
    int mProblema = -2;   ///< o problema da prova em curso; -2 fora dela
};
