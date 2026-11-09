//
// O painel de menu que a pausa e o resultado da batalha compartilham.
//

#include "PainelDeMenu.h"

#include <algorithm>
#include <memory>

#include "Scene.h"
#include "TextoDeCena.h"
#include "../Actors/Actor.h"
#include "../Motor/Game.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {
    /// Um texto que encolhe para caber, em vez de esticar (TextoDeCena.h).
    Actor* Texto(Scene& cena, Font* fonte, const std::string& texto, const Vector2& onde,
                 const int largura, const int altura, const int tamanho, const int ordem) {
        return CriarTexto(cena, fonte, texto, onde, tamanho, largura, altura, ordem)->GetOwner();
    }

    void Visivel(Actor* a, const bool visivel) {
        if (!a) return;
        if (auto* c = a->GetComponent<DrawCaixaComponent>()) c->SetIsVisible(visivel);
        if (auto* t = a->GetComponent<DrawTextComponent>()) t->SetIsVisible(visivel);
    }
}

PainelDeMenu::PainelDeMenu(Scene& cena, Font* fonte, const std::string& titulo,
                           const std::vector<std::string>& opcoes, const std::string& rodape,
                           const int linhas, const Medidas medidas)
    : mMedidas(medidas)
{
    const auto w = static_cast<float>(cena.GetGame()->GetWindowWidth());
    const auto h = static_cast<float>(cena.GetGame()->GetWindowHeight());
    mMeio = Vector2(w / 2.0f, h / 2.0f);
    const float topo = mMeio.y - static_cast<float>(medidas.alturaDaPlaca) / 2.0f;
    const float baixo = mMeio.y + static_cast<float>(medidas.alturaDaPlaca) / 2.0f;
    const int o = medidas.ordem;

    auto veu = std::make_unique<Actor>(&cena);
    veu->SetPosition(mMeio);
    veu->AddComponent<DrawCaixaComponent>(static_cast<int>(w), static_cast<int>(h), SDL_Color{0, 0, 0, 0},
                                          SDL_Color{6, 8, 16, 165}, 0, o);
    mVeu = cena.AddActor(std::move(veu));

    auto placa = std::make_unique<Actor>(&cena);
    placa->SetPosition(mMeio);
    placa->AddComponent<DrawCaixaComponent>(medidas.larguraDaPlaca, medidas.alturaDaPlaca,
                                            SDL_Color{120, 140, 180, 230}, SDL_Color{12, 13, 17, 225},
                                            3, o + 1);
    mPlaca = cena.AddActor(std::move(placa));

    mTitulo = Texto(cena, fonte, titulo, Vector2(mMeio.x, topo + 62.0f),
                    medidas.larguraDaPlaca - 60, 64, 52, o + 3);

    for (int i = 0; i < linhas; ++i) {
        mLinhas.push_back(Texto(cena, fonte, "", Vector2(mMeio.x, topo + 128.0f + static_cast<float>(i) * kPassoDasLinhas),
                                medidas.larguraDaPlaca - 60, 34, 26, o + 3));
    }

    // A moldura da opcao em foco entra antes dos textos e com ordem menor, para ficar atras deles.
    auto selecao = std::make_unique<Actor>(&cena);
    selecao->SetPosition(mMeio);
    selecao->AddComponent<DrawCaixaComponent>(kLarguraDaSelecao, kAlturaDaSelecao,
                                              SDL_Color{235, 235, 235, 255}, SDL_Color{34, 38, 48, 150},
                                              3, o + 2);
    mSelecao = cena.AddActor(std::move(selecao));

    // Criadas todas antes de posicionar.
    for (const std::string& texto : opcoes) {
        mOpcoes.push_back(Texto(cena, fonte, texto, mMeio, kLarguraDaSelecao - 40, 42, 34, o + 3));
    }
    for (size_t i = 0; i < mOpcoes.size(); ++i) {
        mOpcoes[i]->SetPosition(Vector2(mMeio.x, YDaOpcao(static_cast<int>(i))));
    }

    mRodape = Texto(cena, fonte, rodape, Vector2(mMeio.x, baixo - 40.0f),
                    medidas.larguraDaPlaca - 40, 28, 18, o + 3);

    Selecionar(0);
    Mostrar(false);
}

float PainelDeMenu::YDaOpcao(const int indice) const {
    const float baixo = mMeio.y + static_cast<float>(mMedidas.alturaDaPlaca) / 2.0f;
    const int depois = static_cast<int>(mOpcoes.size()) - 1 - indice;
    return baixo - 98.0f - static_cast<float>(depois) * kPassoDasOpcoes;
}

void PainelDeMenu::Mostrar(const bool visivel) {
    mVisivel = visivel;
    for (Actor* a : {mVeu, mPlaca, mTitulo, mSelecao, mRodape}) Visivel(a, visivel);
    for (Actor* a : mOpcoes) Visivel(a, visivel);
    for (size_t i = 0; i < mLinhas.size(); ++i) Visivel(mLinhas[i], visivel && i < mLinhasEmUso);
}

void PainelDeMenu::Selecionar(const int indice) {
    if (!mSelecao || indice < 0 || indice >= static_cast<int>(mOpcoes.size())) return;
    mSelecao->SetPosition(Vector2(mMeio.x, YDaOpcao(indice)));
}

void PainelDeMenu::SetTitulo(const std::string& texto, const Vector3& cor) {
    if (!mTitulo) return;
    auto* t = mTitulo->GetComponent<DrawTextComponent>();
    t->SetText(texto);
    t->SetColor(cor);
}

void PainelDeMenu::ColorirLinha(const int indice, const Vector3& cor) {
    if (indice < 0 || static_cast<size_t>(indice) >= mLinhas.size()) return;
    mLinhas[static_cast<size_t>(indice)]->GetComponent<DrawTextComponent>()->SetColor(cor);
}

Vector2 PainelDeMenu::OndeDaLinha(const int indice) const {
    const float topo = mMeio.y - static_cast<float>(mMedidas.alturaDaPlaca) / 2.0f;
    return Vector2(mMeio.x, topo + 128.0f + static_cast<float>(indice) * kPassoDasLinhas);
}

int PainelDeMenu::AlturaPara(const int linhas) {
    // As quatro linhas de sempre cabiam em 480; cada uma a mais pede um passo.
    return 480 + static_cast<int>(kPassoDasLinhas) * std::max(0, linhas - 4);
}

void PainelDeMenu::SetLinhas(const std::vector<std::string>& linhas, const int destaque) {
    mLinhasEmUso = std::min(linhas.size(), mLinhas.size());
    for (size_t i = 0; i < mLinhasEmUso; ++i) {
        auto* t = mLinhas[i]->GetComponent<DrawTextComponent>();
        t->SetText(linhas[i].empty() ? " " : linhas[i]);
        t->SetColor(static_cast<int>(i) == destaque ? Color::Gold : Color::White);
    }
    Mostrar(mVisivel);
}
