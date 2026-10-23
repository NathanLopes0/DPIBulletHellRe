//
// A pilha da fase 1 do Salles.
//

#include "Pilha.h"

namespace Pilha {

    Operacao Proxima(const int altura, const std::uint32_t sorteio) {
        Operacao op;
        op.valor = static_cast<int>((sorteio >> 12) % 10u);
        if (altura >= kCapacidade) {
            op.tipo = Tipo::Overflow;
            return op;
        }
        if (altura <= 0) {
            op.tipo = Tipo::Push;
            return op;
        }
        const float chance = static_cast<float>(sorteio & 1023u) / 1024.0f;
        op.tipo = chance < kChanceDePush ? Tipo::Push : Tipo::Pop;
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
}
