//
// Created by nslop on 10/12/2024.
//

#pragma once

#include <memory>
#include <string>

class Projectile;
class Scene;
class Actor;

/**
 * @interface ProjectileFactory
 * @brief Define o contrato para qualquer "fábrica"
 * que saiba como construir um tipo de projétil.
 */
class ProjectileFactory {
public:
    virtual ~ProjectileFactory() = default;

    /**
     * @brief O método principal da fábrica.
     * @param scene A cena onde o projétil será criado.
     * @param owner O ator que "disparou" o projétil (pode ser um Boss ou Player).
     * @return Um unique_ptr para o Projétil recém-criado.
     */
    virtual std::unique_ptr<Projectile> createProjectile(Scene *scene, Actor *owner) = 0;
    void SetDataPath(std::string path);
    void SetSpritePath(std::string path);

    /**
     * @brief Pede um Projétil pronto para uso, SEM parâmetros de posição/cena/owner
     * explícitos por chamada. É o ponto de entrada do Object Pooling: a Factory
     * concreta decide, internamente, se reaproveita um objeto Inactive do seu
     * ProjectilePool ou se cria um novo via createProjectile (mesmo custo de hoje).
     *
     * @param scene A cena onde o projétil vai atuar.
     * @param owner O ator dono do disparo (pode mudar entre chamadas, mesmo
     * reciclando o mesmo objeto físico — por isso Reset()+SetOwner() acontecem
     * aqui dentro).
     * @return Um Projectile pronto (ActorState::Active), novo ou reciclado.
     */
    virtual std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) = 0;

    /**
     * @brief Devolve um Projétil ao pool desta Factory para reuso futuro.
     * Chamado pelo ProjectileManager quando encontra um projétil
     * ActorState::Inactive cuja GetOriginFactory() é esta Factory.
     * NÃO destrói o objeto — apenas o move de volta para o pool interno.
     */
    virtual void Release(std::unique_ptr<Projectile> projectile) = 0;

    /**
     * @brief Pré-aquece o pool desta Factory, criando 'count' instâncias
     * ANTES da batalha começar (tipicamente numa tela de carregamento) e
     * devolvendo todas ao pool ao final. Diferente de chamar Acquire()+Release()
     * em sequência (que reciclaria o MESMO objeto repetidamente e nunca criaria
     * mais que 1), este método mantém todas as instâncias vivas simultaneamente
     * durante a criação, garantindo que o pool termine com 'count' objetos
     * DISTINTOS disponíveis — cobrindo o pior caso de projéteis simultâneos
     * na tela sem nenhuma alocação/carga de textura durante o gameplay real.
     *
     * @param scene A cena onde os projéteis serão temporariamente instanciados.
     * @param owner O dono (Boss concreto) — necessário porque createProjectile
     * valida o tipo do owner internamente (ex: dynamic_cast<Andre*>).
     * @param count Quantas instâncias criar e deixar disponíveis no pool.
     */
    virtual void Prewarm(Scene* scene, Actor* owner, int count) = 0;

protected:
    std::string mSpritePath;
    std::string mDataPath;
};