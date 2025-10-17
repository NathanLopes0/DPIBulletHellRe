//
// Created by nslop on 04/04/2024.
//

#ifndef DPIBULLETHELLRE_ACTOR_H
#define DPIBULLETHELLRE_ACTOR_H

#include <algorithm>
#include <vector>
#include <memory>
#include <SDL_stdinc.h>
#include "../Math.h"
#include "../Components/Component.h"

enum class ActorState
{
    Active,
    Paused,
    Destroy
};


class Actor {

public:
    Actor(class Scene* scene);
    virtual ~Actor() = default;

    // Update function called from Game (not overridable)
    void Update(float deltaTime);
    // ProcessInput function called from Game (not overridable)
    void ProcessInput(const Uint8* keyState);

    // Position getter/setter
    const Vector2& GetPosition() const { return mPosition; }
    void SetPosition(const Vector2& pos) { mPosition = pos; }

    Vector2 GetForward() const { return Vector2(Math::Cos(mRotation), -Math::Sin(mRotation)); }

    // Scale getter/setter
    float GetScale() const { return mScale; }
    void SetScale(float scale) { mScale = scale; }

    // Rotation getter/setter
    float GetRotation() const { return mRotation; }
    void SetRotation(float rotation) { mRotation = rotation; }

    // State getter/setter
    ActorState GetState() const { return mState; }
    void SetState(ActorState state) { mState = state; }

    // Game getter
    class Scene* GetScene() { return mScene; }

    // Returns component of type T, or null if doesn't exist
    template <typename T>
    T* GetComponent() const
    {
        for (const auto& c : mComponents)
        {
            T* t = dynamic_cast<T*>(c.get());
            if (t != nullptr)
            {
                return t;
            }
        }

        return nullptr;
    }

    template <typename T, typename... TArgs>
    T* AddComponent(TArgs&&... args) {
        auto newComponent = std::make_unique<T>(this, std::forward<TArgs>(args)...);
        T *rawPtr = newComponent.get();
        mComponents.emplace_back(std::move(newComponent));

        std::sort(mComponents.begin(), mComponents.end(),
            [](const std::unique_ptr<Component>& a, const std::unique_ptr<Component>& b) {
                return a->GetUpdateOrder() < b->GetUpdateOrder();
            });

        return rawPtr;
    }

    // Any actor-specific collision code (overridable)
    virtual void OnCollision();
    virtual void Kill();

protected:
    class Scene* mScene;

    // Any actor-specific update code (overridable)
    virtual void OnUpdate(float deltaTime);
    // Any actor-specific update code (overridable)
    virtual void OnProcessInput(const Uint8* keyState);

    // Actor's state
    ActorState mState;

    // Transform
    Vector2 mPosition;
    float mScale;
    float mRotation;

    // Components
    std::vector<std::unique_ptr<class Component>> mComponents;

private:
    friend class Component;

};


#endif //DPIBULLETHELLRE_ACTOR_H
