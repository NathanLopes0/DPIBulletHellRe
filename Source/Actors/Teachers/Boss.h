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
#include "../../Regras/Dificuldade.h"
#include "../../Regras/Variacao.h"

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

    // ---- Ciclo de Vida ----
    void Start();
    void OnUpdate(float deltaTime) override;
    void OnCollision(Actor *other) override;

    struct AttackDefinition {
        std::unique_ptr<IAttackStrategy> strategy; // A lógica (Angled, Circle, etc)
        std::unique_ptr<AttackParams> params;                       // Parâmetros (Speed, Angle, NumProj)
        ProjectileConfigurator configurator;       // Lambda para customização (behaviors, animação)
        float cooldownTotal;                       // Tempo entre disparos desse ataque
        float currentTimer;                        // Timer atual desse ataque específico

        /// O trecho da fase em que este ataque dispara, em segundos desde o comeco dela. O padrao e
        /// a fase inteira.
        float janelaDe = 0.0f;
        float janelaAte = 1.0e9f;

        /// O gatilho que o dispara, de fases.json. Vazio = o relogio: o ataque dispara a cada
        /// cooldown.
        std::string gatilho;
    };

    // Quando sai o primeiro disparo de cada ataque (kPrimeiroDisparoDaFase e
    // kPrimeiroDisparoDaJanela) mora em FasesDeAtaque.h, e nao aqui.

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

    /// Retorna os nomes de todas as ProjectileFactory registradas por este Boss (ex: {"Baloes",
    /// "Arduino", ...}).
    [[nodiscard]] std::vector<std::string> GetProjectileFactoryNames() const;

    [[nodiscard]] Vector2 GetDirectionToPlayer() const;

    /// Direcao ate onde o jogador estara daqui a leadTime segundos, extrapolando pela velocidade
    /// atual dele.
    [[nodiscard]] Vector2 GetPredictedPlayerDirection(float leadTime) const;

    /// Distancia em pixels ate o jogador. Devolve 0 se nao houver jogador.
    [[nodiscard]] float GetDistanceToPlayer() const;

    void RegisterMovementStrategy(const std::string& stateName, std::unique_ptr<IMovementStrategy> strategy);

    // ---- A fase que se divide em dois corpos ----

    /// Anota que, ao entrar nesta fase, o chefe se divide. So uma anotacao: quem cria o segundo
    /// corpo e a batalha, no momento em que a fase comeca.
    void RegistrarClone(const std::string& fase, const std::string& ataquesDoClone);

    /// De qual fase o clone desta fase tira os ataques. Vazio = sem clone.
    [[nodiscard]] std::string CloneDaFase(const std::string& fase) const;

    // ---- O projetil derrubado ----

    /// Um tiro do aluno derrubou um projetil derrubavel deste chefe.
    virtual void OnProjetilDerrubado() {}

    /// Um tiro do aluno acertou um projetil derrubavel deste chefe e nao o derrubou: a resistencia
    /// dele (DescricaoDeProjetil::resistencia) comeu o tiro.
    virtual void OnProjetilBarrado() {}

    /// O mesmo aviso de OnProjetilDerrubado, dizendo qual projetil caiu. Chamado logo depois de
    /// OnProjetilDerrubado; o padrao nao faz nada.
    virtual void OnDerrubou(BossProjectile* /*qual*/) {}

    // ---- O escudo ----

    /// O padrao e sim.
    [[nodiscard]] virtual bool AceitaDano() const { return true; }

    /// Alguem de fora quebrou o escudo - a bala de prata da Maria Lucia, como orientadora. Devolve
    /// se havia escudo para quebrar.
    virtual bool QuebrarEscudo() { return false; }

    /// O hlt (o poder do Ricardo, como orientador): o chefe para por 'segundos' - nem anda, nem a
    /// fase corre, nem ataca.
    void Congelar(float segundos);
    [[nodiscard]] bool Congelado() const { return mCongelado > 0.0f; }

    /// O padrao nao tem.
    [[nodiscard]] virtual std::vector<Vector2> AlvosDestrutiveis() const { return {}; }

    // ---- O placar ----

    /// Anota o que o placar mostra enquanto esta fase dura ("anuncio" em fases.json). Como o clone,
    /// e so uma anotacao: quem desenha e a batalha.
    void RegistrarAnuncio(const std::string& fase, const std::string& texto);

    /// O anuncio desta fase. Vazio = a fase nao anuncia nada.
    [[nodiscard]] std::string AnuncioDaFase(const std::string& fase) const;

    /// Com 't' negativo, o da fase.
    [[nodiscard]] std::string AnuncioDaFaseEm(const std::string& fase, float t) const;

    /// Anota as etapas da fase: onde cada uma acaba, em segundos desde o comeco da fase, e o que o
    /// placar diz durante ela.
    void RegistrarEtapas(const std::string& fase, std::vector<std::pair<float, std::string>> fimETexto);

    /// As etapas registradas da fase (onde cada uma acaba e o texto dela), ou vazia.
    [[nodiscard]] std::vector<std::pair<float, std::string>> EtapasDa(const std::string& fase) const;

    // ---- O intercambio ----

    /// Anota que nesta fase os alunos trocam de lugar a cada 'aCada' segundos, com aviso 'aviso'
    /// segundos antes.
    void RegistrarIntercambio(const std::string& fase, float aCada, float aviso);

    /// O periodo e o aviso do intercambio desta fase, se ela tiver um.
    [[nodiscard]] std::optional<std::pair<float, float>>
    IntercambioDaFase(const std::string& fase) const;

    /// O que o placar diz durante o aviso do intercambio, quando o chefe nao
    /// diz outra coisa.
    static inline const std::string kAvisoDoIntercambio = "Programa de Interc\xC3\xA2mbio";

    /// O que o placar diz durante o aviso do intercambio. O padrao e o nome do programa; o Vladimir
    /// avisa tambem o que a troca faz com as letras.
    [[nodiscard]] virtual std::string AvisoDoIntercambio() const { return kAvisoDoIntercambio; }

    /// Os alunos acabaram de trocar de lugar.
    virtual void OnIntercambio() {}

    /// A batalha acabou: a tela da nota vai entrar. Quem chama e Battle::FinishBattle, depois de
    /// limpar os projeteis.
    virtual void AoTerminarABatalha() {}

    /// Para onde o professor quer ir, quando e ele quem diz - o movimento "Conduzido" de fases.json
    /// o leva ate la. Sem alvo, o Conduzido o deixa parado.
    [[nodiscard]] virtual std::optional<Vector2> AlvoDoMovimento() const { return std::nullopt; }

    /// A fase em que o chefe esta agindo - a mesma que escolhe o movimento e os ataques do quadro.
    /// Vazio antes do primeiro OnUpdate.
    [[nodiscard]] const std::string& FaseAtual() const { return mLastStateName; }

    /// O modo da batalha em que o chefe luta (Battle::NivelDeDificuldade). Normal fora de uma
    /// Battle - nos testes, por exemplo.
    [[nodiscard]] Dificuldade::Nivel NivelDeDificuldade() const;

    /// O valor do modo da batalha: `normal` no Normal, `dificil` no Dificil.
    template <typename T>
    [[nodiscard]] T NoNivel(const T normal, const T dificil) const {
        return Dificuldade::Escolher(NivelDeDificuldade(), normal, dificil);
    }

    /// A sequencia da fase `fase` nesta batalha (Variacao::SorteioDa).
    [[nodiscard]] Variacao::Sorteio SorteioDaFase(const std::string& fase) const;

    /// Este corpo e o segundo, e nao o original?
    [[nodiscard]] bool EhClone() const { return mEhClone; }
    void MarcarComoClone() { mEhClone = true; }

protected:
    /// Gancho para o que depende do disparo anterior - o unico pedaco do balanceamento que nao cabe
    /// em fases.json, porque um arquivo nao tem memoria.
    virtual void CustomizeAttackParams(AttackParams& params, const std::string& stateName);

    /// Gancho chamado logo depois de a strategy criar os projeteis e antes de eles serem entregues
    /// ao ProjectileManager.
    virtual void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                      const std::string& stateName);

    /// De onde uma PontosAttack atira: os pontos do posto nomeado.
    [[nodiscard]] virtual std::vector<Vector2> PontosDoPosto(const std::string& posto) const;

    /// Quem puxa e o chefe concreto, quando acontece o que o gatilho nomeia - na Maria Lucia, o
    /// aluno mudar de estado (Observer.h).
    void Disparar(const std::string& gatilho, int alvo = -1, float rapidez = 1.0f);

private:
    // Função interna para executar um ataque específico.
    void ExecuteAttack(const AttackDefinition& attackDef, const std::string& stateName, float rapidez = 1.0f);

    void CalculateNextDropThreshold();
    int mHitCounter = 0;
    int mNextDropThreshold = 0;

    std::string mInitialState;

    /// Por fase, de qual fase o clone dela tira os ataques.
    std::map<std::string, std::string> mClonePorFase;

    /// Por fase, o texto do placar.
    std::map<std::string, std::string> mAnuncioPorFase;

    /// Por fase, as etapas: onde cada uma acaba e o texto dela.
    std::map<std::string, std::vector<std::pair<float, std::string>>> mEtapasPorFase;

    /// Por fase, o periodo e o aviso do intercambio.
    std::map<std::string, std::pair<float, float>> mIntercambioPorFase;

    bool mEhClone = false;
    float mCongelado = 0.0f;   ///< Os segundos de hlt que faltam.

    // Mapeia: Nome do projétil -> Fábrica dele (Populado na Fábrica)
    std::unordered_map<std::string, std::unique_ptr<ProjectileFactory>> mProjectileFactories;

    // Mapeia: Nome do Estado -> Lista de Definições de Ataque.
    std::map<std::string, std::vector<AttackDefinition>> mAttacksMap;

    std::map<std::string, std::unique_ptr<IMovementStrategy>> mMovementStrategies;
    IMovementStrategy* mCurrentMovementStrategy = nullptr;
    std::string mLastStateName {}; // Para detectar mudança de estado

};