//
// Os efeitos do campo.
//

#include "EfeitosDaBatalha.h"

#include <algorithm>

#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../Regras/PontoExtra.h"

EfeitosDaBatalha::EfeitosDaBatalha(Scene& cena) {
    auto efeitos = std::make_unique<Actor>(&cena);
    mLinhas = efeitos->AddComponent<DrawLinhasComponent>(kOrdem);
    cena.AddActor(std::move(efeitos));
}

void EfeitosDaBatalha::Faiscar(const Vector2& onde, const Hud::Faisca tipo, const int quem) {
    if (mFaiscas.size() >= kMaximoDeFaiscas) mFaiscas.erase(mFaiscas.begin());
    mFaiscas.push_back({onde, tipo, 0.0f, quem});
}

void EfeitosDaBatalha::SoltarOnda(const Vector2& centro) {
    mOndas.push_back(Onda{centro, 0.0f});
}

void EfeitosDaBatalha::AvancarOndas(const float deltaTime, const std::vector<std::unique_ptr<BossProjectile>>& projeteis) {
    for (Onda& o : mOndas) {
        o.t += deltaTime;
        const float raio = PontoExtra::RaioDaOnda(o.t);
        // O mesmo filtro do ClearAtaquesDoChefe.
        for (const auto& p : projeteis) {
            if (!p || p->Enfeite()) continue;
            const auto estado = p->GetState();
            if (estado != ActorState::Active && estado != ActorState::Paused) continue;
            if ((p->GetPosition() - o.centro).Length() > raio) continue;
            const Vector2 onde = p->GetPosition();
            p->Kill();
            Faiscar(onde, Hud::Faisca::Dissolvido);
            if (mMoedas.size() < kMaximoDeMoedas) mMoedas.push_back(Moeda{onde, 0.0f});
        }
    }
    mOndas.erase(std::remove_if(mOndas.begin(), mOndas.end(),
                                [](const Onda& o) { return o.t >= PontoExtra::kDuracaoDaOnda; }),
                 mOndas.end());
    for (Moeda& m : mMoedas) m.t += deltaTime;
    mMoedas.erase(std::remove_if(mMoedas.begin(), mMoedas.end(),
                                 [](const Moeda& m) { return m.t >= kVooDaMoeda; }),
                  mMoedas.end());
}

void EfeitosDaBatalha::Desenhar(const float deltaTime, const Vector2& pontaDaBarra) {
    mLinhas->Limpar();
    for (Faisca& f : mFaiscas) f.idade += deltaTime;
    mFaiscas.erase(std::remove_if(mFaiscas.begin(), mFaiscas.end(),
                                  [](const Faisca& f) { return f.idade >= Hud::Vida(f.tipo); }),
                   mFaiscas.end());

    const auto alfa = [](const float a) { return static_cast<Uint8>(std::clamp(a, 0.0f, 1.0f) * 255.0f); };
    const auto raios = [this](const Vector2& c, const float de, const float ate, const int n, const float giro,
                              const SDL_Color cor) {
        for (int k = 0; k < n; ++k) {
            const float a = giro + 2.0f * Math::Pi * static_cast<float>(k) / static_cast<float>(n);
            const Vector2 u(Math::Cos(a), Math::Sin(a));
            mLinhas->Acrescentar(c + u * de, c + u * ate, cor);
        }
    };
    const auto anel = [this](const Vector2& c, const float r, const SDL_Color cor) {
        for (const auto& [a, b] : PontoExtra::Anel(c, r, 24)) mLinhas->Acrescentar(a, b, cor);
    };

    for (const Faisca& f : mFaiscas) {
        const float r = Hud::Raio(f.tipo, f.idade);
        const float b = Hud::Brilho(f.tipo, f.idade);
        // O giro de cada uma sai de onde ela nasceu: duas lado a lado nao sao a mesma estrela repetida.
        const float giro = f.onde.x * 0.37f + f.onde.y * 0.11f;
        switch (f.tipo) {
            case Hud::Faisca::Acerto:
                // Miuda e branca: sao dez por segundo, e nao podem cobrir o chefe.
                raios(f.onde, r * 0.45f, r, 5, giro, SDL_Color{255, 250, 220, alfa(0.9f * b)});
                break;
            case Hud::Faisca::Derrubada:
                raios(f.onde, r * 0.35f, r, 8, giro, SDL_Color{255, 160, 60, alfa(b)});
                anel(f.onde, r * 0.7f, SDL_Color{255, 210, 120, alfa(0.6f * b)});
                break;
            case Hud::Faisca::Dano:
                anel(f.onde, r, SDL_Color{255, 70, 60, alfa(b)});
                anel(f.onde, r - 3.0f, SDL_Color{255, 70, 60, alfa(0.5f * b)});
                raios(f.onde, r * 0.25f, r * 0.6f, 8, giro, SDL_Color{255, 120, 100, alfa(0.8f * b)});
                break;
            case Hud::Faisca::Coleta:
                anel(f.onde, r, SDL_Color{255, 215, 80, alfa(b)});
                raios(f.onde, r * 0.8f, r * 1.15f, 4, giro + f.idade * 6.0f, SDL_Color{255, 240, 160, alfa(b)});
                break;
            case Hud::Faisca::Dissolvido:
                // O tiro vira brilho: uma estrela dourada de quatro pontas.
                raios(f.onde, 0.0f, r, 4, giro, SDL_Color{255, 225, 120, alfa(b)});
                raios(f.onde, 0.0f, r * 0.55f, 4, giro + Math::Pi / 4.0f, SDL_Color{255, 250, 210, alfa(b)});
                break;
            case Hud::Faisca::Resposta:
            case Hud::Faisca::RespostaDePerto: {
                // O "!" sobe e some, na cor do aluno, com o miolo claro e um contorno escuro. De perto
                // sao dois: "!!".
                const Hud::Rgb c = Hud::CorDoAluno(f.quem < 0 ? 9 : static_cast<size_t>(f.quem));
                const SDL_Color cor{c.r, c.g, c.b, alfa(b)};
                const SDL_Color miolo{255, 255, 240, alfa(b)};
                const SDL_Color contorno{20, 22, 34, alfa(0.85f * b)};
                const Vector2 p = f.onde + Vector2(0.0f, -10.0f - r);
                const auto exclamacao = [&](const float x0, const int largura, const float folga, const SDL_Color cor) {
                    for (int dx = -largura; dx <= largura; ++dx) {
                        const float x = x0 + static_cast<float>(dx);
                        mLinhas->Acrescentar(p + Vector2(x, -16.0f - folga), p + Vector2(x, -4.0f + folga), cor);
                        mLinhas->Acrescentar(p + Vector2(x, -1.0f - folga), p + Vector2(x, 2.0f + folga), cor);
                    }
                };
                const bool dois = f.tipo == Hud::Faisca::RespostaDePerto;
                constexpr float kLados[] = {-5.0f, 5.0f};
                for (int i = 0; i < (dois ? 2 : 1); ++i) {
                    const float x0 = dois ? kLados[i] : 0.0f;
                    exclamacao(x0, 3, 1.0f, contorno);
                    exclamacao(x0, 2, 0.0f, cor);
                    exclamacao(x0, 0, -1.0f, miolo);
                }
                break;
            }
            case Hud::Faisca::Cobertura: {
                // A derrubada dourada, com o anel na cor de quem cobriu: salvou o parceiro.
                const Hud::Rgb c = Hud::CorDoAluno(f.quem < 0 ? 9 : static_cast<size_t>(f.quem));
                raios(f.onde, r * 0.3f, r, 8, giro, SDL_Color{255, 220, 110, alfa(b)});
                anel(f.onde, r * 0.75f, SDL_Color{c.r, c.g, c.b, alfa(0.9f * b)});
                anel(f.onde, r * 0.75f - 2.0f, SDL_Color{255, 245, 200, alfa(0.6f * b)});
                break;
            }
            case Hud::Faisca::Raspao:
                // Tres riscos azul-claros, na borda do aluno, com um contorno escuro que os separa do
                // desenho dele: passou perto.
                raios(f.onde, r * 0.25f, r, 3, giro, SDL_Color{20, 22, 34, alfa(0.6f * b)});
                raios(f.onde + Vector2(1.0f, 0.0f), r * 0.25f, r, 3, giro, SDL_Color{20, 22, 34, alfa(0.6f * b)});
                raios(f.onde + Vector2(0.0f, -1.0f), r * 0.3f, r, 3, giro, SDL_Color{170, 225, 255, alfa(b)});
                break;
        }
    }

    for (const Onda& o : mOndas) {
        const float raio = PontoExtra::RaioDaOnda(o.t);
        const float some = 1.0f - o.t / PontoExtra::kDuracaoDaOnda;
        const int lados = std::clamp(static_cast<int>(raio / 6.0f), 24, 160);
        for (int k = 0; k < 3; ++k) {
            const float rk = raio - static_cast<float>(k) * 7.0f;
            if (rk <= 0.0f) continue;
            const SDL_Color cor{255, static_cast<Uint8>(225 - 25 * k), static_cast<Uint8>(120 - 30 * k),
                                alfa(some * (1.0f - 0.3f * static_cast<float>(k)))};
            for (const auto& [a, b] : PontoExtra::Anel(o.centro, rk, lados)) mLinhas->Acrescentar(a, b, cor);
        }
    }

    // As moedas voam ate a ponta da barra da nota, num arco: o ganho tem de onde vir.
    for (const Moeda& m : mMoedas) {
        const float u = std::clamp(m.t / kVooDaMoeda, 0.0f, 1.0f);
        const float e = u * u;                                  // devagar, depois depressa
        const Vector2 meio = (m.de + pontaDaBarra) * 0.5f + Vector2(0.0f, -140.0f);
        const Vector2 p = m.de * ((1.0f - e) * (1.0f - e)) + meio * (2.0f * e * (1.0f - e)) + pontaDaBarra * (e * e);
        const SDL_Color ouro{255, 215, 80, alfa(1.0f - 0.3f * u)};
        mLinhas->Acrescentar(p + Vector2(-4.0f, 0.0f), p + Vector2(0.0f, -4.0f), ouro);
        mLinhas->Acrescentar(p + Vector2(0.0f, -4.0f), p + Vector2(4.0f, 0.0f), ouro);
        mLinhas->Acrescentar(p + Vector2(4.0f, 0.0f), p + Vector2(0.0f, 4.0f), ouro);
        mLinhas->Acrescentar(p + Vector2(0.0f, 4.0f), p + Vector2(-4.0f, 0.0f), ouro);
    }
}
