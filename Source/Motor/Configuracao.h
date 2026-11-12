//
// As opcoes do jogo, e o texto do arquivo em que elas moram.
//

#pragma once

#include <string>
#include <vector>

/// Quem abre o arquivo e ConfiguracaoArquivo.cpp.
namespace Configuracao {

    /// O volume vai de 0 (mudo) a kVolumeMaximo, que e o volume de sempre.
    constexpr int kVolumeMaximo = 10;

    struct Dados {
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

    /// O volume 'v' andado 'passo' casas, preso entre 0 e o maximo - nao da a volta: do mudo para a
    /// esquerda continua mudo.
    int MoverVolume(int v, int passo);

    /// O fator que o som toca, de 0 a 1.
    float FatorDoVolume(int v);

    /// O texto do arquivo, com um comentario no alto dizendo o que ele e.
    std::string Serializar(const Dados& dados);

    struct Lida {
        Dados dados;
        /// O que nao deu para ler, uma frase por problema, para o log. Uma linha estragada vira o
        /// padrao daquela opcao, e as outras valem.
        std::vector<std::string> problemas;
    };

    /// Le o texto do arquivo. Nunca falha: o que nao for entendido fica no padrao e vai para
    /// `problemas`.
    Lida Ler(const std::string& texto);
}
