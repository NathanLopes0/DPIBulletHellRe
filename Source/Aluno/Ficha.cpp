//
// A ficha do aluno: a matricula mais o progresso dele, em texto.
//

#include "Ficha.h"

#include <cmath>
#include <optional>
#include <set>
#include <sstream>

#include "../Motor/JsonDeDados.h"

namespace {

    /// A faixa que a batalha permite (Math::Clamp em Battle.cpp). Uma nota fora
    /// disto nao pode ter sido produzida jogando.
    constexpr float kNotaMinima = 0.0f;
    constexpr float kNotaMaxima = 100.0f;

    bool NotaServe(const float n) {
        return std::isfinite(n) && n >= kNotaMinima && n <= kNotaMaxima;
    }

    /// Escreve o numero sem notacao cientifica e sem zeros a toa, para que o
    /// arquivo continue legivel por quem abrir no editor.
    std::string Numero(const float v) {
        std::ostringstream s;
        s.precision(6);
        s << std::noshowpoint << v;
        return s.str();
    }

    using Modo = Progresso::Modo;
    using Dificuldade::Nivel;

    /// O recorde de solo no normal e obrigatorio no arquivo, ao lado da retomada.
    constexpr const char* kRecorde = "recorde";

    /// Os outros tres, numa tabela so: gravar e ler percorrem a mesma lista, entao nao ha como um
    /// recorde ser gravado e esquecido na leitura.
    struct Campos {
        Modo modo;
        Nivel nivel;
        const char* nota;
        const char* parceiro;   ///< nullptr no solo
        const char* quando;     ///< nullptr no solo
    };
    constexpr Campos kOpcionais[] = {
        {Modo::Dupla, Nivel::Normal,  "recordeDupla",        "parceiro",        "quandoDupla"},
        {Modo::Solo,  Nivel::Dificil, "recordeDificil",      nullptr,           nullptr},
        {Modo::Dupla, Nivel::Dificil, "recordeDuplaDificil", "parceiroDificil", "quandoDuplaDificil"},
    };
}

namespace Ficha {

std::string Serializar(const Dados& dados, const Materias::Lista& materias) {

    std::ostringstream s;
    s << "{\n";
    s << "  \"versao\": " << kVersaoAtual << ",\n";
    s << "  \"matricula\": \"" << dados.matricula << "\",\n";

    // So escreve a aparencia quando ha uma.
    if (!dados.aparencia.Vazia()) {
        s << "  \"aparencia\": {";
        bool primeiraEscolha = true;
        for (const auto& par : dados.aparencia.escolhas) {
            s << (primeiraEscolha ? "\n" : ",\n");
            primeiraEscolha = false;
            s << "    \"" << par.first << "\": { \"peca\": \"" << par.second.peca
              << "\", \"cor\": \"" << par.second.cor << "\" }";
        }
        s << "\n  },\n";
    }

    s << "  \"materias\": [";

    // Entradas() ja devolve em ordem de materia, entao o mesmo estado produz sempre o mesmo texto.
    const auto entradas = dados.progresso.Entradas();
    bool primeira = true;
    for (const auto& e : entradas) {

        const std::string codigo = materias.CodigoDe(e.materia);
        if (codigo.empty()) continue;

        s << (primeira ? "\n" : ",\n");
        primeira = false;
        s << "    { \"materia\": \"" << codigo
          << "\", \"" << kRecorde << "\": " << Numero(e.De(Modo::Solo, Nivel::Normal).nota)
          << ", \"retomada\": " << Numero(e.retomada);

        // Um recorde opcional so aparece se existir.
        for (const Campos& c : kOpcionais) {
            const Progresso::Marca& m = e.De(c.modo, c.nivel);
            if (m.nota <= 0.0f) continue;
            s << ", \"" << c.nota << "\": " << Numero(m.nota);

            // O parceiro pode faltar mesmo havendo recorde: visitante nao tem matricula.
            if (c.parceiro && !m.parceiro.empty()) s << ", \"" << c.parceiro << "\": \"" << m.parceiro << "\"";
            if (c.quando && !m.quando.empty()) s << ", \"" << c.quando << "\": \"" << m.quando << "\"";
        }

        // So escreve a data quando ha uma.
        if (!e.quando.empty()) s << ", \"quando\": \"" << e.quando << "\"";

        s << " }";
    }
    if (!primeira) s << "\n  ";
    s << "]\n";
    s << "}\n";

    return s.str();
}

namespace {

    /// O JSON, a versao e a matricula. Devolve false quando a ficha nao serve.
    bool LerCabecalho(const std::string& texto, nlohmann::json& raiz, int& versao,
                      std::vector<std::string>& problemas) {
        try {
            raiz = LerJsonDeDados(texto);
        }
        catch (const std::exception& e) {
            problemas.emplace_back(std::string("a ficha nao e um JSON valido: ") + e.what());
            return false;
        }

        if (!raiz.is_object()) {
            problemas.emplace_back("a ficha deveria ser um objeto");
            return false;
        }

        if (!raiz.contains("versao") || !raiz["versao"].is_number_integer()) {
            problemas.emplace_back("falta \"versao\": todo save gravado por este jogo tem uma,"
                                   " entao um arquivo sem ela nao veio daqui");
            return false;
        }

        versao = raiz["versao"].get<int>();
        if (versao > kVersaoAtual) {
            // Recusar e melhor do que ler errado.
            problemas.emplace_back("esta ficha e da versao " + std::to_string(versao) + " e este jogo le ate a " +
                                   std::to_string(kVersaoAtual) + ". Atualize o jogo para abrir este save.");
            return false;
        }

        if (!raiz.contains("matricula") || !raiz["matricula"].is_string() ||
            raiz["matricula"].get<std::string>().empty()) {
            problemas.emplace_back("falta \"matricula\": sem ela nao da para saber de quem"
                                   " e este progresso");
            return false;
        }
        return true;
    }

    /// As escolhas de aparencia; as malformadas sao descartadas, com o motivo.
    void LerAparencia(const nlohmann::json& raiz, Personagens::Aparencia& aparencia,
                      std::vector<std::string>& problemas) {
        if (raiz.contains("aparencia")) {
            if (!raiz["aparencia"].is_object()) {
                problemas.emplace_back("\"aparencia\" deveria ser um objeto; foi ignorada");
            }
            else {
                for (auto it = raiz["aparencia"].begin(); it != raiz["aparencia"].end(); ++it) {

                    if (!it.value().is_object()) {
                        problemas.emplace_back("a aparencia de \"" + it.key() + "\" nao e um objeto; descartada");
                        continue;
                    }
                    if (!it.value().contains("peca") || !it.value()["peca"].is_string() ||
                        !it.value().contains("cor") || !it.value()["cor"].is_string()) {
                        problemas.emplace_back("a aparencia de \"" + it.key() +
                                               "\" precisa de \"peca\" e \"cor\" em texto;"
                                               " descartada");
                        continue;
                    }

                    aparencia.Definir(it.key(), Personagens::Escolha{it.value()["peca"].get<std::string>(),
                                                                     it.value()["cor"].get<std::string>()});
                }
            }
        }
    }

    /// O indice da materia, pelo codigo (ou pela posicao, num save da versao 1); -1 se ela nao
    /// serve. 'codigo' sai preenchido.
    int IndiceDaMateria(const nlohmann::json& m, const int versao, const Materias::Lista& materias, std::string& codigo,
                        std::vector<std::string>& problemas) {
        // O campo mudou de tipo entre as versoes, e e aqui que a migracao acontece.
        int materia = -1;

        if (versao <= 1) {
            if (!m["materia"].is_number_integer()) {
                problemas.emplace_back("ha uma materia que nao e um numero num save da"
                                       " versao 1; foi descartada");
                return -1;
            }
            materia = m["materia"].get<int>();
            codigo = materias.CodigoDe(materia);
            if (codigo.empty()) {
                problemas.emplace_back("materia " + std::to_string(materia) +
                                       " do save antigo nao existe mais na lista de"
                                       " materias; descartada");
                return -1;
            }
        }
        else {
            if (!m["materia"].is_string()) {
                problemas.emplace_back("ha uma materia que nao e um codigo de texto;"
                                       " foi descartada");
                return -1;
            }
            codigo = m["materia"].get<std::string>();
            materia = materias.IndiceDe(codigo);
            if (materia < 0) {
                // A materia saiu do curso.
                problemas.emplace_back("a materia \"" + codigo +
                                       "\" nao esta na lista"
                                       " de materias; a nota dela foi descartada");
                return -1;
            }
        }
        return materia;
    }

    /// Uma materia inteira: a retomada, os quatro recordes e a data; vazio se ela foi descartada.
    std::optional<Progresso::Entrada> LerMateria(const nlohmann::json& m, const int materia, const std::string& codigo,
                                                 std::vector<std::string>& problemas) {
        const std::string onde = "materia " + codigo + ": ";

        if (!m.contains(kRecorde) || !m[kRecorde].is_number() || !m.contains("retomada") ||
            !m["retomada"].is_number()) {
            problemas.emplace_back(onde + "falta \"recorde\" ou \"retomada\"; descartada");
            return std::nullopt;
        }

        const float recorde = m[kRecorde].get<float>();
        const float retomada = m["retomada"].get<float>();

        if (!NotaServe(recorde) || !NotaServe(retomada)) {
            problemas.emplace_back(onde + "nota fora de 0 a 100, que e a faixa que a"
                                          " batalha permite; descartada");
            return std::nullopt;
        }

        Progresso::Entrada e;
        e.materia = materia;
        e.retomada = retomada;
        e.De(Modo::Solo, Nivel::Normal).nota = recorde;

        // ---- Os outros recordes, opcionais (dupla: versao 5; Dificil: 6) ----
        for (const Campos& c : kOpcionais) {
            if (!m.contains(c.nota) || !m[c.nota].is_number()) continue;

            const float nota = m[c.nota].get<float>();
            if (!NotaServe(nota)) {
                problemas.emplace_back(onde + "\"" + c.nota +
                                       "\" esta fora de 0 a 100;"
                                       " esse recorde foi descartado");
                continue;
            }
            Progresso::Marca& marca = e.De(c.modo, c.nivel);
            marca.nota = nota;

            // O parceiro e a data so fazem sentido havendo recorde.
            if (nota <= 0.0f) continue;
            if (c.parceiro && m.contains(c.parceiro) && m[c.parceiro].is_string()) {
                marca.parceiro = m[c.parceiro].get<std::string>();
            }
            if (c.quando && m.contains(c.quando) && m[c.quando].is_string()) {
                marca.quando = m[c.quando].get<std::string>();
            }
        }

        // A retomada e comparada com o maior dos recordes.
        if (retomada > e.MaiorRecorde()) {
            // Estado impossivel: o recorde e o maximo de todas as notas.
            problemas.emplace_back(onde + "a retomada e maior que todos os recordes, o que"
                                          " nao pode acontecer jogando; descartada");
            return std::nullopt;
        }

        // Opcional: saves anteriores a este campo simplesmente nao tem data, e
        // isso nao e erro.
        if (m.contains("quando") && m["quando"].is_string()) {
            e.quando = m["quando"].get<std::string>();
        }
        return e;
    }
} // namespace

Lida Desserializar(const std::string& texto, const Materias::Lista& materias) {

    Lida saida;
    nlohmann::json raiz;
    int versao = 0;
    if (!LerCabecalho(texto, raiz, versao, saida.problemas)) return saida;

    saida.dados.matricula = raiz["matricula"].get<std::string>();
    LerAparencia(raiz, saida.dados.aparencia, saida.problemas);

    // Sem lista de materias e um aluno que se identificou e ainda nao jogou.
    if (!raiz.contains("materias")) {
        saida.ok = true;
        return saida;
    }

    if (!raiz["materias"].is_array()) {
        saida.problemas.emplace_back("\"materias\" deveria ser uma lista");
        return saida;
    }

    std::vector<Progresso::Entrada> entradas;
    std::set<int> vistas;

    for (const auto& m : raiz["materias"]) {

        if (!m.is_object() || !m.contains("materia")) {
            saida.problemas.emplace_back("ha uma materia sem o campo \"materia\"; foi descartada");
            continue;
        }

        std::string codigo;
        const int materia = IndiceDaMateria(m, versao, materias, codigo, saida.problemas);
        if (materia < 0) continue;

        const std::optional<Progresso::Entrada> e = LerMateria(m, materia, codigo, saida.problemas);
        if (!e) continue;
        const std::string onde = "materia " + codigo + ": ";

        // Repetida: fica valendo a ultima, inteira.
        if (!vistas.insert(materia).second) {
            saida.problemas.emplace_back(onde + "aparece repetida; fica valendo a ultima");
            for (auto& anterior : entradas) {
                if (anterior.materia == materia) { anterior = *e; break; }
            }
            continue;
        }

        entradas.push_back(*e);
    }

    saida.dados.progresso.Restaurar(entradas);
    saida.ok = true;
    return saida;
}

}
