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
        return Roteiro(r, PlanoDeSempre(), Degrau{});
    }

    std::vector<Evento> Roteiro(const Rodada r, const Plano& plano, const Degrau& d) {
        std::vector<Evento> e;
        switch (r) {
            case Rodada::Assincrona:
                // O FILHO MANDA SEM ESPERAR, mais depressa do que o pai recebe:
                // a caixa postal enche, e depois esvazia.
                for (int i = 0; i < d.assincronos; ++i) {
                    e.push_back({0.3f + d.entreEnvios * static_cast<float>(i), Acontece::Envia, kFilho, 0, 1.3f, false});
                }
                for (int j = 0; j < d.assincronos; ++j) {
                    e.push_back({1.7f + d.entreRecebidos * static_cast<float>(j), Acontece::Recebe, kPai});
                }
                break;
            case Rodada::Sincrona:
                // O PAI MANDA E ESPERA: quando o envelope chega, o rendezvous.
                for (int i = 0; i < d.sincronos; ++i) {
                    const float t = 0.3f + d.entreSincronos * static_cast<float>(i);
                    e.push_back({t, Acontece::Envia, kPai, 0, d.vooSincrono, false});
                    e.push_back({t + d.vooSincrono, Acontece::Encontro, kPai});
                }
                break;
            case Rodada::Rpc: {
                // A CHAMADA REMOTA: o pedido vai, o filho calcula, a resposta
                // volta - e o pai so segue quando ela chega.
                float t = 0.3f;
                for (int i = 0; i < kChamadas; ++i) {
                    const int valor = Soma(plano.chamadas[static_cast<size_t>(i)]);
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
                for (int i = 0; i < d.depoisDoTimeout; ++i) {
                    const float t = 2.6f + d.entreDepoisDoTimeout * static_cast<float>(i);
                    e.push_back({t, Acontece::Envia, kPai, 0, 1.0f, false});
                    e.push_back({t, Acontece::Envia, kFilho, 0, 1.0f, false});
                }
                for (int j = 0; j < d.depoisDoTimeout; ++j) {
                    const float t = 3.7f + d.entreDepoisDoTimeout * static_cast<float>(j);
                    e.push_back({t, Acontece::Recebe, kPai});
                    e.push_back({t, Acontece::Recebe, kFilho});
                }
                break;
        }
        std::stable_sort(e.begin(), e.end(), [](const Evento& a, const Evento& b) { return a.quando < b.quando; });
        return e;
    }

    namespace {
        // A FRACAO DA LARGURA E A ALTURA de cada um, rodada a rodada: o canal
        // muda de inclinacao, e o fundo do arco anda pelo campo.
        constexpr float kLugares[kRodadas][2][2] = {
            {{0.19f, 150.0f}, {0.81f, 150.0f}},
            {{0.28f, 210.0f}, {0.72f, 120.0f}},
            {{0.21f, 120.0f}, {0.79f, 220.0f}},
            {{0.17f, 170.0f}, {0.83f, 170.0f}},
        };

        /// Quanto um lugar sorteado pode sair do de sempre.
        constexpr float kFolgaEmX = 0.04f;
        constexpr float kFolgaEmY = 30.0f;
        constexpr float kMaisAlto = 120.0f;     ///< abaixo do placar, com o cartao do professor
        constexpr float kMaisBaixo = 250.0f;
    }

    Ponto Lugar(const int processo, const int k, const float largura) {
        return Lugar(processo, k, largura, PlanoDeSempre());
    }

    Ponto Lugar(const int processo, const int k, const float largura, const Plano& plano) {
        const int r = ((k % kRodadas) + kRodadas) % kRodadas;
        const int p = processo == kPai ? 0 : 1;
        const Ponto l = plano.lugares[static_cast<size_t>(r)][static_cast<size_t>(p)];
        return {l.x * largura, l.y};
    }

    Plano PlanoDeSempre() {
        Plano p;
        for (int k = 0; k < kRodadas; ++k) {
            p.rodadas[static_cast<size_t>(k)] = RodadaDaVez(k);
            for (int q = 0; q < 2; ++q) {
                p.lugares[static_cast<size_t>(k)][static_cast<size_t>(q)] = {kLugares[k][q][0], kLugares[k][q][1]};
            }
        }
        for (int i = 0; i < kChamadas; ++i) p.chamadas[static_cast<size_t>(i)] = ChamadaDaVez(i);
        return p;
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p = PlanoDeSempre();
        std::vector<Rodada> resto(p.rodadas.begin() + 1, p.rodadas.end());
        s.Embaralhar(resto);
        for (size_t i = 0; i < resto.size(); ++i) p.rodadas[i + 1] = resto[i];

        // DUAS SOMAS DIFERENTES, cada uma partida em dois numeros.
        std::vector<int> somas(std::begin(kSomas), std::end(kSomas));
        s.Embaralhar(somas);
        for (int i = 0; i < kChamadas; ++i) {
            const int soma = somas[static_cast<size_t>(i)];
            const int a = s.Inteiro(1, soma - 1);
            p.chamadas[static_cast<size_t>(i)] = {a, soma - a};
        }

        for (auto& par : p.lugares) {
            for (Ponto& l : par) {
                l.x += s.Real(-kFolgaEmX, kFolgaEmX);
                l.y = std::clamp(l.y + s.Real(-kFolgaEmY, kFolgaEmY), kMaisAlto, kMaisBaixo);
            }
        }
        return p;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n == Dificuldade::Nivel::Dificil) {
            d.sincronos = 4;
            d.vooSincrono = 0.9f;
            d.entreSincronos = 1.0f;
            d.lequeDoRecebido = 3;
            d.aberturaDoRecebido = 12.0f;
            d.assincronos = 15;
            d.entreEnvios = 0.13f;
            d.entreRecebidos = 0.18f;
            d.depoisDoTimeout = 6;
            d.entreDepoisDoTimeout = 0.15f;
            d.anelDoEncontro = 10;
            d.rapidez = 1.25f;
        }
        return d;
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
        return Placar(r, naCaixa, chamada, bloqueado, estourou, PlanoDeSempre());
    }

    std::string Placar(const Rodada r, const int naCaixa, const int chamada, const bool bloqueado, const bool estourou,
                       const Plano& plano) {
        switch (r) {
            case Rodada::Assincrona:
                return "send sem esperar" + std::string(kPonto) + "caixa do pai: " + std::to_string(naCaixa);
            case Rodada::Sincrona:
                return bloqueado ? "send s\xc3\xadncrono" + std::string(kPonto) + "pai bloqueado" : std::string("rendezvous");
            case Rodada::Rpc: {
                const Chamada c = plano.chamadas[static_cast<size_t>(((chamada % kChamadas) + kChamadas) % kChamadas)];
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
