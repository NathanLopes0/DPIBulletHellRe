//
// Created by nslop on 26/08/2024.
//

#ifndef DPIBULLETHELLRE_BUTTON_H
#define DPIBULLETHELLRE_BUTTON_H


#include "../Actor.h"

class Button : public Actor {

public:
    explicit Button(Scene* scene);
    virtual void onClick();


private:
    bool isSelected;

protected:
    int mWidth;
    int mHeight;
};


#endif //DPIBULLETHELLRE_BUTTON_H
