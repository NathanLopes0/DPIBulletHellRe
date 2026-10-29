//
// O registro de cada tentativa de batalha: os dados para a analise do TCC.
//

#include "Tentativas.h"

#include <algorithm>
#include <cstdio>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <tuple>

#include "Exportacao.h"

namespace Tentativas {

    const char* Nome(const Resultado r) {
        switch (r) {
            case Resultado::Aprovado:  return "aprovado";
            case Resultado::Reprovado: return "reprovado";
            case Resultado::Abandonou: return "abandonou";
        }
        return "abandonou";
    }

    namespace {

        /// UM NUMERO COM PONTO, nunca virgula: o snprintf segue o locale do
        /// processo, e um locale brasileiro quebraria as colunas.
        std::string Num(const float valor, const int casas = 2) {
            char buffer[48];
            std::snprintf(buffer, sizeof(buffer), "%.*f", casas, static_cast<double>(valor));
            std::string s = buffer;
            std::replace(s.begin(), s.end(), ',', '.');
            return s;
        }

        std::string Bit(const bool b) { return b ? "1" : "0"; }

        /// O inverso de Num, tambem sem locale.
        float Numero(const std::string& texto) {
            std::istringstream entrada(texto);
            entrada.imbue(std::locale::classic());
            float v = 0.0f;
            entrada >> v;
            return entrada.fail() ? 0.0f : v;
        }

        std::string Juntar(const std::vector<std::string>& campos) {
            std::string linha;
            for (size_t i = 0; i < campos.size(); ++i) {
                if (i) linha += ',';
                linha += Exportacao::Campo(campos[i]);
            }
            return linha;
        }

        /// "AAAA-MM-DD HH:MM:SS" -> as duas metades.
        std::string Data(const std::string& instante) { return instante.substr(0, 10); }
        std::string Hora(const std::string& instante) {
            return instante.size() > 11 ? instante.substr(11) : "";
        }

        int Digitos(const std::string& s, const size_t de, const size_t quantos, bool& ok) {
            int v = 0;
            for (size_t i = de; i < de + quantos; ++i) {
                if (i >= s.size() || s[i] < '0' || s[i] > '9') { ok = false; return 0; }
                v = v * 10 + (s[i] - '0');
            }
            return v;
        }

        const std::vector<std::string> kColunasDasTentativas = {
            "formato", "id", "versao_do_jogo", "matricula", "visitante", "posicao", "parceiro",
            "modo", "vez", "materia", "nome_da_materia", "professor", "dificuldade",
            "resultado", "aprovado", "inicio", "fim", "data", "hora", "dia_da_semana",
            "duracao_s", "pausa_s", "nota_final", "nota_maxima", "nota_minima", "nota_cheia",
            "fez_exame", "fases_jogadas", "fases_limpas", "acertos_sofridos_dupla",
            "acertos_sofridos", "tiros", "acertos_no_professor", "precisao",
            "pontos_extras_pegos", "pontos_extras_usados", "orientador",
            "acertos_do_orientador", "poderes_do_orientador", "semente",
        };

        const std::vector<std::string> kColunasDasFases = {
            "formato", "id", "materia", "dificuldade", "modo", "matricula_1", "matricula_2",
            "resultado", "ordem", "fase", "anuncio", "inicio_s", "fim_s", "duracao_s",
            "nota_no_inicio", "nota_no_fim", "variacao", "acertos_sofridos", "limpa", "completa",
        };
    }

    std::string NovoId(const std::string& inicio, const std::uint32_t sal) {
        std::string compacto;
        for (const char c : inicio) {
            if (c >= '0' && c <= '9') compacto += c;
            else if (c == ' ') compacto += '-';
        }
        char cauda[16];
        std::snprintf(cauda, sizeof(cauda), "%08x", static_cast<unsigned>(sal));
        return compacto + "-" + cauda;
    }

    std::string DiaDaSemana(const std::string& data) {
        bool ok = data.size() >= 10 && data[4] == '-' && data[7] == '-';
        int a = Digitos(data, 0, 4, ok);
        const int m = Digitos(data, 5, 2, ok);
        const int d = Digitos(data, 8, 2, ok);
        if (!ok || m < 1 || m > 12 || d < 1 || d > 31) return "";

        // Sakamoto: 0 e domingo.
        static constexpr int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
        if (m < 3) a -= 1;
        const int dia = (a + a / 4 - a / 100 + a / 400 + t[m - 1] + d) % 7;
        static const char* const kNomes[] = {"domingo", "segunda", "terca", "quarta",
                                             "quinta", "sexta", "sabado"};
        return kNomes[dia];
    }

    // ---- AS LINHAS ------------------------------------------------------------------

    std::string CabecalhoDasTentativas() { return Juntar(kColunasDasTentativas); }

    std::vector<std::string> LinhasDaTentativa(const Tentativa& t, const std::vector<int>& vezes) {
        std::vector<std::string> linhas;
        for (size_t i = 0; i < t.jogadores.size(); ++i) {
            const Jogador& j = t.jogadores[i];
            const int vez = i < vezes.size() ? vezes[i] : 0;
            const std::string precisao = j.tiros > 0
                ? Num(static_cast<float>(j.acertosNoProfessor) / static_cast<float>(j.tiros), 3)
                : "";
            linhas.push_back(Juntar({
                std::to_string(kVersaoDoFormato), t.id, t.versaoDoJogo, j.matricula,
                Bit(j.visitante), std::to_string(j.posicao), j.parceiro, t.modo,
                std::to_string(vez), t.materia, t.nomeDaMateria, t.professor, t.dificuldade,
                Nome(t.resultado), Bit(t.resultado == Resultado::Aprovado), t.inicio, t.fim,
                Data(t.inicio), Hora(t.inicio), DiaDaSemana(t.inicio),
                Num(t.duracao), Num(t.tempoEmPausa), Num(t.notaFinal), Num(t.notaMaxima),
                Num(t.notaMinima), Bit(t.notaCheia), Bit(t.fezExame),
                std::to_string(t.fases.size()), std::to_string(t.fasesLimpas),
                std::to_string(t.acertosSofridos), std::to_string(j.acertosSofridos),
                std::to_string(j.tiros), std::to_string(j.acertosNoProfessor), precisao,
                std::to_string(j.pontosExtrasPegos), std::to_string(j.pontosExtrasUsados),
                t.orientador, std::to_string(t.acertosDoOrientador),
                std::to_string(t.poderesUsados), std::to_string(t.semente),
            }));
        }
        return linhas;
    }

    std::string CabecalhoDasFases() { return Juntar(kColunasDasFases); }

    std::vector<std::string> LinhasDasFases(const Tentativa& t) {
        const std::string m1 = !t.jogadores.empty() ? t.jogadores[0].matricula : "";
        const std::string m2 = t.jogadores.size() > 1 ? t.jogadores[1].matricula : "";
        std::vector<std::string> linhas;
        for (const Fase& f : t.fases) {
            linhas.push_back(Juntar({
                std::to_string(kVersaoDoFormato), t.id, t.materia, t.dificuldade, t.modo, m1, m2,
                Nome(t.resultado), std::to_string(f.ordem), f.nome, f.anuncio,
                Num(f.inicio), Num(f.fim), Num(f.fim - f.inicio),
                Num(f.notaNoInicio), Num(f.notaNoFim), Num(f.notaNoFim - f.notaNoInicio),
                std::to_string(f.acertosSofridos), Bit(f.limpa), Bit(f.completa),
            }));
        }
        return linhas;
    }

    // ---- A LEITURA DE VOLTA ---------------------------------------------------------

    std::vector<std::string> Campos(const std::string& linha) {
        std::vector<std::string> campos;
        std::string atual;
        bool entreAspas = false;
        for (size_t i = 0; i < linha.size(); ++i) {
            const char c = linha[i];
            if (entreAspas) {
                if (c == '"') {
                    if (i + 1 < linha.size() && linha[i + 1] == '"') { atual += '"'; ++i; }
                    else entreAspas = false;
                } else {
                    atual += c;
                }
            } else if (c == '"') {
                entreAspas = true;
            } else if (c == ',') {
                campos.push_back(atual);
                atual.clear();
            } else if (c != '\r') {
                atual += c;
            }
        }
        campos.push_back(atual);
        return campos;
    }

    std::vector<Linha> Ler(const std::string& csv) {
        std::vector<Linha> linhas;
        std::istringstream entrada(csv);
        std::string texto;

        // O CABECALHO DIZ ONDE ESTA CADA COLUNA. Sem ele, ou sem uma das que o
        // resumo usa, o arquivo nao e deste formato: nada se le.
        if (!std::getline(entrada, texto)) return linhas;
        std::map<std::string, size_t> coluna;
        const auto nomes = Campos(texto);
        for (size_t i = 0; i < nomes.size(); ++i) coluna[nomes[i]] = i;
        for (const char* precisa : {"matricula", "visitante", "materia", "nome_da_materia",
                                    "dificuldade", "modo", "resultado", "inicio", "nota_final",
                                    "duracao_s"}) {
            if (!coluna.count(precisa)) return linhas;
        }

        while (std::getline(entrada, texto)) {
            if (texto.empty() || texto == "\r") continue;
            const auto c = Campos(texto);
            const auto campo = [&](const char* nome) -> std::string {
                const size_t i = coluna.at(nome);
                return i < c.size() ? c[i] : "";
            };
            Linha l;
            l.matricula = campo("matricula");
            if (l.matricula.empty()) continue;   // linha cortada no meio
            l.visitante = campo("visitante") == "1";
            l.materia = campo("materia");
            l.nomeDaMateria = campo("nome_da_materia");
            l.dificuldade = campo("dificuldade");
            l.modo = campo("modo");
            l.resultado = campo("resultado");
            l.inicio = campo("inicio");
            l.nota = Numero(campo("nota_final"));
            l.duracao = Numero(campo("duracao_s"));
            linhas.push_back(std::move(l));
        }
        return linhas;
    }

    int ProximaVez(const std::vector<Linha>& linhas, const std::string& matricula, const std::string& materia) {
        int vez = 1;
        for (const Linha& l : linhas) {
            if (!l.visitante && l.matricula == matricula && l.materia == materia) ++vez;
        }
        return vez;
    }

    std::string Resumo(const std::vector<Linha>& linhas) {

        // A CHAVE: aluno, materia, dificuldade e modo. As linhas chegam na ordem
        // do arquivo, que e a do tempo - e isso que da a "vez em que passou".
        using Chave = std::tuple<std::string, std::string, std::string, std::string>;
        struct Conta {
            std::string nomeDaMateria;
            int tentativas = 0, aprovacoes = 0, reprovacoes = 0, abandonos = 0;
            int ateAprovar = 0;
            std::string primeira, primeiraAprovacao, ultima;
            std::set<std::string> dias;
            float melhor = 0.0f, somaDasNotas = 0.0f, tempo = 0.0f;
        };
        std::map<Chave, Conta> contas;

        for (const Linha& l : linhas) {
            if (l.visitante) continue;   // visitantes nao sao a mesma pessoa
            Conta& c = contas[Chave{l.matricula, l.materia, l.dificuldade, l.modo}];
            c.nomeDaMateria = l.nomeDaMateria;
            ++c.tentativas;
            if (l.resultado == "aprovado") ++c.aprovacoes;
            else if (l.resultado == "reprovado") ++c.reprovacoes;
            else ++c.abandonos;
            if (l.Aprovou() && c.ateAprovar == 0) {
                c.ateAprovar = c.tentativas;
                c.primeiraAprovacao = l.inicio;
            }
            if (c.primeira.empty() || l.inicio < c.primeira) c.primeira = l.inicio;
            if (l.inicio > c.ultima) c.ultima = l.inicio;
            c.dias.insert(Data(l.inicio));
            c.melhor = std::max(c.melhor, l.nota);
            c.somaDasNotas += l.nota;
            c.tempo += l.duracao;
        }

        std::string saida = Juntar({"matricula", "materia", "nome_da_materia", "dificuldade", "modo",
                                    "tentativas", "aprovacoes", "reprovacoes", "abandonos",
                                    "tentativas_ate_aprovar", "primeira_tentativa",
                                    "primeira_aprovacao", "ultima_tentativa", "dias_distintos",
                                    "melhor_nota", "nota_media", "tempo_total_s"}) + "\n";
        for (const auto& [chave, c] : contas) {
            const auto& [matricula, materia, dificuldade, modo] = chave;
            saida += Juntar({
                matricula, materia, c.nomeDaMateria, dificuldade, modo,
                std::to_string(c.tentativas), std::to_string(c.aprovacoes),
                std::to_string(c.reprovacoes), std::to_string(c.abandonos),
                c.ateAprovar > 0 ? std::to_string(c.ateAprovar) : "", c.primeira,
                c.primeiraAprovacao, c.ultima, std::to_string(c.dias.size()), Num(c.melhor),
                Num(c.somaDasNotas / static_cast<float>(c.tentativas)), Num(c.tempo),
            }) + "\n";
        }
        return saida;
    }
}
