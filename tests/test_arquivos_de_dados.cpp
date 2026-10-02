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

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include "../Source/Attacks/FasesDeAtaque.h"
#include "../Source/Attacks/PathShapes.h"
#include "../Source/Attacks/RegrasDeAtaque.h"
#include "../Source/Attacks/ProjeteisDeChefe.h"
#include "../Source/JsonDeDados.h"

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
    for (const auto& par : r.conjuntos) {
        const std::string& chefe = par.first;
        const auto& fases = par.second;
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

    for (const auto& par : fases.conjuntos) {
        const std::string& chefe = par.first;
        const auto& lista = par.second;
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

    for (const auto& par : regras.conjuntos) {
        const std::string& nome = par.first;
        const auto& lista = par.second;
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

    for (const auto& par : fases.conjuntos) {
        const std::string& chefe = par.first;
        const auto& lista = par.second;
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

    for (const auto& par : fases.conjuntos) {
        const std::string& chefe = par.first;
        const auto& lista = par.second;
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

    for (const auto& par : r.conjuntos) {
        const std::string& chefe = par.first;
        const auto& lista = par.second;
        for (const auto& f : lista) {
            for (const auto& a : f.ataques) {
                CAPTURE(chefe);
                CAPTURE(f.nome);
                CHECK(a.cooldown > 0.0f);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Os tipos de projetil
//
// Um erro aqui e da mesma familia dos tres que abrem este arquivo: nao quebra o
// jogo, deixa-o silenciosamente errado. Um "projetil" citado numa fase com nome
// que nao existe faz cada disparo daquele ataque virar uma linha de log e nenhum
// tiro em tela - o chefe parece estar com defeito. Uma animacao que cita um
// quadro que a folha nao tem desenha o quadro errado, ou nada.
// ---------------------------------------------------------------------------

TEST_CASE("Dados: Assets/Attacks/projeteis.json nao tem problema nenhum") {
    const auto r = LerProjeteis(LerArquivo("Attacks/projeteis.json"));
    CHECK_MESSAGE(r.problemas.empty(), "problemas em projeteis.json:" << Juntar(r.problemas));
    CHECK(r.conjuntos.size() > 0);
}

TEST_CASE("Dados: todo chefe tem um conjunto de projeteis") {
    // Sem conjunto, o chefe monta e anda, mas nenhum ataque encontra a fabrica
    // que pede: ele fica inofensivo, o que em jogo parece bug e nao erro de dados.
    const auto r = LerProjeteis(LerArquivo("Attacks/projeteis.json"));
    for (const auto& chefe : kChefes) {
        CAPTURE(chefe);
        CHECK(r.conjuntos.count(chefe) == 1);
    }
}

TEST_CASE("Dados: todo projetil citado em fases.json existe em projeteis.json") {
    const auto fases = LerFases(LerArquivo("Attacks/fases.json"));
    const auto projeteis = LerProjeteis(LerArquivo("Attacks/projeteis.json"));

    for (const auto& chefe : kChefes) {
        REQUIRE(fases.conjuntos.count(chefe) == 1);
        REQUIRE(projeteis.conjuntos.count(chefe) == 1);

        const auto& disponiveis = projeteis.conjuntos.at(chefe);

        for (const auto& f : fases.conjuntos.at(chefe)) {
            for (const auto& a : f.ataques) {
                const std::string onde = chefe + "/" + f.nome;
                CAPTURE(onde);
                CAPTURE(a.projetil);
                CHECK(disponiveis.count(a.projetil) == 1);
            }
        }
    }
}

TEST_CASE("Dados: o sprite e o atlas de todo projetil existem no disco") {
    // Este teste pega o erro que passou anos escondido na fabrica da capivara:
    // ela declarava "Capivara.json", arquivo que nunca existiu. Ninguem notou
    // porque o codigo repetia o caminho certo na mao e nunca lia o campo errado.
    const auto r = LerProjeteis(LerArquivo("Attacks/projeteis.json"));

    for (const auto& conjunto : r.conjuntos) {
        for (const auto& entrada : conjunto.second) {
            const std::string onde = conjunto.first + "/" + entrada.first;
            CAPTURE(onde);

            const std::string png = std::string(DPI_ASSETS_DIR) + "/" + entrada.second.sprite;
            const std::string json = std::string(DPI_ASSETS_DIR) + "/" + entrada.second.dados;

            std::ifstream a(png);
            std::ifstream b(json);
            CHECK_MESSAGE(a.is_open(), "nao existe: " << png);
            CHECK_MESSAGE(b.is_open(), "nao existe: " << json);
        }
    }
}

TEST_CASE("Dados: nenhuma animacao cita um quadro que a folha nao tem") {
    // O DrawAnimatedComponent nao reclama de indice fora da lista de quadros; ele
    // desenha o que achar. Entao um 12 numa folha de 12 quadros (indices 0 a 11) e
    // exatamente o tipo de erro que so aparece em jogo, e so as vezes.
    const auto r = LerProjeteis(LerArquivo("Attacks/projeteis.json"));

    for (const auto& conjunto : r.conjuntos) {
        for (const auto& entrada : conjunto.second) {

            const std::string onde = conjunto.first + "/" + entrada.first;
            CAPTURE(onde);

            const auto atlas = LerJsonDeDados(LerArquivo(entrada.second.dados));
            REQUIRE(atlas.contains("frames"));

            // O Aseprite exporta "frames" como lista ou como objeto, conforme a
            // opcao de exportacao, e o jogo tem exemplos dos dois.
            const size_t total = atlas["frames"].size();
            CHECK(total > 0);

            for (const auto& anim : entrada.second.animacoes) {
                CAPTURE(anim.nome);
                for (const int quadro : anim.quadros) {
                    CAPTURE(quadro);
                    CHECK(static_cast<size_t>(quadro) < total);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// A tranca da migracao
//
// Os valores abaixo foram lidos das SEIS fabricas em C++ antes de elas serem
// apagadas. Este teste existe para que a migracao de C++ para dados possa ser
// chamada de equivalente com alguma base, e nao so porque o jogo abriu.
//
// Ele e deliberadamente rigido: mexer de proposito no balanceamento de um
// projetil QUEBRA este teste. Isso e o comportamento desejado - a falha obriga a
// mudar o numero aqui tambem, e o diff do commit passa a mostrar a intencao.
// ---------------------------------------------------------------------------

TEST_CASE("Dados: os projeteis mantem exatamente o que as fabricas em C++ tinham") {

    const auto r = LerProjeteis(LerArquivo("Attacks/projeteis.json"));

    struct Esperado {
        const char* conjunto;
        const char* nome;
        const char* sprite;
        float escala;
        int ordem;
        const char* inicial;
        const char* dimensao;
        float divisor;
        bool posicionaNoDono;
        float margemEmSprites;
        float margemDivisor;
        size_t animacoes;
    };

    const std::vector<Esperado> esperados = {
        {"salles",  "Capivara", "Teachers/Projectiles/DPIBHSallesCapivara.png",
         1.0f, 90, "Normal", "largura", 2.0f, true,  1.0f, 12.0f, 2},
        {"ricardo", "Arduino",  "Teachers/Projectiles/DPIBHRicardoProjectile.png",
         2.0f, 90, "Normal", "largura", 2.0f, true,  1.0f, 12.0f, 1},
        {"julio",   "Dados",    "Teachers/Projectiles/DPIBHJulioDado.png",
         2.0f, 90, "Coleta", "largura", 2.0f, true,  1.0f, 12.0f, 3},
        // Os baloes sao o unico que difere em tres campos de uma vez, e os tres
        // eram de proposito: hitbox menor que o desenho (divisor 4), nao nascer na
        // posicao do chefe (a BaloonAttack escolhe o lado da tela) e morrer com
        // folga de dois sprites sem parcela de tela.
        {"andre",   "Baloes",   "Teachers/Projectiles/DPIBHAndreBaloon.png",
         2.0f, 90, "Red",    "largura", 4.0f, false, 2.0f,  0.0f, 3},

        // Os dois do redesenho do Salles por Estruturas de Dados. O "Duplamente"
        // tira o raio do colisor da ALTURA porque o no e bem mais largo que alto -
        // os campos PREV e NEXT ficam lado a lado - e pela largura a hitbox
        // redonda sairia bem maior que o desenho.
        {"salles",  "Estruturas", "Teachers/Projectiles/DPIBHSallesEstruturas.png",
         1.0f, 90, "ListaSimples", "largura", 2.0f, true,  1.0f, 12.0f, 2},
        {"salles",  "Duplamente", "Teachers/Projectiles/DPIBHSallesDuplamente.png",
         0.75f, 90, "Normal",     "altura",  2.0f, false, 1.0f, 12.0f, 1},
    };

    for (const auto& e : esperados) {
        CAPTURE(e.conjunto);
        CAPTURE(e.nome);

        REQUIRE(r.conjuntos.count(e.conjunto) == 1);
        REQUIRE(r.conjuntos.at(e.conjunto).count(e.nome) == 1);

        const auto& p = r.conjuntos.at(e.conjunto).at(e.nome);
        CHECK(p.sprite == e.sprite);
        CHECK(p.escala == doctest::Approx(e.escala));
        CHECK(p.ordemDeDesenho == e.ordem);
        CHECK(p.animacaoInicial == e.inicial);
        CHECK(p.colisorDimensao == e.dimensao);
        CHECK(p.colisorDivisor == doctest::Approx(e.divisor));
        CHECK(p.posicionarNoDono == e.posicionaNoDono);
        CHECK(p.margemEmSprites == doctest::Approx(e.margemEmSprites));
        CHECK(p.margemDivisorDeTela == doctest::Approx(e.margemDivisor));
        CHECK(p.animacoes.size() == e.animacoes);
    }
}

TEST_CASE("Dados: as animacoes dos projeteis mantem os quadros exatos") {
    // Separado do caso acima porque a comparacao e de listas e a mensagem de falha
    // precisa dizer QUAL quadro mudou.
    const auto r = LerProjeteis(LerArquivo("Attacks/projeteis.json"));

    struct AnimEsperada {
        const char* conjunto;
        const char* projetil;
        const char* animacao;
        std::vector<int> quadros;
    };

    const std::vector<AnimEsperada> esperadas = {
        {"salles",  "Capivara", "Normal",      {0}},
        {"salles",  "Capivara", "Homing",      {1}},
        {"ricardo", "Arduino",  "Normal",      {0}},
        {"julio",   "Dados",    "Coleta",      {0, 1, 2, 3}},
        {"julio",   "Dados",    "Perseguicao", {4, 5, 6, 7}},
        {"julio",   "Dados",    "Previsao",    {8, 9, 10, 11}},
        // A folha dos baloes tem 24 quadros e as animacoes comecam no 6: os seis
        // primeiros sao de uma versao antiga e nenhuma animacao os usa.
        {"andre",   "Baloes",   "Red",         {6, 7, 8, 9, 10, 11}},
        {"andre",   "Baloes",   "Blue",        {12, 13, 14, 15, 16, 17}},
        {"andre",   "Baloes",   "Yellow",      {18, 19, 20, 21, 22, 23}},
        {"salles",  "Estruturas", "ListaSimples", {0, 1, 2, 3}},
        {"salles",  "Estruturas", "Arvore",       {4, 5, 6, 7}},
        {"salles",  "Duplamente", "Normal",       {0, 1, 2, 3, 4, 5, 6, 7, 8}},
    };

    for (const auto& e : esperadas) {
        CAPTURE(e.conjunto);
        CAPTURE(e.projetil);
        CAPTURE(e.animacao);

        REQUIRE(r.conjuntos.count(e.conjunto) == 1);
        REQUIRE(r.conjuntos.at(e.conjunto).count(e.projetil) == 1);

        const auto& anims = r.conjuntos.at(e.conjunto).at(e.projetil).animacoes;

        const AnimacaoDeProjetil* achada = nullptr;
        for (const auto& a : anims) {
            if (a.nome == e.animacao) { achada = &a; break; }
        }
        REQUIRE(achada != nullptr);
        CHECK(achada->quadros == e.quadros);
    }
}

// ---------------------------------------------------------------------------
// O acoplamento entre a onda e o caminho
//
// Quando um ataque e WaveAttack e as regras dele tem um Path com atraso, os dois
// numeros NAO sao independentes. A WaveAttack da a cada projetil um
// DeactivateBehavior(0) e um ActivateBehavior(i * intervalo): o no i fica parado,
// com velocidade ZERO, ate o instante de despertar.
//
// O PathBehavior captura a direcao do caminho no instante em que ativa, a partir
// da velocidade do projetil. Se ele ativar num no que ainda dorme, nao ha direcao
// para capturar, ele cai na rotacao identidade e o no sai voando para a direita em
// vez de seguir a fila.
//
// Ou seja: o atraso do caminho tem de ser maior que o despertar do ULTIMO no, que
// e (projeteis - 1) * intervalo. Isto nao aparece em nenhuma API - nada no C++
// impede a combinacao errada, e em jogo ela nao parece bug de dados, parece bug de
// motor: um tiro da fila sai torto de vez em quando. Por isso o acoplamento e
// conferido aqui.
// ---------------------------------------------------------------------------

TEST_CASE("Dados: num WaveAttack, o atraso do caminho supera o despertar do ultimo no") {

    const auto fases = LerFases(LerArquivo("Attacks/fases.json"));
    const auto regras = LerRegras(LerArquivo("Attacks/regras.json"));

    int combinacoesConferidas = 0;

    for (const auto& chefe : kChefes) {
        REQUIRE(fases.conjuntos.count(chefe) == 1);

        for (const auto& f : fases.conjuntos.at(chefe)) {
            for (const auto& a : f.ataques) {

                if (a.estrategia != "WaveAttack") continue;
                if (a.regrasNome.empty() || regras.conjuntos.count(a.regrasNome) == 0) continue;

                // Sem intervalo, a WaveAttack dispara tudo junto e nao ha onda -
                // nada a conferir.
                if (!a.intervalo) continue;

                const int quantos = a.projeteis ? *a.projeteis : 0;
                if (quantos <= 1) continue;

                const float ultimoDespertar =
                    static_cast<float>(quantos - 1) * (*a.intervalo);

                for (const auto& r : regras.conjuntos.at(a.regrasNome)) {
                    if (!r.temMotion || r.motion.tipo != "Path") continue;

                    const std::string onde = chefe + "/" + f.nome + " -> " + a.regrasNome;
                    CAPTURE(onde);
                    CAPTURE(quantos);
                    CAPTURE(ultimoDespertar);
                    CAPTURE(r.motion.atraso);

                    CHECK_MESSAGE(r.motion.atraso > ultimoDespertar,
                        "o caminho ativaria num no ainda parado: atraso " << r.motion.atraso
                        << "s nao passa do despertar do ultimo no (" << ultimoDespertar << "s)");

                    ++combinacoesConferidas;
                }
            }
        }
    }

    // O redesenho do Salles por Estruturas de Dados tem exatamente uma combinacao
    // destas (a fase 2, a lista duplamente encadeada que volta). Se este numero
    // cair a zero, o teste passou a nao conferir nada - foi assim que eu quase
    // deixei passar uma medicao invalida antes.
    CHECK(combinacoesConferidas >= 1);
}

TEST_CASE("Dados: toda animacao pedida por uma regra existe no projetil que o ataque dispara") {

    // O quarto erro silencioso desta familia, e o unico que ainda nao tinha rede.
    // DrawAnimatedComponent::SetAnimation com um nome que nao existe nao quebra
    // nada: o projetil simplesmente CONTINUA na animacao anterior. Entao uma regra
    // pedindo "Arvore" num projetil que so tem "ListaSimples" produz uma fase
    // inteira com a arte errada, sem um erro em tela e sem uma linha no log.
    //
    // O cruzamento so e possivel aqui porque este teste ve os tres arquivos ao
    // mesmo tempo: fases.json diz qual projetil e qual conjunto de regras um ataque
    // usa, regras.json diz qual animacao cada regra pede, e projeteis.json diz
    // quais animacoes aquele projetil tem. Nenhum dos tres sabe disso sozinho.

    const auto fases = LerFases(LerArquivo("Attacks/fases.json"));
    const auto regras = LerRegras(LerArquivo("Attacks/regras.json"));
    const auto projeteis = LerProjeteis(LerArquivo("Attacks/projeteis.json"));

    int pedidosConferidos = 0;

    for (const auto& chefe : kChefes) {
        REQUIRE(fases.conjuntos.count(chefe) == 1);
        REQUIRE(projeteis.conjuntos.count(chefe) == 1);

        for (const auto& f : fases.conjuntos.at(chefe)) {
            for (const auto& a : f.ataques) {

                const auto projetil = projeteis.conjuntos.at(chefe).find(a.projetil);
                if (projetil == projeteis.conjuntos.at(chefe).end()) continue;  // outro teste cobre

                std::set<std::string> tem;
                for (const auto& anim : projetil->second.animacoes) tem.insert(anim.nome);

                // As regras em linha contam igual as de conjunto nomeado.
                std::vector<Regra> todas = a.regras;
                if (!a.regrasNome.empty() && regras.conjuntos.count(a.regrasNome) == 1) {
                    const auto& doConjunto = regras.conjuntos.at(a.regrasNome);
                    todas.insert(todas.end(), doConjunto.begin(), doConjunto.end());
                }

                for (const auto& r : todas) {
                    if (r.animacao.empty()) continue;

                    const std::string onde = chefe + "/" + f.nome + " projetil \"" + a.projetil + "\"";
                    CAPTURE(onde);
                    CAPTURE(r.animacao);

                    CHECK_MESSAGE(tem.count(r.animacao) == 1,
                        "a regra pede a animacao \"" << r.animacao << "\", que o projetil \""
                        << a.projetil << "\" nao tem - em jogo ele ficaria com a animacao anterior");

                    ++pedidosConferidos;
                }
            }
        }
    }

    // Se cair a zero, o teste deixou de conferir qualquer coisa.
    CHECK(pedidosConferidos >= 1);
}
