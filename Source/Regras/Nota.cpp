//
// Como a nota cresce durante uma batalha.
//

#include "Nota.h"

#include <cmath>

namespace Nota {

float Ganho(const float notaAtual, const float ganhoBruto) {

    if (ganhoBruto <= 0.0f) return ganhoBruto;      // perda nao e assunto daqui
    if (notaAtual < kFaixaFacil) return ganhoBruto;

    // O fator vale 1 em kFaixaFacil e mingua ate zero em kAssintota.
    const float faltaParaAssintota = kAssintota - notaAtual;
    if (faltaParaAssintota <= 0.0f) return 0.0f;    // so se alguem mexer nas constantes

    const float fator = std::pow(faltaParaAssintota / (kAssintota - kFaixaFacil), kExpoente);
    return ganhoBruto * fator;
}

bool PerdeuACheia(const int acertosSofridos) {
    return acertosSofridos >= kAcertosQuePerdemACheia;
}

bool ECheia(const float nota) {
    // >= e nao ==: Somar ja limita em kNotaMaxima, entao passar nao deveria acontecer.
    return nota >= kNotaMaxima;
}

float TetoCom(const int acertosSofridos) {
    return PerdeuACheia(acertosSofridos) ? kTetoComDano : kNotaMaxima;
}

Desfecho AposTerceiraFase(const float nota) {

    if (ECheia(nota))             return Desfecho::VaiParaFinal;
    if (nota >= kNotaAprovacao)   return Desfecho::Aprovado;
    if (nota >= kNotaDeExame)     return Desfecho::VaiParaFinal;
    return Desfecho::Reprovado;
}

float Somar(const float notaAtual, const float ganhoBruto, const int acertosSofridos) {

    const float nova = notaAtual + Ganho(notaAtual, ganhoBruto);
    const float teto = TetoCom(acertosSofridos);

    // O teto e aplicado depois da curva, e nao no lugar dela.
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

float PorJogador(const float bruto, const size_t quantosJogadores) {
    if (quantosJogadores <= 1) return bruto;
    return bruto / static_cast<float>(quantosJogadores);
}

float PerdaPorJogador(const float bruto, const size_t quantosJogadores) {

    const float repartido = PorJogador(bruto, quantosJogadores);

    // Sozinho nao ha alivio.
    if (quantosJogadores <= 1) return repartido;

    return repartido * kAlivioDaPerdaEmGrupo;
}

float FatorDePerto(const float distancia) {
    if (distancia <= kPerto) return kFatorDePerto;
    if (distancia >= kLonge) return 1.0f;
    const float u = (kLonge - distancia) / (kLonge - kPerto);
    return 1.0f + (kFatorDePerto - 1.0f) * u;
}

}
