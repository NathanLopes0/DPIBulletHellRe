//
// O que se desenha por cima dos alunos.
//

#include "DesenhoSobreOsAlunos.h"

#include <algorithm>
#include <memory>

#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Actors/Player/Player.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../Interface/Comunicacao.h"
#include "../../Interface/Hud.h"
#include "../../Regras/PontoExtra.h"

namespace {

/// O centro do colisor do aluno, ou a posicao dele se nao houver colisor.
Vector2 CentroDe(const Player* p) {
    const auto* c = p->GetComponent<CircleColliderComponent>();
    return c ? c->GetCenter() : p->GetPosition();
}

} // namespace

DesenhoSobreOsAlunos::DesenhoSobreOsAlunos(Scene& cena) : mCena(cena) {}

void DesenhoSobreOsAlunos::Feixe(const Player* de, const Player* ate) {
    mFeixes.push_back(FeixeEmCampo{de, ate, 0.0f});
}

void DesenhoSobreOsAlunos::Sinal(const Player* quem, const std::size_t cor) {
    mSinais.push_back(SinalEmCampo{quem, cor, 0.0f});
}

void DesenhoSobreOsAlunos::Valeu(const Player* coberto, const std::size_t cor) {
    mValeus.push_back(ValeuEmCampo{coberto, cor, 0.0f});
}

void DesenhoSobreOsAlunos::Atualizar(const float deltaTime, const std::vector<Player*>& jogadores, const int foco,
                                     const bool hitbox) {
    if (!mLinhas) {
        if (jogadores.size() < 2 && !hitbox) return;   // so a dupla tem feixe e escudo
        auto ator = std::make_unique<Actor>(&mCena);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdem);
        mCena.AddActor(std::move(ator));
    }
    mLinhas->Limpar();

    // Os feixes: a cabeca corre ate o parceiro, e depois a linha se apaga.
    for (FeixeEmCampo& f : mFeixes) f.t += deltaTime;
    mFeixes.erase(std::remove_if(mFeixes.begin(), mFeixes.end(),
        [](const FeixeEmCampo& f) { return !PontoExtra::FeixeAceso(f.t); }), mFeixes.end());
    for (const FeixeEmCampo& f : mFeixes) {
        if (!f.de || !f.ate) continue;
        const Vector2 de = CentroDe(f.de), ate = CentroDe(f.ate);
        const PontoExtra::Feixe feixe = PontoExtra::FeixeEm(de, ate, f.t);
        const auto alfa = static_cast<Uint8>(110.0f * feixe.rastro);
        if (alfa > 0) mLinhas->Acrescentar(de, ate, SDL_Color{150, 210, 255, alfa});
        if (feixe.cabeca) {
            // A cabeca tem corpo: tres tracos lado a lado, o do meio mais claro.
            Vector2 lado(ate.y - de.y, de.x - ate.x);
            if (lado.LengthSq() > 0.0f) lado.Normalize();
            mLinhas->Acrescentar(feixe.cabecaDe + lado * 2.0f, feixe.cabecaAte + lado * 2.0f, SDL_Color{150, 210, 255, 200});
            mLinhas->Acrescentar(feixe.cabecaDe - lado * 2.0f, feixe.cabecaAte - lado * 2.0f, SDL_Color{150, 210, 255, 200});
            mLinhas->Acrescentar(feixe.cabecaDe, feixe.cabecaAte, SDL_Color{235, 250, 255, 255});
        }
    }

    mPulso += deltaTime;
    for (const Player* p : jogadores) {
        if (!p || p->TempoDeImunidade() <= 0.0f) continue;
        const float pulso = 0.5f + 0.5f * Math::Sin(mPulso * (p->TempoDeImunidade() < 0.8f ? 22.0f : 8.0f));
        const auto alfa = static_cast<Uint8>(120.0f + 120.0f * pulso);
        // Dois aneis, o de dentro mais apagado: a borda do escudo tem espessura.
        for (const auto& [a, b] : PontoExtra::Anel(CentroDe(p), PontoExtra::kRaioDoEscudo)) {
            mLinhas->Acrescentar(a, b, SDL_Color{120, 190, 255, alfa});
        }
        for (const auto& [a, b] : PontoExtra::Anel(CentroDe(p), PontoExtra::kRaioDoEscudo - 3.0f)) {
            mLinhas->Acrescentar(a, b, SDL_Color{120, 190, 255, static_cast<Uint8>(alfa / 2)});
        }
    }

    DesenharComunicacao(deltaTime);

    // A mira presa (Papeis.h): um "v" vermelho em cima de quem o professor esta olhando. So quando ela
    // esta presa - enquanto os dois revezam ele piscaria a cada disparo, e nao diria nada.
    if (foco >= 0 && static_cast<size_t>(foco) < jogadores.size() && jogadores[static_cast<size_t>(foco)]) {
        const Vector2 c = CentroDe(jogadores[static_cast<size_t>(foco)]) + Vector2(0.0f, -kAlturaDaMira);
        const float pulso = 0.5f + 0.5f * Math::Sin(mPulso * 6.0f);
        const auto alfa = static_cast<Uint8>(170.0f + 85.0f * pulso);
        const auto v = [&](const float dy, const float abre, const SDL_Color cor) {
            mLinhas->Acrescentar(c + Vector2(-abre, dy - abre * 0.8f), c + Vector2(0.0f, dy), cor);
            mLinhas->Acrescentar(c + Vector2(abre, dy - abre * 0.8f), c + Vector2(0.0f, dy), cor);
        };
        for (float dy = -1.0f; dy <= 3.0f; dy += 1.0f) v(dy, 9.0f, SDL_Color{20, 22, 34, static_cast<Uint8>(alfa * 0.7f)});
        for (float dy = 0.0f; dy <= 2.0f; dy += 1.0f) v(dy, 7.0f, SDL_Color{255, 80, 70, alfa});
    }

    // A hitbox (opcao da tela de Opcoes): o circulo de colisao de verdade, e um ponto no meio.
    if (!hitbox) return;
    for (const Player* p : jogadores) {
        const auto* c = p ? p->GetComponent<CircleColliderComponent>() : nullptr;
        if (!c) continue;
        const SDL_Color vermelho{255, 70, 70, 230};
        for (const auto& [a, b] : PontoExtra::Anel(c->GetCenter(), c->GetRadius(), 16)) {
            mLinhas->Acrescentar(a, b, vermelho);
        }
        const Vector2 m = c->GetCenter();
        mLinhas->Acrescentar(m - Vector2(2.0f, 0.0f), m + Vector2(2.0f, 0.0f), SDL_Color{255, 255, 255, 255});
    }
}

void DesenhoSobreOsAlunos::DesenharComunicacao(const float deltaTime) {
    const auto corDe = [](const size_t quem, const float alfa) {
        const Hud::Rgb c = Hud::CorDoAluno(quem);
        return SDL_Color{c.r, c.g, c.b, static_cast<Uint8>(255.0f * alfa)};
    };

    // O sinal: um anel que abre em volta de quem chamou, e um segundo, mais apagado, logo dentro.
    for (SinalEmCampo& s : mSinais) s.t += deltaTime;
    mSinais.erase(std::remove_if(mSinais.begin(), mSinais.end(),
        [](const SinalEmCampo& s) { return Comunicacao::AlfaDoSinal(s.t) <= 0.0f; }), mSinais.end());
    for (const SinalEmCampo& s : mSinais) {
        if (!s.quem) continue;
        const float raio = Comunicacao::RaioDoSinal(s.t);
        const float alfa = Comunicacao::AlfaDoSinal(s.t);
        for (const auto& [a, b] : PontoExtra::Anel(CentroDe(s.quem), raio, 28)) {
            mLinhas->Acrescentar(a, b, corDe(s.cor, alfa));
        }
        for (const auto& [a, b] : PontoExtra::Anel(CentroDe(s.quem), raio - 5.0f, 28)) {
            mLinhas->Acrescentar(a, b, corDe(s.cor, alfa * 0.5f));
        }
    }

    // O valeu: o coracao sobe da cabeca de quem foi coberto, na cor de quem cobriu, e apaga.
    for (ValeuEmCampo& v : mValeus) v.t += deltaTime;
    mValeus.erase(std::remove_if(mValeus.begin(), mValeus.end(),
        [](const ValeuEmCampo& v) { return Comunicacao::AlfaDoValeu(v.t) <= 0.0f; }), mValeus.end());
    constexpr float p = static_cast<float>(Comunicacao::kPixelDoCoracao);
    for (const ValeuEmCampo& v : mValeus) {
        if (!v.coberto) continue;
        const Vector2 meio = CentroDe(v.coberto) -
                             Vector2(0.0f, Comunicacao::kAlturaDoValeu + Comunicacao::SubidaDoValeu(v.t));
        const Vector2 canto = meio - Vector2(p * Comunicacao::kLarguraDoCoracao, p * Comunicacao::kAlturaDoCoracao) * 0.5f;
        const SDL_Color cor = corDe(v.cor, Comunicacao::AlfaDoValeu(v.t));
        for (const Comunicacao::Faixa& f : Comunicacao::Coracao()) {
            for (int dy = 0; dy < Comunicacao::kPixelDoCoracao; ++dy) {
                const float y = canto.y + p * static_cast<float>(f.linha) + static_cast<float>(dy);
                mLinhas->Acrescentar(Vector2(canto.x + p * static_cast<float>(f.de), y),
                                     Vector2(canto.x + p * static_cast<float>(f.ate + 1) - 1.0f, y),
                                     cor);
            }
        }
    }
}
