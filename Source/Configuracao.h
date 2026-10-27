//
// As opcoes do jogo, e o texto do arquivo em que elas moram.
//

#pragma once

#include <string>
#include <vector>

#include "Dificuldade.h"

/**
 * CAMADA PURA: so texto. Quem abre o arquivo e ConfiguracaoArquivo.cpp.
 * Coberto por test_configuracao.cpp.
 *
 * AS OPCOES SAO DA MAQUINA, E NAO DO ALUNO. Ficam em Saves/opcoes.cfg, um
 * arquivo so, e valem para quem jogar ali - como o volume de um televisor. A
 * ficha de cada aluno (Ficha.h) continua sendo so o progresso e a aparencia.
 *
 * O FORMATO E chave=valor, uma por linha, com # para comentario. Nao e JSON de
 * proposito: a pasta de saves e varrida atras de *.json, um por aluno
 * (FichaArquivo::ListarMatriculas), e um arquivo de opcoes com essa extensao
 * seria um aluno a mais esperando para aparecer. E chave=valor e o que da para
 * editar a mao no gabinete sem quebrar nada.
 */
namespace Configuracao {

    /// O volume vai de 0 (mudo) a kVolumeMaximo, que e o volume de sempre.
    constexpr int kVolumeMaximo = 10;

    struct Dados {
        Dificuldade::Nivel dificuldade = Dificuldade::Nivel::Normal;

        /// A musica (o som em laco, o da batalha) e os efeitos (o tiro, o
        /// Ponto Extra, os ataques), cada um de 0 a kVolumeMaximo.
        int volumeMusica = kVolumeMaximo;
        int volumeEfeitos = kVolumeMaximo;

        /// A janela em tela cheia. No gabinete ela e cheia de qualquer jeito.
        bool telaCheia = false;

        /// O circulo de colisao do aluno desenhado na batalha: onde um tiro
        /// acerta de verdade, que e menor que o sprite.
        bool mostrarHitbox = false;
    };

    /// @brief O volume 'v' andado 'passo' casas, preso entre 0 e o maximo -
    /// nao da a volta: do mudo para a esquerda continua mudo.
    int MoverVolume(int v, int passo);

    /// @brief O fator que o som toca, de 0 a 1.
    float FatorDoVolume(int v);


    /// O texto do arquivo, com um comentario no alto dizendo o que ele e.
    std::string Serializar(const Dados& dados);

    struct Lida {
        Dados dados;
        /// O que nao deu para ler, uma frase por problema, para o log. Uma
        /// linha estragada vira o padrao DAQUELA opcao, e as outras valem.
        std::vector<std::string> problemas;
    };

    /**
     * @brief Le o texto do arquivo. NUNCA FALHA: o que nao for entendido fica
     * no padrao e vai para `problemas`.
     *
     * Um arquivo de opcoes estragado nao pode impedir ninguem de jogar, pela
     * mesma regra da ficha: no pior caso o jogo abre no Normal.
     *
     * Aceita espaco em volta da chave e do valor, linha em branco, comentario e
     * fim de linha do Windows. Chave desconhecida e problema, e nao erro: e o
     * que um arquivo de uma versao mais nova traria.
     */
    Lida Ler(const std::string& texto);
}
