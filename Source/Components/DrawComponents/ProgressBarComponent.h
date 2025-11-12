//
// Created by gensh on 11/11/2025.
//

#pragma once
#include "DrawComponent.h"
#include "SDL_pixels.h"


class ProgressBarComponent : public DrawComponent {


public:

    enum class Orientation {
        Horizontal,
        Vertical
    };

    explicit ProgressBarComponent(Actor* owner, int drawOrder = 200);

    void Draw(SDL_Renderer *renderer) override;

    void SetOrientation(Orientation orientation) { mOrientation = orientation;}
    void SetProgress(float progress);
    void SetColors(SDL_Color background, SDL_Color foreground);
    void SetSize(int width, int height);

private:
    float mProgress;
    int mWidth;
    int mHeight;

    SDL_Color mBackgroundColor;
    SDL_Color mForegroundColor;

    Orientation mOrientation;

};