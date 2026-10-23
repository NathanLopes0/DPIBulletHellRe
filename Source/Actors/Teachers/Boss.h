//
// Created by nslop on 25/09/2024.
//

#pragma once

#include <functional>

#include "../Actor.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <utility>

#include "../ProjectileFactory.h"
#include "../../Attacks/IAttackStrategy.h"
#include "../../Attacks/AttackParameters/AttackParams.h"
#include "Bosses/BossesProjectiles/BossProjectile.h"

class IMovementStrategy;
class FSMComponent;
class ProjectileManager;
class Player;
class ProjectileFactory;
class Projectile;

using ProjectileConfigurator = std::function<void(Projectile* projectile, int index)>;


class Boss : public Actor {
public:

    explicit Boss(Scene* scene);
    ~Boss() override;


    // --- Ciclo de Vida ---
    void Start();
    void OnUpdate(float deltaTime) override;
    void OnCollision(Actor *other) override;

    struct AttackDefinition {
        std::unique_ptr<IAttackStrategy> strategy; // A lógica (Angled, Circle, etc)
        std::unique_ptr<AttackParams> params;                       // Parâmetros (Speed, Angle, NumProj)
        ProjectileConfigurator configurator;       // Lambda para customização (behaviors, animação)
        float cooldownTotal;                       // Tempo entre disparos desse ataque
        float currentTimer;                        // Timer atual desse ataque específico

        /// O TRECHO DA FASE em que este ataque dispara, em segundos desde o
        /// comeco dela. O padrao e a fase inteira. Fora do trecho o ataque nao
        /// conta o cooldown nem dispara; quando o trecho abre, o primeiro disparo
        /// sai em kPrimeiroDisparoDaJanela. Vem das "etapas" de fases.json.
        float janelaDe = 0.0f;
        float janelaAte = 1.0e9f;

        /// O GATILHO que o dispara, de fases.json. Vazio = o relogio: o ataque
        /// dispara a cada cooldown. Com gatilho, o relogio nao conta, e o
        /// ataque so sai quando o chefe chama Disparar(gatilho).
        std::string gatilho;
    };

    // QUANDO SAI O PRIMEIRO DISPARO de cada ataque (kPrimeiroDisparoDaFase e
    // kPrimeiroDisparoDaJanela) mora em FasesDeAtaque.h, e nao aqui: os dados
    // precisam conferir a mesma conta (PrimeiroDisparoNaJanela), e o teste
    // deles nao enxerga o Boss.

    void AddAttackPattern(const std::string& stateName,
                          std::unique_ptr<IAttackStrategy> strategy,
                          std::unique_ptr<AttackParams> params,
                          float cooldown,
                          ProjectileConfigurator config = nullptr,
                          float janelaDe = 0.0f,
                          float janelaAte = 1.0e9f,
                          std::string gatilho = "");

    void SetInitialState(const std::string& stateName);

    [[nodiscard]] ProjectileFactory* GetProjectileFactory(const std::string& factoryName) const;

    void AddProjectileFactory(const std::string& projectileName, std::unique_ptr<ProjectileFactory> factory);

    /**
     * @brief Retorna os nomes de TODAS as ProjectileFactory registradas por
     * este Boss (ex: {"Baloes", "Arduino", ...}). Usado por sistemas
     * externos como a tela de carregamento (Battle::UpdateLoadingStep) para
     * saber quantos/quais tipos existem sem hardcoding — cada Boss concreto
     * pode registrar tipos diferentes, e esta lista reflete exatamente
     * mProjectileFactories no momento da chamada.
     */
    [[nodiscard]] std::vector<std::string> GetProjectileFactoryNames() const;

    /**
     * @brief Pré-aquece TODAS as ProjectileFactory registradas por este Boss,
     * criando 'countPerType' instâncias de cada tipo e devolvendo-as aos
     * respectivos pools. Pensado para ser chamado numa tela de carregamento,
     * ANTES da batalha começar — depois disso, os primeiros disparos de cada
     * tipo já encontram objetos prontos no pool, sem custo de alocação nem
     * de carregar textura/JSON durante o gameplay real.
     * @param countPerType Quantas instâncias criar para CADA tipo de projétil
     * registrado (não é um total agregado — é por tipo).
     */
    void PrewarmProjectilePools(int countPerType);

    [[nodiscard]] Vector2 GetDirectionToPlayer() const;

    /**
     * @brief Direcao ate onde o jogador ESTARA daqui a leadTime segundos,
     * extrapolando pela velocidade atual dele.
     *
     * Com leadTime = 0 e identico a GetDirectionToPlayer(). Quanto maior, mais
     * o boss "chuta na frente" do jogador.
     *
     * O efeito de jogo e o que interessa: quem anda em linha reta (ou fica
     * parado) e acertado, e quem troca de direcao no momento do disparo faz o
     * tiro passar longe. Isso ensina o jogador a variar o movimento sem
     * precisar explicar nada a ele.
     *
     * @param leadTime segundos de antecipacao. O jogador anda a 300 px/s,
     *        entao 0.5f equivale a prever cerca de 150 px a frente: desvio bem
     *        visivel na tela, mas ainda desviavel.
     */
    [[nodiscard]] Vector2 GetPredictedPlayerDirection(float leadTime) const;

    /**
     * @brief Distancia em pixels ate o jogador. Devolve 0 se nao houver
     * jogador. Util para calcular tempo de voo de projetil.
     */
    [[nodiscard]] float GetDistanceToPlayer() const;

    void RegisterMovementStrategy(const std::string& stateName, std::unique_ptr<IMovementStrategy> strategy);

    // ---- A FASE QUE SE DIVIDE EM DOIS CORPOS -------------------------------

    /**
     * @brief Anota que, ao entrar nesta fase, o chefe se divide.
     *
     * So uma anotacao: quem cria o segundo corpo e a batalha, no momento em que
     * a fase comeca. Criar antes seria pagar por um corpo escondido durante a
     * luta inteira.
     *
     * @param fase        em qual fase a divisao acontece
     * @param ataquesDoClone de qual fase o segundo corpo tira os ataques
     */
    void RegistrarClone(const std::string& fase, const std::string& ataquesDoClone);

    /// @brief De qual fase o clone desta fase tira os ataques. Vazio = sem clone.
    [[nodiscard]] std::string CloneDaFase(const std::string& fase) const;

    // ---- O PROJETIL DERRUBADO ----------------------------------------------

    /**
     * @brief Um tiro do aluno derrubou um projetil derrubavel deste chefe.
     *
     * Quem chama e Battle, no laco de colisao, SEMPRE no corpo original - mesmo
     * que o projetil tenha saido de um clone. O padrao nao faz nada; o Lucas
     * usa para o supervisor reiniciar o processo ("let it crash").
     */
    virtual void OnProjetilDerrubado() {}

    /**
     * @brief Um tiro do aluno acertou um projetil derrubavel deste chefe e NAO o
     * derrubou: a resistencia dele (DescricaoDeProjetil::resistencia) comeu o
     * tiro. Mesma regra de quem e chamado do OnProjetilDerrubado. O padrao nao
     * faz nada; o Julio usa para o guardrail dizer que nao pode ajudar.
     */
    virtual void OnProjetilBarrado() {}

    // ---- O PLACAR ---------------------------------------------------------

    /// @brief Anota o que o placar mostra enquanto esta fase dura ("anuncio" em
    /// fases.json). Como o clone, e so uma anotacao: quem desenha e a batalha.
    void RegistrarAnuncio(const std::string& fase, const std::string& texto);

    /// @brief O anuncio desta fase. Vazio = a fase nao anuncia nada.
    ///
    /// NUMA FASE COM ETAPAS, e a fase em que o chefe esta agindo, o anuncio e o
    /// da ETAPA corrente, contada pelo relogio da maquina de estados. Depois da
    /// ultima etapa volta o anuncio da fase.
    [[nodiscard]] std::string AnuncioDaFase(const std::string& fase) const;

    /// @brief O anuncio que a fase tem no instante 't' dela, contando as
    /// etapas - o de agora ou o de daqui a pouco. Com 't' negativo, o da fase.
    ///
    /// Para quem decide AGORA uma coisa que acontece DEPOIS: o Ricardo sorteia
    /// a palavra para a porta que ela vai atravessar, e a porta esta escrita no
    /// placar da etapa em que a travessia cai.
    [[nodiscard]] std::string AnuncioDaFaseEm(const std::string& fase, float t) const;

    /// @brief Anota as etapas da fase: onde cada uma ACABA, em segundos desde o
    /// comeco da fase, e o que o placar diz durante ela. Ver AnuncioDaFase.
    void RegistrarEtapas(const std::string& fase, std::vector<std::pair<float, std::string>> fimETexto);

    // ---- O INTERCAMBIO ----------------------------------------------------

    /// @brief Anota que nesta fase os alunos trocam de lugar a cada 'aCada'
    /// segundos, com aviso 'aviso' segundos antes. So uma anotacao, como o
    /// anuncio: quem move os alunos e a batalha (Battle::AtualizarIntercambio).
    void RegistrarIntercambio(const std::string& fase, float aCada, float aviso);

    /// @brief O periodo e o aviso do intercambio desta fase, se ela tiver um.
    [[nodiscard]] std::optional<std::pair<float, float>>
    IntercambioDaFase(const std::string& fase) const;

    /// O que o placar diz durante o aviso do intercambio, quando o chefe nao
    /// diz outra coisa.
    static inline const std::string kAvisoDoIntercambio = "Programa de Interc\xC3\xA2mbio";

    /// @brief O que o placar diz durante o aviso do intercambio. O padrao e o
    /// nome do programa; o Vladimir avisa tambem o que a troca faz com as letras.
    [[nodiscard]] virtual std::string AvisoDoIntercambio() const { return kAvisoDoIntercambio; }

    /**
     * @brief Os alunos acabaram de trocar de lugar.
     *
     * Quem chama e Battle (AtualizarIntercambio), no quadro da troca, e so no
     * corpo original - como OnProjetilDerrubado. O padrao nao faz nada; o
     * Vladimir troca tambem as letras em voo (Alfabeto.h).
     */
    virtual void OnIntercambio() {}

    /**
     * @brief A batalha acabou: a tela da nota vai entrar.
     *
     * Quem chama e Battle::FinishBattle, depois de limpar os projeteis. O que
     * nao e projetil - um painel de codigo, as linhas no campo - nao sai com
     * eles, e o chefe que pos isso em campo esconde aqui. O padrao nao faz
     * nada.
     */
    virtual void AoTerminarABatalha() {}

    /**
     * @brief Para onde o professor quer ir, quando e ele quem diz - o movimento
     * "Conduzido" de fases.json o leva ate la. Sem alvo, o Conduzido o deixa
     * parado. O padrao nao tem alvo; o Vladimir voa de estado em estado do
     * automato dele.
     */
    [[nodiscard]] virtual std::optional<Vector2> AlvoDoMovimento() const { return std::nullopt; }

    /// @brief A fase em que o chefe esta AGINDO - a mesma que escolhe o
    /// movimento e os ataques do quadro. Vazio antes do primeiro OnUpdate.
    [[nodiscard]] const std::string& FaseAtual() const { return mLastStateName; }

    /**
     * @brief Este corpo e o SEGUNDO, e nao o original?
     *
     * O clone roda a mesma maquina de estados do original - e a mesma fabrica
     * que o monta -, mas tem de ficar PRESO na fase que lhe coube: sem isto ele
     * avancaria sozinho e, ao chegar na fase que se divide, criaria um clone do
     * clone. Quem respeita esta marca e BossAttackState, que nao transiciona um
     * corpo marcado.
     */
    [[nodiscard]] bool EhClone() const { return mEhClone; }
    void MarcarComoClone() { mEhClone = true; }


protected:
    /**
     * @brief Gancho para o que depende do DISPARO ANTERIOR - o unico pedaco do
     * balanceamento que nao cabe em fases.json, porque um arquivo nao tem
     * memoria. Dai vem a fresta giratoria do Vitor e o escopo que cresce da
     * Maria Lucia.
     *
     * ARMADILHA, e ela custou uma medicao: O 'params' QUE CHEGA AQUI E SEMPRE O
     * MESMO OBJETO. Ele e construido UMA vez, na montagem da fase, com os
     * valores de fases.json, e Boss::ExecuteAttack entrega esse mesmo objeto em
     * todo disparo. Entao:
     *
     *   params.centralAngle = x;    OK  - escreve o valor inteiro
     *   params.numProjectiles += 2; NAO - soma sobre o disparo anterior, e nao
     *                                     sobre o que o arquivo pediu
     *
     * Quem precisa crescer a partir do arquivo tem de GUARDAR o valor de
     * partida no primeiro disparo e escrever o total depois. Ver
     * MariaLucia::mProjeteisDePartida. Com a versao relativa, a fase 1 dela
     * terminava com 54 projeteis num leque de 285 graus em vez de voltar a 6.
     *
     * E so o NOME DA FASE chega aqui, e nao qual dos ataques dela esta
     * disparando. Numa fase com dois ataques, o que for escrito vale para os
     * dois.
     */
    virtual void CustomizeAttackParams(AttackParams& params, const std::string& stateName);

    /**
     * @brief Gancho chamado logo DEPOIS de a strategy criar os projeteis e
     * ANTES de eles serem entregues ao ProjectileManager.
     *
     * Este e o ponto de extensao para customizacoes que valem para a leva
     * inteira e que a strategy nao tem como saber - o Andre, por exemplo,
     * sorteia UMA cor por rajada.
     *
     * Existe porque a alternativa era sobrescrever ExecuteAttack por completo,
     * que foi o que o Andre fazia: 59 linhas copiadas da base para inserir 8.
     * O custo disso nao e estetico - a copia ja tinha ficado para tras e
     * perdido a guarda de params/strategy nulos que a base ganhou depois.
     *
     * @param projectiles Os projeteis recem-criados, ainda como Projectile.
     * @param stateName Nome do estado da FSM que disparou o ataque.
     */
    virtual void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                      const std::string& stateName);

    /**
     * @brief De onde uma PontosAttack atira: os pontos do posto nomeado.
     *
     * O padrao sao os projeteis do chefe em campo que estao naquele posto
     * (BossProjectile::DefinirPosto) - as historias paradas no quadro da
     * sprint, por exemplo. Um chefe cujo posto nao e feito de projeteis
     * sobrescreve. Vazio = a PontosAttack nao dispara nada.
     */
    [[nodiscard]] virtual std::vector<Vector2> PontosDoPosto(const std::string& posto) const;

    /**
     * @brief PUXA O GATILHO: dispara agora todo ataque da fase corrente que tem
     * este gatilho (e cuja etapa esta aberta), sem esperar relogio nenhum.
     *
     * Quem puxa e o chefe concreto, quando acontece o que o gatilho nomeia - na
     * Maria Lucia, o aluno mudar de estado (Observer.h). Um gatilho que
     * nenhum ataque da fase tem nao faz nada.
     *
     * 'alvo', se nao for negativo, e a posicao do jogador que o disparo mira
     * (Battle::DarAVez): o aluno que mudou de estado e quem recebe os avisos.
     */
    void Disparar(const std::string& gatilho, int alvo = -1);

private:
    // Função interna para executar um ataque específico.
    // NAO e virtual de proposito: a sequencia de passos (customizar params ->
    // executar strategy -> configurator -> converter -> entregar ao manager ->
    // audio) e a mesma para todo boss. Quem precisa variar usa os ganchos
    // CustomizeAttackParams, OnProjectilesCreated e PontosDoPosto.
    void ExecuteAttack(const AttackDefinition& attackDef, const std::string& stateName);

    void CalculateNextDropThreshold();
    int mHitCounter = 0;
    int mNextDropThreshold = 0;

    std::string mInitialState;

    /// Por fase, de qual fase o clone dela tira os ataques. Ver RegistrarClone.
    std::map<std::string, std::string> mClonePorFase;

    /// Por fase, o texto do placar. Ver RegistrarAnuncio.
    std::map<std::string, std::string> mAnuncioPorFase;

    /// Por fase, as etapas: onde cada uma acaba e o texto dela. Ver RegistrarEtapas.
    std::map<std::string, std::vector<std::pair<float, std::string>>> mEtapasPorFase;

    /// Por fase, o periodo e o aviso do intercambio. Ver RegistrarIntercambio.
    std::map<std::string, std::pair<float, float>> mIntercambioPorFase;

    bool mEhClone = false;

    // Mapeia: Nome do projétil -> Fábrica dele (Populado na Fábrica)
    std::unordered_map<std::string, std::unique_ptr<ProjectileFactory>> mProjectileFactories;

    // Mapeia: Nome do Estado -> Lista de Definições de Ataque
    std::map<std::string, std::vector<AttackDefinition>> mAttacksMap;

    std::map<std::string, std::unique_ptr<IMovementStrategy>> mMovementStrategies;
    IMovementStrategy* mCurrentMovementStrategy = nullptr;
    std::string mLastStateName {}; // Para detectar mudança de estado

};