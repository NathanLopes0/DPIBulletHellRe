//
// A condicao de um experimento: as mecanicas que esta execucao desliga.
//

#include "Experimento.h"

namespace Experimento {

    const char* Nome(const Mecanica m) {
        switch (m) {
            case Mecanica::Brilho:     return "brilho";
            case Mecanica::Cobertura:  return "cobertura";
            case Mecanica::Papeis:     return "papeis";
            case Mecanica::Raspao:     return "raspao";
            case Mecanica::Perto:      return "perto";
            case Mecanica::Devolutiva: return "devolutiva";
            case Mecanica::Sincronia:  return "sincronia";
        }
        return "";
    }

    std::string Condicao::Nome() const {
        std::string nome;
        for (const Mecanica m : kTodas) {
            if ((mDesligadas & Bit(m)) == 0) continue;
            if (!nome.empty()) nome += "+";
            nome += std::string("sem-") + Experimento::Nome(m);
        }
        return nome.empty() ? "normal" : nome;
    }

    Condicao Ler(const std::string& texto, std::vector<std::string>* problemas) {
        Condicao condicao;
        std::string parte;
        const auto fechar = [&]() {
            // Espacos em volta nao contam: "sem-brilho, sem-papeis".
            const size_t de = parte.find_first_not_of(' ');
            const size_t ate = parte.find_last_not_of(' ');
            const std::string nome = de == std::string::npos ? "" : parte.substr(de, ate - de + 1);
            parte.clear();
            if (nome.empty() || nome == "normal") return;
            for (const Mecanica m : kTodas) {
                if (nome == std::string("sem-") + Nome(m)) {
                    condicao.Desligar(m);
                    return;
                }
            }
            if (problemas) problemas->push_back("condicao desconhecida: \"" + nome + "\"");
        };
        for (const char c : texto) {
            if (c == ',' || c == '+') fechar();
            else parte += c;
        }
        fechar();
        return condicao;
    }
}
