//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Automato.h"
#include "../../../Attacks/Fita.h"
#include "../../../Attacks/Compilador.h"
#include "../../../Attacks/Parada.h"

class DrawLinhasComponent;

/// O professor de Linguagens Formais e Automatos (INF 331).
class Vladimir : public Boss {
public:
    explicit Vladimir(Scene* scene);
    ~Vladimir() override = default;

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] std::string AvisoDoIntercambio() const override;
    void OnIntercambio() override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;

    /// O Tracking que o "a" ganha ao virar "b" na troca: a forca e a duracao do "b" da fase 1, que
    /// e o que o aluno aprendeu a esperar de um "b".
    static constexpr float kAtrasoDaTroca = 0.15f;
    static constexpr float kForcaDaTroca = 1.4f;
    static constexpr float kDuracaoDaTroca = 2.2f;

    /// As caras dos estados, apagados e o aceso (projeteis.json, "Estado").
    [[nodiscard]] static std::string CaraDoEstado(int q, bool aceso);

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A luta sorteada ----

    void SortearALuta();

    bool mLutaSorteada = false;
    Compilador::Plano mPlanoDoCompilador;
    Compilador::Degrau mDegrauDoCompilador;
    Fita::Plano mPlanoDaFita;
    Fita::Degrau mDegrauDaFita;

    Parada::Plano mPlanoDaParada;
    Parada::Degrau mDegrauDaParada;
    int mProgramaDaVez = 0;

    // ---- O automato da fase 1 ----

    void AtualizarAutomato();

    /// Os projeteis do chefe que estao no posto nomeado.
    [[nodiscard]] std::vector<BossProjectile*> NoPosto(const char* posto) const;

    [[nodiscard]] static Vector2 CentroDoEstado(int q);

    /// Dispara 'gatilho' de 'onde', com a cara 'cara' no que nascer.
    void Formar(const char* gatilho, Vector2 onde, std::string cara);

    enum class Momento { Anuncio, Lendo, Veredito };

    Automato::Leitura mLeitura;
    Momento mMomento = Momento::Anuncio;
    bool mNoAutomato = false;
    bool mAutomatoFormado = false;
    float mProximoPasso = 0.0f;
    std::string mAnuncioDoAutomato;   ///< o da fase em fases.json, devolvido no fim

    std::optional<Vector2> mAlvo;

    /// Formando o automato: de onde sai o que nasce, e com que cara.
    bool mFormando = false;
    Vector2 mDisparoDoFormando;
    std::string mCaraDoFormando;

    /// Quanto depois do comeco da fase o automato se desenha, e quando a
    /// primeira palavra comeca: o placar mostra a linguagem ate la.
    static constexpr float kFormaOAutomato = 0.2f;
    static constexpr float kPrimeiraPalavra = 1.6f;

    /// Quanto dura cada simbolo lido, e o veredito no fim da palavra.
    static constexpr float kPasso = 1.1f;
    static constexpr float kVeredito = 1.3f;

    // ---- O compilador da fase 2 ----

    void AtualizarCompilador(float deltaTime);
    void ComecarRodada(int rodada, float inicio);
    void DesenharArestas();

    /// Dispara 'gatilho' de 'onde', no rumo 'rumo' - ou mirado no aluno, sem rumo -, com a cara
    /// 'cara' no que nascer. Devolve o que nasceu.
    std::vector<BossProjectile*> DispararDe(const char* gatilho, Vector2 onde, std::optional<float> rumo,
                                            const std::string& cara);

    [[nodiscard]] Vector2 PosicaoDoAluno() const;

    bool mNoCompilador = false;
    std::string mAnuncioDoCompilador;
    int mRodada = -1;                    ///< -1 antes da primeira
    float mInicioDaRodada = 0.0f;        ///< no relogio da fase
    std::string mLinha;
    std::vector<Compilador::Token> mTokens;
    Compilador::Arvore mArvore;
    std::vector<int> mOrdem;             ///< a pos-ordem: a montagem e a geracao
    std::vector<Compilador::Ponto> mNaLinha;
    std::vector<Compilador::Ponto> mNaArvore;
    Compilador::Marcos mMarcos;
    Compilador::Etapa mEtapa = Compilador::Etapa::Pronto;
    int mLidos = 0;
    int mMontados = 0;
    int mEmitidos = 0;
    std::vector<BossProjectile*> mNos;   ///< o projetil de cada no em campo, por indice
    std::vector<BossProjectile*> mLexemas;   ///< a ficha de cada token caindo, por indice
    bool mFilaParada = false;            ///< os tokens ja pararam para o parser

    /// As arestas da arvore, e a cabeca da fita: um desenho so, que cada fase
    /// limpa ao sair.
    DrawLinhasComponent* mLinhas = nullptr;
    void CriarLinhas();

    /// Disparando de um lugar (DispararDe): de onde e para onde sai o que
    /// nasce, com que cara, e quem nasceu.
    bool mDisparandoDe = false;
    Vector2 mOndeDisparar;
    float mRumoDoDisparo = 90.0f;
    std::string mCaraDoDisparo;
    std::vector<BossProjectile*> mDisparados;

    /// Quao depressa um no chega ao lugar dele na arvore (Ancora::Aproximar).
    static constexpr float kRapidezDoNo = 9.0f;

    // ---- A fita da fase 3 ----

    void AtualizarFita();
    void DesenharCabeca();
    bool mNaFita = false;
    Fita::Estado mFita;
    float mProximoPassoDaFita = 0.0f;
    std::array<BossProjectile*, Fita::kCasas> mMarcas{};   ///< a marca de cada casa, se houver
    std::string mAnuncioDaFita;

    // ---- O problema da parada da repescagem ----

    void AtualizarParada(float deltaTime);
    bool mNaParada = false;
    std::uint32_t mX = 0;     ///< o x do programa; 0 = nenhum rodando
    float mProximoPassoDaParada = 0.0f;
    float mXDaParada = 0.0f;
    float mSentidoDaParada = 1.0f;
    std::string mAnuncioDaParada;

    /// De lado a lado, devagar: o tiro do aluno leva quase 1s ate ele.
    static constexpr float kVarreduraDaParada = 110.0f;
    static constexpr float kMargemDaParada = 170.0f;
};
