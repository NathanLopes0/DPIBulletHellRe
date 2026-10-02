//
// A ficha do aluno: a matricula mais o progresso dele, em texto.
//

#include "Ficha.h"

#include <cmath>
#include <set>
#include <sstream>

#include "JsonDeDados.h"

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
}

namespace Ficha {

std::string Serializar(const Dados& dados) {

    std::ostringstream s;
    s << "{\n";
    s << "  \"versao\": " << kVersaoAtual << ",\n";
    s << "  \"matricula\": \"" << dados.matricula << "\",\n";
    s << "  \"materias\": [";

    // Entradas() ja devolve em ordem de materia, entao o mesmo estado produz
    // sempre o mesmo texto - o que torna dois saves comparaveis num diff.
    const auto entradas = dados.progresso.Entradas();
    for (size_t i = 0; i < entradas.size(); ++i) {
        s << (i == 0 ? "\n" : ",\n");
        s << "    { \"materia\": " << entradas[i].materia
          << ", \"recorde\": "  << Numero(entradas[i].recorde)
          << ", \"retomada\": " << Numero(entradas[i].retomada) << " }";
    }
    if (!entradas.empty()) s << "\n  ";
    s << "]\n";
    s << "}\n";

    return s.str();
}

Lida Desserializar(const std::string& texto) {

    Lida saida;

    nlohmann::json raiz;
    try {
        raiz = LerJsonDeDados(texto);
    }
    catch (const std::exception& e) {
        saida.problemas.emplace_back(std::string("a ficha nao e um JSON valido: ") + e.what());
        return saida;
    }

    if (!raiz.is_object()) {
        saida.problemas.emplace_back("a ficha deveria ser um objeto");
        return saida;
    }

    if (!raiz.contains("versao") || !raiz["versao"].is_number_integer()) {
        saida.problemas.emplace_back("falta \"versao\": todo save gravado por este jogo tem uma,"
                                     " entao um arquivo sem ela nao veio daqui");
        return saida;
    }

    const int versao = raiz["versao"].get<int>();
    if (versao > kVersaoAtual) {
        // Recusar e melhor do que ler errado: um formato mais novo pode ter mudado
        // o sentido de um campo que esta leitura ainda reconhece.
        saida.problemas.emplace_back("esta ficha e da versao " + std::to_string(versao) +
                                     " e este jogo le ate a " + std::to_string(kVersaoAtual) +
                                     ". Atualize o jogo para abrir este save.");
        return saida;
    }

    if (!raiz.contains("matricula") || !raiz["matricula"].is_string()
        || raiz["matricula"].get<std::string>().empty()) {
        saida.problemas.emplace_back("falta \"matricula\": sem ela nao da para saber de quem"
                                     " e este progresso");
        return saida;
    }

    saida.dados.matricula = raiz["matricula"].get<std::string>();

    // Sem lista de materias e um aluno que se identificou e ainda nao jogou - e
    // uma ficha legitima, nao um erro.
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

        if (!m.is_object() || !m.contains("materia") || !m["materia"].is_number_integer()) {
            saida.problemas.emplace_back("ha uma materia sem o campo \"materia\"; foi descartada");
            continue;
        }

        const int materia = m["materia"].get<int>();
        const std::string onde = "materia " + std::to_string(materia) + ": ";

        if (materia < 0) {
            saida.problemas.emplace_back(onde + "numero negativo; descartada");
            continue;
        }

        if (!m.contains("recorde") || !m["recorde"].is_number()
            || !m.contains("retomada") || !m["retomada"].is_number()) {
            saida.problemas.emplace_back(onde + "falta \"recorde\" ou \"retomada\"; descartada");
            continue;
        }

        const float recorde  = m["recorde"].get<float>();
        const float retomada = m["retomada"].get<float>();

        if (!NotaServe(recorde) || !NotaServe(retomada)) {
            saida.problemas.emplace_back(onde + "nota fora de 0 a 100, que e a faixa que a"
                                                " batalha permite; descartada");
            continue;
        }

        if (retomada > recorde) {
            // Estado impossivel: o recorde e o maximo de todas as notas. Se
            // passasse, "melhor nota" deixaria de ser a melhor e as regras de
            // aprovacao ficariam erradas.
            saida.problemas.emplace_back(onde + "a retomada e maior que o recorde, o que nao"
                                                " pode acontecer jogando; descartada");
            continue;
        }

        if (!vistas.insert(materia).second) {
            saida.problemas.emplace_back(onde + "aparece repetida; fica valendo a ultima");
            for (auto& e : entradas) {
                if (e.materia == materia) { e.recorde = recorde; e.retomada = retomada; break; }
            }
            continue;
        }

        Progresso::Entrada e;
        e.materia = materia;
        e.recorde = recorde;
        e.retomada = retomada;
        entradas.push_back(e);
    }

    saida.dados.progresso.Restaurar(entradas);
    saida.ok = true;
    return saida;
}

}
