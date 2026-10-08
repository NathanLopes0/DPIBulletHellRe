//
// Created by Claude on 08/10/2026.
//

#include "Gabinete.h"

#include <cstdlib>

namespace {

/**
 * @brief Converte o texto INTEIRO em segundos, ou devolve false sem escrever.
 *
 * Nao usa std::stof de proposito: ela joga excecao em texto invalido, e um
 * valor errado no script que abre o gabinete tem de cair no padrao, nao
 * derrubar o jogo antes de a janela existir - numa maquina sem teclado ninguem
 * veria o erro.
 */
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

        if (argumento == "--ociosidade") {
            // O valor vem no argumento SEGUINTE. Se faltar ou nao for numero, o
            // padrao vale e o i NAO avanca - assim "--ociosidade --arcade"
            // ainda liga o modo arcade em vez de engolir a flag como valor.
            if (i + 1 < argumentos.size() && LerSegundos(argumentos[i + 1], configuracao.ociosidade)) {
                ociosidadeDita = true;
                ++i;
            }
            continue;
        }

        // Qualquer outra coisa e ignorada. Ver o comentario de Ler.
    }

    // Fora do laco para a ORDEM NAO IMPORTAR: "--arcade --ociosidade 20" e
    // "--ociosidade 20 --arcade" tem de dar o mesmo resultado.
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
