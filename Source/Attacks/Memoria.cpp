//
// A gerencia de memoria da fase 3 do Luizinho.
//

#include "Memoria.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <deque>

namespace Memoria {

    namespace {
        Bloco* AcharMutavel(Heap& h, const int id) {
            for (Bloco& b : h.blocos) if (b.id == id) return &b;
            return nullptr;
        }

        constexpr std::array<Passo, kPassos> kRoteiro = {
            Passo::NovosAeB, Passo::NovoC, Passo::LacoJava, Passo::Coleta,
            Passo::NovoP, Passo::Libera, Passo::Escreve, Passo::LacoCpp};


        constexpr float kPi = 3.14159265358979f;
    }

    // ---- O HEAP -------------------------------------------------------------

    int Novo(Heap& h, const int celulas) {
        Bloco b;
        b.id = h.proximoId++;
        b.celulas = std::max(1, celulas);
        h.blocos.push_back(b);
        return b.id;
    }

    void Apontar(Heap& h, const std::string& raiz, const int id) {
        h.raizes[raiz] = id;
    }

    void ApontarProx(Heap& h, const int de, const int para) {
        if (Bloco* b = AcharMutavel(h, de)) b->prox = para;
    }

    void Liberar(Heap& h, const std::string& raiz) {
        const auto it = h.raizes.find(raiz);
        if (it == h.raizes.end()) return;
        if (Bloco* b = AcharMutavel(h, it->second)) b->estado = Estado::Liberado;
    }

    bool Pendente(const Heap& h, const std::string& raiz) {
        const auto it = h.raizes.find(raiz);
        if (it == h.raizes.end() || it->second < 0) return false;
        const Bloco* b = Achar(h, it->second);
        return b && b->estado == Estado::Liberado;
    }

    const Bloco* Achar(const Heap& h, const int id) {
        for (const Bloco& b : h.blocos) if (b.id == id) return &b;
        return nullptr;
    }

    std::set<int> Alcancaveis(const Heap& h) {
        std::set<int> marcados;
        std::deque<int> fila;
        for (const auto& [nome, id] : h.raizes) if (id >= 0) fila.push_back(id);
        while (!fila.empty()) {
            const int id = fila.front();
            fila.pop_front();
            const Bloco* b = Achar(h, id);
            // O LIBERADO NAO E SEGUIDO: o que o delete levou nao e memoria de
            // ninguem, mesmo com um ponteiro pendente ainda nele.
            if (!b || b->estado == Estado::Liberado || marcados.count(id)) continue;
            marcados.insert(id);
            if (b->prox >= 0) fila.push_back(b->prox);
        }
        return marcados;
    }

    std::vector<int> Lixo(const Heap& h) {
        const auto vivos = Alcancaveis(h);
        std::vector<int> lixo;
        for (const Bloco& b : h.blocos) {
            if (b.estado == Estado::Vivo && vivos.count(b.id) == 0) lixo.push_back(b.id);
        }
        return lixo;
    }

    std::vector<int> Varrer(Heap& h) {
        const auto lixo = Lixo(h);
        h.blocos.erase(std::remove_if(h.blocos.begin(), h.blocos.end(), [&lixo](const Bloco& b) {
            return std::find(lixo.begin(), lixo.end(), b.id) != lixo.end();
        }), h.blocos.end());
        return lixo;
    }

    std::vector<int> Vazar(Heap& h) {
        const auto lixo = Lixo(h);
        for (const int id : lixo) if (Bloco* b = AcharMutavel(h, id)) b->estado = Estado::Vazou;
        return lixo;
    }

    // ---- O ROTEIRO ----------------------------------------------------------

    Linguagem LinguagemDe(const Passo p) {
        switch (p) {
            case Passo::NovosAeB:
            case Passo::NovoC:
            case Passo::LacoJava:
            case Passo::Coleta:
                return Linguagem::Java;
            default:
                return Linguagem::Cpp;
        }
    }

    std::vector<Evento> Eventos(const Passo p) { return Eventos(p, Degrau{}); }

    std::vector<Evento> Eventos(const Passo p, const Degrau& d) { return Eventos(p, d, NumerosDeSempre()); }

    std::vector<Evento> Eventos(const Passo p, const Degrau& d, const Numeros& n) {
        std::vector<Evento> e;
        auto alocar = [&e](const float quando, const int celulas, const std::string& raiz, const bool noProx) {
            Evento ev;
            ev.quando = quando;
            ev.acao = Acao::Alocar;
            ev.celulas = celulas;
            ev.raiz = raiz;
            ev.noProx = noProx;
            e.push_back(ev);
        };
        auto so = [&e](const float quando, const Acao acao, const std::string& raiz, const int indice) {
            Evento ev;
            ev.quando = quando;
            ev.acao = acao;
            ev.raiz = raiz;
            ev.indice = indice;
            e.push_back(ev);
        };
        switch (p) {
            case Passo::NovosAeB:
                alocar(0.3f, 2, "a", false);
                alocar(1.2f, 2, "a", true);
                break;
            case Passo::NovoC:
                alocar(0.3f, 2, "a", false);
                break;
            case Passo::LacoJava:
                for (int k = 0; k < n.noLacoJava; ++k) {
                    alocar(kPrimeiraAlocacao + static_cast<float>(k) * d.entreAlocacoes, n.celulasDoLacoJava, "", false);
                }
                break;
            case Passo::Coleta:
                so(0.3f, Acao::Parar, "", 0);
                so(1.0f, Acao::Varrer, "", 0);
                break;
            case Passo::NovoP:
                alocar(0.3f, n.celulasDeP, "p", false);
                break;
            case Passo::Libera:
                so(0.3f, Acao::Liberar, "p", 0);
                break;
            case Passo::Escreve:
                so(0.5f, Acao::Escrever, "p", n.indice);
                break;
            case Passo::LacoCpp:
                for (int k = 0; k < d.noLacoCpp; ++k) {
                    alocar(kPrimeiraAlocacao + static_cast<float>(k) * d.entreAlocacoes, n.celulasDoLacoCpp, "", false);
                }
                so(kEstouro, Acao::Estourar, "", 0);
                break;
        }
        return e;
    }

    float Duracao(const Passo p) {
        switch (p) {
            case Passo::NovosAeB: return 2.4f;
            case Passo::NovoC:    return 2.0f;
            case Passo::LacoJava: return 2.6f;
            case Passo::Coleta:   return 2.5f;
            case Passo::NovoP:    return 2.0f;
            case Passo::Libera:   return 1.8f;
            case Passo::Escreve:  return 2.2f;
            case Passo::LacoCpp:  return 4.0f;
        }
        return 0.0f;
    }

    float Inicio(const Passo p) {
        float t = kPrimeiro;
        for (const Passo q : kRoteiro) {
            if (q == p) return t;
            t += Duracao(q);
        }
        return t;
    }

    int PassoEm(const float t) { return PassoEm(t, kRoteiro); }

    Roteiro RoteiroDeSempre() { return kRoteiro; }

    Roteiro Sortear(Variacao::Sorteio& s) {
        if (!s.Chance(0.5f)) return kRoteiro;
        // O C++ PRIMEIRO: as duas metades trocam de lugar, cada uma inteira.
        constexpr int kMetade = kPassos / 2;
        Roteiro r{};
        for (int i = 0; i < kPassos; ++i) r[static_cast<size_t>(i)] = kRoteiro[static_cast<size_t>((i + kMetade) % kPassos)];
        return r;
    }

    float InicioDaVez(const int i, const Roteiro& roteiro) {
        float t = kPrimeiro;
        for (int k = 0; k < i && k < kPassos; ++k) t += Duracao(roteiro[static_cast<size_t>(k)]);
        return t;
    }

    int PassoEm(const float t, const Roteiro& roteiro) {
        if (t < kPrimeiro) return -1;
        float inicio = kPrimeiro;
        for (int i = 0; i < kPassos; ++i) {
            inicio += Duracao(roteiro[static_cast<size_t>(i)]);
            if (t < inicio) return i;
        }
        return kPassos - 1;
    }

    bool TrocaDeLinguagem(const int i, const Roteiro& roteiro) {
        if (i <= 0 || i >= kPassos) return false;
        return LinguagemDe(roteiro[static_cast<size_t>(i)]) != LinguagemDe(roteiro[static_cast<size_t>(i - 1)]);
    }

    std::string Placar(const Passo p, const bool depois) { return Placar(p, depois, NumerosDeSempre()); }

    std::string Placar(const Passo p, const bool depois, const Numeros& n) {
        // \xc2\xb7 e o ponto do meio, \xc3\xaa o e circunflexo, \xc3\xa1 o a
        // agudo e \xe2\x86\x92 a seta: o MSVC nao le acento em literal.
        const std::string cabecalho = LinguagemDe(p) == Linguagem::Java
            ? "Java \xc2\xb7 coletor de lixo"
            : "C++ \xc2\xb7 ger\xc3\xaa" "ncia manual";
        std::string codigo;
        std::string fim;
        switch (p) {
            case Passo::NovosAeB: codigo = "No a = new No();  a.prox = new No();"; break;
            case Passo::NovoC:    codigo = "a = new No();"; break;
            case Passo::LacoJava:
                codigo = "for (int i = 0; i < " + std::to_string(n.noLacoJava) + "; i++) new int[" +
                         std::to_string(n.celulasDoLacoJava) + "];";
                break;
            case Passo::Coleta:   codigo = "System.gc();"; fim = "  // stop the world"; break;
            case Passo::NovoP:    codigo = "int* p = new int[" + std::to_string(n.celulasDeP) + "];"; break;
            case Passo::Libera:   codigo = "delete[] p;"; fim = "  // p ainda aponta para l\xc3\xa1"; break;
            case Passo::Escreve:
                codigo = "p[" + std::to_string(n.indice) + "] = " + std::to_string(n.valor) + ";";
                fim = "  \xe2\x86\x92  comportamento indefinido"; break;
            case Passo::LacoCpp:
                codigo = "while (1) new int[" + std::to_string(n.celulasDoLacoCpp) + "];";
                fim = "  \xe2\x86\x92  bad_alloc"; break;
        }
        return cabecalho + "\n" + codigo + (depois ? fim : "");
    }

    // ---- A GEOMETRIA --------------------------------------------------------

    bool Retangulo::Cruza(const Retangulo& o) const {
        return x < o.x + o.w && o.x < x + w && y < o.y + o.h && o.y < y + h;
    }

    Retangulo Area(const Ponto centro, const int celulas) {
        const float w = static_cast<float>(std::max(1, celulas) - 1) * kEntreCelulas + kLadoDaCelula;
        const float h = kLadoDaCelula;
        return {centro.x - w / 2.0f, centro.y - h / 2.0f, w, h};
    }

    std::vector<Ponto> Celulas(const Ponto centro, const int celulas) {
        std::vector<Ponto> c;
        const int n = std::max(1, celulas);
        for (int i = 0; i < n; ++i) {
            c.push_back({centro.x + (static_cast<float>(i) - static_cast<float>(n - 1) / 2.0f) * kEntreCelulas, centro.y});
        }
        return c;
    }

    Ponto OndeAlocar(const int celulas, const Ponto aluno, const std::vector<Retangulo>& ocupados,
                     const float largura, const float altura) {
        const Retangulo molde = Area({0.0f, 0.0f}, celulas);
        const float xMin = kMargem + molde.w / 2.0f;
        const float xMax = largura - kMargem - molde.w / 2.0f;
        const float yMin = kAlturaMinima + molde.h / 2.0f;
        const float yMax = altura - kMargem - molde.h / 2.0f;

        auto livre = [&](const Ponto c) {
            Retangulo r = Area(c, celulas);
            r = {r.x - kFolga, r.y - kFolga, r.w + 2.0f * kFolga, r.h + 2.0f * kFolga};
            return std::none_of(ocupados.begin(), ocupados.end(), [&r](const Retangulo& o) { return r.Cruza(o); });
        };
        auto dentro = [&](const Ponto c) {
            return Ponto{std::clamp(c.x, xMin, xMax), std::clamp(c.y, yMin, yMax)};
        };

        // AS DIRECOES, de lado primeiro: o bloco vizinho fica na mesma altura,
        // e o aluno que anda de lado continua tendo para onde ir.
        constexpr std::array<float, 12> kAngulos = {0.0f, 180.0f, 30.0f, 150.0f, 330.0f, 210.0f,
                                                    60.0f, 120.0f, 300.0f, 240.0f, 90.0f, 270.0f};
        const Ponto meio = dentro(aluno);
        if (livre(meio)) return meio;
        for (int anel = 1; anel <= 30; ++anel) {
            const float r = 36.0f * static_cast<float>(anel);
            for (const float a : kAngulos) {
                const float rad = a * kPi / 180.0f;
                const Ponto c = dentro({aluno.x + r * std::cos(rad), aluno.y + r * std::sin(rad)});
                if (livre(c)) return c;
            }
        }
        return meio;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.avisoDaAlocacao = Dificuldade::Escolher(n, kAvisoDaAlocacao, 0.55f);
        d.entreAlocacoes = Dificuldade::Escolher(n, kEntreAlocacoes, 0.24f);
        d.noLacoCpp = Dificuldade::Escolher(n, kNoLacoCpp, 9);
        d.estilhacosDoPendente = Dificuldade::Escolher(n, kEstilhacosDoPendente, 20);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.2f);
        return d;
    }
    Numeros NumerosDeSempre() { return Numeros{}; }

    Numeros SortearNumeros(Variacao::Sorteio& s) {
        Numeros n;
        n.noLacoJava = s.Inteiro(4, 7);
        n.celulasDoLacoJava = s.Inteiro(2, 4);
        n.celulasDeP = s.Inteiro(3, 5);
        n.indice = s.Inteiro(0, n.celulasDeP - 1);
        n.valor = s.Inteiro(1, 9);
        n.celulasDoLacoCpp = s.Inteiro(3, 5);
        return n;
    }
}
