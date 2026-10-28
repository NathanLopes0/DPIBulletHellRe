//
// A final do Vitor: o jantar dos filosofos.
//

#include "Filosofos.h"

#include <algorithm>
#include <cmath>

namespace Filosofos {

    namespace {
        // \xc2\xb7 e o ponto do meio, e os nomes levam acento: o MSVC nao le
        // acento nem simbolo em literal.
        const char* const kPonto = " \xc2\xb7 ";

        constexpr float kGrau = 3.14159265f / 180.0f;

        int Antes(const int f) { return (f + kFilosofos - 1) % kFilosofos; }

        bool Vivo(const Mesa& m, const int f) {
            return m.estado[static_cast<size_t>(f)] != Estado::Morto
                && m.estado[static_cast<size_t>(f)] != Estado::DeFora;
        }

        bool DoLado(const int f, const int g) {
            return g == GarfoDaEsquerda(f) || g == GarfoDaDireita(f);
        }

        void SoltarTudo(Mesa& m, const int f) {
            for (int& dono : m.dono) if (dono == f) dono = -1;
            m.quer[static_cast<size_t>(f)] = -1;
        }

        Ponto NoCirculo(const float largura, const float graus, const float raio) {
            const Ponto c = Centro(largura);
            return {c.x + raio * std::cos(graus * kGrau), c.y + raio * std::sin(graus * kGrau)};
        }
    }

    std::string Nome(const int f) {
        switch (((f % kFilosofos) + kFilosofos) % kFilosofos) {
            case 0:  return "S\xc3\xb3" "crates";
            case 1:  return "Plat\xc3\xa3o";
            case 2:  return "Arist\xc3\xb3teles";
            case 3:  return "Kant";
            default: return "Descartes";
        }
    }

    int GarfoDaEsquerda(const int f) { return ((f % kFilosofos) + kFilosofos) % kFilosofos; }
    int GarfoDaDireita(const int f) { return (GarfoDaEsquerda(f) + 1) % kFilosofos; }

    Rodada RodadaDaVez(const int k) {
        switch (((k % kRodadas) + kRodadas) % kRodadas) {
            case 0:  return Rodada::Jantar;
            case 1:  return Rodada::Deadlock;
            case 2:  return Rodada::Ordem;
            default: return Rodada::Garcom;
        }
    }

    std::vector<Passo> Roteiro(const Rodada r, const int escolhido) {
        std::vector<Passo> p;
        const int x = GarfoDaEsquerda(escolhido);   // so normaliza o indice
        switch (r) {
            case Rodada::Jantar: {
                // DOIS DE CADA VEZ, sem vizinhos: ninguem disputa garfo.
                const int pares[3][2] = {{0, 2}, {1, 3}, {4, -1}};
                const float comeca[3] = {0.3f, 1.85f, 3.4f};
                const float dura[3] = {1.2f, 1.2f, 1.1f};
                for (int k = 0; k < 3; ++k) {
                    for (const int f : pares[k]) {
                        if (f < 0) continue;
                        p.push_back({comeca[k], f, Acao::Pega, GarfoDaEsquerda(f)});
                        p.push_back({comeca[k], f, Acao::Pega, GarfoDaDireita(f)});
                    }
                    for (const int f : pares[k]) if (f >= 0) p.push_back({comeca[k] + 0.15f, f, Acao::Come});
                    for (const int f : pares[k]) if (f >= 0) p.push_back({comeca[k] + 0.15f + dura[k], f, Acao::Solta});
                }
                break;
            }
            case Rodada::Deadlock: {
                // TODOS PEGAM O DA ESQUERDA, e todos esperam o da direita.
                for (int f = 0; f < kFilosofos; ++f) p.push_back({0.3f, f, Acao::Pega, GarfoDaEsquerda(f)});
                for (int f = 0; f < kFilosofos; ++f) p.push_back({0.75f, f, Acao::Espera, GarfoDaDireita(f)});
                // kill -9: a vitima solta o garfo dela, e quem o esperava come.
                const int a = Antes(x);
                const int b = Antes(a);
                p.push_back({2.1f, x, Acao::Morre});
                p.push_back({2.3f, a, Acao::Pega, GarfoDaDireita(a)});
                p.push_back({2.45f, a, Acao::Come});
                p.push_back({3.5f, a, Acao::Solta});
                p.push_back({3.65f, b, Acao::Pega, GarfoDaDireita(b)});
                p.push_back({3.8f, b, Acao::Come});
                p.push_back({4.7f, b, Acao::Solta});
                break;
            }
            case Rodada::Ordem: {
                // O DE MENOR NUMERO PRIMEIRO: o ultimo quer o garfo 0, que o
                // primeiro pegou - e espera sem segurar nada.
                const int ultimo = kFilosofos - 1;
                for (int f = 0; f < ultimo; ++f) p.push_back({0.3f, f, Acao::Pega, GarfoDaEsquerda(f)});
                p.push_back({0.3f, ultimo, Acao::Espera, GarfoDaDireita(ultimo)});
                for (int f = 0; f < ultimo - 1; ++f) p.push_back({0.6f, f, Acao::Espera, GarfoDaDireita(f)});
                p.push_back({0.6f, ultimo - 1, Acao::Pega, GarfoDaDireita(ultimo - 1)});
                // e a fila anda de tras para a frente
                float t = 0.75f;
                for (int f = ultimo - 1; f >= 1; --f) {
                    if (f < ultimo - 1) {
                        p.push_back({t - 0.15f, f, Acao::Pega, GarfoDaDireita(f)});
                    }
                    p.push_back({t, f, Acao::Come});
                    p.push_back({t + 1.1f, f, Acao::Solta});
                    t += 1.4f;
                }
                break;
            }
            case Rodada::Garcom: {
                // O GARCOM TIRA UM DA MESA: com quatro, sempre sobra um garfo.
                p.push_back({0.2f, x, Acao::Levanta});
                for (int f = 0; f < kFilosofos; ++f) {
                    if (f != x) p.push_back({0.4f, f, Acao::Pega, GarfoDaEsquerda(f)});
                }
                const int a = Antes(x);
                for (int f = 0; f < kFilosofos; ++f) {
                    if (f != x && f != a) p.push_back({0.7f, f, Acao::Espera, GarfoDaDireita(f)});
                }
                p.push_back({0.7f, a, Acao::Pega, GarfoDaDireita(a)});
                float t = 0.85f;
                int f = a;
                for (int k = 0; k < 3; ++k) {
                    if (k > 0) p.push_back({t - 0.15f, f, Acao::Pega, GarfoDaDireita(f)});
                    p.push_back({t, f, Acao::Come});
                    p.push_back({t + 1.1f, f, Acao::Solta});
                    t += 1.4f;
                    f = Antes(f);
                }
                break;
            }
        }
        std::stable_sort(p.begin(), p.end(), [](const Passo& a, const Passo& b) { return a.quando < b.quando; });
        return p;
    }

    Mesa MesaPosta() {
        Mesa m;
        m.dono.fill(-1);
        m.estado.fill(Estado::Pensando);
        m.quer.fill(-1);
        return m;
    }

    bool Aplicar(Mesa& m, const Passo& p) {
        const int f = p.filosofo;
        if (f < 0 || f >= kFilosofos || !Vivo(m, f)) return false;
        const auto i = static_cast<size_t>(f);
        switch (p.acao) {
            case Acao::Pega: {
                if (p.garfo < 0 || p.garfo >= kFilosofos || !DoLado(f, p.garfo)) return false;
                if (m.dono[static_cast<size_t>(p.garfo)] != -1) return false;
                m.dono[static_cast<size_t>(p.garfo)] = f;
                if (m.quer[i] == p.garfo) m.quer[i] = -1;
                if (m.quer[i] < 0) m.estado[i] = Estado::Pensando;
                return true;
            }
            case Acao::Espera: {
                if (p.garfo < 0 || p.garfo >= kFilosofos || !DoLado(f, p.garfo)) return false;
                const int dono = m.dono[static_cast<size_t>(p.garfo)];
                if (dono == -1 || dono == f) return false;
                m.quer[i] = p.garfo;
                m.estado[i] = Estado::Esperando;
                return true;
            }
            case Acao::Come: {
                if (m.dono[static_cast<size_t>(GarfoDaEsquerda(f))] != f) return false;
                if (m.dono[static_cast<size_t>(GarfoDaDireita(f))] != f) return false;
                m.quer[i] = -1;
                m.estado[i] = Estado::Comendo;
                return true;
            }
            case Acao::Solta:
                SoltarTudo(m, f);
                m.estado[i] = Estado::Pensando;
                return true;
            case Acao::Morre:
                SoltarTudo(m, f);
                m.estado[i] = Estado::Morto;
                return true;
            case Acao::Levanta:
                for (const int dono : m.dono) if (dono == f) return false;
                m.quer[i] = -1;
                m.estado[i] = Estado::DeFora;
                return true;
        }
        return false;
    }

    std::vector<std::pair<int, int>> Esperas(const Mesa& m) {
        std::vector<std::pair<int, int>> e;
        for (int f = 0; f < kFilosofos; ++f) {
            const int g = m.quer[static_cast<size_t>(f)];
            if (g < 0) continue;
            const int dono = m.dono[static_cast<size_t>(g)];
            if (dono >= 0 && dono != f) e.emplace_back(f, dono);
        }
        return e;
    }

    bool EmDeadlock(const Mesa& m) {
        const auto e = Esperas(m);
        std::array<int, kFilosofos> proximo{};
        proximo.fill(-1);
        for (const auto& [quem, de] : e) proximo[static_cast<size_t>(quem)] = de;
        for (int inicio = 0; inicio < kFilosofos; ++inicio) {
            int f = proximo[static_cast<size_t>(inicio)];
            for (int passos = 0; f >= 0 && passos < kFilosofos; ++passos) {
                if (f == inicio) return true;
                f = proximo[static_cast<size_t>(f)];
            }
        }
        return false;
    }

    std::vector<int> Comendo(const Mesa& m) {
        std::vector<int> c;
        for (int f = 0; f < kFilosofos; ++f) if (m.estado[static_cast<size_t>(f)] == Estado::Comendo) c.push_back(f);
        return c;
    }

    Ponto Centro(const float largura) {
        return {largura / 2.0f, kAlturaDaMesa};
    }

    Ponto Lugar(const int f, const float largura) {
        return NoCirculo(largura, -90.0f + 72.0f * static_cast<float>(GarfoDaEsquerda(f)), kRaioDosFilosofos);
    }

    Ponto LugarDoGarfo(const int g, const float largura) {
        return NoCirculo(largura, -90.0f + 72.0f * static_cast<float>(GarfoDaEsquerda(g)) - 36.0f, kRaioDosGarfos);
    }

    Ponto NaMao(const int f, const int g, const float largura) {
        const Ponto quem = Lugar(f, largura);
        const Ponto garfo = LugarDoGarfo(g, largura);
        return {quem.x + (garfo.x - quem.x) * 0.42f, quem.y + (garfo.y - quem.y) * 0.42f};
    }

    std::vector<Ponto> Cerco(const int de, const int para, const float largura, const int porAresta) {
        const Ponto a = Lugar(de, largura);
        const Ponto b = Lugar(para, largura);
        std::vector<Ponto> pontos;
        for (int i = 1; i <= porAresta; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(porAresta + 1);
            pontos.push_back({a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t});
        }
        return pontos;
    }

    std::string Placar(const Rodada r, const Mesa& m) {
        return Placar(r, m, PlanoDeSempre());
    }

    std::string Pensador(const int i) {
        switch (((i % kPensadores) + kPensadores) % kPensadores) {
            case 0:  return "S\xc3\xb3" "crates";
            case 1:  return "Plat\xc3\xa3o";
            case 2:  return "Arist\xc3\xb3teles";
            case 3:  return "Kant";
            case 4:  return "Descartes";
            case 5:  return "Hume";
            case 6:  return "Hip\xc3\xa1tia";
            case 7:  return "Conf\xc3\xba" "cio";
            case 8:  return "Espinosa";
            default: return "Beauvoir";
        }
    }

    Plano PlanoDeSempre() {
        Plano p;
        for (int k = 0; k < kRodadas; ++k) p.rodadas[static_cast<size_t>(k)] = RodadaDaVez(k);
        for (int f = 0; f < kFilosofos; ++f) p.nomes[static_cast<size_t>(f)] = Nome(f);
        return p;
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p = PlanoDeSempre();
        if (s.Chance(0.5f)) std::swap(p.rodadas[2], p.rodadas[3]);
        std::vector<int> quem;
        for (int i = 0; i < kPensadores; ++i) quem.push_back(i);
        s.Embaralhar(quem);
        for (int f = 0; f < kFilosofos; ++f) p.nomes[static_cast<size_t>(f)] = Pensador(quem[static_cast<size_t>(f)]);
        p.giro = s.Inteiro(0, kFilosofos - 1);
        return p;
    }

    std::vector<Passo> Rodar(std::vector<Passo> passos, const int giro) {
        const int g = ((giro % kFilosofos) + kFilosofos) % kFilosofos;
        for (Passo& p : passos) {
            p.filosofo = (p.filosofo + g) % kFilosofos;
            if (p.garfo >= 0) p.garfo = (p.garfo + g) % kFilosofos;
        }
        return passos;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreGarfadas = Dificuldade::Escolher(n, kEntreGarfadas, 0.33f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.15f);
        d.porAresta = Dificuldade::Escolher(n, kPorAresta, 6);
        d.velocidadeDoEstouro = Dificuldade::Escolher(n, kVelocidadeDoEstouro, 260.0f);
        return d;
    }

    std::string Placar(const Rodada r, const Mesa& m, const Plano& plano) {
        if (EmDeadlock(m)) return "deadlock" + std::string(kPonto) + "espera circular";
        auto nome = [&plano](const int f) { return plano.nomes[static_cast<size_t>(f)]; };

        std::string texto;
        switch (r) {
            case Rodada::Jantar:   texto = "jantar"; break;
            case Rodada::Deadlock: texto = "todos pegam o garfo da esquerda"; break;
            case Rodada::Ordem:    texto = "menor garfo primeiro"; break;
            case Rodada::Garcom:   texto = "gar\xc3\xa7om: 4 \xc3\xa0 mesa"; break;
        }
        for (int f = 0; f < kFilosofos; ++f) {
            if (m.estado[static_cast<size_t>(f)] == Estado::Morto) texto = "kill -9 " + nome(f);
        }
        const auto c = Comendo(m);
        if (!c.empty()) {
            texto += std::string(kPonto) + "comendo:";
            for (size_t i = 0; i < c.size(); ++i) texto += (i == 0 ? " " : ", ") + nome(c[i]);
        }
        return texto;
    }
}
