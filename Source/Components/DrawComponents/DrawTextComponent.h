//
// Created by nslop on 04/06/2024.
//

#ifndef DPIBULLETHELLRE_DRAWTEXTCOMPONENT_H
#define DPIBULLETHELLRE_DRAWTEXTCOMPONENT_H

#include "DrawComponent.h"
#include <string>

class DrawTextComponent : public DrawComponent {

public:

    DrawTextComponent(class Actor* owner, const std::string &text, class Font* font,
                        int width, int height, int fontSize = 24, int drawOrder = 100);
    ~DrawTextComponent();

    void SetText(const std::string &text);

    void Draw(SDL_Renderer* renderer) override;

protected:
    SDL_Texture* mTextSurface;
    class Font* mFont;

    int mSize;
    int mWidth;
    int mHeight;
};


#endif //DPIBULLETHELLRE_DRAWTEXTCOMPONENT_H
