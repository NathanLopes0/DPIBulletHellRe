//
// Os sistemas de tipos da fase 2 do Luizinho.
//

#include "Coercao.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <utility>

namespace Coercao {

    namespace {
        /// Os produtos do "a" * "b": de 8 a 16, para o anel ter o que contar.
        constexpr std::array<std::pair<int, int>, 8> kProdutos = {{
            {2, 4}, {2, 5}, {3, 4}, {2, 7}, {3, 5}, {4, 4}, {2, 6}, {4, 3}}};

        constexpr float kPi = 3.14159265358979f;

        int Entre(std::mt19937& g, const int de, const int ate) {
            return std::uniform_int_distribution<int>(de, ate)(g);
        }

        std::string Algarismo(const int n) { return std::to_string(n); }
    }

    Tipagem TipagemDe(const Linguagem l) {
        switch (l) {
            case Linguagem::JavaScript: return Tipagem::Fraca;
            case Linguagem::PHP:        return Tipagem::Fraca;
            case Linguagem::Python:     return Tipagem::ForteDinamica;
            case Linguagem::Haskell:    return Tipagem::ForteEstatica;
        }
        return Tipagem::Fraca;
    }

    std::string Nome(const Linguagem l) {
        switch (l) {
            case Linguagem::JavaScript: return "JavaScript";
            case Linguagem::PHP:        return "PHP";
            case Linguagem::Python:     return "Python";
            case Linguagem::Haskell:    return "Haskell";
        }
        return "";
    }

    Valor Numero(const int n) { return Valor{Tipo::Numero, n, ""}; }
    Valor Texto(const std::string& s) { return Valor{Tipo::Texto, 0, s}; }

    std::string Escrito(const Valor& v) {
        return v.tipo == Tipo::Numero ? std::to_string(v.numero) : "\"" + v.texto + "\"";
    }

    std::string Simbolo(const Operacao op) {
        return op == Operacao::Soma ? "+" : "*";
    }

    Linguagem LinguagemDe(const Forma f) {
        switch (f) {
            case Forma::JsSoma:       return Linguagem::JavaScript;
            case Forma::PhpSoma:      return Linguagem::PHP;
            case Forma::PythonSoma:   return Linguagem::Python;
            case Forma::JsProduto:    return Linguagem::JavaScript;
            case Forma::PythonRepete: return Linguagem::Python;
            case Forma::HaskellSoma:  return Linguagem::Haskell;
        }
        return Linguagem::JavaScript;
    }

    Operacao OperacaoDe(const Forma f) {
        return (f == Forma::JsProduto || f == Forma::PythonRepete) ? Operacao::Produto : Operacao::Soma;
    }

    Valor Esquerdo(const Comando& c) {
        if (c.forma == Forma::JsProduto || c.forma == Forma::PythonRepete) return Texto(Algarismo(c.a));
        return Numero(c.a);
    }

    Valor Direito(const Comando& c) {
        if (c.forma == Forma::PythonRepete) return Numero(c.b);
        return Texto(Algarismo(c.b));
    }

    Resultado Avaliar(const Comando& c) {
        Resultado r;
        r.esquerdoDepois = Esquerdo(c);
        r.direitoDepois = Direito(c);
        switch (c.forma) {
            case Forma::JsSoma:
                // NO JAVASCRIPT, + COM UM TEXTO E CONCATENAR: o numero vira texto.
                r.coagidoEsquerdo = true;
                r.esquerdoDepois = Texto(Algarismo(c.a));
                r.valor = Texto(Algarismo(c.a) + Algarismo(c.b));
                break;
            case Forma::PhpSoma:
                // NO PHP, + E SEMPRE SOMA: o texto vira numero.
                r.coagidoDireito = true;
                r.direitoDepois = Numero(c.b);
                r.valor = Numero(c.a + c.b);
                break;
            case Forma::JsProduto:
                // O * SO EXISTE PARA NUMERO: os dois textos viram.
                r.coagidoEsquerdo = true;
                r.coagidoDireito = true;
                r.esquerdoDepois = Numero(c.a);
                r.direitoDepois = Numero(c.b);
                r.valor = Numero(c.a * c.b);
                break;
            case Forma::PythonRepete:
                // TEXTO VEZES NUMERO E REPETIR - a operacao existe para os dois
                // tipos como estao, e nada e convertido.
                r.valor = Texto(std::string(static_cast<size_t>(std::max(0, c.b)), Algarismo(c.a)[0]));
                break;
            case Forma::PythonSoma:
                r.desfecho = Desfecho::ErroDeExecucao;
                break;
            case Forma::HaskellSoma:
                r.desfecho = Desfecho::ErroDeCompilacao;
                break;
        }
        return r;
    }

    Forma OrdemDaFase(const int vez) {
        constexpr std::array<Forma, kComandos> kOrdem = {
            Forma::JsSoma, Forma::PhpSoma, Forma::PythonSoma, Forma::JsProduto,
            Forma::JsSoma, Forma::PythonRepete, Forma::PhpSoma, Forma::HaskellSoma};
        return kOrdem[static_cast<size_t>(((vez % kComandos) + kComandos) % kComandos)];
    }

    Comando Proximo(const int vez, const std::uint32_t sorteio) {
        std::mt19937 g(sorteio);
        Comando c;
        c.forma = OrdemDaFase(vez);
        switch (c.forma) {
            case Forma::PhpSoma:
                // A SOMA DA DE 7 A 13: o anel tem o que contar e ainda se le.
                c.a = Entre(g, 2, 9);
                c.b = Entre(g, std::max(1, 7 - c.a), std::min(9, 13 - c.a));
                break;
            case Forma::JsProduto: {
                const auto& par = kProdutos[static_cast<size_t>(Entre(g, 0, static_cast<int>(kProdutos.size()) - 1))];
                c.a = par.first;
                c.b = par.second;
                break;
            }
            case Forma::PythonRepete:
                c.a = Entre(g, 1, 9);
                c.b = Entre(g, 3, 5);
                break;
            default:
                c.a = Entre(g, 1, 9);
                c.b = Entre(g, 1, 9);
                break;
        }
        return c;
    }

    namespace {
        std::string Cabecalho(const Comando& c) {
            const Linguagem l = LinguagemDe(c.forma);
            std::string como;
            switch (TipagemDe(l)) {
                case Tipagem::Fraca:         como = "tipagem fraca"; break;
                // \xc3\xa2 e o a circunflexo e \xc3\xa1 o agudo: o MSVC nao le
                // acento em literal.
                case Tipagem::ForteDinamica: como = "forte e din\xc3\xa2" "mica"; break;
                case Tipagem::ForteEstatica: como = "forte e est\xc3\xa1" "tica"; break;
            }
            return Nome(l) + " \xc2\xb7 " + como;
        }

        std::string Expressao(const Comando& c) {
            return Escrito(Esquerdo(c)) + " " + Simbolo(OperacaoDe(c.forma)) + " " + Escrito(Direito(c));
        }
    }

    std::string Placar(const Comando& c) {
        return Cabecalho(c) + "\n" + Expressao(c);
    }

    std::string PlacarComResultado(const Comando& c) {
        const Resultado r = Avaliar(c);
        std::string fim;
        switch (r.desfecho) {
            case Desfecho::Valor:            fim = Escrito(r.valor); break;
            case Desfecho::ErroDeExecucao:   fim = "TypeError"; break;
            case Desfecho::ErroDeCompilacao: fim = "erro de compila\xc3\xa7\xc3\xa3" "o"; break;
        }
        // \xe2\x86\x92 e a seta.
        return Cabecalho(c) + "\n" + Expressao(c) + "  \xe2\x86\x92  " + fim;
    }

    std::string CaraDoNumero(const int n) {
        return "Numero" + std::to_string(std::clamp(n, 0, kMaiorNumero));
    }

    std::string CaraDoOperando(const Valor& v) {
        if (v.tipo == Tipo::Numero) return CaraDoNumero(v.numero);
        return "Texto" + v.texto.substr(0, 1);
    }

    std::string CaraDaLetra(const char c) {
        if (c == '"') return "Aspas";
        return std::string("Letra") + c;
    }

    std::string CaraDoOperador(const Operacao op, const bool erro) {
        return std::string(op == Operacao::Soma ? "Mais" : "Vezes") + (erro ? "Erro" : "");
    }

    std::vector<char> Letras(const Valor& texto) {
        std::vector<char> letras = {'"'};
        for (const char c : texto.texto) letras.push_back(c);
        letras.push_back('"');
        return letras;
    }

    float AberturaDoAnel(const int n) {
        if (n <= 1) return 0.0f;
        return 360.0f * static_cast<float>(n - 1) / static_cast<float>(n);
    }

    float CentroDoAnel(const int n, const float alvo) {
        if (n <= 0 || n % 2 == 1) return alvo;
        return alvo - 180.0f / static_cast<float>(n);
    }

    std::vector<Ponto> DesviosDaPalavra(const int letras, const float direcao) {
        // De frente para quem recebe: a palavra que desce le da esquerda para
        // a direita na tela.
        const float rad = direcao * kPi / 180.0f;
        const float px = std::sin(rad);
        const float py = -std::cos(rad);
        std::vector<Ponto> desvios;
        for (int i = 0; i < letras; ++i) {
            const float k = (static_cast<float>(i) - static_cast<float>(letras - 1) / 2.0f) * kEntreLetras;
            desvios.push_back({k * px, k * py});
        }
        return desvios;
    }

    Ponto OndeFicaOOperador(const float xDoAluno, const float largura) {
        return {std::clamp(xDoAluno, kMargemDoOperador, largura - kMargemDoOperador), kYDoOperador};
    }

    std::vector<int> OrdemDosComandos(Variacao::Sorteio& s) {
        std::vector<int> ordem = Variacao::Ordem(s, kComandos - 1, 1);
        ordem.push_back(kComandos - 1);
        return ordem;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.voo = Dificuldade::Escolher(n, kVoo, 0.5f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.2f);
        return d;
    }
}
