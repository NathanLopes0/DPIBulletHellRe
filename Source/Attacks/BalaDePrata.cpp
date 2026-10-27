//
// A repescagem da Maria Lucia: nao ha bala de prata.
//

#include "BalaDePrata.h"

namespace BalaDePrata {

    const std::vector<std::string>& Promessas() {
        // \xc3\xa7 e \xc3\xa3 sao c cedilha e a til, \xc3\xa9 e e agudo,
        // \xc3\xa1 a agudo: o MSVC nao le acento em literal.
        static const std::vector<std::string> promessas = {
            "IA", "UML", "Orienta\xc3\xa7\xc3\xa3o a objetos", "M\xc3\xa9todos \xc3\xa1geis",
            "Nuvem", "Low-code", "Microsservi\xc3\xa7os", "Ferramenta CASE"};
        return promessas;
    }

    bool SeDesfaz(const float distancia, const float voo) {
        return distancia <= kPerto || voo >= kVooMaximo;
    }

    std::vector<int> Sortear(Variacao::Sorteio& s) {

        return Variacao::Ordem(s, static_cast<int>(Promessas().size()), 0);
    }

    std::string Desfeita(const std::string& promessa) {
        return promessa + "? N\xc3\xa3o h\xc3\xa1 bala de prata.";
    }

    std::string Testada(const std::string& promessa) {
        return promessa + " \xc2\xb7 teste com usu\xc3\xa1rio: reprovada";
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreBalas = Dificuldade::Escolher(n, d.entreBalas, 1.4f);
        return d;
    }
}
