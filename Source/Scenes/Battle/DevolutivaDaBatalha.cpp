//
// A devolutiva da prova, anotada enquanto a luta acontece.
//

#include "DevolutivaDaBatalha.h"

#include "Battle.h"
#include "../../Actors/Player/Player.h"

DevolutivaDaBatalha::DevolutivaDaBatalha(const std::size_t jogadores, const Experimento::Condicao& condicao)
    : mPartes(jogadores), mContaCoberturas(condicao.Ligada(Experimento::Mecanica::Cobertura)) {}

void DevolutivaDaBatalha::AoTrocarDeFase(Battle&, const std::string& fase, const std::string& anuncio) {
    mCaderno.ComecarFase(anuncio, fase == "StateFinal");
}

void DevolutivaDaBatalha::AoPerderNota(Battle& b, const Player& aluno, const std::string& causa, const float perda) {
    mCaderno.Perder(causa, perda);
    if (Devolutiva::Parte* parte = ParteDe(b, &aluno)) ++parte->sofridos;
}

void DevolutivaDaBatalha::AoAcertarOProfessor(Battle& b, const Actor* quemAtirou, const bool ouviu) {
    // O escudo nao ouve a pergunta, e ela nao conta.
    if (!ouviu) return;
    if (Devolutiva::Parte* parte = ParteDe(b, quemAtirou)) ++parte->acertos;
}

void DevolutivaDaBatalha::AoCobrir(Battle& b, const Player& quemCobriu, const Player&) {
    // O experimento sem cobertura nao a paga, e a parte de quem cobriu tambem nao a mostra.
    if (!mContaCoberturas) return;
    if (Devolutiva::Parte* parte = ParteDe(b, &quemCobriu)) ++parte->coberturas;
}

Devolutiva::Parte* DevolutivaDaBatalha::ParteDe(const Battle& b, const Actor* ator) {
    const auto& jogadores = b.Jogadores();
    for (std::size_t i = 0; i < jogadores.size() && i < mPartes.size(); ++i) {
        if (ator && jogadores[i] == ator) return &mPartes[i];
    }
    return nullptr;
}
