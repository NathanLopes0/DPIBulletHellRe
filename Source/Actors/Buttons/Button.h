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
    void changeSelected() { isSelected = !isSelected; }


private:


protected:
    int mWidth;
    int mHeight;
    bool isSelected;
};


#endif //DPIBULLETHELLRE_BUTTON_H
