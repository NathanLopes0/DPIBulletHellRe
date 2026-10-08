//
// Created by nslop on 26/08/2024.
//

#include "StageSelectButton.h"
#include "../../Font.h"
#include "../../Scenes/Scene.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"

#include "../../CaminhosArquivo.h"

StageSelectButton::StageSelectButton(Scene* scene, const std::string& buttonText,
                                     const int subject, const std::string& fontPath,
                                     const bool isLocked)
    : Button(scene)
    , mSubject(subject)
    , mFont(std::make_unique<Font>()) // Cria e assume a posse da fonte
    , mIsLocked(isLocked)
{

    // --- Configuração do Ator (herdado de Button) ---
    mWidth = 128;
    mHeight = 64;

    // --- Carregamento da Fonte ---
    mFont->Load(fontPath);

    // --- Adição de Componentes ---
    auto* animComp = AddComponent<DrawAnimatedComponent>(Caminhos::Asset("Icons/DPIBHStageSelectButton.png"),
                                                         Caminhos::Asset("Icons/DPIBHStageSelectButton.json"));
    if (animComp) {
        animComp->AddAnimation("Button", {0});
        animComp->AddAnimation("Selected", {1});
        animComp->AddAnimation("Locked", {2});
        animComp->SetAnimation("Button");
        animComp->SetAnimFPS(10.0f);
    }

    // Passando a fonte para o DrawTextComponent usando mFont.get() porque to usando unique_ptr agr
    const auto textComp = AddComponent<DrawTextComponent>(buttonText, mFont.get(),
                                                          mWidth / 2, mHeight / 4, 24, 120);

    if (mIsLocked && textComp) {
        // DENTRO do losango fechado nao cabe nome: as correntes e o cadeado sao
        // desenhados bem no meio dele, exatamente onde o texto cairia. O nome da
        // materia fechada vai EMBAIXO - ver StageSelect::CreateButton.
        textComp->SetIsVisible(false);
    }

}


void StageSelectButton::OnUpdate(float deltaTime) {
    // Pede o componente de animação
    if (auto* anim = GetComponent<DrawAnimatedComponent>()) {
        // Checa o estado do botão (mIsSelected é herdado da classe Button)

        if (mIsLocked) {
            // A SELECAO SO CLAREIA UM POUCO a materia fechada. Clarear ate o
            // branco, como nas abertas, apagava o nome escrito em cima dela -
            // e fazia a fechada em foco parecer tao jogavel quanto as outras.
            anim->SetAnimation("Locked");
            anim->SetColor(mIsSelected ? 140 : 95, mIsSelected ? 140 : 95,
                           mIsSelected ? 200 : 140);

        } else if (mIsSelected) {
            anim->SetAnimation("Selected");
            anim->SetColor(255,255,255);

        } else if (mAprovado) {
            // VERDE PARA O QUE JA PASSOU. E o progresso do curso visivel de
            // relance, sem o aluno precisar passar a seta por materia nenhuma.
            anim->SetAnimation("Button");
            anim->SetColor(110,190,130);

        } else {
            anim->SetAnimation("Button");
            anim->SetColor(100,100,200);
        }
    }

    // O NOME EMBAIXO DO CADEADO ACENDE COM O FOCO. E o que diz onde a seta
    // esta numa materia fechada: o losango fechado quase nao muda de cor, e
    // clarea-lo ate o branco o faria parecer tao jogavel quanto os abertos.
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
