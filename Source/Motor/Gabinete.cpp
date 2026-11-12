//
// Created by Claude on 08/10/2026.
//

#include "Gabinete.h"

#include <cstdlib>

namespace {

/// Converte o texto inteiro em segundos, ou devolve false sem escrever.
bool LerSegundos(const std::string& texto, float& destino) {
    if (texto.empty()) return false;

    const char* inicio = texto.c_str();
    char* fim = nullptr;
    const float valor = std::strtof(inicio, &fim);

    // Sobrou texto depois do numero ("20s", "abc", "--arcade"): nao e valor.
    if (fim != inicio + texto.size()) return false;

    destino = valor;
    return true;
}

} // namespace

namespace Gabinete {

Configuracao Ler(const std::vector<std::string>& argumentos) {
    Configuracao configuracao;
    bool ociosidadeDita = false;

    for (size_t i = 0; i < argumentos.size(); ++i) {
        const std::string& argumento = argumentos[i];

        if (argumento == "--arcade") {
            configuracao.arcade = true;
            continue;
        }

        if (argumento == "--condicao") {
            // O valor vem no argumento seguinte; quem o le e Experimento::Ler.
            if (i + 1 < argumentos.size()) configuracao.condicao = argumentos[++i];
            continue;
        }

        if (argumento == "--ociosidade") {
            // O valor vem no argumento seguinte.
            if (i + 1 < argumentos.size() && LerSegundos(argumentos[i + 1], configuracao.ociosidade)) {
                ociosidadeDita = true;
                ++i;
            }
            continue;
        }

        // Qualquer outra coisa e ignorada.
    }

    // Fora do laco para a ordem nao importar.
    if (configuracao.arcade && !ociosidadeDita) {
        configuracao.ociosidade = kOciosidadePadrao;
    }

    // Negativo nao e "menos de zero segundo", e desligado.
    if (configuracao.ociosidade < 0.0f) {
        configuracao.ociosidade = 0.0f;
    }

    return configuracao;
}

bool Contagem::Passou(const float deltaTime, const bool continua) {
    if (!continua) {
        mAcumulado = 0.0f;
        return false;
    }

    if (!Ligada()) return false;

    mAcumulado += deltaTime;
    if (mAcumulado < mLimite) return false;

    mAcumulado = 0.0f;
    return true;
}

} // namespace Gabinete
