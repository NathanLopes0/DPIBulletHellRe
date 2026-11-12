//
// Os efeitos do campo: as faiscas, as ondas do Ponto Extra e as moedas que voam para a nota.
//

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "../../Interface/Hud.h"
#include "../../Motor/Math.h"

class BossProjectile;
class DrawLinhasComponent;
class Scene;

/// O que o campo desenha por cima da luta e some sozinho: a faisca de cada acontecimento
/// (Hud::Faisca), a onda do Ponto Extra - que dissolve os projeteis do chefe que alcanca - e as
/// moedas em que eles viram, voando ate a ponta da barra da nota.
class EfeitosDaBatalha {
public:
    explicit EfeitosDaBatalha(Scene& cena);

    /// Uma faisca em 'onde'; 'quem' e o aluno (a cor do "!" da resposta), -1 se ninguem. Com muitas ao
    /// mesmo tempo, as mais velhas saem.
    void Faiscar(const Vector2& onde, Hud::Faisca tipo, int quem = -1);

    /// A onda do Ponto Extra abre de 'centro' (PontoExtra::RaioDaOnda).
    void SoltarOnda(const Vector2& centro);

    /// Anda as ondas; cada projetil do chefe que uma alcanca se dissolve, faisca e vira moeda.
    void AvancarOndas(float deltaTime, const std::vector<std::unique_ptr<BossProjectile>>& projeteis);

    /// Redesenha as faiscas, as ondas e as moedas, que voam ate 'pontaDaBarra'.
    void Desenhar(float deltaTime, const Vector2& pontaDaBarra);

private:
    struct Faisca {
        Vector2 onde;
        Hud::Faisca tipo = Hud::Faisca::Acerto;
        float idade = 0.0f;
        int quem = -1;   ///< o aluno (a cor do "!" da Resposta); -1 = ninguem
    };
    struct Onda {
        Vector2 centro;
        float t = 0.0f;
    };
    struct Moeda {
        Vector2 de;
        float t = 0.0f;
    };

    std::vector<Faisca> mFaiscas;
    std::vector<Onda> mOndas;
    std::vector<Moeda> mMoedas;
    DrawLinhasComponent* mLinhas = nullptr;

    static constexpr std::size_t kMaximoDeFaiscas = 64;
    static constexpr std::size_t kMaximoDeMoedas = 40;
    static constexpr float kVooDaMoeda = 0.55f;
    /// Por cima dos alunos (100) e do que se desenha sobre eles (102).
    static constexpr int kOrdem = 104;
};
