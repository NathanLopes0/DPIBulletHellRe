//
// A fase 3 do Goulart: o escalonamento do disco.
//

#include "Disco.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Disco {

    const char* Nome(const Algoritmo a) {
        switch (a) {
            case Algoritmo::FCFS: return "FCFS";
            case Algoritmo::SSTF: return "SSTF";
            case Algoritmo::SCAN: return "SCAN";
        }
        return "";
    }

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    float XDoCilindro(const int c, const float x, const float largura) {
        return x + largura * static_cast<float>(std::clamp(c, 0, kCilindros - 1)) / static_cast<float>(kCilindros - 1);
    }

    int CilindroDoX(const float px, const float x, const float largura) {
        if (largura <= 0.0f) return 0;
        const float u = (px - x) / largura;
        return std::clamp(static_cast<int>(std::lround(u * static_cast<float>(kCilindros - 1))), 0, kCilindros - 1);
    }

    int Alvo(const int cabeca, const std::vector<int>& pedidos, const Algoritmo a, int& sentido) {
        if (pedidos.empty()) return -1;
        if (a == Algoritmo::FCFS) return pedidos.front();
        if (a == Algoritmo::SSTF) {
            int melhor = pedidos.front();
            for (const int p : pedidos) {
                if (std::abs(p - cabeca) < std::abs(melhor - cabeca)) melhor = p;
            }
            return melhor;
        }
        // SCAN: o mais perto adiante; sem nada adiante, a ponta - e la o sentido vira.
        if (sentido == 0) sentido = -1;
        for (int volta = 0; volta < 2; ++volta) {
            int melhor = -1;
            for (const int p : pedidos) {
                const bool adiante = sentido > 0 ? p >= cabeca : p <= cabeca;
                if (adiante && (melhor < 0 || std::abs(p - cabeca) < std::abs(melhor - cabeca))) melhor = p;
            }
            if (melhor >= 0) return melhor;
            const int ponta = sentido > 0 ? kCilindros - 1 : 0;
            if (cabeca != ponta) return ponta;
            sentido = -sentido;
        }
        return -1;
    }

    int Movimento(int cabeca, std::vector<int> pedidos, const Algoritmo a, int sentido) {
        int total = 0;
        while (!pedidos.empty()) {
            const int alvo = Alvo(cabeca, pedidos, a, sentido);
            if (alvo < 0) break;
            total += std::abs(alvo - cabeca);
            cabeca = alvo;
            if (const auto it = std::find(pedidos.begin(), pedidos.end(), alvo); it != pedidos.end()) pedidos.erase(it);
        }
        return total;
    }

    std::string Placar(const Algoritmo a, const int andou, const int atendidos, const int fcfsSeria) {
        const std::string pedidos = std::to_string(atendidos) + (atendidos == 1 ? " pedido" : " pedidos");
        // \xc3\xa7 e o c cedilha; \xc2\xb7 o ponto do meio.
        if (a == Algoritmo::FCFS) {
            return std::string("FCFS: o bra\xc3\xa7o andou ") + std::to_string(andou) + " cilindros em " + pedidos;
        }
        return std::string(Nome(a)) + ": " + std::to_string(andou) + " cilindros em " + pedidos +
               "  \xc2\xb7  em FCFS seriam " + std::to_string(fcfsSeria);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entrePedidos = 0.45f;
        d.maisPedidos = 8;
        return d;
    }
}
