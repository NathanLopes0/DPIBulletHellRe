//
// Created by nslop on 26/08/2024.
//

#include "StageSelectButton.h"
#include "../../Font.h"
#include "../../Scenes/Scene.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"

StageSelectButton::StageSelectButton(Scene* scene, const std::string& buttonText,
                                     const Game::GameSubject subject, const std::string& fontPath)
    : Button(scene)
    , mSubject(subject)
    , mFont(std::make_unique<Font>()) // Cria e assume a posse da fonte
{

    // --- Configuração do Ator (herdado de Button) ---
    mWidth = 128;
    mHeight = 64;

    // --- Carregamento da Fonte ---
    mFont->Load(fontPath);

    // --- Adição de Componentes ---
    auto* animComp = AddComponent<DrawAnimatedComponent>("../Assets/Icons/DPIBHStageSelectButton.png",
                                                         "../Assets/Icons/DPIBHStageSelectButton.json");
    if (animComp) {
        animComp->AddAnimation("Button", {0});
        animComp->AddAnimation("Selected", {1});
        animComp->SetAnimation("Button");
        animComp->SetAnimFPS(10.0f);
    }

    // Passando a fonte para o DrawTextComponent usando mFont.get() porque to usando unique_ptr agr
    AddComponent<DrawTextComponent>(buttonText, mFont.get(), mWidth / 2, mHeight / 4, 24, 120);
}


void StageSelectButton::OnUpdate(float deltaTime) {
    // Pede o componente de animação
    if (auto* anim = GetComponent<DrawAnimatedComponent>()) {
        // Checa o estado do botão (mIsSelected é herdado da classe Button)
        if (mIsSelected) {
            anim->SetAnimation("Selected");
        } else {
            anim->SetAnimation("Button");
        }
    }
}

void StageSelectButton::SetText(const std::string& newText) {
    if (auto* textComp = GetComponent<DrawTextComponent>()) {
        textComp->SetText(newText);
    }
}
