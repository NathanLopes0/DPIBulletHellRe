//
// A ficha do aluno: a matricula mais o progresso dele, em texto.
//

#include "Ficha.h"

#include <cmath>
#include <set>
#include <sstream>

#include "../Data/JsonDeDados.h"

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

std::string Serializar(const Dados& dados, const Materias::Lista& materias) {

    std::ostringstream s;
    s << "{\n";
    s << "  \"versao\": " << kVersaoAtual << ",\n";
    s << "  \"matricula\": \"" << dados.matricula << "\",\n";

    // So escreve a aparencia quando ha uma. Um aluno sem aparencia escolhida -
    // um save da versao 3, ou um visitante - nao ganha um objeto vazio no
    // arquivo, que nao diria nada que a ausencia ja nao diga.
    //
    // As escolhas saem em ordem de categoria porque Aparencia guarda num mapa:
    // o mesmo estado produz sempre o mesmo texto, e dois saves dao para comparar
    // num diff.
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

    // Entradas() ja devolve em ordem de materia, entao o mesmo estado produz
    // sempre o mesmo texto - o que torna dois saves comparaveis num diff.
    const auto entradas = dados.progresso.Entradas();
    bool primeira = true;
    for (const auto& e : entradas) {

        // Uma nota cujo indice nao existe mais na lista nao tem como ser gravada:
        // sem codigo, ela viraria lixo que nenhuma leitura futura entende.
        const std::string codigo = materias.CodigoDe(e.materia);
        if (codigo.empty()) continue;

        s << (primeira ? "\n" : ",\n");
        primeira = false;
        s << "    { \"materia\": \"" << codigo
          << "\", \"recorde\": "  << Numero(e.recorde)
          << ", \"retomada\": " << Numero(e.retomada);

        // O REGISTRO DE DUPLA SO APARECE SE EXISTIR. Mesma regra da aparencia:
        // um aluno que nunca jogou acompanhado nao ganha tres campos zerados no
        // arquivo, e um save da versao 4 continua abrindo sem conversao nenhuma.
        if (e.recordeDeDupla > 0.0f) {
            s << ", \"recordeDupla\": " << Numero(e.recordeDeDupla);

            // O parceiro pode faltar mesmo havendo recorde: visitante nao tem
            // matricula. A nota vale para o progresso; a dupla e que fica de
            // fora do ranking (requisitos-coop, RF6).
            if (!e.parceiroDoRecorde.empty()) {
                s << ", \"parceiro\": \"" << e.parceiroDoRecorde << "\"";
            }
            if (!e.quandoDaDupla.empty()) {
                s << ", \"quandoDupla\": \"" << e.quandoDaDupla << "\"";
            }
        }

        // So escreve a data quando ha uma: um campo vazio no arquivo nao diria
        // nada que a ausencia dele ja nao diga.
        if (!e.quando.empty()) s << ", \"quando\": \"" << e.quando << "\"";

        s << " }";
    }
    if (!primeira) s << "\n  ";
    s << "]\n";
    s << "}\n";

    return s.str();
}

Lida Desserializar(const std::string& texto, const Materias::Lista& materias) {

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

    // ----- aparencia -----
    //
    // OPCIONAL: as versoes ate a 3 nao tinham o campo, e um save delas abre sem
    // conversao nenhuma - a aparencia fica vazia e cai na padrao na hora de
    // compor. Os IDS NAO SAO CONFERIDOS contra o catalogo aqui; ver o comentario
    // em Ficha::Dados.
    if (raiz.contains("aparencia")) {
        if (!raiz["aparencia"].is_object()) {
            saida.problemas.emplace_back("\"aparencia\" deveria ser um objeto; foi ignorada");
        }
        else {
            for (auto it = raiz["aparencia"].begin(); it != raiz["aparencia"].end(); ++it) {

                if (!it.value().is_object()) {
                    saida.problemas.emplace_back("a aparencia de \"" + it.key() +
                                                 "\" nao e um objeto; descartada");
                    continue;
                }
                if (!it.value().contains("peca") || !it.value()["peca"].is_string()
                    || !it.value().contains("cor") || !it.value()["cor"].is_string()) {
                    saida.problemas.emplace_back("a aparencia de \"" + it.key() +
                                                 "\" precisa de \"peca\" e \"cor\" em texto;"
                                                 " descartada");
                    continue;
                }

                saida.dados.aparencia.Definir(
                    it.key(), Personagens::Escolha{it.value()["peca"].get<std::string>(),
                                                   it.value()["cor"].get<std::string>()});
            }
        }
    }

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

        if (!m.is_object() || !m.contains("materia")) {
            saida.problemas.emplace_back("ha uma materia sem o campo \"materia\"; foi descartada");
            continue;
        }

        // O CAMPO MUDOU DE TIPO ENTRE AS VERSOES, e e aqui que a migracao acontece.
        //
        // Na versao 1 era a POSICAO da materia na lista; na 2 e o CODIGO. Converter
        // a 1 e procurar o codigo que estava naquela posicao - o que so funciona
        // porque a ordem de materias.json ainda e a do enum antigo, e ha um teste
        // em test_arquivos_de_dados.cpp que trava essa ordem justamente ate os
        // saves da versao 1 terem desaparecido.
        int materia = -1;
        std::string codigo;

        if (versao <= 1) {
            if (!m["materia"].is_number_integer()) {
                saida.problemas.emplace_back("ha uma materia que nao e um numero num save da"
                                             " versao 1; foi descartada");
                continue;
            }
            materia = m["materia"].get<int>();
            codigo = materias.CodigoDe(materia);
            if (codigo.empty()) {
                saida.problemas.emplace_back("materia " + std::to_string(materia) +
                                             " do save antigo nao existe mais na lista de"
                                             " materias; descartada");
                continue;
            }
        }
        else {
            if (!m["materia"].is_string()) {
                saida.problemas.emplace_back("ha uma materia que nao e um codigo de texto;"
                                             " foi descartada");
                continue;
            }
            codigo = m["materia"].get<std::string>();
            materia = materias.IndiceDe(codigo);
            if (materia < 0) {
                // A materia saiu do curso. A nota some junto, e isso e o correto:
                // guardar nota de materia que nao existe nao serve a ninguem.
                saida.problemas.emplace_back("a materia \"" + codigo + "\" nao esta na lista"
                                             " de materias; a nota dela foi descartada");
                continue;
            }
        }

        const std::string onde = "materia " + codigo + ": ";

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

        // ---- O registro de dupla, opcional (versao 5) --------------------
        //
        // Saves ate a versao 4 nao tem nenhum destes campos, e isso NAO e erro:
        // e a verdade sobre aquele aluno, que nunca jogou acompanhado.
        float recordeDeDupla = 0.0f;
        if (m.contains("recordeDupla") && m["recordeDupla"].is_number()) {
            recordeDeDupla = m["recordeDupla"].get<float>();

            if (!NotaServe(recordeDeDupla)) {
                saida.problemas.emplace_back(onde + "a nota de dupla esta fora de 0 a 100;"
                                                    " o registro de dupla foi descartado");
                recordeDeDupla = 0.0f;
            }
        }

        // A RETOMADA E COMPARADA COM A MAIOR DAS DUAS.
        //
        // Era comparada so com o recorde de solo, como estado impossivel - e
        // deixou de ser impossivel com o co-op: um aluno com recorde 50 sozinho
        // que faz 80 em dupla fica com retomada 80 e recorde de solo 50. Essa
        // ficha e legitima, e a regra antiga a descartaria inteira.
        if (retomada > std::max(recorde, recordeDeDupla)) {
            // Estado impossivel: o recorde e o maximo de todas as notas. Se
            // passasse, "melhor nota" deixaria de ser a melhor e as regras de
            // aprovacao ficariam erradas.
            saida.problemas.emplace_back(onde + "a retomada e maior que os dois recordes, o que"
                                                " nao pode acontecer jogando; descartada");
            continue;
        }

        Progresso::Entrada e;
        e.materia = materia;
        e.recorde = recorde;
        e.retomada = retomada;
        e.recordeDeDupla = recordeDeDupla;

        // Opcional: saves anteriores a este campo simplesmente nao tem data, e
        // isso nao e erro.
        if (m.contains("quando") && m["quando"].is_string()) {
            e.quando = m["quando"].get<std::string>();
        }

        // O parceiro e a data da dupla so fazem sentido havendo recorde de
        // dupla: sem ele, seriam o nome de alguem com quem nunca se jogou.
        if (recordeDeDupla > 0.0f) {
            if (m.contains("parceiro") && m["parceiro"].is_string()) {
                e.parceiroDoRecorde = m["parceiro"].get<std::string>();
            }
            if (m.contains("quandoDupla") && m["quandoDupla"].is_string()) {
                e.quandoDaDupla = m["quandoDupla"].get<std::string>();
            }
        }

        // REPETIDA: fica valendo a ultima, INTEIRA. Antes so o recorde e a
        // retomada eram trocados, entao a data da primeira ficava pendurada na
        // nota da segunda - e agora seriam tres campos a esquecer em vez de um.
        if (!vistas.insert(materia).second) {
            saida.problemas.emplace_back(onde + "aparece repetida; fica valendo a ultima");
            for (auto& anterior : entradas) {
                if (anterior.materia == materia) { anterior = e; break; }
            }
            continue;
        }

        entradas.push_back(e);
    }

    saida.dados.progresso.Restaurar(entradas);
    saida.ok = true;
    return saida;
}

}
