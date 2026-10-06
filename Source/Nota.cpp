//
// Como a nota cresce durante uma batalha.
//

#include "Nota.h"

#include <cmath>

namespace Nota {

float Ganho(const float notaAtual, const float ganhoBruto) {

    if (ganhoBruto <= 0.0f) return ganhoBruto;      // perda nao e assunto daqui
    if (notaAtual < kFaixaFacil) return ganhoBruto;

    // O fator vale 1 em kFaixaFacil e mingua ate zero em kAssintota. Como a
    // assintota esta acima de 100, em 100 ele ainda e positivo - pequeno, mas
    // positivo, que e o que mantem a nota cheia alcancavel.
    const float faltaParaAssintota = kAssintota - notaAtual;
    if (faltaParaAssintota <= 0.0f) return 0.0f;    // so se alguem mexer nas constantes

    const float fator = std::pow(faltaParaAssintota / (kAssintota - kFaixaFacil), kExpoente);
    return ganhoBruto * fator;
}

bool PerdeuACheia(const int acertosSofridos) {
    return acertosSofridos >= kAcertosQuePerdemACheia;
}

bool ECheia(const float nota) {
    // >= e nao ==: Somar ja limita em kNotaMaxima, entao passar nao deveria
    // acontecer - mas se um dia acontecer, o certo e acender o dourado, e nao
    // apaga-lo justamente para quem foi melhor de todos.
    return nota >= kNotaMaxima;
}

float TetoCom(const int acertosSofridos) {
    return PerdeuACheia(acertosSofridos) ? kTetoComDano : kNotaMaxima;
}

float Somar(const float notaAtual, const float ganhoBruto, const int acertosSofridos) {

    const float nova = notaAtual + Ganho(notaAtual, ganhoBruto);
    const float teto = TetoCom(acertosSofridos);

    // O teto e aplicado DEPOIS da curva, e nao no lugar dela: quem levou dano
    // demais continua subindo normalmente ate 99.99 e so para ali. Trocar a
    // ordem faria o dano tambem desacelerar a subida, que e punir duas vezes.
    if (nova > teto) return teto;
    if (nova < kNotaMinima) return kNotaMinima;
    return nova;
}

float Subtrair(const float notaAtual, const float perda) {

    const float nova = notaAtual - perda;
    if (nova < kNotaMinima) return kNotaMinima;

    // Subir o teto nao e trabalho daqui: perder nunca aproxima ninguem de 100.
    if (nova > kNotaMaxima) return kNotaMaxima;
    return nova;
}

}
