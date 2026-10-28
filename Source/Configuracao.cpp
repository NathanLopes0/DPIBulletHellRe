//
// As opcoes do jogo, e o texto do arquivo em que elas moram.
//

#include "Configuracao.h"

#include <sstream>

namespace {

    const char* SimOuNao(const bool b) { return b ? "sim" : "nao"; }

    bool DeSimOuNao(const std::string& s, bool& saida) {
        if (s == "sim") { saida = true; return true; }
        if (s == "nao") { saida = false; return true; }
        return false;
    }

    bool DeVolume(const std::string& s, int& saida) {
        if (s.empty() || s.size() > 2) return false;
        for (const char c : s) if (c < '0' || c > '9') return false;
        const int v = std::stoi(s);
        if (v > Configuracao::kVolumeMaximo) return false;
        saida = v;
        return true;
    }

    std::string Aparado(const std::string& s) {
        const char* const branco = " \t\r\n";
        const auto ini = s.find_first_not_of(branco);
        if (ini == std::string::npos) return "";
        const auto fim = s.find_last_not_of(branco);
        return s.substr(ini, fim - ini + 1);
    }
}

namespace Configuracao {

int MoverVolume(const int v, const int passo) {
    const int depois = v + passo;
    return depois < 0 ? 0 : (depois > kVolumeMaximo ? kVolumeMaximo : depois);
}

float FatorDoVolume(const int v) {
    return static_cast<float>(MoverVolume(v, 0)) / static_cast<float>(kVolumeMaximo);
}

std::string Serializar(const Dados& dados) {
    std::ostringstream saida;
    saida << "# Opcoes do DPIBulletHellRe, uma por linha (chave=valor).\n"
          << "# Valem para todos que jogam nesta maquina. A tela de Opcoes escreve este\n"
          << "# arquivo; editar a mao tambem funciona, com o jogo fechado.\n"
          << "#\n"
          << "# volume_musica e volume_efeitos: de 0 (mudo) a " << kVolumeMaximo << "\n"
          << "volume_musica=" << dados.volumeMusica << "\n"
          << "volume_efeitos=" << dados.volumeEfeitos << "\n"
          << "# tela_cheia e hitbox: sim ou nao\n"
          << "tela_cheia=" << SimOuNao(dados.telaCheia) << "\n"
          << "hitbox=" << SimOuNao(dados.mostrarHitbox) << "\n";
    return saida.str();
}

Lida Ler(const std::string& texto) {
    Lida lida;

    std::istringstream entrada(texto);
    std::string linha;
    int numero = 0;
    while (std::getline(entrada, linha)) {
        ++numero;
        const std::string limpa = Aparado(linha);
        if (limpa.empty() || limpa[0] == '#') continue;

        const auto igual = limpa.find('=');
        if (igual == std::string::npos) {
            lida.problemas.push_back("linha " + std::to_string(numero) + ": \"" + limpa +
                                     "\" nao tem '=' e foi ignorada");
            continue;
        }
        const std::string chave = Aparado(limpa.substr(0, igual));
        const std::string valor = Aparado(limpa.substr(igual + 1));

        // A DIFICULDADE DE ANTES: era opcao da maquina, hoje e escolhida a
        // cada partida. A linha de um arquivo antigo nao e problema - so nao
        // vale mais nada.
        if (chave == "dificuldade") continue;

        if (chave == "volume_musica" || chave == "volume_efeitos") {
            int& destino = (chave == "volume_musica") ? lida.dados.volumeMusica : lida.dados.volumeEfeitos;
            if (!DeVolume(valor, destino)) {
                lida.problemas.push_back("linha " + std::to_string(numero) + ": " + chave + " \"" + valor +
                                         "\" nao e um numero de 0 a " + std::to_string(kVolumeMaximo) +
                                         "; ficou " + std::to_string(kVolumeMaximo));
            }
            continue;
        }

        if (chave == "tela_cheia" || chave == "hitbox") {
            bool& destino = (chave == "tela_cheia") ? lida.dados.telaCheia : lida.dados.mostrarHitbox;
            if (!DeSimOuNao(valor, destino)) {
                lida.problemas.push_back("linha " + std::to_string(numero) + ": " + chave + " \"" + valor +
                                         "\" nao e sim nem nao; ficou nao");
            }
            continue;
        }

        lida.problemas.push_back("linha " + std::to_string(numero) + ": a opcao \"" + chave +

                                 "\" nao existe e foi ignorada");
    }
    return lida;
}

}
