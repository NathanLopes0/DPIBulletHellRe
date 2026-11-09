//
// A tela de resultado da batalha: o resumo e o que fazer em seguida.
//

#include "Resultado.h"

#include <cstdio>

namespace Resultado {

    namespace {
        /// Com duas casas e ponto, como a barra da nota.
        std::string Nota(const float n) {
            char b[16];
            std::snprintf(b, sizeof(b), "%.2f", static_cast<double>(n));
            return b;
        }
    }

    const char* Texto(const Opcao o) {
        // \xc3\xa0 e \xc3\xa9 sao a crase e o e agudo: o MSVC nao le acento em literal.
        switch (o) {
            case Opcao::TentarDeNovo: return "Tentar de novo";
            case Opcao::Voltar:       return "Voltar \xc3\xa0s mat\xc3\xa9rias";
        }
        return "";
    }

    int Mover(const int atual, const int passo) {
        const int r = (atual + passo) % kQuantas;
        return r < 0 ? r + kQuantas : r;
    }

    const char* Titulo(const bool aprovado) {
        return aprovado ? "APROVADE!" : "REPROVADE";
    }

    bool Formou(const Resumo& r) {
        // So quem passou se forma; a batalha nem preenche a lista de quem reprovou.
        return r.aprovado && !r.formados.empty();
    }

    const char* Titulo(const Resumo& r) {
        return Formou(r) ? "FORMADE!" : Titulo(r.aprovado);
    }

    bool NovoRecorde(const Resumo& r) {
        return r.temFicha && r.nota >= r.recordeAntes + 0.01f;
    }

    std::vector<std::string> Linhas(const Resumo& r) {
        std::vector<std::string> linhas;
        linhas.push_back("Nota final: " + Nota(r.nota));
        if (NovoRecorde(r)) {
            linhas.push_back("NOVO RECORDE!");
        } else if (r.temFicha) {
            linhas.push_back("Recorde: " + Nota(r.recordeAntes));
        }
        linhas.push_back("Fases limpas: " + std::to_string(r.fasesLimpas));
        linhas.push_back("Acertos sofridos: " + std::to_string(r.acertosSofridos));
        if (Formou(r)) {
            // No solo e "voce"; na dupla, os dois ou o nome de quem se formou agora (o outro ja tinha).
            const std::string quem = r.partes.size() < 2 ? "Voc\xc3\xaa"
                                   : r.formados.size() >= 2 ? "A dupla" : r.formados.front();
            // \xc3\xaa e o e circunflexo. "Para sempre" so quando ficou na ficha.
            linhas.push_back(quem + " ganhou o chap\xc3\xa9u de formatura" +
                             (r.formaturaGravada ? ", para sempre." : "."));
        }
        // A devolutiva: so de quem reprovou.
        if (!r.aprovado && r.piorFase) linhas.push_back(Devolutiva::LinhaDaFase(*r.piorFase));
        if (LinhaDasCausas(r) >= 0) linhas.emplace_back("");
        for (const Devolutiva::Parte& p : r.partes) linhas.push_back(Devolutiva::LinhaDaParte(p));
        return linhas;
    }

    int LinhaDasCausas(const Resumo& r) {
        if (r.aprovado || r.causas.empty()) return -1;
        // Depois das de sempre (tres sem ficha, quatro com) e da fase.
        const int base = r.temFicha ? 4 : 3;
        return base + (r.piorFase ? 1 : 0);
    }

    int LinhaDaParte(const Resumo& r, const size_t quem) {
        if (quem >= r.partes.size()) return -1;
        const int causas = LinhaDasCausas(r);
        const int antes = (r.temFicha ? 4 : 3) + (!r.aprovado && r.piorFase ? 1 : 0) + (causas >= 0 ? 1 : 0) +
                          (Formou(r) ? 1 : 0);
        return antes + static_cast<int>(quem);
    }

    int LinhaDaFormatura(const Resumo& r) {
        // Logo depois das de sempre (tres sem ficha, quatro com): quem passou nao tem devolutiva.
        if (!Formou(r)) return -1;
        return r.temFicha ? 4 : 3;
    }

    int LinhaEmDestaque(const Resumo& r) {
        // A segunda, logo abaixo da nota que bateu o recorde (Linhas).
        return NovoRecorde(r) ? 1 : -1;
    }

    bool VoltaSozinha(const bool arcade, const float segundosNaTela) {
        return arcade && segundosNaTela >= kEsperaNoGabinete;
    }
}
