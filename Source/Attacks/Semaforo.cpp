//
// A fase 1 do Vitor: os semaforos.
//

#include "Semaforo.h"

#include <algorithm>
#include <cmath>

namespace Semaforo {

    namespace {
        // \xc2\xb7 e o ponto do meio: o MSVC nao le simbolo em literal.
        const char* const kPonto = " \xc2\xb7 ";

        /// Um sorteador pequeno e deterministico (xorshift): a mesma semente
        /// da a mesma rodada, que e o que o teste e o arnes precisam.
        std::uint32_t Proximo(std::uint32_t& s) {
            if (s == 0) s = 0x9e3779b9u;
            s ^= s << 13;
            s ^= s >> 17;
            s ^= s << 5;
            return s;
        }

        bool Tem(const std::vector<int>& v, const int t) {
            return std::find(v.begin(), v.end(), t) != v.end();
        }

        void Entrar(Escalonador& e, const int t, const float quando) {
            e.dentro.push_back(t);
            e.entrouEm.push_back(quando);
        }
    }

    Rodada RodadaDaVez(const int k) {
        switch (((k % kRodadas) + kRodadas) % kRodadas) {
            case 0:  return {Tipo::Mutex, 1};
            case 1:  return {Tipo::Contador, 2};
            case 2:  return {Tipo::EsperaOcupada, 1};
            default: return {Tipo::Corrida, 0};
        }
    }

    std::vector<float> Colunas(const float xDoAluno, const float largura, const float entreThreads) {
        const float passo = entreThreads > 1.0f ? entreThreads : kEntreThreads;
        const float de = kMargem;
        const float ate = largura - kMargem;
        if (ate < de) return {largura / 2.0f};
        const float meio = std::clamp(xDoAluno, de, ate);

        // Todas as posicoes da grade que passa pelo aluno, e delas as
        // kMaisThreads mais perto dele.
        std::vector<float> todas;
        float x = meio - std::floor((meio - de) / passo) * passo;
        for (; x <= ate + 0.01f; x += passo) todas.push_back(x);
        std::stable_sort(todas.begin(), todas.end(), [meio](const float a, const float b) {
            return std::fabs(a - meio) < std::fabs(b - meio);
        });
        if (static_cast<int>(todas.size()) > kMaisThreads) todas.resize(static_cast<size_t>(kMaisThreads));
        std::sort(todas.begin(), todas.end());
        return todas;
    }

    int MaisPerto(const std::vector<float>& colunas, const float x) {
        int melhor = -1;
        float menor = 0.0f;
        for (int i = 0; i < static_cast<int>(colunas.size()); ++i) {
            const float d = std::fabs(colunas[static_cast<size_t>(i)] - x);
            if (melhor < 0 || d < menor) { melhor = i; menor = d; }
        }
        return melhor;
    }

    std::string Nome(const int i) {
        return "T" + std::to_string(i + 1);
    }

    Escalonador Comecar(const Rodada& r, const int threads, const int doAluno, std::uint32_t sorteio) {
        Escalonador e;
        e.rodada = r;
        e.sorteio = sorteio;
        for (int i = 0; i < threads; ++i) e.fila.push_back(i);

        // A ORDEM DE CHEGADA: um embaralhamento (Fisher-Yates) e, se a thread
        // do aluno caiu depois da terceira, ela troca de lugar com uma das tres.
        for (int i = threads - 1; i > 0; --i) {
            const int j = static_cast<int>(Proximo(e.sorteio) % static_cast<std::uint32_t>(i + 1));
            std::swap(e.fila[static_cast<size_t>(i)], e.fila[static_cast<size_t>(j)]);
        }
        const auto it = std::find(e.fila.begin(), e.fila.end(), doAluno);
        if (it != e.fila.end() && it - e.fila.begin() >= 3) {
            const int vaga = static_cast<int>(Proximo(e.sorteio) % 3u);
            std::iter_swap(it, e.fila.begin() + vaga);
        }
        return e;
    }

    std::vector<Evento> Avancar(Escalonador& e, const float u) {
        std::vector<Evento> eventos;
        e.agora = std::max(e.agora, u);
        if (u < kAbre) return eventos;

        if (e.rodada.tipo == Tipo::Corrida) {
            // SEM TRAVA: todas passam a porta de uma vez, e ninguem espera.
            for (const int t : e.fila) {
                Entrar(e, t, kAbre);
                eventos.push_back({true, t});
            }
            e.fila.clear();
            return eventos;
        }

        const float fica = NaSecao(e.rodada);

        // A PRIMEIRA VEZ: as 'permissoes' primeiras da fila entram em kAbre.
        if (e.dentro.empty() && e.passaram.empty()) {
            while (!e.fila.empty() && static_cast<int>(e.dentro.size()) < e.rodada.permissoes) {
                const int t = e.fila.front();
                e.fila.erase(e.fila.begin());
                Entrar(e, t, kAbre);
                eventos.push_back({true, t});
            }
        }

        // QUEM SAI ACORDA A SEGUINTE, no instante da saida - entao uma rodada
        // atrasada (um quadro longo) chega ao mesmo escalonamento.
        bool mudou = true;
        while (mudou) {
            mudou = false;
            for (size_t i = 0; i < e.dentro.size(); ++i) {
                const float sai = e.entrouEm[i] + fica;
                if (u < sai) continue;
                const int t = e.dentro[i];
                e.dentro.erase(e.dentro.begin() + static_cast<long>(i));
                e.entrouEm.erase(e.entrouEm.begin() + static_cast<long>(i));
                e.passaram.push_back(t);
                eventos.push_back({false, t});
                if (!e.fila.empty()) {
                    size_t qual = 0;
                    // NA ESPERA OCUPADA NAO HA FILA: entra quem ganhar o
                    // test-and-set, e e sorteado entre as que giram.
                    if (e.rodada.tipo == Tipo::EsperaOcupada) {
                        qual = Proximo(e.sorteio) % static_cast<std::uint32_t>(e.fila.size());
                    }
                    const int seguinte = e.fila[qual];
                    e.fila.erase(e.fila.begin() + static_cast<long>(qual));
                    Entrar(e, seguinte, sai);
                    eventos.push_back({true, seguinte});
                }
                mudou = true;
                break;
            }
        }
        return eventos;
    }

    bool Furar(Escalonador& e, const int t) {
        const auto it = std::find(e.fila.begin(), e.fila.end(), t);
        if (it == e.fila.end() || it == e.fila.begin()) return false;
        e.fila.erase(it);
        e.fila.insert(e.fila.begin(), t);
        return true;
    }

    int Valor(const Escalonador& e) {
        return std::max(0, e.rodada.permissoes - static_cast<int>(e.dentro.size()));
    }

    bool Esperando(const Escalonador& e, const int t) { return Tem(e.fila, t); }
    bool Dentro(const Escalonador& e, const int t) { return Tem(e.dentro, t); }
    bool Passou(const Escalonador& e, const int t) { return Tem(e.passaram, t); }

    int Proxima(const Escalonador& e) {
        if (e.fila.empty() || e.rodada.tipo == Tipo::EsperaOcupada || e.rodada.tipo == Tipo::Corrida) return -1;
        return e.fila.front();
    }

    float AlturaNaPilha(const int i) {
        return kTopoDaSecao - 16.0f - static_cast<float>(i) * kEntreEmpilhadas;
    }

    float AnguloNaRajada(const int i, const float abertura) {
        // 0, +a, -a, +2a, -2a...: o leque abre para os dois lados por igual.
        const int passo = (i + 1) / 2;
        const float lado = (i % 2 == 1) ? 1.0f : -1.0f;
        return 90.0f + lado * static_cast<float>(passo) * abertura;
    }

    Ponto NoGiro(const int i, const int n, const float t) {
        if (n <= 0) return {};
        const float volta = 6.2831853f;
        const float a = volta * (static_cast<float>(i) / static_cast<float>(n) + kVoltasPorSegundo * t);
        return {kRaioDoGiro * std::cos(a), kRaioDoGiro * std::sin(a)};
    }

    float DesvioNaCorrida(std::uint32_t sorteio) {
        const std::uint32_t r = Proximo(sorteio) % 2001u;
        return (static_cast<float>(r) / 1000.0f - 1.0f) * kMaiorDesvio;
    }

    float NaSecao(const Rodada& r) {
        switch (r.tipo) {
            case Tipo::Contador: return 1.0f;
            case Tipo::Corrida:  return kPorRodada;
            default:             return 0.65f;
        }
    }

    std::string CodigoDaSecao(const Tipo t) {
        switch (t) {
            case Tipo::Mutex:
            case Tipo::Contador:      return "P(s);  saldo++;  V(s);";
            case Tipo::EsperaOcupada: return "while (test_and_set(&trava));  saldo++;  trava = 0;";
            case Tipo::Corrida:       return "saldo++;  /* sem trava */";
        }
        return "";
    }

    std::string Placar(const Escalonador& e, const bool alunoBloqueado) {
        std::string texto;
        switch (e.rodada.tipo) {
            case Tipo::Mutex:
            case Tipo::Contador: {
                texto = "s = " + std::to_string(Valor(e)) + kPonto + "fila:";
                if (e.fila.empty()) texto += " vazia";
                for (const int t : e.fila) texto += " " + Nome(t);
                break;
            }
            case Tipo::EsperaOcupada: {
                texto = "espera ocupada" + std::string(kPonto) + "girando:";
                if (e.fila.empty()) texto += " nenhuma";
                for (const int t : e.fila) texto += " " + Nome(t);
                break;
            }
            case Tipo::Corrida: {
                const int n = static_cast<int>(e.dentro.size() + e.fila.size());
                if (e.dentro.empty() || e.agora < kAbre + kSaldoAparece) return "saldo++" + std::string(kPonto) + "sem trava";
                return "saldo = " + std::to_string(SaldoDaCorrida(n)) + kPonto + "esperado " + std::to_string(n);
            }
        }
        if (alunoBloqueado) texto = "aluno em P(s)" + std::string(kPonto) + texto;
        return texto;
    }

    Plano PlanoDeSempre() {
        Plano p;
        for (int k = 0; k < kRodadas; ++k) p.rodadas[static_cast<size_t>(k)] = RodadaDaVez(k);
        return p;
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p = PlanoDeSempre();
        std::vector<Rodada> resto(p.rodadas.begin() + 1, p.rodadas.end());
        s.Embaralhar(resto);
        for (size_t i = 0; i < resto.size(); ++i) p.rodadas[i + 1] = resto[i];
        for (Rodada& r : p.rodadas) {
            if (r.tipo == Tipo::Contador) r.permissoes = s.Inteiro(kMenorContador, kMaiorContador);
        }
        p.entreThreads = s.Real(kMenorEntreThreads, kMaiorEntreThreads);
        return p;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreInstrucoes = Dificuldade::Escolher(n, kEntreInstrucoes, 0.22f);
        d.instrucoesPorThread = Dificuldade::Escolher(n, kInstrucoesPorThread, 11);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.25f);
        d.aberturaDaRajada = Dificuldade::Escolher(n, kAberturaDaRajada, 10.0f);
        return d;
    }

    int SaldoDaCorrida(const int threads) {
        return threads > 0 ? 1 : 0;
    }
}
