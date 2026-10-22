//
// O pipeline da fase 3 do Ricardo.
//

#include "Pipeline.h"

namespace Pipeline {

    const char* NomeDoEstagio(const int estagio) {
        switch (estagio) {
            case IF:  return "IF";
            case ID:  return "ID";
            case EX:  return "EX";
            case MEM: return "MEM";
            case WB:  return "WB";
            default:  return "";
        }
    }

    const std::vector<Instrucao>& Programa() {
        static const std::vector<Instrucao> programa = {
            {Tipo::Lw,   "lw $t0, 0($a0)"},
            {Tipo::Add,  "add $t1, $t1, $t0"},
            {Tipo::Addi, "addi $a0, $a0, 4"},
            {Tipo::Bne,  "bne $a0, $a1, laco", 0},
            {Tipo::Sw,   "sw $t1, 0($a2)"},
            {Tipo::J,    "j main", 0},
        };
        return programa;
    }

    Ciclo Avancar(Estado& e) {
        const auto& programa = Programa();
        const int n = static_cast<int>(programa.size());
        Ciclo c;

        // TODO MUNDO ANDA UM ESTAGIO, e o do WB sai.
        c.saiu = e.estagio[WB];
        for (int s = WB; s > IF; --s) e.estagio[s] = e.estagio[s - 1];

        // A BUSCA, com a previsao: o bne e o j mandam o pc para o alvo deles.
        const int buscada = ((e.pc % n) + n) % n;
        e.estagio[IF] = buscada;
        const Instrucao& b = programa[static_cast<size_t>(buscada)];
        e.pc = (b.tipo == Tipo::Bne || b.tipo == Tipo::J) ? b.alvo : buscada + 1;

        // O BNE QUE CHEGOU AO EX SE RESOLVE. Desvia em todas as voltas menos a
        // ultima; ai a previsao errou, e o que foi buscado pelo caminho errado
        // - o ID e o IF - vira bolha.
        const int noEX = e.estagio[EX];
        if (noEX != kBolha && programa[static_cast<size_t>(noEX)].tipo == Tipo::Bne) {
            ++e.desvios;
            if (e.desvios >= kVoltas) {
                e.estagio[ID] = kBolha;
                e.estagio[IF] = kBolha;
                e.pc = noEX + 1;
                e.desvios = 0;
                c.flush = true;
            }
        }
        return c;
    }

    void Estourar(Estado& e, const int estagio) {
        if (estagio >= 0 && estagio < kEstagios) e.estagio[estagio] = kBolha;
    }

    const char* Animacao(const Tipo t) {
        switch (t) {
            case Tipo::Lw:   return "lw";
            case Tipo::Add:  return "add";
            case Tipo::Addi: return "addi";
            case Tipo::Bne:  return "bne";
            case Tipo::Sw:   return "sw";
            case Tipo::J:    return "j";
        }
        return "";
    }

    std::string Gatilho(const Tipo t, const int estagio) {
        if (estagio == EX && (t == Tipo::Add || t == Tipo::Addi)) return Animacao(t);
        if (estagio == MEM && (t == Tipo::Lw || t == Tipo::Sw)) return Animacao(t);
        return {};
    }
}
