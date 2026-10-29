//
// A fase 2 do Lucas: o DevTools aberto, e o CSS mudando ao vivo.
//

#include "Estilo.h"

#include <cmath>

namespace Estilo {

    namespace {
        std::string Px(const float v) { return std::to_string(static_cast<int>(std::lround(v))) + "px"; }

        std::string Numero(const float v) {
            // 1.6 e nao 1.600000: o placar e CSS.
            const int dez = static_cast<int>(std::lround(v * 10.0f));
            return std::to_string(dez / 10) + (dez % 10 ? "." + std::to_string(dez % 10) : "");
        }

        std::string Transform(const Css& css) {
            return "transform: rotate(" + std::to_string(static_cast<int>(std::lround(css.rotacao))) + "deg) scale(" +
                   Numero(css.escala) + ");";
        }
    }

    Regra Proxima(Variacao::Sorteio& s, Css& css, const Tipo anterior) {
        std::vector<Tipo> tipos;
        for (const Tipo t : {Tipo::Gap, Tipo::Justificar, Tipo::Direcao, Tipo::Girar, Tipo::Escalar, Tipo::Esconder, Tipo::Animar,
                             Tipo::Fixar, Tipo::Hover}) {
            if (t != anterior) tipos.push_back(t);
        }
        // A GRADE TORTA OU GRANDE TENDE A VOLTAR: o tipo que a desfaz entra de novo
        // no sorteio, e sai com o dobro da chance.
        if (css.rotacao != 0.0f && anterior != Tipo::Girar) tipos.push_back(Tipo::Girar);
        if (css.escala != 1.0f && anterior != Tipo::Escalar) tipos.push_back(Tipo::Escalar);

        Regra r;
        r.tipo = s.Um(tipos);
        r.seletor = ".grade";
        switch (r.tipo) {
            case Tipo::Gap:
                css.gap = css.gap == kGapEstreito ? kGapLargo : kGapEstreito;
                r.declaracao = "gap: " + Px(css.gap) + ";";
                break;
            case Tipo::Justificar:
                css.justificar = Flex::Proxima(s, css.justificar);
                r.declaracao = "justify-content: " + Flex::Nome(css.justificar) + ";";
                break;
            case Tipo::Direcao:
                css.coluna = !css.coluna;
                r.declaracao = std::string("flex-direction: ") + (css.coluna ? "column" : "row") + ";";
                break;
            case Tipo::Girar: {
                static const std::vector<float> kAngulos = {30.0f, 45.0f, 90.0f, -45.0f};
                css.rotacao = css.rotacao != 0.0f ? 0.0f : s.Um(kAngulos);
                r.declaracao = Transform(css);
                break;
            }
            case Tipo::Escalar:
                css.escala = css.escala == 1.0f ? kEscalaGrande : 1.0f;
                r.declaracao = Transform(css);
                break;
            case Tipo::Esconder: {
                if (css.linhaOculta >= 0) {
                    r.seletor = ".linha:nth-child(" + std::to_string(css.linhaOculta + 1) + ")";
                    r.declaracao = "display: flex;";
                    css.linhaOculta = -1;
                } else {
                    css.linhaOculta = s.Inteiro(0, kLinhas - 1);
                    r.seletor = ".linha:nth-child(" + std::to_string(css.linhaOculta + 1) + ")";
                    r.declaracao = "display: none;";
                }
                break;
            }
            case Tipo::Animar:
                r.declaracao = "animation: spin 2s linear;";
                break;
            case Tipo::Fixar:
                r.declaracao = "position: fixed; bottom: 0;";
                break;
            case Tipo::Hover:
                r.seletor = ".card:hover";
                r.declaracao = "transform: translateY(" + Px(kDescidaDoHover) + ");";
                break;
        }
        return r;
    }

    bool Repinta(const Tipo t) {
        return t != Tipo::Hover && t != Tipo::Fixar;
    }

    std::string Placar(const Regra& r) {
        return r.seletor + " {\n" + r.declaracao + " }";
    }

    std::array<Ponto, kCartoes> Posicoes(const Css& css, const Ponto centro, const float giro) {
        std::array<Ponto, kCartoes> p{};
        const float a = (css.rotacao + giro) * 3.14159265f / 180.0f;
        const float c = std::cos(a), s = std::sin(a);
        // TRES LINHAS FLEX DE SEIS ITENS, sempre. O justify-content poe os itens
        // de cada linha no conteiner, no eixo principal, como o CSS poe
        // (Flex::Posicoes), cada um do tamanho do gap; as linhas se empilham no
        // eixo cruzado. O flex-direction: column so troca os eixos: o principal
        // fica vertical, e a grade deita.
        const auto principal = Flex::Posicoes(css.justificar, kColunas, css.gap, -kConteiner / 2.0f, kConteiner / 2.0f);
        for (int i = 0; i < kCartoes; ++i) {
            const int linha = i / kColunas, item = i % kColunas;
            const float noEixo = principal[static_cast<size_t>(item)];
            const float cruzado = (static_cast<float>(linha) - static_cast<float>(kLinhas - 1) / 2.0f) * css.gap;
            const float x = (css.coluna ? cruzado : noEixo) * css.escala;
            const float y = (css.coluna ? noEixo : cruzado) * css.escala;
            p[static_cast<size_t>(i)] = {centro.x + x * c - y * s, centro.y + x * s + y * c};
        }
        return p;
    }

    bool Oculto(const Css& css, const int i) {
        return css.linhaOculta >= 0 && i / kColunas == css.linhaOculta;
    }

    Ponto Espalhado(const int i, const float largura, const float y) {
        const float margem = 60.0f;
        return {margem + (static_cast<float>(i) + 0.5f) * (largura - 2.0f * margem) / kCartoes, y};
    }

    float XDaGrade(const float xDoProfessor, const float largura) {
        return largura - xDoProfessor;
    }

    float Hover(const float t) {
        if (t <= 0.0f || t >= kDuracaoDoHover) return 0.0f;
        return kDescidaDoHover * std::sin(3.14159265f * t / kDuracaoDoHover);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.porRegra = Dificuldade::Escolher(n, kPorRegra, 1.6f);
        d.letras = Dificuldade::Escolher(n, kLetrasDaCascata, 6);
        return d;
    }
}
