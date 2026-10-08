//
// O teclado digital da tela de matricula.
//

#include "Teclado.h"

namespace Teclado {

namespace {

    Tecla Numero(const char d, const int coluna, const int linha) {
        return Tecla{Acao::Digito, d, std::string(1, d), coluna, linha};
    }

    Tecla Comando(const Acao acao, const std::string& rotulo, const int linha) {
        return Tecla{acao, '0', rotulo, 3, linha};   // a coluna 3 e a das acoes
    }

    const std::vector<Tecla>& Montar() {
        static const std::vector<Tecla> teclas = {
            Numero('1', 0, 0), Numero('2', 1, 0), Numero('3', 2, 0),
            Numero('4', 0, 1), Numero('5', 1, 1), Numero('6', 2, 1),
            Numero('7', 0, 2), Numero('8', 1, 2), Numero('9', 2, 2),
                               Numero('0', 1, 3),

            Comando(Acao::Apagar,    "APAGAR",    0),
            Comando(Acao::Entrar,    "ENTRAR",    1),
            Comando(Acao::Visitante, "VISITANTE", 2),
            Comando(Acao::Voltar,    "VOLTAR",    3),
        };
        return teclas;
    }
}

const std::vector<Tecla>& Teclas() { return Montar(); }

Navegacao::Grade Grade() {

    const std::vector<Tecla>& teclas = Teclas();

    int maiorColuna = 0;
    for (const auto& t : teclas) if (t.coluna > maiorColuna) maiorColuna = t.coluna;

    Navegacao::Grade grade(static_cast<size_t>(maiorColuna) + 1);

    // Em ordem de LINHA dentro de cada coluna, e nao na ordem em que as teclas
    // aparecem na lista: a navegacao vertical anda por esta ordem, e se ela nao
    // fosse a de cima para baixo, a seta para baixo subiria.
    for (int linha = 0; ; ++linha) {
        bool achouAlguma = false;
        for (size_t i = 0; i < teclas.size(); ++i) {
            if (teclas[i].linha != linha) continue;
            grade[static_cast<size_t>(teclas[i].coluna)].push_back(i);
            achouAlguma = true;
        }
        if (!achouAlguma) break;
    }

    return grade;
}

size_t Inicial() {
    return 0;   // o "1"
}

}
