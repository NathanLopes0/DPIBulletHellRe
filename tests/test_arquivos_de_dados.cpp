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
#include "../Source/Materias.h"
#include "../Source/Navegacao.h"
#include "../Source/Personagens.h"
#include "../Source/Progresso.h"
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

TEST_CASE("Dados: uma parada de caminho aponta para um ponto que a forma tem") {

    // A parada ("pararNoPonto") e o indice de um waypoint. Se ele for maior que o
    // numero de pontos da forma, o projetil NUNCA chega nele e simplesmente nao
    // para - sem erro em tela, sem linha no log, e a fase perde o que a define.
    //
    // E o mesmo tipo de erro das regras por indice que apontam para fora da
    // rajada, e merece a mesma rede: um numero que nao bate com um tamanho que
    // mora em outro arquivo.

    const auto regras = LerRegras(LerArquivo("Attacks/regras.json"));
    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    int paradasConferidas = 0;

    for (const auto& conjunto : regras.conjuntos) {
        for (const auto& r : conjunto.second) {

            if (!r.temMotion || r.motion.pararNoPonto < 0) continue;

            const std::string onde = conjunto.first + ", forma \"" + r.motion.forma + "\"";
            CAPTURE(onde);
            CAPTURE(r.motion.pararNoPonto);

            // Uma forma que vem do codigo (Reta) nao esta no arquivo; a conferencia
            // de nome ja e feita por outro teste, entao aqui so pulo.
            const auto forma = formas.formas.find(r.motion.forma);
            if (forma == formas.formas.end()) continue;

            const int pontos = static_cast<int>(forma->second.size());
            CAPTURE(pontos);

            // A exigencia e pararNoPonto < pontos - 1, nao < pontos: PARAR NO
            // ULTIMO PONTO NAO SERVE PARA NADA. O caminho termina logo depois e o
            // projetil segue reto com a velocidade que tinha, entao a parada vira
            // um soluco antes de sumir, e nunca a espera ANTES de alguma coisa.
            // Descobri isto sabotando a forma da fase 2 para um unico ponto: o
            // teste passava e a fase perdia a volta em silencio.
            CHECK_MESSAGE(r.motion.pararNoPonto < pontos - 1,
                "a parada aponta para o ponto " << r.motion.pararNoPonto
                << " de uma forma com " << pontos << " pontos: ou o projetil nunca"
                << " chega nele, ou para no ultimo e o caminho acaba logo em seguida"
                << " - de um jeito ou de outro a parada nao faz o que se espera");

            ++paradasConferidas;
        }
    }

    // A fase 2 do Salles tem exatamente uma. Zero aqui quer dizer que o teste
    // parou de conferir qualquer coisa.
    CHECK(paradasConferidas >= 1);
}

// ---------------------------------------------------------------------------
// Familias de forma
//
// Uma familia e uma entrada de formas.json que vale por varias formas, uma por
// projetil - "divisoes": 3 numa arvore vira oito folhas. Isso troca o acoplamento
// antigo (N formas escritas a mao + N regras por indice) por um so: a quantidade
// de projeteis do ataque tem de bater com o tamanho da familia.
//
// E esse unico acoplamento que os dois testes abaixo guardam. Sem eles, mudar
// "divisoes" sem mudar "projeteis" nao da erro nenhum: com projeteis de menos,
// metade da arvore simplesmente nao aparece; com projeteis de mais, os extras
// repetem ramos. Nos dois casos a fase fica errada em silencio.
// ---------------------------------------------------------------------------

TEST_CASE("Dados: toda familia citada por formaPorIndice existe em formas.json") {

    const auto regras = LerRegras(LerArquivo("Attacks/regras.json"));
    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    int conferidas = 0;

    for (const auto& conjunto : regras.conjuntos) {
        for (const auto& r : conjunto.second) {
            if (!r.temMotion || r.motion.formaPorIndice.empty()) continue;

            const std::string onde = conjunto.first;
            CAPTURE(onde);
            CAPTURE(r.motion.formaPorIndice);

            CHECK_MESSAGE(formas.familias.count(r.motion.formaPorIndice) == 1,
                "a regra pede a familia \"" << r.motion.formaPorIndice
                << "\", que nao existe em formas.json");

            // O engano mais provavel agora que ha os dois: pedir por indice uma
            // forma que e simples, ou pedir direto uma que e familia.
            CHECK_MESSAGE(formas.formas.count(r.motion.formaPorIndice) == 0,
                "\"" << r.motion.formaPorIndice << "\" e uma forma simples, nao uma familia"
                << " - a regra deveria usar \"forma\"");

            ++conferidas;
        }
    }

    CHECK(conferidas >= 1);
}

TEST_CASE("Dados: um ataque que usa familia dispara exatamente o tamanho dela") {

    // O UNICO numero que precisa andar junto depois que a arvore virou familia.
    // Antes eram tres lugares (formas, regras por indice e projeteis); agora e
    // "divisoes" de um lado e "projeteis" do outro, e este teste liga os dois.

    const auto fases = LerFases(LerArquivo("Attacks/fases.json"));
    const auto regras = LerRegras(LerArquivo("Attacks/regras.json"));
    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    int conferidos = 0;

    for (const auto& chefe : kChefes) {
        REQUIRE(fases.conjuntos.count(chefe) == 1);

        for (const auto& f : fases.conjuntos.at(chefe)) {
            for (const auto& a : f.ataques) {

                if (a.regrasNome.empty() || regras.conjuntos.count(a.regrasNome) == 0) continue;

                for (const auto& r : regras.conjuntos.at(a.regrasNome)) {
                    if (!r.temMotion || r.motion.formaPorIndice.empty()) continue;

                    const auto familia = formas.familias.find(r.motion.formaPorIndice);
                    if (familia == formas.familias.end()) continue;   // o outro teste cobre

                    const int tamanho = static_cast<int>(familia->second.size());
                    const int disparados = a.projeteis ? *a.projeteis : 0;

                    const std::string onde = chefe + "/" + f.nome + " -> " + r.motion.formaPorIndice;
                    CAPTURE(onde);

                    CHECK_MESSAGE(disparados == tamanho,
                        "o ataque dispara " << disparados << " projeteis e a familia tem "
                        << tamanho << " formas: de menos deixa parte da figura sem aparecer,"
                        << " de mais faz os extras repetirem formas");

                    ++conferidos;
                }
            }
        }
    }

    CHECK(conferidos >= 1);
}

TEST_CASE("Dados: toda familia tem pelo menos duas formas") {
    // Uma familia de uma forma so e uma forma simples escrita de um jeito
    // complicado - provavelmente "divisoes": 0 por engano.
    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));
    for (const auto& fam : formas.familias) {
        CAPTURE(fam.first);
        CHECK(fam.second.size() >= 2);
    }
}

// ---------------------------------------------------------------------------
// As materias, e a prova de que o arquivo reproduz o C++ que ele substitui
//
// A migracao das materias para dados so pode ser chamada de equivalente se as
// regras de desbloqueio lidas do arquivo derem a MESMA resposta que o
// Game::IsStageUnlocked de hoje, para qualquer progresso. O ultimo teste deste
// bloco faz essa comparacao exaustivamente.
// ---------------------------------------------------------------------------

TEST_CASE("Dados: Assets/materias.json nao tem problema nenhum") {
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    CHECK_MESSAGE(l.problemas.empty(), "problemas em materias.json:" << Juntar(l.problemas));
    CHECK(l.Quantas() > 0);
}

TEST_CASE("Dados: as materias do curso estao la, com os codigos esperados") {

    // ESTA LISTA NAO E A ORDEM, e sim o CONJUNTO. Ate a versao 1 do save a
    // posicao aqui era a chave gravada em disco, e esta lista travava a ordem
    // por isso; desde a versao 2 o save guarda o codigo, e reordenar passou a
    // ser seguro. O que ainda importa travar e que nenhuma materia suma por
    // descuido de edicao.
    const std::set<std::string> esperadas = {
        "INF110", "INF213", "INF250", "INF220", "INF330", "INF332",
        "INF420", "BIOINF", "INF394", "VISCCP", "TCC"
    };

    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    REQUIRE(l.Quantas() == static_cast<int>(esperadas.size()));

    std::set<std::string> noArquivo;
    for (const auto& m : l.materias) noArquivo.insert(m.codigo);
    CHECK(noArquivo == esperadas);
}

TEST_CASE("Dados: toda materia com chefe aponta para um conjunto que existe em fases.json") {
    // O campo "chefe" liga as duas migracoes: ele e o mesmo nome de conjunto que
    // fases.json, regras.json e projeteis.json usam.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    const auto fases = LerFases(LerArquivo("Attacks/fases.json"));

    int comChefe = 0;
    for (const auto& m : l.materias) {
        if (m.chefe.empty()) continue;   // materia ainda sem chefe e normal
        CAPTURE(m.codigo);
        CAPTURE(m.chefe);
        CHECK(fases.conjuntos.count(m.chefe) == 1);
        ++comChefe;
    }
    CHECK(comChefe == 5);   // hoje sao cinco das onze
}

TEST_CASE("Dados: todo chefe citado em materias.json tem fabrica registrada") {

    // Game::GetFactory procura a fabrica PELO NOME que esta no arquivo. Um nome
    // escrito errado aqui nao da erro de compilacao: a materia abre, nao acha
    // chefe nenhum e a fase volta sozinha para a selecao. Este teste transforma
    // isso numa falha de suite, que e onde da para ver.
    //
    // A lista espelha Game::InitializeBossFactory, como kFormasEmCodigo espelha
    // FormaPeloNome. Registrar um chefe novo la pede acrescenta-lo aqui.
    const std::set<std::string> kFabricasRegistradas = {"salles", "ricardo", "andre", "julio", "thiago"};

    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));

    for (const auto& m : l.materias) {
        if (m.chefe.empty()) continue;
        CHECK_MESSAGE(kFabricasRegistradas.count(m.chefe) == 1,
                      "a materia " << m.codigo << " pede o chefe \"" << m.chefe
                      << "\", que nao tem fabrica em Game::InitializeBossFactory");
    }
}

TEST_CASE("Dados: a progressao do curso e a que o arquivo diz que e") {

    // Este teste nasceu comparando materias.json com a regra que vivia em
    // Game::IsStageUnlocked. Aquela regra ja saiu do C++ - foi ele que tornou a
    // saida segura -, entao agora ele descreve a progressao PRETENDIDA:
    //
    //   INF110 sempre aberta (a porta de entrada do curso);
    //   INF213 abre ao passar em INF110;
    //   coluna 2 (INF250, INF220, INF330, INF332) abre ao passar em INF213;
    //   coluna 3 (INF420, BIOINF, INF394, VISCCP) abre com 2 aprovacoes na 2;
    //   TCC abre com 2 aprovacoes na coluna 3.
    const std::vector<std::string> col2 = {"INF250", "INF220", "INF330", "INF332"};
    const std::vector<std::string> col3 = {"INF420", "BIOINF", "INF394", "VISCCP"};

    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    REQUIRE(l.Quantas() == 11);

    auto pretendida = [&](const std::string& codigo, const Progresso& p) {
        auto aprovado = [&](const std::string& c) { return p.Aprovado(l.IndiceDe(c)); };
        auto quantas = [&](const std::vector<std::string>& lista) {
            int n = 0;
            for (const auto& c : lista) if (aprovado(c)) ++n;
            return n;
        };
        if (codigo == "INF110") return true;
        if (codigo == "INF213") return aprovado("INF110");
        for (const auto& c : col2) if (c == codigo) return aprovado("INF213");
        for (const auto& c : col3) if (c == codigo) return quantas(col2) >= 2;
        if (codigo == "TCC") return quantas(col3) >= 2;
        return false;
    };

    // Varre TODAS as combinacoes de aprovacao das onze materias: 2048 estados.
    // Cobrir o espaco inteiro, em vez de casos escolhidos a dedo, e onde uma
    // diferenca de regra costuma aparecer.
    int comparacoes = 0;
    for (int mascara = 0; mascara < (1 << 11); ++mascara) {

        Progresso p;
        for (int bit = 0; bit < 11; ++bit) {
            if (mascara & (1 << bit)) p.RegistrarNota(bit, 80.0f);
        }

        for (int i = 0; i < l.Quantas(); ++i) {
            const std::string codigo = l.CodigoDe(i);
            const bool doArquivo = l.Desbloqueada(i, p);
            const bool esperado  = pretendida(codigo, p);

            if (doArquivo != esperado) {
                CAPTURE(mascara);
                CAPTURE(codigo);
                CAPTURE(doArquivo);
                CAPTURE(esperado);
                FAIL("o arquivo e a progressao pretendida discordam");
            }
            ++comparacoes;
        }
    }

    CHECK(comparacoes == (1 << 11) * 11);
}

TEST_CASE("Dados: personagens.json e lido sem nenhum problema") {

    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    CHECK_MESSAGE(c.problemas.empty(), "problemas em personagens.json:" << Juntar(c.problemas));
    CHECK_FALSE(c.Vazio());
}

TEST_CASE("Dados: as categorias que a tela de criacao precisa existem") {

    // Travar os nomes aqui parece burocracia, mas e o que transforma "renomeei
    // uma categoria no arquivo" num teste vermelho em vez de numa personagem
    // que perde o cabelo em silencio no proximo carregamento.
    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    // std::string, e nao const char*: com ponteiro de char a mensagem do doctest
    // saia como "falta a categoria "1"" - o ponteiro ia para o stream como
    // booleano, e a falha nao dizia QUAL categoria sumiu.
    for (const std::string& id : {"pele", "cabelo", "camisa", "calca"}) {
        CHECK_MESSAGE(c.Por(id) != nullptr, "falta a categoria \"" << id << "\"");
    }
}

TEST_CASE("Dados: toda arte citada no catalogo existe em disco") {

    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    std::vector<std::string> artes;
    for (const auto& cat : c.categorias) {
        for (const auto& p : cat.pecas) artes.push_back(p.arte);
    }
    for (const auto& f : c.fixas) artes.push_back(f.arte);

    REQUIRE_FALSE(artes.empty());

    for (const auto& arte : artes) {
        // O catalogo guarda o caminho SEM extensao; quem carrega acrescenta.
        for (const char* ext : {".png", ".json"}) {
            const std::string caminho = std::string(DPI_ASSETS_DIR) + "/" + arte + ext;
            std::ifstream a(caminho, std::ios::binary);
            CHECK_MESSAGE(a.is_open(), "arte citada no catalogo e ausente em disco: " << caminho);
        }
    }
}

TEST_CASE("Dados: toda peca tem quatro quadros de 64x64") {

    // NAO E DETALHE DE ARTE. O raio do colisor do jogador sai da largura da
    // sprite (CircleColliderComponent(GetSpriteWidth() / 10.f) em Player.cpp),
    // entao uma peca com quadro maior daria hitbox maior a quem a escolhesse -
    // e a escolha de aparencia, que deve ser so estetica, viraria vantagem de
    // jogo. Ver RNF1 em Documentacao/requisitos-perfil-e-personagem.md.
    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    std::vector<std::string> artes;
    for (const auto& cat : c.categorias) {
        for (const auto& p : cat.pecas) artes.push_back(p.arte);
    }
    for (const auto& f : c.fixas) artes.push_back(f.arte);

    int conferidas = 0;
    for (const auto& arte : artes) {

        const std::string caminho = std::string(DPI_ASSETS_DIR) + "/" + arte + ".json";
        std::ifstream a(caminho);
        if (!a.is_open()) continue;   // a ausencia ja e relatada no teste acima

        std::ostringstream conteudo;
        conteudo << a.rdbuf();

        nlohmann::json atlas;
        try { atlas = LerJsonDeDados(conteudo.str()); }
        catch (const std::exception& e) {
            FAIL("atlas invalido em " << caminho << ": " << e.what());
        }

        REQUIRE_MESSAGE(atlas.contains("frames"), "sem \"frames\": " << caminho);
        CHECK_MESSAGE(atlas["frames"].size() == 4,
                      "esperava 4 quadros em " << caminho << ", achei " << atlas["frames"].size());

        for (const auto& quadro : atlas["frames"]) {
            REQUIRE(quadro.contains("frame"));
            CHECK_MESSAGE(quadro["frame"]["w"].get<int>() == 64, "quadro nao e 64 de largura: " << caminho);
            CHECK_MESSAGE(quadro["frame"]["h"].get<int>() == 64, "quadro nao e 64 de altura: " << caminho);
        }
        ++conferidas;
    }

    // Sem isto o teste passaria por vacuidade se o catalogo viesse vazio.
    CHECK(conferidas >= 10);
}

TEST_CASE("Dados: a aparencia padrao do catalogo real compoe todas as camadas") {

    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    const std::vector<Personagens::Camada> camadas = c.Camadas(c.Padrao());

    // Uma por categoria, mais as fixas.
    CHECK(camadas.size() == c.categorias.size() + c.fixas.size());

    for (size_t i = 1; i < camadas.size(); ++i) {
        CHECK_MESSAGE(camadas[i - 1].ordem < camadas[i].ordem,
                      "duas camadas com a mesma ordem de desenho");
    }
}

TEST_CASE("Dados: as listas do Salles giram com a direcao do movimento") {

    // A arte delas e uma caixa com uma SETA. Com a seta sempre apontando para a
    // direita, uma corrente que desce na diagonal nao le como lista encadeada -
    // os nos ficam soltos. Travar isto aqui evita que alguem desligue o campo
    // sem perceber o que ele sustenta.
    const auto r = LerProjeteis(LerArquivo("Attacks/projeteis.json"));

    REQUIRE(r.conjuntos.count("salles") == 1);
    const auto& salles = r.conjuntos.at("salles");

    for (const std::string& nome : {"Estruturas", "Duplamente"}) {
        REQUIRE_MESSAGE(salles.count(nome) == 1, "falta o projetil \"" << nome << "\"");
        CHECK_MESSAGE(salles.at(nome).rotacionarComAVelocidade,
                      "\"" << nome << "\" precisa girar com a velocidade");
    }

    // E a capivara NAO gira: ela nao tem frente.
    if (salles.count("Capivara") == 1) {
        CHECK_FALSE(salles.at("Capivara").rotacionarComAVelocidade);
    }
}

// ---------------------------------------------------------------------------
// A NAVEGACAO CONTRA O materias.json DE VERDADE
//
// Os testes de test_navegacao.cpp provam a REGRA contra uma grade escrita a mao.
// Isso nao bastaria aqui: foi justamente uma grade escrita a mao, dentro da
// StageSelect, que discordou da tela e fez a seta pular o INF 213. Entao estes
// testes montam a grade do MESMO jeito que a StageSelect monta - percorrendo as
// colunas de materias.json - e perguntam para onde as setas levam.
//
// Se alguem reordenar materias.json, sao estes testes que dizem o que acontece
// com as setas.
// ---------------------------------------------------------------------------

namespace {

    /// A grade como a StageSelect a monta: coluna por coluna, na ordem, e o
    /// indice do botao e a posicao em que ele e criado.
    Navegacao::Grade GradeDe(const Materias::Lista& l) {
        Navegacao::Grade grade(static_cast<size_t>(l.QuantasColunas()));
        size_t proximo = 0;
        for (int c = 0; c < l.QuantasColunas(); ++c) {
            for (size_t i = 0; i < l.DaColuna(c).size(); ++i) {
                grade[static_cast<size_t>(c)].push_back(proximo++);
            }
        }
        return grade;
    }

    /// De indice de botao para codigo de materia, pela mesma ordem de criacao.
    std::vector<std::string> CodigosNaOrdemDosBotoes(const Materias::Lista& l) {
        std::vector<std::string> codigos;
        for (int c = 0; c < l.QuantasColunas(); ++c) {
            for (const int m : l.DaColuna(c)) {
                const Materias::Materia* mat = l.Por(m);
                codigos.push_back(mat ? mat->codigo : "?");
            }
        }
        return codigos;
    }
}

TEST_CASE("Navegacao no materias.json: a seta da direita no INF 110 leva ao INF 213") {

    // O DEFEITO RELATADO. O INF 213 e a unica materia que o INF 110 abre, e a
    // seta pulava direto para o INF 250, do outro lado da tela.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    const auto grade = GradeDe(l);
    const auto codigos = CodigosNaOrdemDosBotoes(l);

    const auto inf110 = std::find(codigos.begin(), codigos.end(), "INF110");
    REQUIRE(inf110 != codigos.end());

    const size_t destino = Navegacao::Direita(grade, static_cast<size_t>(inf110 - codigos.begin()));
    REQUIRE(destino < codigos.size());
    CHECK(codigos[destino] == "INF213");
}

TEST_CASE("Navegacao no materias.json: descer no INF 250 nao passa pelo INF 213") {

    // O OUTRO DEFEITO RELATADO: a descida entrava num ciclo de quatro que
    // misturava duas colunas da tela.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    const auto grade = GradeDe(l);
    const auto codigos = CodigosNaOrdemDosBotoes(l);

    const auto inicio = std::find(codigos.begin(), codigos.end(), "INF250");
    REQUIRE(inicio != codigos.end());

    size_t onde = static_cast<size_t>(inicio - codigos.begin());
    for (int passo = 0; passo < 12; ++passo) {
        onde = Navegacao::Baixo(grade, onde);
        REQUIRE(onde < codigos.size());
        CAPTURE(passo);
        CAPTURE(codigos[onde]);
        CHECK(codigos[onde] != "INF213");
    }
}

TEST_CASE("Navegacao no materias.json: toda materia e alcancavel pelas setas") {

    // O DEFEITO QUE NINGUEM TINHA VISTO AINDA. Com a grade antiga, tres botoes
    // diferentes levavam todos ao VISCCP e NADA levava ao TCC - a ultima materia
    // do curso era inalcancavel. So nao apareceu porque o TCC exige duas
    // aprovacoes na coluna 3 para ser jogavel, e ninguem tinha chegado la.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    const auto grade = GradeDe(l);
    const auto codigos = CodigosNaOrdemDosBotoes(l);
    REQUIRE(!codigos.empty());

    // Busca em largura a partir do primeiro botao, pelas quatro setas.
    std::set<size_t> vistos{0};
    std::vector<size_t> fila{0};
    while (!fila.empty()) {
        const size_t onde = fila.back();
        fila.pop_back();
        for (const auto mover : {Navegacao::Cima, Navegacao::Baixo,
                                 Navegacao::Esquerda, Navegacao::Direita}) {
            if (const size_t destino = mover(grade, onde); vistos.insert(destino).second) {
                fila.push_back(destino);
            }
        }
    }

    for (size_t i = 0; i < codigos.size(); ++i) {
        CAPTURE(codigos[i]);
        CHECK(vistos.count(i) == 1);
    }
}

TEST_CASE("Navegacao no materias.json: nenhuma seta sai da grade") {

    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    const auto grade = GradeDe(l);
    const auto quantos = CodigosNaOrdemDosBotoes(l).size();

    for (size_t i = 0; i < quantos; ++i) {
        for (const auto mover : {Navegacao::Cima, Navegacao::Baixo,
                                 Navegacao::Esquerda, Navegacao::Direita}) {
            CAPTURE(i);
            CHECK(mover(grade, i) < quantos);
        }
    }
}
