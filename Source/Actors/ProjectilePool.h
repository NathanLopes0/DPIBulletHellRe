//
// Object Pool genérico para Projéteis.
//

#pragma once

#include <memory>
#include <vector>
#include <functional>

/// Guarda instâncias de um tipo concreto de Projétil (ex.
template <typename T>
class ProjectilePool {

public:

    ProjectilePool() = default;

    // Empresta uma instância de T. - Se houver algo disponível no pool, devolve aquele objeto
    // reciclado (já chama T::Reset() nele antes de devolver)..
    std::unique_ptr<T> Acquire(const std::function<std::unique_ptr<T>()>& createFn) {
        if (!mAvailable.empty()) {
            std::unique_ptr<T> recycled = std::move(mAvailable.back());
            mAvailable.pop_back();
            recycled->Reset();
            return recycled;
        }

        auto novo = createFn();

        // Marca a aparencia de fabrica aqui, e nao dentro de cada createProjectile.
        if (novo) {
            novo->MarcarPadraoDeFabrica();
        }

        return novo;
    }

    // Devolve uma instância ao pool para reuso futuro.
    void Release(std::unique_ptr<T> projectile) {
        if (!projectile) { return; }

        // Normaliza o objeto antes de guardar.
        projectile->OnEnterPool();

        mAvailable.push_back(std::move(projectile));
    }

private:

    std::vector<std::unique_ptr<T>> mAvailable;

};