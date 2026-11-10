//
// A pausa da batalha, no computador.
//

#include "PausaDaBatalha.h"

#include <string>
#include <vector>

#include "../PainelDeMenu.h"
#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"

PausaDaBatalha::PausaDaBatalha(Scene& cena, Font* fonte, const Painel::Jeito jeito, const SDL_FRect& campo) {
    std::vector<std::string> opcoes;
    for (const Pausa::Opcao o : Pausa::kOpcoes) opcoes.emplace_back(Pausa::Texto(o));

    // O rodape sai do painel, como o de toda tela (Painel.h).
    const std::string rodape = Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher"),
                                               {Painel::Jogador::Um, Painel::Botao::Um, "confirmar"},
                                               {std::string("ESC"), std::string(), std::string("continuar")}},
                                              jeito);
    PainelDeMenu::Medidas medidas;
    medidas.ordem = kOrdem;
    mPainel = std::make_unique<PainelDeMenu>(cena, fonte, "PAUSA", opcoes, rodape, 0, medidas);

    // O numero da contagem, no meio do campo, sobre uma placa escura pequena.
    auto numero = std::make_unique<Actor>(&cena);
    numero->SetPosition(Vector2(campo.x + campo.w / 2.0f, campo.y + campo.h / 2.0f));
    numero->AddComponent<DrawCaixaComponent>(130, 130, SDL_Color{120, 140, 180, 230}, SDL_Color{12, 13, 17, 200},
                                             3, kOrdem)->SetIsVisible(false);
    auto* texto = numero->AddComponent<DrawTextComponent>(" ", fonte, 110, 110, 72, kOrdem + 1);
    texto->SetAjustarAoTexto(true);
    texto->SetIsVisible(false);
    mNumero = cena.AddActor(std::move(numero));
}

PausaDaBatalha::~PausaDaBatalha() = default;

void PausaDaBatalha::Abrir() {
    Pausa::Abrir(mMenu);
    mRetomada = 0.0f;
    MostrarNumero(false);

    // Quem estava segurando o manche ou o tiro quando apertou ESC nao anda no menu por isso.
    mLeitor.Segurar();

    mPainel->Selecionar(mMenu.selecionada);
    mPainel->Mostrar(true);
}

PausaDaBatalha::Pedido PausaDaBatalha::Processar(const Uint8* keyState, const bool escNovo) {
    const LeitorDoMenu::Batidas b = mLeitor.Ler(keyState);
    if (escNovo) {
        Fechar();
        return Pedido::Nada;
    }
    if (b.cima) Pausa::Mover(mMenu, -1);
    if (b.baixo) Pausa::Mover(mMenu, 1);
    mPainel->Selecionar(mMenu.selecionada);
    if (!b.confirmar) return Pedido::Nada;

    switch (Pausa::Escolhida(mMenu)) {
        case Pausa::Opcao::Continuar:
            Fechar();
            break;
        case Pausa::Opcao::Recomecar:
            return Pedido::Recomecar;
        case Pausa::Opcao::Sair:
            return Pedido::Sair;
    }
    return Pedido::Nada;
}

bool PausaDaBatalha::Avancar(const float deltaTime) {
    if (!Retomando()) return false;
    mRetomada -= deltaTime;
    if (Retomando()) {
        mNumero->GetComponent<DrawTextComponent>()->SetText(std::to_string(Pausa::Contagem(mRetomada)));
        return false;
    }
    mRetomada = 0.0f;
    MostrarNumero(false);
    return true;
}

void PausaDaBatalha::Fechar() {
    Pausa::Fechar(mMenu);
    mPainel->Mostrar(false);

    // A luta nao volta ainda.
    mRetomada = Pausa::kRetomada;
    mNumero->GetComponent<DrawTextComponent>()->SetText(std::to_string(Pausa::Contagem(mRetomada)));
    MostrarNumero(true);
}

void PausaDaBatalha::MostrarNumero(const bool visivel) {
    mNumero->GetComponent<DrawCaixaComponent>()->SetIsVisible(visivel);
    mNumero->GetComponent<DrawTextComponent>()->SetIsVisible(visivel);
}
