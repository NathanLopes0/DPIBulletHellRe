//
// Created by nslop on 05/04/2024.
//

#pragma once
#include <SDL_stdinc.h>

class Component {

public:
    // Constructor
    // (the lower the update order, the earlier the component updates)
    explicit Component(class Actor* owner, int updateOrder = 100);
    // Destructor
    virtual ~Component();
    // Update this component by delta time
    virtual void Update(float deltaTime);
    // Process input for this component (if needed)
    virtual void ProcessInput(const Uint8* keyState);

    [[nodiscard]] int GetUpdateOrder() const { return mUpdateOrder; }
    [[nodiscard]] class Actor* GetOwner() const { return mOwner; }
    [[nodiscard]] class Scene* GetScene() const;

    void SetEnabled(const bool enabled) { mIsEnabled = enabled; };
    [[nodiscard]] bool IsEnabled() const { return mIsEnabled; };

protected:
    // Owning actor
    class Actor* mOwner;
    // Update order
    int mUpdateOrder;

    bool mIsEnabled;
};
