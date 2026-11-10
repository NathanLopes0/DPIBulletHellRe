//
// A moldura do campo.
//

#include "MolduraDoCampo.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Teachers/Boss.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/DrawFundoDoCampoComponent.h"
#include "../../Components/DrawComponents/DrawMolduraDoCampoComponent.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"

MolduraDoCampo::MolduraDoCampo(Scene& cena, const SDL_FRect& campo, const int larguraDaTela, const int alturaDaTela,
                               const std::vector<Player*>& jogadores)
    : mCena(cena) {
    // O caderno quadriculado embaixo de tudo, do tamanho do campo.
    auto fundo = std::make_unique<Actor>(&cena);
    fundo->AddComponent<DrawFundoDoCampoComponent>(campo, kOrdemDoFundo);
    cena.AddActor(std::move(fundo));

    float largura = 32.0f, altura = 48.0f;
    if (!jogadores.empty() && jogadores.front()) {
        if (const auto* sprite = jogadores.front()->GetComponent<DrawAnimatedComponent>()) {
            largura = static_cast<float>(sprite->GetSpriteWidth());
            altura = static_cast<float>(sprite->GetSpriteHeight());
        }
    }
    mAreaDoAluno = Hud::AreaDoAluno({campo.x, campo.y, campo.x + campo.w, campo.y + campo.h}, largura, altura);

    auto moldura = std::make_unique<Actor>(&cena);
    mDesenho = moldura->AddComponent<DrawMolduraDoCampoComponent>(kOrdemDaMoldura);
    mDesenho->Definir(static_cast<float>(larguraDaTela), static_cast<float>(alturaDaTela), mAreaDoAluno);
    cena.AddActor(std::move(moldura));
}

void MolduraDoCampo::Atualizar(const std::vector<Player*>& jogadores, const std::vector<Boss*>& corpos,
                               const float rede, Font* fonte) {
    std::vector<std::pair<Vector2, Vector2>> alunos;
    for (const Player* p : jogadores) {
        if (!p) continue;
        const auto* sprite = p->GetComponent<DrawAnimatedComponent>();
        if (!sprite) continue;
        alunos.emplace_back(p->GetPosition(), Vector2(static_cast<float>(sprite->GetSpriteWidth()) / 2.0f,
                                                      static_cast<float>(sprite->GetSpriteHeight()) / 2.0f));
    }
    mDesenho->SetAlunos(std::move(alunos));

    std::vector<float> xs;
    for (const Boss* corpo : corpos) {
        if (corpo && corpo->GetState() == ActorState::Active) {
            xs.push_back(std::clamp(corpo->GetPosition().x, mAreaDoAluno.esquerda, mAreaDoAluno.direita));
        }
    }
    while (mRotulos.size() < xs.size() && fonte) {
        auto ator = std::make_unique<Actor>(&mCena);
        auto* texto = ator->AddComponent<DrawTextComponent>("prof", fonte, 60, 18, 14, kOrdemDaMoldura + 2);
        texto->SetAjustarAoTexto(true);
        texto->SetColor(Vector3(1.0f, 0.45f, 0.4f));
        mRotulos.push_back(mCena.AddActor(std::move(ator)));
    }
    for (size_t i = 0; i < mRotulos.size(); ++i) {
        auto* texto = mRotulos[i]->GetComponent<DrawTextComponent>();
        const bool aceso = i < xs.size();
        if (texto) texto->SetIsVisible(aceso);
        if (aceso) mRotulos[i]->SetPosition(Vector2(xs[i], mAreaDoAluno.baixo + 13.0f));
    }
    mDesenho->SetProfessores(std::move(xs));
    mDesenho->SetRede(rede);
}
