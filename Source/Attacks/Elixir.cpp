//
// A fase 3 do Lucas: a arvore de supervisao do Elixir.
//

#include "Elixir.h"

#include <algorithm>

namespace Elixir {

    std::array<float, 2> Lugar(const int k, const float x, const float y, const float largura) {
        // A ARVORE INTEIRA CABE NA JANELA: o meio dela e empurrado para dentro
        // quando o supervisor esta perto da borda.
        const float meiaArvore = kEntreWorkers * static_cast<float>(kWorkers - 1) / 2.0f;
        const float meio = std::clamp(x, meiaArvore + 60.0f, largura - meiaArvore - 60.0f);
        return {meio + (static_cast<float>(k) - static_cast<float>(kWorkers - 1) / 2.0f) * kEntreWorkers, y + kAbaixo};
    }

    std::string Nome(const Estrategia e) {
        switch (e) {
            case Estrategia::UmPorUm:    return ":one_for_one";
            case Estrategia::TodosPorUm: return ":one_for_all";
            case Estrategia::RestoPorUm: return ":rest_for_one";
        }
        return "";
    }

    std::vector<int> Reiniciam(const Estrategia e, const int caido, const int workers) {
        std::vector<int> quem;
        for (int k = 0; k < workers; ++k) {
            if (e == Estrategia::UmPorUm && k != caido) continue;
            if (e == Estrategia::RestoPorUm && k < caido) continue;
            quem.push_back(k);
        }
        return quem;
    }

    std::array<Estrategia, 3> Sortear(Variacao::Sorteio& s) {
        std::vector<Estrategia> todas = {Estrategia::UmPorUm, Estrategia::TodosPorUm, Estrategia::RestoPorUm};
        s.Embaralhar(todas);
        return {todas[0], todas[1], todas[2]};
    }

    int Destino(Variacao::Sorteio& s, const std::vector<int>& vivos, const int anterior) {
        if (vivos.empty()) return -1;
        std::vector<int> outros;
        for (const int k : vivos) if (k != anterior) outros.push_back(k);
        return outros.empty() ? vivos.front() : s.Um(outros);
    }

    namespace {
        std::string Pid(const int worker) { return ":w" + std::to_string(worker + 1); }
        std::string NaLinhaDeBaixo(const Elixir::Estrategia e) { return "\nstrategy: " + Nome(e); }
    }

    std::string Cast(const int worker, const Elixir::Estrategia e) {
        return "GenServer.cast(" + Pid(worker) + ", :duvida)" + NaLinhaDeBaixo(e);
    }

    std::string Handle(const int worker, const Elixir::Estrategia e) {
        return Pid(worker) + " handle_cast(:duvida)" + NaLinhaDeBaixo(e);
    }

    std::string Crash(const int worker, const Elixir::Estrategia e) {
        return "** (EXIT) " + Pid(worker) + " killed" + NaLinhaDeBaixo(e);
    }

    std::string Pipe() { return "duvidas\n|> Enum.map(&responde/1)"; }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreMensagens = Dificuldade::Escolher(n, kEntreMensagens, 0.35f);
        d.etapas = Dificuldade::Escolher(n, kEtapasDoPipe, 6);
        return d;
    }
}
