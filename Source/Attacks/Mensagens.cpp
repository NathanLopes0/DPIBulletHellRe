//
// A fase 3 do Vitor: o fork e a troca de mensagens.
//

#include "Mensagens.h"

#include <algorithm>

namespace Mensagens {

    namespace {
        // \xc2\xb7 e o ponto do meio, e "nao" e "sincrono" levam acento: o
        // MSVC nao le acento nem simbolo em literal.
        const char* const kPonto = " \xc2\xb7 ";

        int Outro(const int processo) { return processo == kPai ? kFilho : kPai; }
    }

    std::string Nome(const int processo) {
        return processo == kPai ? "pai" : "filho";
    }

    Rodada RodadaDaVez(const int k) {
        switch (((k % kRodadas) + kRodadas) % kRodadas) {
            case 0:  return Rodada::Assincrona;
            case 1:  return Rodada::Sincrona;
            case 2:  return Rodada::Rpc;
            default: return Rodada::Impasse;
        }
    }

    Chamada ChamadaDaVez(const int i) {
        return (((i % kChamadas) + kChamadas) % kChamadas) == 0 ? Chamada{2, 3} : Chamada{3, 4};
    }

    int Soma(const Chamada& c) { return c.a + c.b; }

    std::vector<Evento> Roteiro(const Rodada r) {
        std::vector<Evento> e;
        switch (r) {
            case Rodada::Assincrona:
                // O FILHO MANDA SEM ESPERAR, mais depressa do que o pai recebe:
                // a caixa postal enche, e depois esvazia.
                for (int i = 0; i < 12; ++i) {
                    e.push_back({0.3f + 0.18f * static_cast<float>(i), Acontece::Envia, kFilho, 0, 1.3f, false});
                }
                for (int j = 0; j < 12; ++j) {
                    e.push_back({1.7f + 0.25f * static_cast<float>(j), Acontece::Recebe, kPai});
                }
                break;
            case Rodada::Sincrona:
                // O PAI MANDA E ESPERA: quando o envelope chega, o rendezvous.
                for (const float t : {0.3f, 1.75f, 3.2f}) {
                    e.push_back({t, Acontece::Envia, kPai, 0, 1.1f, false});
                    e.push_back({t + 1.1f, Acontece::Encontro, kPai});
                }
                break;
            case Rodada::Rpc: {
                // A CHAMADA REMOTA: o pedido vai, o filho calcula, a resposta
                // volta - e o pai so segue quando ela chega.
                float t = 0.3f;
                for (int i = 0; i < kChamadas; ++i) {
                    const int valor = Soma(ChamadaDaVez(i));
                    e.push_back({t, Acontece::Envia, kPai, valor, 0.85f, false});
                    e.push_back({t + 1.15f, Acontece::Responde, kFilho, valor, 0.85f, false});
                    e.push_back({t + 2.0f, Acontece::Resultado, kPai, valor});
                    t += 2.1f;
                }
                break;
            }
            case Rodada::Impasse:
                // OS DOIS MANDAM E OS DOIS ESPERAM: ninguem recebe.
                e.push_back({0.3f, Acontece::Envia, kPai, 0, 1.1f, true});
                e.push_back({0.3f, Acontece::Envia, kFilho, 0, 1.1f, true});
                e.push_back({2.3f, Acontece::Estoura, kPai});
                // E DEPOIS DO TIMEOUT, os dois mandam sem esperar.
                for (int i = 0; i < 4; ++i) {
                    const float t = 2.6f + 0.25f * static_cast<float>(i);
                    e.push_back({t, Acontece::Envia, kPai, 0, 1.0f, false});
                    e.push_back({t, Acontece::Envia, kFilho, 0, 1.0f, false});
                }
                for (int j = 0; j < 4; ++j) {
                    const float t = 3.7f + 0.25f * static_cast<float>(j);
                    e.push_back({t, Acontece::Recebe, kPai});
                    e.push_back({t, Acontece::Recebe, kFilho});
                }
                break;
        }
        std::stable_sort(e.begin(), e.end(), [](const Evento& a, const Evento& b) { return a.quando < b.quando; });
        return e;
    }

    Ponto Lugar(const int processo, const int k, const float largura) {
        // A FRACAO DA LARGURA E A ALTURA de cada um, rodada a rodada: o canal
        // muda de inclinacao, e o fundo do arco anda pelo campo.
        static const float kLugares[kRodadas][2][2] = {
            {{0.19f, 150.0f}, {0.81f, 150.0f}},
            {{0.28f, 210.0f}, {0.72f, 120.0f}},
            {{0.21f, 120.0f}, {0.79f, 220.0f}},
            {{0.17f, 170.0f}, {0.83f, 170.0f}},
        };
        const int r = ((k % kRodadas) + kRodadas) % kRodadas;
        const int p = processo == kPai ? 0 : 1;
        return {kLugares[r][p][0] * largura, kLugares[r][p][1]};
    }

    Ponto NoArco(const Ponto de, const Ponto para, const float s, const float fundo) {
        const float u = std::clamp(s, 0.0f, 1.0f);
        const float meio = (de.y + para.y) / 2.0f;
        const float flecha = std::max(0.0f, fundo - meio);
        return {de.x + (para.x - de.x) * u, de.y + (para.y - de.y) * u + 4.0f * flecha * u * (1.0f - u)};
    }

    float Fundo(const Rodada r) {
        switch (r) {
            case Rodada::Assincrona: return 700.0f;
            case Rodada::Sincrona:   return 520.0f;
            case Rodada::Rpc:        return 560.0f;
            case Rodada::Impasse:    return 700.0f;
        }
        return 560.0f;
    }

    int NaCaixa(const std::vector<Evento>& roteiro, const int processo, const float u) {
        int n = 0;
        for (const Evento& e : roteiro) {
            if (e.o == Acontece::Envia && e.de == Outro(processo) && !e.paraNaPorta && e.quando + e.voo <= u) ++n;
            if (e.o == Acontece::Recebe && e.de == processo && e.quando <= u) --n;
        }
        return std::max(0, n);
    }

    std::string Placar(const Rodada r, const int naCaixa, const int chamada, const bool bloqueado, const bool estourou) {
        switch (r) {
            case Rodada::Assincrona:
                return "send sem esperar" + std::string(kPonto) + "caixa do pai: " + std::to_string(naCaixa);
            case Rodada::Sincrona:
                return bloqueado ? "send s\xc3\xadncrono" + std::string(kPonto) + "pai bloqueado" : std::string("rendezvous");
            case Rodada::Rpc: {
                const Chamada c = ChamadaDaVez(chamada);
                if (bloqueado) {
                    return "r = soma(" + std::to_string(c.a) + ", " + std::to_string(c.b) + ")" + kPonto + "rpc no filho";
                }
                return "return " + std::to_string(Soma(c));
            }
            case Rodada::Impasse:
                return estourou ? "timeout" + std::string(kPonto) + "send sem esperar"
                                : "send() e send()" + std::string(kPonto) + "deadlock";
        }
        return "";
    }
}
