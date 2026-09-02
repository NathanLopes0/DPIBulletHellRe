//
// Object Pool genérico para Projéteis.
//

#pragma once

#include <memory>
#include <vector>
#include <functional>

/**
 * @class ProjectilePool
 * @brief Guarda instâncias de UM tipo concreto de Projétil (ex: AndreBaloonProjectile)
 * para reaproveitamento, evitando o custo de alocar + montar Components
 * (RigidBody, DrawAnimated com carga de sprite/JSON, Collider) toda vez que
 * um ataque precisa disparar.
 *
 * O Pool é intencionalmente "burro": ele não sabe COMO construir um T (isso é
 * responsabilidade exclusiva da ProjectileFactory concreta, que já sabe montar
 * o "shape" certo de Components para aquele tipo). O Pool só empresta e recebe
 * de volta ponteiros já existentes, criando um novo apenas quando não há
 * nenhum disponível.
 *
 * @tparam T O tipo CONCRETO de Projectile (ex: AndreBaloonProjectile).
 * Reciclagem só faz sentido entre objetos do MESMO tipo concreto: eles têm o
 * mesmo conjunto de Components e a mesma classe C++, então "resetar" um objeto
 * reciclado é seguro. Nunca instancie ProjectilePool<Projectile> diretamente
 * (tipo abstrato); cada ProjectileFactory concreta possui o seu
 * ProjectilePool<TipoConcretoDoProjetilDela>.
 */
template <typename T>
class ProjectilePool {

public:

    ProjectilePool() = default;

    // Empresta uma instância de T.
    // - Se houver algo disponível no pool, devolve aquele objeto reciclado
    //   (já chama T::Reset() nele antes de devolver).
    // - Se o pool estiver vazio, chama createFn() para construir um novo
    //   (mesmo custo de hoje, sem pooling: só acontece na primeira vez que
    //   aquele tipo é necessário, ou quando todos os objetos já emprestados
    //   estão em uso simultâneo na tela).
    //
    // @param createFn Função que sabe montar um T do zero (normalmente um
    // lambda que encapsula o que hoje está em ProjectileFactory::createProjectile).
    std::unique_ptr<T> Acquire(const std::function<std::unique_ptr<T>()>& createFn) {
        if (!mAvailable.empty()) {
            std::unique_ptr<T> recycled = std::move(mAvailable.back());
            mAvailable.pop_back();
            recycled->Reset();
            return recycled;
        }

        return createFn();
    }

    // Devolve uma instância ao pool para reuso futuro. NÃO destrói o objeto:
    // o unique_ptr passado é movido para dentro do pool, então quem chamar
    // Release() perde a posse, mas o objeto continua vivo na memória.
    void Release(std::unique_ptr<T> projectile) {
        if (!projectile) { return; }

        // Normaliza o objeto ANTES de guardar. Este e o unico ponto por onde
        // qualquer objeto entra no pool, entao e aqui que o invariante "o que
        // esta guardado esta invisivel e inerte" tem de ser garantido - e nao
        // espalhado por quem chama (que ja esqueceu duas vezes: Prewarm e
        // ClearBossProjectiles).
        projectile->OnEnterPool();

        mAvailable.push_back(std::move(projectile));
    }

    // Quantos objetos estão disponíveis para reuso imediato (não em uso na tela).
    [[nodiscard]] size_t GetAvailableCount() const { return mAvailable.size(); }

private:

    std::vector<std::unique_ptr<T>> mAvailable;

};