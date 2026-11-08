//
// A tela que cobre a batalha enquanto os projeteis sao preparados.
//

#pragma once

#include <vector>

#include "../../Curso/Materias.h"
#include "../../Interface/Carregamento.h"
#include "../../Regras/Dificuldade.h"

class Actor;
class Font;
class Scene;

/// O veu, o nome da materia e a barra que enche; quando a prova fica pronta, esmaece.
class TelaDeCarregamento {
public:
    TelaDeCarregamento(Scene& cena, const Materias::Materia* materia, Dificuldade::Nivel nivel,
                       Font* fonteDoTitulo, Font* fonteDoTexto);

    /// Leva a barra ate 'feito' (0 a 1); pronta, apaga o texto e esmaece o veu.
    void Avancar(float feito, float deltaTime);

    /// A tela ja sumiu de todo.
    [[nodiscard]] bool Acabou() const { return Carregamento::Acabou(mEstado); }

    /// Tira os atores da tela de cena.
    void Remover();

private:
    Carregamento::Estado mEstado;
    Actor* mVeu = nullptr;
    Actor* mBarra = nullptr;
    std::vector<Actor*> mTextos;
};
