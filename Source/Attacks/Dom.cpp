//
// As fases 1 e 2 do Lucas: a <div> que as regras CSS e JavaScript mudam.
//

#include "Dom.h"

#include <algorithm>
#include <cmath>

namespace Dom {

    namespace {
        Vector2 Clamp(const Vector2 p, const Campo& c, const float mx, const float my) {
            return Vector2(std::clamp(p.x, c.x + mx, std::max(c.x + mx, c.x + c.w - mx)),
                           std::clamp(p.y, c.y + my, std::max(c.y + my, c.y + c.h - my)));
        }

        /// A meia largura e a meia altura do que a <div> ocupa inteira (a grade inclusive).
        Vector2 Meias(const Div& d) {
            return Vector2(d.largura / 2.0f, d.altura / 2.0f);
        }
    }

    Div Nova(const Campo& c, const float rapidez, Variacao::Sorteio& s) {
        Div d;
        d.centro = Vector2(c.x + c.w * s.Real(0.3f, 0.7f), c.y + c.h * 0.35f);
        const float a = Math::ToRadians(s.Chance(0.5f) ? s.Real(20.0f, 60.0f) : s.Real(120.0f, 160.0f));
        d.velocidade = Vector2(Math::Cos(a), Math::Sin(a)) * rapidez;
        return d;
    }

    bool Visivel(const Div& d, const float agora) {
        return agora >= d.escondidaAte;
    }

    std::vector<Caixa> Caixas(const Div& d, const float agora) {
        std::vector<Caixa> v;
        if (!Visivel(d, agora)) return v;
        if (!d.grade) {
            v.push_back({d.centro, d.largura, d.altura, d.redonda});
            return v;
        }
        // display: grid - quatro caixas, com o vao no meio.
        const float w = std::max(kMenorLado, (d.largura - kVaoDaGrade) / 2.0f);
        const float h = std::max(kMenorLado, (d.altura - kVaoDaGrade) / 2.0f);
        const float dx = (w + kVaoDaGrade) / 2.0f, dy = (h + kVaoDaGrade) / 2.0f;
        for (const float sx : {-1.0f, 1.0f}) {
            for (const float sy : {-1.0f, 1.0f}) v.push_back({d.centro + Vector2(sx * dx, sy * dy), w, h, d.redonda});
        }
        return v;
    }

    bool Toca(const Div& d, const Vector2 p, const float raio, const float agora) {
        for (const Caixa& k : Caixas(d, agora)) {
            if (k.redonda) {
                const float r = std::min(k.largura, k.altura) / 2.0f;
                if ((p - k.centro).Length() <= r + raio) return true;
                continue;
            }
            const float mx = std::max(0.0f, std::fabs(p.x - k.centro.x) - k.largura / 2.0f);
            const float my = std::max(0.0f, std::fabs(p.y - k.centro.y) - k.altura / 2.0f);
            if (mx * mx + my * my <= raio * raio) return true;
        }
        return false;
    }

    std::vector<Vector2> Pecas(const Div& d, const float agora, const float passo) {
        std::vector<Vector2> v;
        const float p = std::max(4.0f, passo);
        for (const Caixa& k : Caixas(d, agora)) {
            const int nx = std::max(1, static_cast<int>(std::round(k.largura / p)));
            const int ny = std::max(1, static_cast<int>(std::round(k.altura / p)));
            const float r = std::min(k.largura, k.altura) / 2.0f;
            for (int i = 0; i < nx; ++i) {
                for (int j = 0; j < ny; ++j) {
                    const Vector2 q = k.centro + Vector2(((static_cast<float>(i) + 0.5f) / static_cast<float>(nx) - 0.5f) * k.largura,
                                                         ((static_cast<float>(j) + 0.5f) / static_cast<float>(ny) - 0.5f) * k.altura);
                    if (k.redonda && (q - k.centro).Length() > r) continue;
                    v.push_back(q);
                }
            }
            if (k.redonda && v.empty()) v.push_back(k.centro);
        }
        return v;
    }

    void Andar(Div& d, const float dt, const float agora, const Vector2 aluno, const Campo& c, Variacao::Sorteio& s) {
        const Vector2 meia = Meias(d);
        // Math.random(): salta para um lugar sorteado, longe das bordas.
        if (d.teleporteACada > 0.0f && agora >= d.proximoTeleporte) {
            d.proximoTeleporte = agora + d.teleporteACada;
            d.centro = Clamp(Vector2(c.x + c.w * s.Real(0.15f, 0.85f), c.y + c.h * s.Real(0.2f, 0.75f)), c, meia.x, meia.y);
        }
        if (agora < d.paradaAte) return;   // position: fixed
        const float rapidez = std::max(1.0f, d.velocidade.Length());
        // mousemove: a direcao vira para o aluno, devagar - da para fugir dela.
        if (d.segue) {
            Vector2 para = aluno - d.centro;
            if (para.Length() > 1.0f) {
                para = para * (1.0f / para.Length());
                const Vector2 agoraV = d.velocidade * (1.0f / rapidez);
                const float u = std::clamp(dt * 1.8f, 0.0f, 1.0f);
                Vector2 nova = agoraV + (para - agoraV) * u;
                if (nova.Length() > 1e-3f) d.velocidade = nova * (rapidez / nova.Length());
            }
        }
        d.centro = d.centro + d.velocidade * dt;
        // Quica nas bordas do campo.
        if (d.centro.x - meia.x < c.x) { d.centro.x = c.x + meia.x; d.velocidade.x = std::fabs(d.velocidade.x); }
        if (d.centro.x + meia.x > c.x + c.w) { d.centro.x = c.x + c.w - meia.x; d.velocidade.x = -std::fabs(d.velocidade.x); }
        if (d.centro.y - meia.y < c.y) { d.centro.y = c.y + meia.y; d.velocidade.y = std::fabs(d.velocidade.y); }
        if (d.centro.y + meia.y > c.y + c.h) { d.centro.y = c.y + c.h - meia.y; d.velocidade.y = -std::fabs(d.velocidade.y); }
    }

    bool EhJs(const Efeito e) {
        return static_cast<int>(e) >= static_cast<int>(Efeito::Intervalo);
    }

    bool Ajuda(const Efeito e) {
        return e == Efeito::Some || e == Efeito::Estreita || e == Efeito::Fixa || e == Efeito::Remove;
    }

    const char* Texto(const Efeito e) {
        switch (e) {
            case Efeito::Largura:   return "width: 200%;";
            case Efeito::Altura:    return "height: 200%;";
            case Efeito::Padding:   return "padding: 40px;";
            case Efeito::Redonda:   return "border-radius: 50%;";
            case Efeito::Rapida:    return "transition: 0.1s;";
            case Efeito::Some:      return "display: none;";
            case Efeito::Estreita:  return "width: 50%;";
            case Efeito::Grade:     return "display: grid;";
            case Efeito::Opaca:     return "opacity: 0.4;";
            case Efeito::Fixa:      return "position: fixed;";
            case Efeito::Intervalo: return "setInterval(atirar, 1500);";
            case Efeito::Revide:    return "div.onclick = revidar;";
            case Efeito::Segue:     return "on(\"mousemove\", seguir);";
            case Efeito::Aleatoria: return "div.x = Math.random();";
            case Efeito::Bomba:     return "setTimeout(explodir, 3000);";
            case Efeito::Remove:    return "div.remove();";
            case Efeito::Clone:     return "div.cloneNode(true);";
            case Efeito::Acelera:   return "requestAnimationFrame(acelerar);";
        }
        return "";
    }

    Efeito Sortear(const bool js, Variacao::Sorteio& s) {
        std::vector<Efeito> bolsa;
        const auto encher = [&](const Efeito* de, const Efeito* ate) {
            for (const Efeito* e = de; e != ate; ++e) {
                // As que aliviam entram uma vez; as outras, duas.
                bolsa.push_back(*e);
                if (!Ajuda(*e)) bolsa.push_back(*e);
            }
        };
        if (js) encher(std::begin(kDoJs), std::end(kDoJs));
        else encher(std::begin(kDoCss), std::end(kDoCss));
        return bolsa[static_cast<size_t>(s.Inteiro(0, static_cast<int>(bolsa.size()) - 1))];
    }

    bool Aplicar(Div& d, const Efeito e, const float agora) {
        ++d.mudancas;
        const float rapidez = d.velocidade.Length();
        const auto mudarRapidez = [&](const float fator) {
            if (rapidez < 1e-3f) return;
            const float nova = std::min(kMaiorRapidez, rapidez * fator);
            d.velocidade = d.velocidade * (nova / rapidez);
        };
        switch (e) {
            case Efeito::Largura:  d.largura = std::min(kMaiorLargura, d.largura * 1.7f); break;
            case Efeito::Altura:   d.altura = std::min(kMaiorAltura, d.altura * 1.7f); break;
            case Efeito::Padding:
                d.largura = std::min(kMaiorLargura, d.largura + 36.0f);
                d.altura = std::min(kMaiorAltura, d.altura + 36.0f);
                break;
            case Efeito::Redonda:  d.redonda = true; break;
            case Efeito::Rapida:   mudarRapidez(1.5f); break;
            case Efeito::Some:     d.escondidaAte = std::max(d.escondidaAte, agora + kSome); break;
            case Efeito::Estreita:
                d.largura = std::max(kMenorLado, d.largura * 0.6f);
                d.altura = std::max(kMenorLado, d.altura * 0.6f);
                break;
            case Efeito::Grade:
                if (d.grade) d.largura = std::min(kMaiorLargura, d.largura * 1.3f);
                d.grade = true;
                break;
            case Efeito::Opaca:    d.opacidade = std::max(0.25f, d.opacidade * 0.55f); break;
            case Efeito::Fixa:     d.paradaAte = std::max(d.paradaAte, agora + kFixa); break;
            case Efeito::Intervalo:
                // Mais um setInterval: o ritmo aperta.
                d.intervalo = d.intervalo > 0.0f ? std::max(0.6f, d.intervalo * 0.7f) : 1.5f;
                d.proximoTiro = agora + 0.8f;
                break;
            case Efeito::Revide:   d.revida = true; break;
            case Efeito::Segue:    d.segue = true; break;
            case Efeito::Aleatoria:
                d.teleporteACada = d.teleporteACada > 0.0f ? std::max(1.2f, d.teleporteACada * 0.7f) : 2.5f;
                d.proximoTeleporte = agora + 1.0f;
                break;
            case Efeito::Bomba:    if (d.bombaEm < agora) d.bombaEm = agora + kPavio; break;
            case Efeito::Remove:   d.escondidaAte = std::max(d.escondidaAte, agora + kRemovida); break;
            case Efeito::Clone:    return true;
            case Efeito::Acelera:  mudarRapidez(1.35f); break;
        }
        return false;
    }

    Disparo Atirar(Div& d, const float agora) {
        if (!Visivel(d, agora)) return Disparo::Nenhum;
        if (d.bombaEm >= 0.0f && agora >= d.bombaEm) {
            d.bombaEm = -1.0f;
            return Disparo::Explosao;
        }
        if (d.intervalo > 0.0f && agora >= d.proximoTiro) {
            d.proximoTiro = agora + d.intervalo;
            return Disparo::Anel;
        }
        return Disparo::Nenhum;
    }

    bool Revidar(Div& d, const float agora) {
        if (!d.revida || !Visivel(d, agora) || agora < d.proximoRevide) return false;
        d.proximoRevide = agora + kEntreRevides;
        return true;
    }

    Vector2 Destino(const Campo& c, const Div* alvo, Variacao::Sorteio& s) {
        if (alvo && s.Chance(kChanceDeMirarADiv)) return alvo->centro;
        return Vector2(c.x + c.w * s.Real(0.08f, 0.92f), c.y + c.h * s.Real(0.45f, 0.95f));
    }

    Vector2 Curvar(const Vector2 velocidade, const Vector2 de, const Vector2 para, const float giroPorSegundo, const float dt) {
        const float rapidez = velocidade.Length();
        const Vector2 d = para - de;
        if (rapidez < 1e-3f || d.Length() < 1e-3f) return velocidade;
        const float atual = std::atan2(velocidade.y, velocidade.x);
        float falta = std::atan2(d.y, d.x) - atual;
        while (falta > Math::Pi) falta -= Math::TwoPi;
        while (falta < -Math::Pi) falta += Math::TwoPi;
        const float passo = Math::ToRadians(std::max(0.0f, giroPorSegundo)) * std::max(0.0f, dt);
        const float novo = atual + std::clamp(falta, -passo, passo);
        return Vector2(std::cos(novo), std::sin(novo)) * rapidez;
    }

    Degrau DegrauDo(const Dificuldade::Nivel nivel) {
        Degrau g;
        if (nivel == Dificuldade::Nivel::Dificil) {
            g.calma = 2.5f;
            g.entreLances = 1.5f;
            g.regrasPorLance = 3;
            g.velocidadeDasRegras = 210.0f;
            g.rapidezDaDiv = 140.0f;
            g.tirosDoAnel = 12;
            g.tirosDaExplosao = 22;
            g.tirosDoRevide = 5;
            g.entreTags = 1.9f;
            g.tagsPorLeque = 7;
        }
        return g;
    }
}
