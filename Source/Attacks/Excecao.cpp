//
// Os sequenciadores da final do Luizinho: as excecoes.
//

#include "Excecao.h"

#include <algorithm>
#include <cmath>

namespace Excecao {

    namespace {
        Quadro Chamada(const std::string& funcao) { return Quadro{funcao, std::nullopt, false}; }
        Quadro Catch(const std::string& funcao, const Tipo t) { return Quadro{funcao, t, false}; }
        Quadro Finally(const std::string& funcao) { return Quadro{funcao, std::nullopt, true}; }

        // \xc2\xb7 e o ponto do meio, \xe2\x86\x92 a seta: o MSVC nao le
        // acento nem simbolo em literal.
        const char* const kPonto = " \xc2\xb7 ";
        const char* const kSeta = " \xe2\x86\x92 ";
    }

    Tipo Pai(const Tipo t) {
        switch (t) {
            case Tipo::NullPointerException: return Tipo::RuntimeException;
            default: return Tipo::Exception;
        }
    }

    bool EhUm(const Tipo t, const Tipo base) {
        Tipo atual = t;
        while (true) {
            if (atual == base) return true;
            if (atual == Tipo::Exception) return false;
            atual = Pai(atual);
        }
    }

    std::string Nome(const Tipo t) {
        switch (t) {
            case Tipo::Exception:            return "Exception";
            case Tipo::IOException:          return "IOException";
            case Tipo::RuntimeException:     return "RuntimeException";
            case Tipo::NullPointerException: return "NullPointerException";
            case Tipo::Reprovado:            return "Reprovado";
        }
        return "";
    }

    Rodada RodadaDaVez(const int k) {
        switch (((k % kRodadas) + kRodadas) % kRodadas) {
            // O catch de f trata a IOException: so h e g caem, e o finally de g roda.
            case 0: return {{Catch("main", Tipo::Exception), Catch("f", Tipo::IOException), Finally("g"), Chamada("h")},
                            Tipo::IOException};
            // A MESMA PILHA, outra excecao: f nao a trata, e ela vai ate o main.
            case 1: return {{Catch("main", Tipo::Exception), Catch("f", Tipo::IOException), Finally("g"), Chamada("h")},
                            Tipo::NullPointerException};
            // A HIERARQUIA: o catch de RuntimeException trata a NullPointerException.
            case 2: return {{Chamada("main"), Catch("f", Tipo::RuntimeException), Chamada("h")},
                            Tipo::NullPointerException};
            // NINGUEM TRATA: a pilha inteira cai, e o programa capota.
            case 3: return {{Chamada("main"), Finally("f"), Catch("g", Tipo::IOException), Chamada("h")},
                            Tipo::NullPointerException};
            // A REPESCAGEM E UM CATCH.
            default: return {{Catch("main", Tipo::Reprovado), Chamada("f"), Chamada("h")}, Tipo::Reprovado};
        }
    }

    std::vector<Passo> Propagar(const Rodada& r) {
        std::vector<Passo> passos;
        const int ultimo = static_cast<int>(r.pilha.size()) - 1;
        for (int i = ultimo; i >= 0; --i) {
            const Quadro& q = r.pilha[static_cast<size_t>(i)];
            const float quando = static_cast<float>(ultimo - i) * kPorQuadro;
            if (q.captura && EhUm(r.lancada, *q.captura)) {
                passos.push_back({i, Acontece::Captura, quando});
                return passos;
            }
            if (q.finally) passos.push_back({i, Acontece::Finally, quando});
            passos.push_back({i, Acontece::Desempilha, quando});
        }
        passos.push_back({-1, Acontece::Escapa, static_cast<float>(ultimo) * kPorQuadro});
        return passos;
    }

    int QuemCaptura(const Rodada& r) {
        for (const Passo& p : Propagar(r)) if (p.o == Acontece::Captura) return p.quadro;
        return -1;
    }

    std::string Rotulo(const Quadro& q) {
        std::string texto = q.funcao + "()";
        if (q.captura) texto += std::string(kPonto) + "catch (" + Nome(*q.captura) + " e)";
        if (q.finally) texto += std::string(kPonto) + "finally";
        return texto;
    }

    std::string PlacarDasChamadas(const Rodada& r) {
        std::string texto;
        for (size_t i = 0; i < r.pilha.size(); ++i) {
            if (i > 0) texto += kSeta;
            texto += r.pilha[i].funcao + "()";
        }
        return texto;
    }

    std::string PlacarDoLancamento(const Rodada& r) {
        const std::string quem = r.pilha.empty() ? std::string("main") : r.pilha.back().funcao;
        return PlacarDasChamadas(r) + "\n" + quem + "(): throw new " + Nome(r.lancada) + "();";
    }

    std::string PlacarDoDesfecho(const Rodada& r) {
        const int k = QuemCaptura(r);
        if (k < 0) return "Exception in thread \"main\"\njava.lang." + Nome(r.lancada);
        const Quadro& q = r.pilha[static_cast<size_t>(k)];
        const std::string linha = "throw new " + Nome(r.lancada) + "();";
        if (r.lancada == Tipo::Reprovado) return linha + "\ncatch (Reprovado e) { repescagem(); }";
        return linha + "\ncapturada em " + q.funcao + "()";
    }

    std::string CaraDaExcecao(const Tipo t) {
        return Nome(t);
    }

    Retangulo Faixa(const int k, const float largura) {
        return {kMargem, kTopoDaPilha + static_cast<float>(k) * (kAlturaDaFaixa + kEntreFaixas),
                largura - 2.0f * kMargem, kAlturaDaFaixa};
    }

    std::vector<float> Cortina(const float xDoAluno, const float largura) {
        return Cortina(xDoAluno, largura, kEntreDestrocos);
    }

    std::vector<float> Cortina(const float xDoAluno, const float largura, const float entre) {
        const float passo = std::max(1.0f, entre);
        const float de = kMargem;
        const float ate = largura - kMargem;
        const float meio = std::clamp(xDoAluno, de, ate);
        float x = meio - std::floor((meio - de) / passo) * passo;
        std::vector<float> xs;
        for (; x <= ate + 0.01f; x += passo) xs.push_back(x);
        return xs;
    }


    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreDestrocos = Dificuldade::Escolher(n, kEntreDestrocos, 52.0f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.2f);
        return d;
    }
    Rodada Gerar(Variacao::Sorteio& s) {
        static const char* const kFuncoes[] = {"main", "f", "g", "h"};
        static const std::vector<Tipo> kLancaveis = {Tipo::IOException, Tipo::RuntimeException,
                                                     Tipo::NullPointerException};
        static const std::vector<Tipo> kCapturaveis = {Tipo::Exception, Tipo::IOException,
                                                       Tipo::RuntimeException, Tipo::NullPointerException};
        Rodada r;
        const int n = s.Inteiro(3, kMaiorPilha);
        for (int i = 0; i < n; ++i) {
            Quadro q = Chamada(kFuncoes[i]);
            const bool lanca = (i == n - 1);
            if (!lanca && s.Chance(0.5f)) q.captura = s.Um(kCapturaveis);
            q.finally = s.Chance(lanca ? 0.25f : 0.35f);
            r.pilha.push_back(q);
        }
        r.lancada = s.Um(kLancaveis);
        return r;
    }

    Rodada RodadaDaLuta(Variacao::Sorteio& s, const int k) {
        if (((k % kRodadas) + kRodadas) % kRodadas == kRodadas - 1) return RodadaDaVez(kRodadas - 1);
        return Gerar(s);
    }
}
