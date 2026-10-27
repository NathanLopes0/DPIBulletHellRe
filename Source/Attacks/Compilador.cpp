//
// A fase 2 do Vladimir: a construcao de um compilador, de ponta a ponta.
//

#include "Compilador.h"

#include <algorithm>
#include <cctype>
#include <functional>

namespace Compilador {

    std::vector<Token> Lexico(const std::string& linha) {
        std::vector<Token> tokens;
        for (const char c : linha) {
            const auto u = static_cast<unsigned char>(c);
            if (std::isspace(u)) continue;
            Token t;
            t.texto = std::string(1, c);
            if (std::isalpha(u)) t.tipo = Tipo::Id;
            else if (std::isdigit(u)) t.tipo = Tipo::Num;
            else if (c == '=') t.tipo = Tipo::Igual;
            else if (c == ';') t.tipo = Tipo::Fim;
            else t.tipo = Tipo::Op;
            tokens.push_back(t);
        }
        return tokens;
    }

    namespace {
        int Precedencia(const std::string& op) { return op == "*" ? 2 : 1; }

        struct Parser {
            const std::vector<Token>& t;
            size_t i = 0;
            Arvore& a;
            int deslocamento = 0;   ///< o corpo comeca depois do "x ="

            int Folha() {
                const int token = static_cast<int>(i) + deslocamento;
                const Token& k = t[i++];
                a.nos.push_back(No{k.tipo, k.texto, -1, -1, token});
                return static_cast<int>(a.nos.size()) - 1;
            }

            // PRECEDENCE CLIMBING: junta a esquerda com tudo de precedencia >=
            // 'minima'; o de precedencia maior desce mais fundo na arvore.
            int Expressao(const int minima) {
                int esq = Folha();
                while (i < t.size() && t[i].tipo == Tipo::Op && Precedencia(t[i].texto) >= minima) {
                    const int token = static_cast<int>(i) + deslocamento;
                    const std::string op = t[i++].texto;
                    const int dir = Expressao(Precedencia(op) + 1);
                    a.nos.push_back(No{Tipo::Op, op, esq, dir, token});
                    esq = static_cast<int>(a.nos.size()) - 1;
                }
                return esq;
            }
        };
    }

    Arvore Sintatico(const std::vector<Token>& tokens) {
        Arvore a;
        if (tokens.size() < 3 || tokens[1].tipo != Tipo::Igual) return a;
        std::vector<Token> corpo(tokens.begin() + 2, tokens.end());
        if (!corpo.empty() && corpo.back().tipo == Tipo::Fim) corpo.pop_back();
        if (corpo.empty()) return a;
        a.nos.push_back(No{tokens[0].tipo, tokens[0].texto, -1, -1, 0});
        const int alvo = 0;
        Parser p{corpo, 0, a, 2};
        const int expr = p.Expressao(1);
        a.nos.push_back(No{Tipo::Igual, "=", alvo, expr, 1});

        a.raiz = static_cast<int>(a.nos.size()) - 1;
        return a;
    }

    std::vector<int> PosOrdem(const Arvore& a) {
        std::vector<int> ordem;
        std::function<void(int)> visitar = [&](const int n) {
            if (n < 0) return;
            const No& no = a.nos[static_cast<size_t>(n)];
            visitar(no.esq);
            visitar(no.dir);
            ordem.push_back(n);
        };
        visitar(a.raiz);
        return ordem;
    }

    std::string Instrucao(const Arvore& a, const int no) {
        if (no < 0 || no >= static_cast<int>(a.nos.size())) return "";
        const No& n = a.nos[static_cast<size_t>(no)];
        switch (n.tipo) {
            case Tipo::Igual: return "STORE " + a.nos[static_cast<size_t>(n.esq)].texto;
            case Tipo::Num:   return "PUSH " + n.texto;
            case Tipo::Op:    return n.texto == "+" ? "ADD" : (n.texto == "-" ? "SUB" : "MUL");
            case Tipo::Id:
                // O ALVO DO = nao e lido: e onde o STORE escreve.
                if (a.raiz >= 0 && a.nos[static_cast<size_t>(a.raiz)].esq == no) return "";
                return "LOAD " + n.texto;
            case Tipo::Fim:   break;
        }
        return "";
    }

    std::vector<std::string> Codigo(const Arvore& a) {
        std::vector<std::string> codigo;
        for (const int n : PosOrdem(a)) {
            const std::string i = Instrucao(a, n);
            if (!i.empty()) codigo.push_back(i);
        }
        return codigo;
    }

    int Avaliar(const Arvore& a, const int a_, const int b_) {
        std::function<int(int)> valor = [&](const int n) -> int {
            const No& no = a.nos[static_cast<size_t>(n)];
            switch (no.tipo) {
                case Tipo::Num: return no.texto[0] - '0';
                case Tipo::Id:  return no.texto == "a" ? a_ : (no.texto == "b" ? b_ : 0);
                case Tipo::Op: {
                    const int e = valor(no.esq), d = valor(no.dir);
                    return no.texto == "+" ? e + d : (no.texto == "-" ? e - d : e * d);
                }
                case Tipo::Igual: return valor(no.dir);
                case Tipo::Fim: break;
            }
            return 0;
        };
        return a.raiz < 0 ? 0 : valor(a.raiz);
    }

    std::string Gerar(Variacao::Sorteio& s) {
        const int ops = s.Inteiro(2, 3);
        std::vector<std::string> folhas;
        for (int k = 0; k <= ops; ++k) {
            const int r = s.Inteiro(0, 9);
            folhas.push_back(r < 3 ? "a" : (r < 6 ? "b" : std::string(1, static_cast<char>('1' + s.Inteiro(0, 8)))));
        }
        // a E b APARECEM, as duas: sao as letras do Vladimir.
        auto tem = [&](const char* x) { return std::find(folhas.begin(), folhas.end(), x) != folhas.end(); };
        if (!tem("a")) folhas[static_cast<size_t>(s.Inteiro(0, ops))] = "a";
        if (!tem("b")) {
            std::vector<size_t> livres;
            for (size_t k = 0; k < folhas.size(); ++k) if (folhas[k] != "a") livres.push_back(k);
            folhas[livres.empty() ? 0 : livres[static_cast<size_t>(s.Inteiro(0, static_cast<int>(livres.size()) - 1))]] = "b";
        }
        static const std::vector<std::string> kOps = {"+", "-", "*"};
        std::string linha = "x = " + folhas[0];
        for (int k = 0; k < ops; ++k) linha += " " + s.Um(kOps) + " " + folhas[static_cast<size_t>(k) + 1];
        return linha + ";";
    }

    std::string CaraDoLexema(const std::string& texto) {
        if (texto == "=") return "Igual";
        if (texto == "+") return "Mais";
        if (texto == "-") return "Menos";
        if (texto == "*") return "Vezes";
        if (texto == ";") return "Fim";
        if (texto.size() == 1 && texto[0] >= '1' && texto[0] <= '9') return "N" + texto;
        if (texto == "x") return "X";
        return "X";
    }

    std::string CaraDaInstrucao(const std::string& instrucao) {
        return instrucao.substr(0, instrucao.find(' '));
    }

    std::string CaraDoNo(const Arvore& a, const int no) {
        const std::string& texto = a.nos[static_cast<size_t>(no)].texto;
        if (texto == "a") return "IdA";
        if (texto == "b") return "IdB";
        return CaraDoLexema(texto);
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p;
        for (auto& linha : p.linhas) linha = Gerar(s);
        return p;
    }

    std::vector<Ponto> PosicoesDaLinha(
const std::vector<Token>& tokens, const float largura) {
        std::vector<Ponto> p;
        const float passo = 58.0f;
        const float x0 = largura / 2.0f - passo * static_cast<float>(tokens.size() - 1) / 2.0f;
        for (size_t k = 0; k < tokens.size(); ++k) p.push_back({x0 + passo * static_cast<float>(k), kYDaLinha});
        return p;
    }

    std::vector<Ponto> PosicoesDaArvore(const Arvore& a, const float largura) {
        std::vector<Ponto> p(a.nos.size());
        std::vector<int> profundidade(a.nos.size(), 0);
        std::vector<int> emOrdem;
        std::function<void(int, int)> visitar = [&](const int n, const int d) {
            if (n < 0) return;
            const No& no = a.nos[static_cast<size_t>(n)];
            profundidade[static_cast<size_t>(n)] = d;
            visitar(no.esq, d + 1);
            emOrdem.push_back(n);
            visitar(no.dir, d + 1);
        };
        visitar(a.raiz, 0);
        const float x0 = largura / 2.0f - kEntreColunas * static_cast<float>(emOrdem.size() - 1) / 2.0f;
        for (size_t k = 0; k < emOrdem.size(); ++k) {
            const auto n = static_cast<size_t>(emOrdem[k]);
            p[n] = {x0 + kEntreColunas * static_cast<float>(k), kTopoDaArvore + kEntreNiveis * static_cast<float>(profundidade[n])};
        }
        return p;
    }

    Marcos MarcosDa(const int tokens, const int nos, const float porInstrucao) {
        Marcos m;
        m.sintatico = static_cast<float>(tokens) * kPorToken + kPausa;
        m.codigo = m.sintatico + static_cast<float>(nos) * kPorNo + kPausa;
        m.pronto = m.codigo + static_cast<float>(nos) * porInstrucao;
        return m;
    }

    Etapa EtapaEm(const float t, const Marcos& m) {
        if (t < m.sintatico) return Etapa::Lexico;
        if (t < m.codigo) return Etapa::Sintatico;
        if (t < m.pronto) return Etapa::Codigo;
        return Etapa::Pronto;
    }

    std::string Placar(const std::string& linha, const Etapa e, const std::string& instrucao) {
        // \xc3\xa9 e agudo, \xc3\xa1 a agudo, \xc3\xa1 a agudo: o MSVC nao le acento.
        switch (e) {
            case Etapa::Lexico:    return linha + "\nan\xc3\xa1lise l\xc3\xa9xica";
            case Etapa::Sintatico: return linha + "\nan\xc3\xa1lise sint\xc3\xa1tica";
            case Etapa::Codigo:    return linha + "\n" + (instrucao.empty() ? std::string("gera\xc3\xa7\xc3\xa3o de c\xc3\xb3" "digo") : instrucao);
            case Etapa::Pronto:    return linha + "\ncompilado";
        }
        return linha;
    }

    std::string Legenda(const Etapa e) {
        switch (e) {
            case Etapa::Lexico:
                return "An\xc3\xa1lise l\xc3\xa9xica: o scanner - um aut\xc3\xb4mato, como o da fase 1 - "
                       "quebra a linha em tokens, e cada um cai.";
            case Etapa::Sintatico:
                return "An\xc3\xa1lise sint\xc3\xa1tica: o parser monta a \xc3\xa1rvore pela gram\xc3\xa1tica - "
                       "o * antes do + -, e cada redu\xc3\xa7\xc3\xa3o solta um leque.";
            case Etapa::Codigo:
                return "Gera\xc3\xa7\xc3\xa3o de c\xc3\xb3" "digo: a \xc3\xa1rvore \xc3\xa9 percorrida de baixo para "
                       "cima, e cada n\xc3\xb3 vira uma instru\xc3\xa7\xc3\xa3o da m\xc3\xa1quina de pilha.";
            case Etapa::Pronto:
                return "Compilado: o STORE guarda o resultado em x.";
        }
        return "";
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.porInstrucao = Dificuldade::Escolher(n, kPorInstrucao, 0.24f);
        return d;
    }
}
