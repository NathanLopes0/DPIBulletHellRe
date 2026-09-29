// Os arquivos de dados DE VERDADE, os que o jogo carrega.
//
// ESTE ARQUIVO E DIFERENTE DOS OUTROS DA SUITE, DE PROPOSITO.
//
// Os demais testes sao puros: montam um JSON pequeno em memoria e conferem que o
// leitor entende. Eles respondem "o leitor esta certo?".
//
// Este aqui abre Assets/ e le o que vai junto com o jogo. Ele responde uma
// pergunta diferente: "os DADOS estao certos?". Sao coisas independentes - um
// leitor impecavel le um arquivo com erro de digitacao sem reclamar nada.
//
// POR QUE ISSO EXISTE
//
// Tres erros de digitacao nestes arquivos NAO quebram o jogo; eles o deixam
// silenciosamente errado:
//
//   1. "regras": "salles_fase1x" em fases.json - as fases carregam, o ataque
//      dispara, e os projeteis saem SEM COMPORTAMENTO NENHUM. Nenhum erro em
//      tela, so uma linha no log que ninguem le durante o jogo.
//   2. "forma": "LacoRapidoo" em regras.json - o projetil voa RETO em vez de
//      fazer o laco. A fase inteira perde o carater e continua jogavel.
//   3. "proximo": "StateTow" em fases.json - o conjunto e recusado e o chefe
//      fica parado.
//
// A configuracao de reserva em C++ nunca protegeu contra os dois primeiros,
// porque neles as fases carregam sem problema. Este teste protege, e e ele que
// torna seguro apagar a reserva: um erro nos dados passa a QUEBRAR A SUITE, que
// e onde da para ver, em vez de aparecer como uma fase estranha meia hora depois.
//
// O caminho de Assets/ vem do CMake (DPI_ASSETS_DIR), e nao de um caminho
// relativo, porque o diretorio de trabalho do executavel de teste depende de
// quem o chama - o CLion, o ctest e o terminal usam diretorios diferentes.

#include "doctest.h"

#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include "../Source/Attacks/FasesDeAtaque.h"
#include "../Source/Attacks/PathShapes.h"
#include "../Source/Attacks/RegrasDeAtaque.h"

#ifndef DPI_ASSETS_DIR
#error "DPI_ASSETS_DIR nao foi definido. Veja target_compile_definitions(dpi_tests ...) no CMakeLists."
#endif

namespace {

    std::string LerArquivo(const std::string& caminhoRelativo) {
        const std::string caminho = std::string(DPI_ASSETS_DIR) + "/" + caminhoRelativo;
        std::ifstream arquivo(caminho);
        REQUIRE_MESSAGE(arquivo.is_open(), "nao consegui abrir " << caminho);
        std::ostringstream conteudo;
        conteudo << arquivo.rdbuf();
        return conteudo.str();
    }

    /// Junta as frases de problema numa mensagem so, para a falha do teste dizer
    /// O QUE esta errado em vez de so "esperava lista vazia".
    std::string Juntar(const std::vector<std::string>& problemas) {
        std::string s;
        for (const auto& p : problemas) s += "\n    - " + p;
        return s;
    }

    /// Os nomes de forma que existem em codigo e portanto nao precisam estar no
    /// arquivo. Espelha FormaPeloNome em RegrasDeAtaqueArquivo.cpp.
    const std::set<std::string> kFormasEmCodigo = {"Reta", ""};

    const std::vector<std::string> kChefes = {"salles", "ricardo", "andre", "julio"};

}

// ---------------------------------------------------------------------------
// Cada arquivo, por si
// ---------------------------------------------------------------------------

TEST_CASE("Dados: Assets/Paths/formas.json nao tem problema nenhum") {
    const auto r = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));
    CHECK_MESSAGE(r.problemas.empty(), "problemas em formas.json:" << Juntar(r.problemas));
    CHECK(r.formas.size() > 0);
}

TEST_CASE("Dados: Assets/Attacks/regras.json nao tem problema nenhum") {
    const auto r = LerRegras(LerArquivo("Attacks/regras.json"));
    CHECK_MESSAGE(r.problemas.empty(), "problemas em regras.json:" << Juntar(r.problemas));
    CHECK(r.conjuntos.size() > 0);
}

TEST_CASE("Dados: Assets/Attacks/fases.json nao tem problema nenhum") {
    const auto r = LerFases(LerArquivo("Attacks/fases.json"));
    CHECK_MESSAGE(r.problemas.empty(), "problemas em fases.json:" << Juntar(r.problemas));
}

// ---------------------------------------------------------------------------
// Os quatro chefes
// ---------------------------------------------------------------------------

TEST_CASE("Dados: os quatro chefes existem e suas transicoes fecham") {
    // Sem a reserva em C++, um conjunto recusado aqui e um chefe parado em tela.
    const auto r = LerFases(LerArquivo("Attacks/fases.json"));

    for (const auto& chefe : kChefes) {
        CAPTURE(chefe);
        REQUIRE_MESSAGE(r.conjuntos.count(chefe) == 1,
                        "falta o conjunto \"" << chefe << "\" em fases.json");
        const auto problemas = ValidarTransicoes(r.conjuntos.at(chefe), chefe);
        CHECK_MESSAGE(problemas.empty(), "transicoes de \"" << chefe << "\":" << Juntar(problemas));
    }
}

TEST_CASE("Dados: todo chefe tem as quatro fases") {
    // Nao e exigencia do formato - um chefe pode ter tres. E exigencia DESTE
    // jogo: os quatro chefes atuais tem StateOne, Two, Three e Final, e perder
    // uma delas por um recorte errado seria dificil de notar jogando.
    const auto r = LerFases(LerArquivo("Attacks/fases.json"));
    for (const auto& chefe : kChefes) {
        CAPTURE(chefe);
        REQUIRE(r.conjuntos.count(chefe) == 1);
        std::set<std::string> nomes;
        for (const auto& f : r.conjuntos.at(chefe)) nomes.insert(f.nome);
        CHECK(nomes.count("StateOne") == 1);
        CHECK(nomes.count("StateTwo") == 1);
        CHECK(nomes.count("StateThree") == 1);
        CHECK(nomes.count("StateFinal") == 1);
    }
}

TEST_CASE("Dados: toda fase tem pelo menos um ataque") {
    // LerFases ja descarta fase sem ataque, entao isto pega o descarte: a fase
    // some do conjunto e o chefe fica parado 17 segundos naquele trecho.
    const auto r = LerFases(LerArquivo("Attacks/fases.json"));
    for (const auto& [chefe, fases] : r.conjuntos) {
        for (const auto& f : fases) {
            CAPTURE(chefe);
            CAPTURE(f.nome);
            CHECK(f.ataques.size() >= 1);
        }
    }
}

// ---------------------------------------------------------------------------
// As referencias entre os arquivos
//
// E aqui que moram os erros que o jogo NAO denuncia em tela.
// ---------------------------------------------------------------------------

TEST_CASE("Dados: todo conjunto de regras citado em fases.json existe em regras.json") {
    // O erro silencioso numero 1: o nome nao bate, o ataque dispara normalmente e
    // os projeteis saem sem comportamento nenhum.
    const auto fases = LerFases(LerArquivo("Attacks/fases.json"));
    const auto regras = LerRegras(LerArquivo("Attacks/regras.json"));

    for (const auto& [chefe, lista] : fases.conjuntos) {
        for (const auto& f : lista) {
            for (size_t k = 0; k < f.ataques.size(); ++k) {
                const auto& a = f.ataques[k];
                if (a.regrasNome.empty()) continue;   // sem regras, ou regras em linha
                CAPTURE(chefe);
                CAPTURE(f.nome);
                CAPTURE(k);
                CAPTURE(a.regrasNome);
                CHECK_MESSAGE(regras.conjuntos.count(a.regrasNome) == 1,
                              "o conjunto de regras \"" << a.regrasNome << "\" nao existe em regras.json");
            }
        }
    }
}

TEST_CASE("Dados: toda forma citada em regras.json existe em formas.json") {
    // O erro silencioso numero 2: o nome nao bate, PathShapes::DoArquivo devolve
    // uma reta, e o projetil voa reto em vez de fazer a curva.
    const auto regras = LerRegras(LerArquivo("Attacks/regras.json"));
    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    auto conferir = [&](const std::string& conjunto, const DescricaoDeBehavior& d) {
        if (d.tipo != "Path") return;
        if (kFormasEmCodigo.count(d.forma) == 1) return;
        CAPTURE(conjunto);
        CAPTURE(d.forma);
        CHECK_MESSAGE(formas.formas.count(d.forma) == 1,
                      "a forma \"" << d.forma << "\" nao existe em formas.json");
    };

    for (const auto& [nome, lista] : regras.conjuntos) {
        for (const auto& r : lista) {
            if (r.temMotion) conferir(nome, r.motion);
            for (const auto& m : r.modifiers) conferir(nome, m);
        }
    }
}

TEST_CASE("Dados: as formas citadas nas regras EM LINHA de fases.json tambem existem") {
    // Um ataque pode trazer as regras escritas nele mesmo, e elas citam formas do
    // mesmo jeito. O caminho e outro, o erro de digitacao e o mesmo.
    const auto fases = LerFases(LerArquivo("Attacks/fases.json"));
    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    for (const auto& [chefe, lista] : fases.conjuntos) {
        for (const auto& f : lista) {
            for (const auto& a : f.ataques) {
                for (const auto& r : a.regras) {
                    if (!r.temMotion || r.motion.tipo != "Path") continue;
                    if (kFormasEmCodigo.count(r.motion.forma) == 1) continue;
                    CAPTURE(chefe);
                    CAPTURE(f.nome);
                    CAPTURE(r.motion.forma);
                    CHECK(formas.formas.count(r.motion.forma) == 1);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Coerencia interna dos ataques
// ---------------------------------------------------------------------------

TEST_CASE("Dados: uma regra por indice nao aponta para fora da rajada") {
    // O caso concreto: salles_final_variado tem regras para os indices 0, 1 e 2,
    // e o ataque dispara 3 projeteis. Se alguem aumentar para 4 em fases.json e
    // esquecer das regras, o quarto projetil sai sem caminho nenhum e voa reto,
    // sem nenhum aviso. Se DIMINUIR para 2, a regra do indice 2 nunca se aplica.
    const auto fases = LerFases(LerArquivo("Attacks/fases.json"));
    const auto regras = LerRegras(LerArquivo("Attacks/regras.json"));

    for (const auto& [chefe, lista] : fases.conjuntos) {
        for (const auto& f : lista) {
            for (const auto& a : f.ataques) {

                // Quantos projeteis o ataque dispara. Ausente = o padrao do
                // AttackParams, que e 10.
                const int n = a.projeteis.value_or(10);

                std::vector<Regra> aplicaveis = a.regras;
                if (!a.regrasNome.empty() && regras.conjuntos.count(a.regrasNome)) {
                    const auto& r = regras.conjuntos.at(a.regrasNome);
                    aplicaveis.insert(aplicaveis.end(), r.begin(), r.end());
                }

                // Todo indice entre 0 e n-1 precisa ser coberto por alguma regra
                // com condicao Indices, se o conjunto usar esse estilo.
                bool usaIndices = false;
                int maiorIndice = -1;
                for (const auto& r : aplicaveis) {
                    if (r.condicao != Regra::Indices) continue;
                    usaIndices = true;
                    for (const int i : r.indices) maiorIndice = std::max(maiorIndice, i);
                }
                if (!usaIndices) continue;

                CAPTURE(chefe);
                CAPTURE(f.nome);
                CAPTURE(a.regrasNome);
                CAPTURE(n);
                CAPTURE(maiorIndice);
                // O maior indice citado tem de caber na rajada...
                CHECK(maiorIndice < n);
                // ...e a rajada nao pode ter projetil alem do ultimo indice
                // citado, senao ele sai sem regra.
                CHECK(maiorIndice == n - 1);
            }
        }
    }
}

TEST_CASE("Dados: um cooldown derivado do ritmo tem mesmo um ritmo para derivar") {
    // LerFases ja recusa isso, entao este teste confere que nenhum ataque REAL
    // caiu na armadilha - o ataque inteiro sumiria do conjunto, e o chefe
    // atacaria de menos sem nenhum sinal em tela.
    const auto r = LerFases(LerArquivo("Attacks/fases.json"));
    CHECK_MESSAGE(r.problemas.empty(), "problemas em fases.json:" << Juntar(r.problemas));

    for (const auto& [chefe, lista] : r.conjuntos) {
        for (const auto& f : lista) {
            for (const auto& a : f.ataques) {
                CAPTURE(chefe);
                CAPTURE(f.nome);
                CHECK(a.cooldown > 0.0f);
            }
        }
    }
}
