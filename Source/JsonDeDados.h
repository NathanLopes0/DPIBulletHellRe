//
// A leitura de JSON dos arquivos de dados do jogo.
//

#pragma once

#include <string>

#include "Json.h"

/**
 * @brief Interpreta o texto de um arquivo de dados do jogo.
 *
 * Um envelope fino em volta do nlohmann para que os leitores de dados (formas,
 * regras e fases) concordem num ponto: ARQUIVO DE DADOS DO JOGO ACEITA
 * COMENTARIO.
 *
 * O padrao JSON nao preve comentario, e o nlohmann segue o padrao por omissao.
 * So que estes arquivos substituem codigo C++ que era COMENTADO - por que um
 * leque tem 16 projeteis e nao sete, por que uma correcao de rota e fraca de
 * proposito, por que um laco precisa ser lento para ser lido. Migrar para dados
 * sem comentario significaria jogar essa explicacao fora, e o arquivo viraria
 * uma tabela de numeros sem defesa: quem mexesse nele seis meses depois mudaria
 * o numero sem saber o que estava desfazendo.
 *
 * Os leitores chamam esta funcao em vez de nlohmann::json::parse para que a
 * decisao viva num lugar so - acrescentar um leitor novo nao exige lembrar de
 * repetir os tres argumentos certos.
 *
 * Lanca, como o parse original, quando o texto nao e JSON valido; quem chama ja
 * trata a excecao e a transforma numa frase de problema.
 */
inline nlohmann::json LerJsonDeDados(const std::string& texto) {
    //                                       callback  excecoes  ignora comentarios
    return nlohmann::json::parse(texto, nullptr, true, /*ignore_comments*/ true);
}
