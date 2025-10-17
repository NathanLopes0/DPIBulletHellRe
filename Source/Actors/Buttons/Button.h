//
// Created by nslop on 26/08/2024.
//

#ifndef DPIBULLETHELLRE_BUTTON_H
#define DPIBULLETHELLRE_BUTTON_H


#include "../Actor.h"

class Button : public Actor {

public:
    explicit Button(Scene* scene);
    ~Button() override = default;
    virtual void OnClick();

    void ChangeSelected() { mIsSelected = !mIsSelected; }
    [[nodiscard]] bool IsSelected() const { return mIsSelected; }

private:


protected:
    int mWidth{};
    int mHeight{};
    bool mIsSelected = false;
};


#endif //DPIBULLETHELLRE_BUTTON_H
