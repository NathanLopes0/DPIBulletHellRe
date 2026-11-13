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
    // O losango de Ferramentas/gerar_interface.py: feito no dobro do tamanho, desenhado na metade
    // e com filtro linear, para a tela cheia nao o serrilhar. Cada estado e um quadro ja pintado.
    auto* animComp = AddComponent<DrawAnimatedComponent>(Caminhos::Asset("Icons/DPIBHLosango.png"),
                                                         Caminhos::Asset("Icons/DPIBHLosango.json"));
    if (animComp) {
        animComp->AddAnimation("Aberta", {0});
        animComp->AddAnimation("Foco", {1});
        animComp->AddAnimation("Aprovada", {2});
        animComp->AddAnimation("Fechada", {3});
        animComp->AddAnimation("FechadaEmFoco", {4});
        animComp->SetAnimation("Aberta");
        animComp->SetEscalaPropria(kEscalaDoLosango);
        animComp->SuavizarAoEscalar();
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
        // Em foco, o losango respira de leve; o pulo vem por cima.
        mRespiro += deltaTime;
        const float respiro = mIsSelected ? kRespiro * (0.5f + 0.5f * Math::Sin(mRespiro * kRitmoDoRespiro)) : 0.0f;
        anim->SetEscalaPropria(kEscalaDoLosango * (1.0f + respiro + kPulo * mPulo * mPulo));
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
