//
// Created by nslop on 15/10/2025.
//

#ifndef DPIBULLETHELLRE_PROJECTILEMANAGER_H
#define DPIBULLETHELLRE_PROJECTILEMANAGER_H

#include <vector>
#include <memory>
#include "../../Math.h"

class Actor;
class Scene;
class PlayerProjectile;
class BossProjectile;
class Player;

class ProjectileManager {

public:
    explicit ProjectileManager(Scene* owner);
    ~ProjectileManager();

    // --- Ciclo de Vida
    void Update(float deltaTime);

    // --- Receber Projéteis criados por Bosses e Players
    void AddPlayerProjectile(std::unique_ptr<PlayerProjectile> projectile);

    // --- Função de conveniência (receber vários projectiles de uma vez só)
    void AddBossProjectiles(std::vector<std::unique_ptr<BossProjectile>> projectiles);

    // Acesso para usar para colisão
    [[nodiscard]] const std::vector<std::unique_ptr<PlayerProjectile>>& GetPlayerProjectiles() { return mPlayerProjectiles; }
    [[nodiscard]] const std::vector<std::unique_ptr<BossProjectile>>& GetBossProjectiles() { return mBossProjectiles; }

    void ClearBossProjectiles() const;

    /**
     * @brief Tira de cena os projeteis de um corpo que vai ser destruido.
     *
     * EXISTE POR CAUSA DE UM CRASH, e o crash vale contado. Cada Boss e DONO
     * das fabricas de projetil dele (Boss::mProjectileFactories), e todo
     * projetil guarda um ponteiro cru para a fabrica que o criou, para poder
     * voltar ao pool dela quando sai da tela. Enquanto so existia um chefe por
     * batalha isso nunca doia: o chefe so morria junto com a cena inteira.
     *
     * Com a fase que divide o chefe em dois passou a doer. Ao juntar os
     * corpos, o segundo e destruido COM AS FABRICAS DELE, e os projeteis que
     * ele ja tinha disparado continuam vivos apontando para memoria liberada -
     * o primeiro deles que sai da tela chama Release() num pool que nao existe
     * mais. Na pratica: o jogo fechava ao passar da fase 3 do Vitor para o
     * exame, que e a unica saida em que a batalha CONTINUA depois da divisao.
     *
     * A cura usa o que Projectile::MarkDead() ja fazia: sem fabrica de origem,
     * um projetil morre de verdade em vez de tentar voltar ao pool. Entao aqui
     * a fabrica e desligada ANTES de matar, e o projetil sai pelo caminho do
     * Destroy, que nao passa por Release() nenhum.
     */
    void DescartarProjeteisDe(const Actor* dono) const;

    // Executa uma passada de limpeza fora do Update(). Necessario para a tela
    // de fim de fase, onde Battle::OnUpdate retorna cedo e o Update() deste
    // manager deixa de ser chamado - sem isto, os projeteis marcados por
    // ClearBossProjectiles ficariam presos no vetor, nunca devolvidos ao pool.
    void Cleanup();

private:
    void CleanupProjectiles(); // para remover projéteis mortos

    Scene* mOwnerScene{};

    // --- O Manager vira DONO de todos os projéteis na tela (os Actors passam propriedade pra ele)
    std::vector<std::unique_ptr<PlayerProjectile>> mPlayerProjectiles;
    std::vector<std::unique_ptr<BossProjectile>> mBossProjectiles;


};


#endif //DPIBULLETHELLRE_PROJECTILEMANAGER_H