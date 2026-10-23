//
// A fita da maquina de Turing do Vladimir - a regra de quando uma casa e
// reescrita, sem nada do motor.
//

#include "Fita.h"

#include <cmath>

namespace Fita {

bool CabecaSobreACasa(const float xDaCabeca, const float xDaCasa, const float raio) {
    return std::fabs(xDaCabeca - xDaCasa) <= raio;
}

bool DeveReescrever(const float idade, const float idadeMinima,
                    const std::vector<float>& xDasCabecas, const float xDaCasa, const float raio) {

    if (idade < idadeMinima) return false;

    for (const float x : xDasCabecas) {
        if (CabecaSobreACasa(x, xDaCasa, raio)) return true;
    }
    return false;
}


    std::string Transicao(const bool paraDireita) {
        // q0 ou q1 em subscrito, o delta, a seta: UTF-8 por escape.
        const std::string q = paraDireita ? "q\xE2\x82\x80" : "q\xE2\x82\x81";
        const std::string seta = paraDireita ? "\xE2\x86\x92" : "\xE2\x86\x90";
        return "\xCE\xB4(" + q + ", a) = (" + q + ", b, " + seta + ")";
    }

    std::string Branco(const bool chegouADireita) {
        // O branco e o quadrado vazio (E2 96 A1); o estado de antes e o de quem
        // chegou, e o de depois e o do sentido de volta.
        const std::string branco = "\xE2\x96\xA1";
        const std::string de = chegouADireita ? "q\xE2\x82\x80" : "q\xE2\x82\x81";
        const std::string para = chegouADireita ? "q\xE2\x82\x81" : "q\xE2\x82\x80";
        const std::string seta = chegouADireita ? "\xE2\x86\x90" : "\xE2\x86\x92";
        return "\xCE\xB4(" + de + ", " + branco + ") = (" + para + ", " + branco + ", " + seta + ")";
    }
}
