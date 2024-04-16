//
// Created by nslop on 05/04/2024.
//

#ifndef DPIBULLETHELLRE_DRAWCOMPONENT_H
#define DPIBULLETHELLRE_DRAWCOMPONENT_H

#include "../Component.h"
#include <vector>
#include <SDL.h>

class DrawComponent : public Component {
public:
    
    // Lower drawOrder corresponds with further back
    explicit DrawComponent(class Actor* owner, int drawOrder = 100);
    ~DrawComponent() override;

    virtual void Draw(class SDL_Renderer* renderer);

    [[nodiscard]] bool IsVisible() const { return mIsVisible; }
    void SetIsVisible(const bool isVisible) { mIsVisible = isVisible; }

    [[nodiscard]] int GetDrawOrder() const { return mDrawOrder; }

protected:
    bool mIsVisible;
    int mDrawOrder;
};


#endif //DPIBULLETHELLRE_DRAWCOMPONENT_H
