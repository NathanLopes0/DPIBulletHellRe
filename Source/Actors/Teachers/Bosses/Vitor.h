//
// O chefe da INF 310 - Programacao Concorrente e Distribuida.
//

#pragma once

#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Nuvem.h"

/**
 * @class Vitor
 * @brief O professor de Programacao Concorrente e Distribuida (INF 310).
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/vitor/fases.json e as regras de projetil no regras.json ao
 * lado. O que mora aqui e a UNICA coisa que dados nao sabem fazer - mudar a
 * direcao da fresta da secao critica entre uma rajada e a outra.
 *
 * A IDEIA DO CHEFE: concorrencia e "varias coisas ao mesmo tempo", e e por isso
 * que a fase 3 dele se divide em dois corpos. As duas primeiras fases existem
 * para serem aprendidas SEPARADAS, porque a terceira as poe juntas:
 *
 *   fase 1  TAREFAS EM PARALELO - varias faixas descendo de uma vez
 *   fase 2  EXCLUSAO MUTUA      - um anel com UMA fresta, sempre embaixo
 *   fase 3  FORK                - dois corpos, um fazendo cada uma das duas
 *
 * E ELE TAMBEM DA REDES: na fase 2 ha duas NUVENS em cima dele, a computacao
 * em nuvem, que processam e chovem pacotes (Nuvem.h). O relogio delas, a cara
 * delas e o placar moram aqui (AtualizarNuvens). E e pela nuvem que ele se
 * divide na fase 3: as duas se juntam em cima dele, ele entra, e saem dois
 * (ComecarOFork).
 */
class Vitor : public Boss {
public:
    explicit Vitor(Scene* scene);
    ~Vitor() override = default;

    void OnUpdate(float deltaTime) override;

    /// A fase das nuvens, e a do fork, que a nuvem abre (Nuvem.h).
    static constexpr const char* kFaseDasNuvens = Nuvem::kFaseDasNuvens;
    static constexpr const char* kFaseDoFork = Nuvem::kFaseDoFork;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

private:
    // ---- AS NUVENS DA FASE 2 -----------------------------------------------
    //
    // Pouco depois de a fase comecar o Vitor forma as duas nuvens (o gatilho
    // "nuvens"), e dali elas trabalham no ciclo de Nuvem.h: processam, e no
    // fim do processamento ele puxa o gatilho "chover". SO O CORPO ORIGINAL:
    // o clone da fase 3 roda os ataques da fase 2, mas a nuvem e do Vitor, e
    // nao de cada copia dele.

    void AtualizarNuvens(float deltaTime);

    /// As nuvens em campo: os projeteis no posto das nuvens.
    [[nodiscard]] std::vector<BossProjectile*> Nuvens() const;

    /// Puxa um gatilho das nuvens. O CustomizeAttackParams sabe que o disparo e
    /// delas, e nao do anel - e a fresta do anel nao anda por causa delas.
    void DispararNuvem(const char* gatilho);

    /// O fork pela nuvem, no corpo original: junta as nuvens em cima dele,
    /// manda os dois corpos para dentro dela, e a desfaz quando eles saem.
    void ComecarOFork();
    void AtualizarANuvemDoFork();
    void SairDaNuvem();

    /**
     * @brief Um corpo entra no fork pela nuvem (Nuvem::Fork): fica em 'onde',
     * some ou nasce dentro dela, e quando ela abre anda ate 'destino'. Ate la
     * nao anda sozinho nem ataca - mas o colisor fica: o tiro que entra na
     * nuvem acerta quem esta dentro dela. O original chama isto nos dois.
     */
    void EntrarNaNuvem(Vector2 onde, Vector2 destino, bool nasceuDentro);

    /// Ha quanto tempo o corpo esta no fork (negativo: nao esta), de onde ele
    /// sai e para onde vai.
    float mTempoNoFork = -1.0f;
    bool mNasceuDentro = false;
    Vector2 mSaiDe;
    Vector2 mVaiPara;
    float mEscalaDaNuvem = 1.0f;

    Nuvem::Ciclo mChuva;
    bool mNasNuvens = false;
    bool mNuvensFormadas = false;
    bool mDisparandoNuvem = false;
    float mTempoNasNuvens = 0.0f;
    float mSemNuvensHa = 0.0f;     ///< ha quanto tempo as nuvens formadas sumiram
    std::string mNomeDaFaseDasNuvens;

    /// Quanto depois do comeco da fase as nuvens se formam: antes do primeiro
    /// anel, que sai a 1 s.
    static constexpr float kFormaAsNuvens = 0.6f;

    static constexpr const char* kProcessando = "nuvem: processando...";
    static constexpr const char* kEnviando = "nuvem: enviando pacotes";
    static constexpr const char* kForaDoAr = "nuvem: fora do ar";

    /// O FORK: os tempos dele, e o tamanho da nuvem juntada - as duas viram
    /// uma maior que o professor: o quadro da nuvem tem 32 px e o dele 128, e
    /// a 5x ela fica mais larga que ele.
    static constexpr Nuvem::Fork kFork{};
    static constexpr float kEscalaDaNuvemJunta = 5.0f;
    static constexpr const char* kNoFork = "nuvem: fork()";
    std::string mNomeDaFaseDoFork;

    /**
     * Para onde a fresta do anel aponta, medido a partir da ponta da janela
     * (kFrestaMaisADireita). COMECA NO MEIO, entao o primeiro anel da fase abre
     * reto para baixo e so depois a fresta passa a escorregar para os lados.
     */
    float mDesvioDaFresta = 30.0f;

    /**
     * A JANELA E A METADE DE BAIXO, E NAO A VOLTA INTEIRA. A fresta vai de 60
     * a 120 graus (90 e reto para baixo): no maximo 30 graus inclinada, que na
     * faixa em que o jogador vive - de 330 a 550px abaixo do chefe, conforme a
     * orbita - desloca a porta ate uns 320px para o lado, e ela fica entre
     * x=171 e x=1029 numa tela de 1200. Uma fresta apontando para cima nao
     * seria porta nenhuma, so mais parede.
     */
    static constexpr float kFrestaMaisADireita = 60.0f;
    static constexpr float kJanelaDaFresta = 60.0f;

    /**
     * 37 GRAUS POR RAJADA, PRIMO COM A JANELA DE 60. A fresta zigue-zagueia
     * (90, 67, 104, 81, 118, 95...) e so repete depois de 60 rajadas, entao o
     * jogador nao decora o ciclo. E o mesmo motivo dos 137 graus que a fresta
     * girava antes, quando dava a volta inteira.
     */
    static constexpr float kPassoDaFresta = 37.0f;
};
