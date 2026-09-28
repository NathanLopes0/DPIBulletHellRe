//
// Created by nslop on 18/11/2024.
//

#ifndef DPIBULLETHELLRE_PROJECTILE_H
#define DPIBULLETHELLRE_PROJECTILE_H

#include "Actor.h"
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include "../Math.h"

// Incluido, e nao apenas declarado: os static_assert de insertMotion e
// insertModifier usam std::is_base_of, que exige TIPO COMPLETO. Nao ha ciclo
// porque Behaviors.h so faz forward declaration de Projectile.
#include "../Attacks/Behaviors.h"

class ProjectileFactory;

/**
 * @class Projectile
 * @brief Classe base abstrata para todos os projéteis do jogo.
 * Define o contrato, o gerenciamento de behaviors e a lógica de colisão.
 */
class Projectile : public Actor {

public:

    explicit Projectile(Scene* scene, Actor* owner);
    ~Projectile() override;

    // 4. Métodos de ciclo de vida que as classes filhas podem implementar.
    void OnUpdate(float deltaTime) override;
    void OnCollision(Actor* other) override;

    // 5. Getter para o dono (essencial para o sistema de colisão e behaviors).
    [[nodiscard]] class Actor* GetOwner() const { return mOwner; }
    // Reatribui o dono. Necessário para Object Pooling: um projétil reciclado
    // pode ser reutilizado por um ataque disparado por outro owner (ex: mesmo Boss,
    // mas o parâmetro owner pode variar entre chamadas de Acquire()).
    void SetOwner(Actor* owner) { mOwner = owner; }

    // 6. Getters/Setters para propriedades de gameplay.
    void SetForwardSpeed(float newSpeed);
    [[nodiscard]] float GetForwardSpeed() const;

    // --- Object Pooling ---
    // A ProjectileFactory que originalmente construiu este objeto (via createProjectile).
    // Setado uma única vez, no Acquire(), e usado pelo ProjectileManager para saber
    // para qual Factory devolver o objeto quando ele for marcado como Inactive.
    // Fica nullptr para projéteis que não passam por pooling (ex: PlayerProjectile).
    [[nodiscard]] ProjectileFactory* GetOriginFactory() const { return mOriginFactory; }
    void SetOriginFactory(ProjectileFactory* factory) { mOriginFactory = factory; }

    // Ponto de entrada PUBLICO para matar um projetil de fora da classe
    // (ProjectileManager, Battle). Delega para MarkDead(), entao quem esta de
    // fora nao precisa saber se o projetil vai para o pool ou e destruido - e
    // nao consegue mais pular essa decisao mexendo direto no ActorState.
    void Kill() override;

    // Coloca o objeto no "estado de repouso" exigido de tudo que esta guardado
    // no pool: invisivel, sem colisor, sem velocidade, sem behaviors, Inactive.
    // Chamado pelo ProjectilePool dentro do Release(), ou seja, vale para TODOS
    // os caminhos de entrada no pool - morte normal, Prewarm e limpeza de tela -
    // e nao so para o caminho que passa por MarkDead().
    virtual void OnEnterPool();

    // Restaura o projétil para um estado "recém-nascido" antes de ser reutilizado
    // pelo pool. Limpa o que é comum a todo Projectile (behaviors, velocidade,
    // estado). Classes filhas devem chamar Projectile::Reset() e então
    // complementar com o que for específico delas (ex: reiniciar animação).
    virtual void Reset();

    /**
     * @brief Registra a aparencia com que a fabrica construiu este projetil,
     * para que Reset() possa devolve-la.
     *
     * Chamado por ProjectilePool::Acquire imediatamente depois da construcao -
     * o unico instante em que o projetil esta exatamente como a fabrica o quis,
     * porque nenhum configurator de ataque rodou ainda.
     *
     * O padrao NAO e "escala 1, sem animacao": cada fabrica de projetil tem o
     * seu (a do Julio constroi com escala 2 e animacao "Coleta", a do Ricardo
     * com "Normal"). Por isso o padrao e capturado em vez de ser uma constante.
     */
    void MarcarPadraoDeFabrica();

    /**
     * @brief Define a Motion do projetil: quem ESCREVE a velocidade.
     *
     * Exclusivo por construcao - chamar duas vezes SUBSTITUI a anterior, em vez
     * de empilhar dois escritores brigando frame a frame.
     */
    template <typename M, typename... Args>
    void insertMotion(Args&&... args) {
        static_assert(std::is_base_of_v<ProjectileMotion, M>,
            "insertMotion so aceita ProjectileMotion (Tracking, Wobble, Path). "
            "Para Accelerate/SlowDown/Activate/Deactivate use insertModifier.");
        mMotion = std::make_unique<M>(std::forward<Args>(args)...);
    }

    /**
     * @brief Acrescenta um Modifier: quem MODIFICA a velocidade existente ou
     * agenda quando ela passa a valer. Podem coexistir varios.
     */
    template <typename M, typename... Args>
    void insertModifier(Args&&... args) {
        static_assert(std::is_base_of_v<ProjectileModifier, M>,
            "insertModifier so aceita ProjectileModifier (Accelerate, SlowDown, "
            "Activate, Deactivate). Para Tracking/Wobble/Path use insertMotion.");
        mModifiers.emplace_back(std::make_unique<M>(std::forward<Args>(args)...));
    }

protected:

    Actor* mOwner;
    float mForwardSpeed;

    //Behavior structures and methods
    // A Motion e unica; os Modifiers se acumulam. Separar os dois torna a
    // exclusividade uma propriedade do tipo, e nao um aviso em comentario.
    /**
     * @brief Tempo maximo que um projetil pode viver, em segundos.
     *
     * Rede de seguranca do motor. Ate agora um projetil so morria saindo da
     * tela (IsOffScreen) ou por Kill(), e isso deixa buracos: um projetil lento
     * demais para atravessar a tela dentro da batalha, ou com posicao NaN - na
     * qual toda comparacao e falsa e IsOffScreen nunca dispara - viveria para
     * sempre, ocupando uma vaga do pool.
     *
     * O valor e folgado de proposito: nenhum ataque legitimo chega perto dele,
     * entao isto nao e mecanica de jogo, e sim a ultima linha de defesa contra
     * um ataque mal configurado.
     */
    static constexpr float kTempoDeVidaMaximo = 20.0f;

    float mTempoDeVida = 0.0f;

    /// Aparencia de fabrica, capturada por MarcarPadraoDeFabrica e devolvida por
    /// Reset(). A escala tambem vale para o colisor: CircleColliderComponent
    /// multiplica o raio por GetScale(), entao um projetil que volta do pool com
    /// escala errada tem a hitbox errada, nao so o sprite.
    float mEscalaPadrao = 1.0f;
    std::string mAnimacaoPadrao;
    bool mPadraoMarcado = false;

    std::unique_ptr<ProjectileBehavior> mMotion;
    std::vector<std::unique_ptr<ProjectileBehavior>> mModifiers;

    //Sub-funções de OnUpdate
    [[nodiscard]] virtual bool IsOffScreen() const = 0;

    // Ponto único de decisão de "este projétil morreu" (saiu de tela, colidiu).
    // Se este objeto tem uma factory de origem (foi obtido via
    // ProjectileFactory::Acquire, isto é, participa de Object Pooling),
    // marca Inactive e ESCONDE o sprite (SetIsVisible(false)) para que o
    // ProjectileManager o devolva ao pool sem deixar um "fantasma" visível
    // na última posição onde ele morreu. Caso contrário (ex: PlayerProjectile,
    // que nunca passa por uma ProjectileFactory), marca Destroy como sempre foi feito.
    void MarkDead();

private:

    ProjectileFactory* mOriginFactory{};

};


#endif //DPIBULLETHELLRE_PROJECTILE_H