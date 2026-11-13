//
// Created by nslop on 26/08/2024.
//

#include "StageSelectButton.h"
#include "../../Motor/Font.h"
#include "../../Scenes/Scene.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"

#include "../../Motor/CaminhosArquivo.h"

StageSelectButton::StageSelectButton(Scene* scene, const std::string& buttonText,
                                     const int subject, const std::string& fontPath,
                                     const bool isLocked)
    : Button(scene)
    , mSubject(subject)
    , mFont(std::make_unique<Font>()) // Cria e assume a posse da fonte
    , mIsLocked(isLocked)
{

    // ---- Configuração do Ator (herdado de Button) ----
    mWidth = 128;
    mHeight = 64;

    // ---- Carregamento da Fonte ----
    mFont->Load(fontPath);

    // ---- Adição de Componentes ----
    // O losango de Ferramentas/gerar_interface.py: pixel art de 64 x 32, desenhada em 2x pelo
    // pixel mais proximo. Cada estado e um quadro ja pintado; o foco e o reflexo que atravessa a
    // pedra (quadros 1 a 6) e uma pausa (o 7 repetido), em kQuadrosPorSegundo.
    auto* animComp = AddComponent<DrawAnimatedComponent>(Caminhos::Asset("Icons/DPIBHLosango.png"),
                                                         Caminhos::Asset("Icons/DPIBHLosango.json"));
    if (animComp) {
        animComp->AddAnimation("Aberta", {0});
        animComp->AddAnimation("Foco", {1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7});
        animComp->AddAnimation("Aprovada", {9});
        animComp->AddAnimation("Fechada", {10});
        animComp->AddAnimation("FechadaEmFoco", {11});
        animComp->SetAnimation("Aberta");
        animComp->SetAnimFPS(kQuadrosPorSegundo);
        animComp->SetEscalaPropria(kEscalaDoLosango);
    }

    // No tamanho natural, e nao esticado numa caixa: o nome fica nitido.
    const auto textComp = AddComponent<DrawTextComponent>(buttonText, mFont.get(),
                                                          mWidth, mHeight / 3, kTamanhoDoNome, 120);
    if (textComp) textComp->SetAjustarAoTexto(true);

    if (mIsLocked && textComp) {
        // Dentro do losango fechado nao cabe nome.
        textComp->SetIsVisible(false);
    }

}

void StageSelectButton::OnUpdate(const float deltaTime) {
    // O pulo de quando ganha o foco: cresce de uma vez e volta em kDuracaoDoPulo.
    if (mIsSelected && !mEstavaSelecionado) mPulo = 1.0f;
    mEstavaSelecionado = mIsSelected;
    mPulo = Math::Max(0.0f, mPulo - deltaTime / kDuracaoDoPulo);

    if (auto* anim = GetComponent<DrawAnimatedComponent>()) {
        if (mIsLocked) {
            anim->SetAnimation(mIsSelected ? "FechadaEmFoco" : "Fechada");
        } else if (mIsSelected) {
            anim->SetAnimation("Foco");
        } else {
            anim->SetAnimation(mAprovado ? "Aprovada" : "Aberta");
        }
        // O pulo de quando ganha o foco; parado, a escala volta a ser inteira, e o pixel fica certo.
        anim->SetEscalaPropria(kEscalaDoLosango * (1.0f + kPulo * mPulo * mPulo));
    }

    // O nome embaixo do cadeado acende com o foco.
    if (mIsLocked && mRotuloFechado) {
        if (const auto dc = mRotuloFechado->GetComponent<DrawTextComponent>()) {
            dc->SetColor(mIsSelected ? kNomeFechadoEmFoco : kNomeFechado);
        }
    }
}

void StageSelectButton::SetText(const std::string& newText) {
    if (auto* textComp = GetComponent<DrawTextComponent>()) {
        textComp->SetText(newText);
    }
}
