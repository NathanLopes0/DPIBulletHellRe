//
// O jogo rodando sozinho, para medir e fotografar.
//

#include "Simulacao.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <limits>

namespace Simulacao {

    namespace {

        bool LerInteiro(const std::string& texto, long& valor) {
            if (texto.empty()) return false;
            char* fim = nullptr;
            errno = 0;
            valor = std::strtol(texto.c_str(), &fim, 10);
            return errno == 0 && fim == texto.c_str() + texto.size();
        }

        bool LerSemSinal(const std::string& texto, unsigned& valor) {
            if (texto.empty() || texto[0] == '-') return false;
            char* fim = nullptr;
            errno = 0;
            const unsigned long v = std::strtoul(texto.c_str(), &fim, 10);
            if (errno != 0 || fim != texto.c_str() + texto.size()) return false;
            valor = static_cast<unsigned>(v);
            return true;
        }

        /// Como o arnes antigo lia o passo (std::atof e a conversao para float): o mesmo numero,
        /// bit a bit, para as medidas de antes e de depois continuarem comparaveis.
        bool LerReal(const std::string& texto, float& valor) {
            if (texto.empty()) return false;
            char* fim = nullptr;
            const double v = std::strtod(texto.c_str(), &fim);
            if (fim != texto.c_str() + texto.size()) return false;
            valor = static_cast<float>(v);
            return true;
        }
    }

    bool Configuracao::Ativa() const {
        return !batalha.empty() || semente || variacao || passo > 0.0f || piloto != Piloto::Nenhum ||
               atira || poder || relatorio || sairNoFim || imortal || !teclas.empty() || !fotos.empty() ||
               larguraDaJanela > 0 || fimNoQuadro >= 0 || pausaNoQuadro >= 0 || focoNoQuadro >= 0 ||
               !estado.empty() || ajuste >= 0;
    }

    std::vector<int> LerQuadros(const std::string& lista, std::vector<std::string>* problemas) {
        std::vector<int> quadros;
        size_t i = 0;
        while (i <= lista.size()) {
            size_t j = lista.find(',', i);
            if (j == std::string::npos) j = lista.size();
            const std::string item = lista.substr(i, j - i);
            i = j + 1;
            if (item.empty()) continue;
            long q = 0;
            if (LerInteiro(item, q) && q >= 0) quadros.push_back(static_cast<int>(q));
            else if (problemas) problemas->push_back("quadro que nao se le: \"" + item + "\"");
        }
        return quadros;
    }

    std::vector<Tecla> LerTeclas(const std::string& roteiro, std::vector<std::string>* problemas) {
        std::vector<Tecla> teclas;
        size_t i = 0;
        while (i <= roteiro.size()) {
            size_t j = roteiro.find(',', i);
            if (j == std::string::npos) j = roteiro.size();
            const std::string item = roteiro.substr(i, j - i);
            i = j + 1;
            if (item.empty()) continue;
            const size_t arroba = item.rfind('@');
            long quadro = 0;
            if (arroba == std::string::npos || arroba == 0 || !LerInteiro(item.substr(arroba + 1), quadro) ||
                quadro < 0) {
                if (problemas) problemas->push_back("tecla sem quadro: \"" + item + "\"");
                continue;
            }
            teclas.push_back(Tecla{item.substr(0, arroba), static_cast<int>(quadro)});
        }
        return teclas;
    }

    bool Apertada(const Tecla& tecla, const int quadro) {
        return quadro >= tecla.quadro && quadro < tecla.quadro + kQuadrosDeTecla;
    }

    int Direcao(const float atual, const float alvo) {
        const float d = alvo - atual;
        if (d > 2.0f) return 1;
        if (d < -2.0f) return -1;
        return 0;
    }

    float SentidoDoVaiVem(const float x, const float sentido) {
        if (x > kVaiVemDireita) return -1.0f;
        if (x < kVaiVemEsquerda) return 1.0f;
        return sentido;
    }

    Rumo Desviar(const float x, const float y, const float raio, const float rapidez, const std::vector<Ameaca>& ameacas,
                 const float alvoX, const float alvoY, const Faixa& faixa, const Rumo anterior) {
        // Olha quatro instantes adiante; o mais perto pesa mais. A folga e a distancia em que um tiro
        // ja comeca a preocupar.
        constexpr float kOlhar[] = {0.08f, 0.18f, 0.3f, kOlharAdiante};
        constexpr float kPeso[] = {1.6f, 1.2f, 0.8f, 0.5f};
        constexpr float kFolga = 22.0f;
        Rumo melhor = anterior;
        float menor = std::numeric_limits<float>::max(), doAnterior = menor;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const float n = (dx != 0 && dy != 0) ? 0.70710678f : 1.0f;
                const auto onde = [&](const float t) {
                    return std::make_pair(std::clamp(x + static_cast<float>(dx) * n * rapidez * t, faixa.esquerda, faixa.direita),
                                          std::clamp(y + static_cast<float>(dy) * n * rapidez * t, faixa.topo, faixa.chao));
                };
                float custo = 0.0f;
                for (size_t k = 0; k < std::size(kOlhar); ++k) {
                    const float t = kOlhar[k];
                    const auto [px, py] = onde(t);
                    for (const Ameaca& a : ameacas) {
                        const float d = std::hypot(px - (a.x + a.vx * t), py - (a.y + a.vy * t)) - (raio + a.raio);
                        if (d >= kFolga) continue;
                        const float u = (kFolga - d) / kFolga;
                        custo += kPeso[k] * u * u * (d < 0.0f ? 4.0f : 1.0f);
                    }
                }
                // O alvo puxa de leve, e a parede empurra: encostado nela nao ha para onde fugir.
                const auto [fx, fy] = onde(0.3f);
                custo += 0.004f * std::fabs(fx - alvoX) + 0.002f * std::fabs(fy - alvoY);
                const float margem = std::min({fx - faixa.esquerda, faixa.direita - fx, fy - faixa.topo, faixa.chao - fy});
                if (margem < 30.0f) custo += 0.01f * (30.0f - margem);
                if (dx == anterior.dx && dy == anterior.dy) doAnterior = custo;
                if (custo < menor) {
                    menor = custo;
                    melhor = Rumo{dx, dy};
                }
            }
        }
        return doAnterior <= menor + 0.02f ? anterior : melhor;
    }

    Configuracao Ler(const std::vector<std::string>& argumentos, std::vector<std::string>* problemas) {
        Configuracao c;
        const auto problema = [&](const std::string& p) { if (problemas) problemas->push_back(p); };

        for (size_t i = 0; i < argumentos.size(); ++i) {
            const std::string& a = argumentos[i];
            // O valor, para quem tem um; sem ele, o parametro e ignorado e dito.
            const auto valor = [&](std::string& destino) {
                if (i + 1 >= argumentos.size()) { problema(a + " sem valor"); return false; }
                destino = argumentos[++i];
                return true;
            };
            std::string v;

            if (a == "--batalha") { valor(c.batalha); }
            else if (a == "--orientador") { valor(c.orientador); }
            else if (a == "--candidatos") { valor(c.candidatos); }
            else if (a == "--cena") { valor(c.cena); }
            else if (a == "--atividades") { valor(c.atividades); }
            else if (a == "--estado") { valor(c.estado); }
            else if (a == "--fotos-em") { valor(c.pastaDasFotos); }
            else if (a == "--dificil") { c.dificil = true; }
            else if (a == "--ajuste") {
                unsigned n = 0;
                if (!valor(v)) continue;
                if (LerSemSinal(v, n)) c.ajuste = static_cast<int>(n);
                else problema(a + " precisa de um numero inteiro sem sinal");
            }
            else if (a == "--atira") { c.atira = true; }
            else if (a == "--poder") { c.poder = true; }
            else if (a == "--relatorio") { c.relatorio = true; }
            else if (a == "--sair-no-fim") { c.sairNoFim = true; }
            else if (a == "--imortal") { c.imortal = true; }
            else if (a == "--fim-aprovado") { c.fimAprovado = true; }
            else if (a == "--jogadores") {
                long n = 0;
                if (!valor(v)) continue;
                if (LerInteiro(v, n) && (n == 1 || n == 2)) c.jogadores = static_cast<int>(n);
                else problema("--jogadores precisa de 1 ou 2");
            }
            else if (a == "--semente" || a == "--variacao") {
                unsigned n = 0;
                if (!valor(v)) continue;
                if (LerSemSinal(v, n)) (a == "--semente" ? c.semente : c.variacao) = n;
                else problema(a + " precisa de um numero inteiro sem sinal");
            }
            else if (a == "--passo") {
                float s = 0.0f;
                if (!valor(v)) continue;
                if (LerReal(v, s) && s > 0.0f) c.passo = s;
                else problema("--passo precisa de segundos maiores que zero");
            }
            else if (a == "--fim-nota") {
                float n = 0.0f;
                if (!valor(v)) continue;
                if (LerReal(v, n)) c.fimComNota = n;
                else problema("--fim-nota precisa de um numero");
            }
            else if (a == "--fim-em" || a == "--pausa-em" || a == "--foco-em") {
                long q = 0;
                if (!valor(v)) continue;
                if (LerInteiro(v, q) && q >= 0) {
                    (a == "--fim-em" ? c.fimNoQuadro : a == "--pausa-em" ? c.pausaNoQuadro : c.focoNoQuadro) =
                        static_cast<int>(q);
                } else problema(a + " precisa de um numero de quadro");
            }
            else if (a == "--piloto") {
                if (!valor(v)) continue;
                if (v == "segue") c.piloto = Piloto::Segue;
                else if (v == "parado") c.piloto = Piloto::Parado;
                else if (v == "vaivem") c.piloto = Piloto::VaiVem;
                else if (v == "esquiva") c.piloto = Piloto::Esquiva;
                else problema("piloto desconhecido: \"" + v + "\"");
            }
            else if (a == "--teclas") {
                if (valor(v)) {
                    for (const Tecla& t : LerTeclas(v, problemas)) c.teclas.push_back(t);
                }
            }
            else if (a == "--fotos") {
                if (valor(v)) {
                    for (const int q : LerQuadros(v, problemas)) c.fotos.push_back(q);
                }
            }
            else if (a == "--janela") {
                int l = 0, h = 0;
                char resto = 0;
                if (!valor(v)) continue;
                if (std::sscanf(v.c_str(), "%dx%d%c", &l, &h, &resto) == 2 && l > 0 && h > 0) {
                    c.larguraDaJanela = l;
                    c.alturaDaJanela = h;
                } else problema("--janela precisa de LARGURAxALTURA");
            }
            // O resto e de outro leitor (Gabinete::Ler), ou de ninguem.
        }
        return c;
    }
}
