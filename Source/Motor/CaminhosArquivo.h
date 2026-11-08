//
// A ponte entre a montagem de caminhos e o disco.
//

#pragma once

#include <optional>
#include <string>

/**
 * Separado de Caminhos.cpp pelo mesmo motivo de PathShapesArquivo.cpp,
 * RegrasDeAtaqueArquivo.cpp e FasesDeAtaqueArquivo.cpp: aquele e puro e entra no
 * alvo de testes; este olha o disco, escreve log e usa SDL, entao fica de fora.
 */
namespace Caminhos {

    /**
     * @brief Descobre a pasta base do jogo. Chame UMA vez, no inicio.
     *
     * Procura, subindo, a primeira pasta que contenha um diretorio Assets. Tenta
     * nesta ordem:
     *   1. a pasta do EXECUTAVEL (SDL_GetBasePath), subindo ate 5 niveis;
     *   2. o diretorio de trabalho atual, subindo ate 5 niveis.
     *
     * A pasta do executavel vem primeiro de proposito: ela nao depende de onde o
     * jogo foi lancado, e e isso que faz a maquina de Arcade funcionar. O
     * diretorio de trabalho e a rede de seguranca para o caso de o executavel
     * estar fora da arvore do projeto.
     *
     * Se nenhuma das duas achar, registra o motivo no log e assume "..", que e o
     * comportamento historico - assim o pior caso e igual ao que era antes, e nao
     * pior.
     *
     * Devolve false quando caiu no pior caso, para quem chama poder avisar.
     */
    bool Inicializar();

    /**
     * @brief O caminho completo de um arquivo de Assets.
     *
     * Receba o caminho SEM "Assets/" e sem "../":
     *     Caminhos::Asset("Teachers/DPIBHSalles.png")
     */
    std::string Asset(const std::string& relativo);

    /**
     * @brief O conteudo inteiro de um arquivo de texto, ou vazio (nullopt) se
     * ele nao abriu.
     *
     * ERAM DEZ COPIAS de "abre, testa, despeja num ostringstream" - uma em cada
     * ponte de dados (materias, fichas, opcoes, fases, regras...). Cada uma
     * escolhia o proprio jeito de ler (rdbuf, istreambuf_iterator), e as dez
     * tinham de concordar em tratar o arquivo que falta. Aqui fica a leitura;
     * la fica so o que e de cada uma: a frase que explica o que falta ao jogo
     * sem aquele arquivo.
     *
     * O caminho ja vem pronto (Asset, Save): esta funcao nao resolve nada.
     */
    std::optional<std::string> LerTexto(const std::string& caminho);

    /**
     * @brief O caminho completo de um arquivo de save, em <base>/Saves.
     *
     * Cria a pasta Saves se ela nao existir. Fica ao lado de Assets de proposito:
     * numa maquina de Arcade do departamento, o save precisa ser achavel e
     * copiavel por quem administra a maquina, nao escondido no perfil do usuario.
     */
    std::string Save(const std::string& relativo);

}
