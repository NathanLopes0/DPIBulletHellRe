//
// As transacoes da fase 3 do Thiago.
//

#include "Transacao.h"

#include <algorithm>
#include <random>

namespace Transacao {

    namespace {
        const Tx* Achar(const Txs& txs, const int id) {
            for (const Tx& t : txs) if (t.id == id) return &t;
            return nullptr;
        }
    }

    int Dono(const Txs& txs, const int linha) {
        for (const Tx& t : txs) {
            if (std::find(t.segura.begin(), t.segura.end(), linha) != t.segura.end()) return t.id;
        }
        return 0;
    }

    std::vector<int> CicloDeEspera(const Txs& txs, const int id) {
        std::vector<int> caminho;
        int atual = id;
        // No maximo uma volta por transacao: um caminho mais longo ja repetiu
        // alguem, e se nao voltou a 'id' o ciclo nao e dela.
        for (size_t passo = 0; passo <= txs.size(); ++passo) {
            const Tx* t = Achar(txs, atual);
            if (!t || t->quer < 0) return {};
            caminho.push_back(atual);
            const int dono = Dono(txs, t->quer);
            if (dono == 0 || dono == atual) return {};
            if (dono == id) return caminho;
            if (std::find(caminho.begin(), caminho.end(), dono) != caminho.end()) return {};
            atual = dono;
        }
        return {};
    }

    int Vitima(const std::vector<int>& ciclo) {
        return ciclo.empty() ? 0 : *std::max_element(ciclo.begin(), ciclo.end());
    }

    void Liberar(Txs& txs, const int id) {
        txs.erase(std::remove_if(txs.begin(), txs.end(), [id](const Tx& t) { return t.id == id; }), txs.end());
    }

    int LinhaLivre(const Txs& txs, const int linhas, const std::uint32_t sorteio, const int evitar) {
        std::vector<int> livres;
        for (int l = 0; l < linhas; ++l) if (l != evitar && Dono(txs, l) == 0) livres.push_back(l);
        if (livres.empty()) return -1;
        std::mt19937 g(sorteio);
        return livres[g() % livres.size()];
    }

    int LinhaDeOutra(const Txs& txs, const int id, const std::uint32_t sorteio) {
        std::vector<int> delas;
        for (const Tx& t : txs) {
            if (t.id == id) continue;
            delas.insert(delas.end(), t.segura.begin(), t.segura.end());
        }
        if (delas.empty()) return -1;
        std::mt19937 g(sorteio);
        return delas[g() % delas.size()];
    }

    bool Desfaz(const std::uint32_t sorteio) {
        std::mt19937 g(sorteio);
        return static_cast<float>(g() % 1000u) / 1000.0f < kChanceDeRollback;
    }

    std::string PlacarDoUpdate(const int id, const int nota, const int mat) {
        return "T" + std::to_string(id) + ": UPDATE SET nota=" + std::to_string(nota) + " WHERE mat=" + std::to_string(mat);
    }

    std::string PlacarDoFim(const int id, const bool desfaz) {
        return "T" + std::to_string(id) + (desfaz ? ": ROLLBACK" : ": COMMIT");
    }

    std::string PlacarDaEspera(const int id, const int dono) {
        return "T" + std::to_string(id) + " espera T" + std::to_string(dono);
    }

    std::string PlacarDoDeadlock(const int vitima) {
        return "deadlock! T" + std::to_string(vitima) + ": ROLLBACK";
    }
}
