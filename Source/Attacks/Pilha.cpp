//
// A pilha da fase 1 do Salles.
//

#include "Pilha.h"

namespace Pilha {

    Operacao Proxima(const int altura, const std::uint32_t sorteio) {
        return Proxima(altura, sorteio, Plano{});
    }

    Operacao Proxima(const int altura, const std::uint32_t sorteio, const Plano& p) {
        Operacao op;
        op.valor = static_cast<int>((sorteio >> 12) % 10u);
        if (altura >= p.capacidade) {
            op.tipo = Tipo::Overflow;
            return op;
        }
        if (altura <= 0) {
            op.tipo = Tipo::Push;
            return op;
        }
        const float chance = static_cast<float>(sorteio & 1023u) / 1024.0f;
        op.tipo = chance < p.chanceDePush ? Tipo::Push : Tipo::Pop;
        return op;
    }

    float AbaixoDoProfessor(const int k) {
        return kPrimeiroBloco + static_cast<float>(k < 0 ? 0 : k) * kEntreBlocos;
    }

    std::string Placar(const Operacao& op, const int topo) {
        switch (op.tipo) {
            case Tipo::Push:     return "push(" + std::to_string(op.valor) + ")";
            case Tipo::Pop:      return "pop() \xE2\x86\x92 " + std::to_string(topo);
            case Tipo::Overflow: return "stack overflow!";
        }
        return "";
    }

    std::string CaraDoBloco(const int valor) {
        return "N" + std::to_string(valor < 0 || valor > 9 ? 0 : valor);
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p;
        p.capacidade = s.Inteiro(kMenorCapacidade, kMaiorCapacidade);
        // A CHANCE ANDA COM A CAPACIDADE: numa pilha maior o push e mais
        // provavel, e o overflow continua chegando em quase toda fase.
        p.chanceDePush = s.Real(0.55f, 0.65f) + 0.05f * static_cast<float>(p.capacidade - kMenorCapacidade);
        return p;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.passo = Dificuldade::Escolher(n, kPasso, 0.66f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.1f);
        return d;
    }
}
