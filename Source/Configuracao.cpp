//
// As opcoes do jogo, e o texto do arquivo em que elas moram.
//

#include "Configuracao.h"

#include <sstream>

namespace {

    std::string Aparado(const std::string& s) {
        const char* const branco = " \t\r\n";
        const auto ini = s.find_first_not_of(branco);
        if (ini == std::string::npos) return "";
        const auto fim = s.find_last_not_of(branco);
        return s.substr(ini, fim - ini + 1);
    }
}

namespace Configuracao {

std::string Serializar(const Dados& dados) {
    std::ostringstream saida;
    saida << "# Opcoes do DPIBulletHellRe, uma por linha (chave=valor).\n"
          << "# Valem para todos que jogam nesta maquina. A tela de Opcoes escreve este\n"
          << "# arquivo; editar a mao tambem funciona, com o jogo fechado.\n"
          << "#\n"
          << "# dificuldade: normal ou dificil\n"
          << "dificuldade=" << Dificuldade::Chave(dados.dificuldade) << "\n";
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

        if (chave == "dificuldade") {
            Dificuldade::Nivel nivel;
            if (Dificuldade::DeChave(valor, nivel)) {
                lida.dados.dificuldade = nivel;
            } else {
                lida.problemas.push_back("linha " + std::to_string(numero) + ": dificuldade \"" + valor +
                                         "\" nao existe (normal ou dificil); ficou normal");
            }
            continue;
        }

        lida.problemas.push_back("linha " + std::to_string(numero) + ": a opcao \"" + chave +
                                 "\" nao existe e foi ignorada");
    }
    return lida;
}

}
