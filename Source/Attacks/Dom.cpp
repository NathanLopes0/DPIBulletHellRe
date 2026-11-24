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

        /// A meia largura e a meia altura do maior tamanho que a <div> chega a ter (o pulso inteiro,
        /// a grade inclusive): e com elas que ela quica, e nunca passa da borda.
        Vector2 Meias(const Div& d) {
            const float k = d.pulsa ? 1.0f + kPulso : 1.0f;
            return Vector2(d.largura * k / 2.0f, d.altura * k / 2.0f);
        }

        /// A faixa do campo em que ela anda: a de cima, ou o campo inteiro com o z-index.
        Campo Faixa(const Div& d, const Campo& c) {
            Campo f = c;
            if (!d.desce) f.h = c.h * kFaixaDaDiv;
            // Nunca menor que a propria <div>.
            f.h = std::max(f.h, std::min(c.h, Meias(d).y * 2.0f + 2.0f));
            return f;
        }

        void MudarRapidez(Div& d, const float fator) {
            const float rapidez = d.velocidade.Length();
            if (rapidez < 1e-3f) return;
            d.velocidade = d.velocidade * (std::min(kMaiorRapidez, rapidez * fator) / rapidez);
        }

        constexpr float kMenorIntervalo = 1.0f;
        constexpr float kMenorLeque = 1.2f;
        constexpr float kMenorTeleporte = 1.2f;
    }

    Div Nova(const Campo& c, const float rapidez, const float entreTransbordos, const float agora, Variacao::Sorteio& s) {
        Div d;
        d.centro = Vector2(c.x + c.w * s.Real(0.3f, 0.7f), c.y + c.h * 0.25f);
        const float a = Math::ToRadians(s.Chance(0.5f) ? s.Real(20.0f, 60.0f) : s.Real(120.0f, 160.0f));
        d.velocidade = Vector2(Math::Cos(a), Math::Sin(a)) * rapidez;
        d.entreTransbordos = std::max(kMenorTransbordo, entreTransbordos);
        d.proximoTransbordo = agora + d.entreTransbordos;
        return d;
    }

    float Escala(const Div& d, const float agora) {
        if (!d.pulsa) return 1.0f;
        return 1.0f + kPulso * 0.5f * (1.0f + std::sin(Math::TwoPi * agora / kPeriodoDoPulso));
    }

    std::vector<Caixa> Caixas(const Div& d, const float agora) {
        std::vector<Caixa> v;
        const float k = Escala(d, agora);
        const float largura = d.largura * k, altura = d.altura * k;
        if (!d.grade) {
            v.push_back({d.centro, largura, altura, d.redonda});
            return v;
        }
        // display: grid - quatro caixas, com o vao no meio.
        const float w = std::max(12.0f, (largura - kVaoDaGrade) / 2.0f);
        const float h = std::max(12.0f, (altura - kVaoDaGrade) / 2.0f);
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
            const size_t antes = v.size();
            for (int i = 0; i < nx; ++i) {
                for (int j = 0; j < ny; ++j) {
                    const Vector2 q = k.centro + Vector2(((static_cast<float>(i) + 0.5f) / static_cast<float>(nx) - 0.5f) * k.largura,
                                                         ((static_cast<float>(j) + 0.5f) / static_cast<float>(ny) - 0.5f) * k.altura);
                    if (k.redonda && (q - k.centro).Length() > r) continue;
                    v.push_back(q);
                }
            }
            if (v.size() == antes) v.push_back(k.centro);
        }
        return v;
    }

    void Andar(Div& d, const float dt, const float agora, const Vector2 aluno, const Campo& c, Variacao::Sorteio& s) {
        const Vector2 meia = Meias(d);
        const Campo f = Faixa(d, c);
        // Math.random(): salta para um lugar sorteado da faixa dela.
        if (d.teleporteACada > 0.0f && agora >= d.proximoTeleporte) {
            d.proximoTeleporte = agora + d.teleporteACada;
            d.centro = Clamp(Vector2(f.x + f.w * s.Real(0.15f, 0.85f), f.y + f.h * s.Real(0.2f, 0.8f)), f, meia.x, meia.y);
        }
        const float rapidez = std::max(1.0f, d.velocidade.Length());
        const bool recuando = agora < d.recuoAte;
        // mousemove: a direcao vira para o aluno, devagar - da para fugir dela.
        if (d.segue && !recuando) {
            Vector2 para = aluno - d.centro;
            if (para.Length() > 1.0f) {
                para = para * (1.0f / para.Length());
                const Vector2 agoraV = d.velocidade * (1.0f / rapidez);
                const float u = std::clamp(dt * 1.1f, 0.0f, 1.0f);
                Vector2 nova = agoraV + (para - agoraV) * u;
                if (nova.Length() > 1e-3f) d.velocidade = nova * (rapidez / nova.Length());
            }
        }
        // margin: 0 auto: na horizontal, ela vai para cima do aluno (na rapidez dela); na vertical,
        // segue quicando.
        Vector2 passo = d.velocidade * dt;
        if (d.centraliza && !recuando) passo.x = std::clamp(aluno.x - d.centro.x, -rapidez * dt, rapidez * dt);
        d.centro = d.centro + passo;
        // Quica nas bordas da faixa.
        if (d.centro.x - meia.x < f.x) { d.centro.x = f.x + meia.x; d.velocidade.x = std::fabs(d.velocidade.x); }
        if (d.centro.x + meia.x > f.x + f.w) { d.centro.x = f.x + f.w - meia.x; d.velocidade.x = -std::fabs(d.velocidade.x); }
        if (d.centro.y - meia.y < f.y) { d.centro.y = f.y + meia.y; d.velocidade.y = std::fabs(d.velocidade.y); }
        if (d.centro.y + meia.y > f.y + f.h) { d.centro.y = f.y + f.h - meia.y; d.velocidade.y = -std::fabs(d.velocidade.y); }
        d.centro = Clamp(d.centro, f, meia.x, meia.y);
    }

    void Recuar(Div& d, const Vector2 aluno, const float agora) {
        if (agora < d.recuoAte) return;
        d.recuoAte = agora + kRecuo;
        Vector2 fora = d.centro - aluno;
        if (fora.Length() < 1.0f) fora = Vector2(0.0f, -1.0f);
        // Sempre um pouco para cima: a <div> volta para a faixa dela.
        fora = fora * (1.0f / fora.Length()) + Vector2(0.0f, -0.6f);
        d.velocidade = fora * (std::max(1.0f, d.velocidade.Length()) / fora.Length());
    }

    bool EhJs(const Efeito e) {
        return static_cast<int>(e) >= static_cast<int>(Efeito::Intervalo);
    }

    const char* Texto(const Efeito e) {
        switch (e) {
            case Efeito::Largura:    return "width: 200%;";
            case Efeito::Altura:     return "height: 200%;";
            case Efeito::Padding:    return "padding: 40px;";
            case Efeito::Redonda:    return "border-radius: 50%;";
            case Efeito::Rapida:     return "transition: 0.1s;";
            case Efeito::Grade:      return "display: grid;";
            case Efeito::Pulsa:      return "animation: pulsar 1s infinite;";
            case Efeito::Centraliza: return "margin: 0 auto;";
            case Efeito::Transborda: return "overflow: scroll;";
            case Efeito::Desce:      return "z-index: 9999;";
            case Efeito::Intervalo:  return "setInterval(atirar, 1500);";
            case Efeito::Leque:      return "tiros.forEach(mirar);";
            case Efeito::Revide:     return "div.onclick = revidar;";
            case Efeito::Segue:      return "on(\"mousemove\", seguir);";
            case Efeito::Sobre:      return "div.onmouseover = despejar;";
            case Efeito::Aleatoria:  return "div.x = Math.random();";
            case Efeito::Bomba:      return "setTimeout(explodir, 3000);";
            case Efeito::Clone:      return "div.cloneNode(true);";
            case Efeito::Acelera:    return "requestAnimationFrame(acelerar);";
        }
        return "";
    }

    bool Inutil(const Div& d, const Efeito e) {
        const float rapidez = d.velocidade.Length();
        switch (e) {
            case Efeito::Largura:    return d.largura >= kMaiorLargura - 0.5f;
            case Efeito::Altura:     return d.altura >= kMaiorAltura - 0.5f;
            case Efeito::Padding:    return d.largura >= kMaiorLargura - 0.5f && d.altura >= kMaiorAltura - 0.5f;
            case Efeito::Redonda:    return d.redonda;
            case Efeito::Rapida:
            case Efeito::Acelera:    return rapidez >= kMaiorRapidez - 0.5f;
            case Efeito::Grade:      return d.grade && d.largura >= kMaiorLargura - 0.5f;
            case Efeito::Pulsa:      return d.pulsa;
            case Efeito::Centraliza: return d.centraliza;
            case Efeito::Transborda: return d.entreTransbordos <= kMenorTransbordo + 1e-3f;
            case Efeito::Desce:      return d.desce;
            case Efeito::Intervalo:  return d.intervalo > 0.0f && d.intervalo <= kMenorIntervalo + 1e-3f;
            case Efeito::Leque:      return d.entreLeques > 0.0f && d.entreLeques <= kMenorLeque + 1e-3f;
            case Efeito::Revide:     return d.revida;
            case Efeito::Segue:      return d.segue;
            case Efeito::Sobre:      return d.sobre;
            case Efeito::Aleatoria:  return d.teleporteACada > 0.0f && d.teleporteACada <= kMenorTeleporte + 1e-3f;
            case Efeito::Bomba:      return d.bombaEm >= 0.0f;
            case Efeito::Clone:      return false;
        }
        return false;
    }

    Efeito Sortear(const bool js, Variacao::Sorteio& s, const Div* alvo) {
        const Efeito* de = js ? std::begin(kDoJs) : std::begin(kDoCss);
        const int n = js ? static_cast<int>(std::size(kDoJs)) : static_cast<int>(std::size(kDoCss));
        Efeito e = de[s.Inteiro(0, n - 1)];
        // Umas tentativas a mais para nao jogar uma regra que nao muda nada no alvo.
        for (int k = 0; alvo && k < 6 && Inutil(*alvo, e); ++k) e = de[s.Inteiro(0, n - 1)];
        return e;
    }

    bool Aplicar(Div& d, const Efeito e, const float agora) {
        ++d.mudancas;
        switch (e) {
            case Efeito::Largura:  d.largura = std::min(kMaiorLargura, d.largura * 1.7f); break;
            case Efeito::Altura:   d.altura = std::min(kMaiorAltura, d.altura * 1.7f); break;
            case Efeito::Padding:
                d.largura = std::min(kMaiorLargura, d.largura + 36.0f);
                d.altura = std::min(kMaiorAltura, d.altura + 36.0f);
                break;
            case Efeito::Redonda:  d.redonda = true; break;
            case Efeito::Rapida:   MudarRapidez(d, 1.5f); break;
            case Efeito::Grade:
                if (d.grade) {
                    d.largura = std::min(kMaiorLargura, d.largura * 1.3f);
                    d.altura = std::min(kMaiorAltura, d.altura * 1.2f);
                }
                d.grade = true;
                break;
            case Efeito::Pulsa:      d.pulsa = true; break;
            case Efeito::Centraliza: d.centraliza = true; break;
            case Efeito::Transborda:
                d.entreTransbordos = std::max(kMenorTransbordo, d.entreTransbordos * 0.75f);
                d.proximoTransbordo = std::min(d.proximoTransbordo, agora + 0.5f);
                break;
            case Efeito::Desce:      d.desce = true; break;
            case Efeito::Intervalo:
                // Mais um setInterval: o ritmo aperta.
                d.intervalo = d.intervalo > 0.0f ? std::max(kMenorIntervalo, d.intervalo * 0.7f) : 1.5f;
                d.proximoTiro = agora + 0.8f;
                break;
            case Efeito::Leque:
                d.entreLeques = d.entreLeques > 0.0f ? std::max(kMenorLeque, d.entreLeques * 0.7f) : 2.0f;
                d.proximoLeque = agora + 0.7f;
                break;
            case Efeito::Revide:   d.revida = true; break;
            case Efeito::Segue:    d.segue = true; break;
            case Efeito::Sobre:    d.sobre = true; break;
            case Efeito::Aleatoria:
                d.teleporteACada = d.teleporteACada > 0.0f ? std::max(kMenorTeleporte, d.teleporteACada * 0.7f) : 2.5f;
                d.proximoTeleporte = agora + 1.0f;
                break;
            case Efeito::Bomba:    if (d.bombaEm < 0.0f) d.bombaEm = agora + kPavio; break;
            case Efeito::Clone:    return true;
            case Efeito::Acelera:  MudarRapidez(d, 1.35f); break;
        }
        return false;
    }

    Tiros Atirar(Div& d, const float agora, const Vector2 aluno) {
        Tiros t;
        if (agora >= d.proximoTransbordo) {
            d.proximoTransbordo = agora + d.entreTransbordos;
            t.transbordo = true;
        }
        if (d.bombaEm >= 0.0f && agora >= d.bombaEm) {
            d.bombaEm = -1.0f;
            t.explosao = true;
        }
        if (d.intervalo > 0.0f && agora >= d.proximoTiro) {
            d.proximoTiro = agora + d.intervalo;
            t.anel = true;
        }
        if (d.entreLeques > 0.0f && agora >= d.proximoLeque) {
            d.proximoLeque = agora + d.entreLeques;
            t.leque = true;
        }
        if (d.sobre && agora >= d.proximaQueda && aluno.y > d.centro.y &&
            std::fabs(aluno.x - d.centro.x) <= d.largura * Escala(d, agora) / 2.0f) {
            d.proximaQueda = agora + kEntreQuedas;
            t.queda = true;
        }
        return t;
    }

    bool Revidar(Div& d, const float agora) {
        if (!d.revida || agora < d.proximoRevide) return false;
        d.proximoRevide = agora + kEntreRevides;
        return true;
    }

    std::vector<Gota> Gotas(const Div& d, const float agora) {
        std::vector<Gota> v;
        for (const Caixa& k : Caixas(d, agora)) {
            // Na grade, as duas caixas de cima transbordam para dentro das de baixo: so as de baixo contam.
            if (d.grade && k.centro.y < d.centro.y) continue;
            if (k.redonda) {
                // A redonda transborda pela metade de baixo inteira, em leque - em cima dela, nada.
                const float r = std::min(k.largura, k.altura) / 2.0f;
                const int n = std::max(3, static_cast<int>(std::round(Math::Pi * r / kEspacoDasGotas)));
                for (int i = 0; i < n; ++i) {
                    const float a = Math::Pi * (static_cast<float>(i) + 0.5f) / static_cast<float>(n);
                    const Vector2 rumo(std::cos(a), std::sin(a));
                    v.push_back({k.centro + rumo * r, rumo});
                }
                continue;
            }
            const int n = std::max(1, static_cast<int>(std::round(k.largura / kEspacoDasGotas)));
            for (int i = 0; i < n; ++i) {
                const float x = k.centro.x + ((static_cast<float>(i) + 0.5f) / static_cast<float>(n) - 0.5f) * k.largura;
                v.push_back({Vector2(x, k.centro.y + k.altura / 2.0f), Vector2(0.0f, 1.0f)});
            }
        }
        return v;
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
            g.calma = 1.5f;
            g.entreLances = 1.8f;
            g.regrasPorLance = 3;
            g.velocidadeDasRegras = 215.0f;
            g.rapidezDaDiv = 140.0f;
            g.entreTransbordos = 2.4f;
            g.velocidadeDasGotas = 140.0f;
            g.tirosDoAnel = 12;
            g.tirosDaExplosao = 12;
            g.tirosDoRevide = 4;
            g.tirosDoLeque = 5;
            g.velocidadeDaQueda = 360.0f;
            g.entreTags = 1.9f;
            g.tagsPorLeque = 7;
        }
        return g;
    }
}
