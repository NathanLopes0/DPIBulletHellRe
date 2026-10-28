//
// Created by nslop on 19/11/2024.
//

#ifndef DPIBULLETHELLRE_PLAYERPROJECTILE_H
#define DPIBULLETHELLRE_PLAYERPROJECTILE_H

#include <cstdint>
#include <string>

#include <SDL_pixels.h>

#include "../Projectile.h"

class PlayerProjectile : public Projectile {

    friend class Player;
    friend class Orientador;

public:
    /// O dono e quem atira: o aluno, ou o orientador que luta do lado dele no
    /// TCC. O tiro nasce onde o dono esta, e paga a nota do mesmo jeito.
    PlayerProjectile(class Scene* scene, class Actor* owner);

    /// A DUVIDA DE UM ALUNO: o balao com o "?" na cor dele (0 o primeiro, 1 o
    /// segundo - Ferramentas/gerar_duvida.py).
    PlayerProjectile(class Scene* scene, class Actor* owner, size_t quem);

    /// O tiro com outra cara: o quadro 'quadro' da folha 'folha' (caminho em
    /// Assets, sem extensao), na 'escala', com o contorno 'contorno'. O
    /// alcance continua o do tiro de sempre (kRaioDoTiro).
    PlayerProjectile(class Scene* scene, class Actor* owner, const std::string& folha, int quadro,
                     float escala, SDL_Color contorno);

    /// O raio do tiro de sempre: metade dos 24 px do sprite dele.
    static constexpr float kRaioDoTiro = 12.0f;
    void OnUpdate(float deltaTime) override;
    [[nodiscard]] bool IsOffScreen() const override;
    void OnCollision(Actor* other) override;

    /// Um numero que nenhum outro tiro da partida tem. O manager apaga o tiro
    /// que morre, entao quem acompanha um tiro (o golpe do orientador) guarda
    /// isto, e nao o ponteiro, e o procura a cada quadro.
    [[nodiscard]] std::uint64_t Id() const { return mId; }

    /// Quem atirou: um Player, ou o Orientador. O registro da tentativa
    /// (Tentativas.h) separa os acertos de cada um por aqui.
    [[nodiscard]] Actor* Dono() const { return mOwner; }

    /// @brief O ESCUDO BARROU A PERGUNTA: o balao para, fica cinza, murcha em
    /// Hud::kMurcha e some. Enquanto isso nao acerta mais nada.
    void Murchar();
    [[nodiscard]] bool Murchando() const { return mMurcha >= 0.0f; }


private:

    Actor* mOwner;
    std::uint64_t mId;
    float mMurcha = -1.0f;        ///< segundos murchando; negativo = inteiro
    float mEscalaInteira = 1.0f;

    void ActivateProjectile();
    void DeactivateProjectile();

};


#endif //DPIBULLETHELLRE_PLAYERPROJECTILE_H
