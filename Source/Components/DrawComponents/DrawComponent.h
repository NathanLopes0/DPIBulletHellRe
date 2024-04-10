//
// Created by nslop on 05/04/2024.
//

#ifndef DPIBULLETHELLRE_DRAWCOMPONENT_H
#define DPIBULLETHELLRE_DRAWCOMPONENT_H
#include "../Component.h"

class DrawComponent : public Component {
public:
    explicit DrawComponent(class Actor* owner, int drawOrder = 100);
    ~DrawComponent() override;

    virtual void Draw(class SDL_Renderer* renderer);

    bool IsVisible() const { return mIsVisible; }
    void SetIsVisible(const bool isVisible) { mIsVisible = isVisible; }

    int GetDrawOrder() const { return mDrawOrder; }

protected:
    bool mIsVisible;
    int mDrawOrder;
};


#endif //DPIBULLETHELLRE_DRAWCOMPONENT_H
