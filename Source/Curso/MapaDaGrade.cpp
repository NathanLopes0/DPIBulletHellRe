//
// Onde fica cada coluna da grade.
//

#include "MapaDaGrade.h"

#include <algorithm>
#include <cmath>

namespace MapaDaGrade {

namespace {

    // Os lugares das obrigatorias na primeira pagina: entre as margens, um passo por intervalo.
    std::vector<float> Lugares(const int obrigatorias, const float largura) {
        const float margem = largura / 12.0f;
        const float inicio = margem + kLarguraDoBotao / 2.0f;
        const float fim = largura - (margem + kLarguraDoBotao / 2.0f);
        const float passo = obrigatorias > 1 ? (fim - inicio) / static_cast<float>(obrigatorias - 1) : 0.0f;
        std::vector<float> xs;
        for (int c = 0; c < std::max(1, obrigatorias); ++c) xs.push_back(inicio + passo * static_cast<float>(c));
        return xs;
    }

}

int PorPagina(const int obrigatorias) {
    return obrigatorias >= 2 ? obrigatorias - 1 : 1;
}

std::vector<Coluna> Colunas(const int obrigatorias, const int total, const float largura) {
    const std::vector<float> lugares = Lugares(obrigatorias, largura);
    const int porPagina = PorPagina(obrigatorias);
    const int primeiroLugar = obrigatorias >= 2 ? 1 : 0;
    std::vector<Coluna> colunas;
    for (int c = 0; c < total; ++c) {
        Coluna col;
        if (c < obrigatorias) {
            col.x = lugares[static_cast<size_t>(c)];
        } else {
            const int k = c - obrigatorias;
            col.pagina = 1 + k / porPagina;
            col.x = static_cast<float>(col.pagina) * largura +
                    lugares[static_cast<size_t>(primeiroLugar + k % porPagina)];
        }
        colunas.push_back(col);
    }
    return colunas;
}

int Paginas(const std::vector<Coluna>& colunas) {
    int maior = 0;
    for (const Coluna& c : colunas) maior = std::max(maior, c.pagina);
    return maior + 1;
}

float DeslocamentoDa(const int pagina, const float largura) {
    return -static_cast<float>(pagina) * largura;
}

float Aproximar(const float atual, const float alvo, const float deltaTime) {
    const float novo = alvo + (atual - alvo) * std::exp(-kRapidez * std::max(0.0f, deltaTime));
    return std::fabs(novo - alvo) < 0.5f ? alvo : novo;
}

}

std::vector<MapaDaGrade::Aceso> MapaDaGrade::Caminho(const std::vector<Trecho>& trechos, const int anterior,
                                                     const int novo, const float passo) {
    auto toca = [&](const int botao, const int tronco) {
        for (const Trecho& t : trechos) if (t.botao == botao && t.tronco == tronco) return true;
        return false;
    };
    // O tronco comum de maior numero: na mesma coluna, o da direita.
    int comum = -1;
    if (anterior >= 0 && anterior != novo) {
        for (const Trecho& t : trechos) {
            if (t.botao == novo && toca(anterior, t.tronco) && t.tronco > comum) comum = t.tronco;
        }
    }
    std::vector<Aceso> acesos;
    if (comum < 0) return acesos;
    float yDe = 0.0f, yPara = 0.0f;
    for (const Trecho& t : trechos) {
        if (t.tronco != comum) continue;
        if (t.botao == anterior) yDe = t.y0;
        if (t.botao == novo) yPara = t.y0;
    }
    const float baixo = yDe < yPara ? yDe : yPara;
    const float alto = yDe < yPara ? yPara : yDe;
    const float distancia = alto - baixo;
    constexpr float kFolga = 0.5f;
    for (size_t i = 0; i < trechos.size(); ++i) {
        const Trecho& t = trechos[i];
        if (t.tronco != comum) continue;
        if (t.botao == anterior) acesos.push_back({i, 0.0f});
        else if (t.botao == novo) acesos.push_back({i, 2.0f * passo});
        else if (t.botao == -1 && t.y0 >= baixo - kFolga && t.y1 <= alto + kFolga && distancia > 0.0f) {
            // O pedaco mais perto de quem saiu acende antes.
            const float meio = (t.y0 + t.y1) / 2.0f;
            const float ida = (meio > yDe ? meio - yDe : yDe - meio) / distancia;
            acesos.push_back({i, passo * (0.5f + ida)});
        }
    }
    return acesos;
}

float MapaDaGrade::Luz(const float t) {
    if (t < 0.0f) return 0.0f;
    if (t < kSubidaDaLuz) return t / kSubidaDaLuz;
    if (t < kSubidaDaLuz + kLuzCheia) return 1.0f;
    const float descendo = (t - kSubidaDaLuz - kLuzCheia) / kDescidaDaLuz;
    if (descendo >= 1.0f) return 0.0f;
    // Some depressa no comeco e devagar no fim, como um brilho que esfria.
    return (1.0f - descendo) * (1.0f - descendo);
}
