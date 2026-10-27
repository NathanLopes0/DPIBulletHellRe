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
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "../Source/Attacks/FasesDeAtaque.h"
#include "../Source/Attacks/PathShapes.h"
#include "../Source/Attacks/RegrasDeAtaque.h"
#include "../Source/Attacks/ProjeteisDeChefe.h"
#include "../Source/Attacks/Sprint.h"
#include "../Source/Attacks/Observer.h"
#include "../Source/Attacks/ReAct.h"
#include "../Source/Attacks/Nuvem.h"
#include "../Source/Attacks/Semaforo.h"
#include "../Source/Attacks/Tcp.h"
#include "../Source/Attacks/Filosofos.h"
#include "../Source/Attacks/Mensagens.h"
#include "../Source/Attacks/Logica.h"
#include "../Source/Attacks/Relogio.h"
#include "../Source/Attacks/Pipeline.h"
#include "../Source/Attacks/Blink.h"
#include "../Source/Attacks/Alfabeto.h"
#include "../Source/Attacks/Automato.h"
#include "../Source/Attacks/Compilador.h"
#include "../Source/Attacks/Parada.h"
#include "../Source/Attacks/Fita.h"
#include "../Source/Attacks/Capivara.h"
#include "../Source/Attacks/Pilha.h"
#include "../Source/Attacks/Hash.h"
#include "../Source/Attacks/Arvore.h"
#include "../Source/Attacks/Ordenacao.h"
#include "../Source/Attacks/Banco.h"
#include "../Source/Attacks/Transacao.h"
#include "../Source/Attacks/Cascata.h"
#include "../Source/Attacks/Condicional.h"
#include "../Source/Attacks/Laco.h"
#include "../Source/Attacks/Funcao.h"
#include "../Source/Attacks/Prova.h"
#include "../Source/Attacks/Escopo.h"
#include "../Source/Attacks/Coercao.h"
#include "../Source/Attacks/Memoria.h"
#include "../Source/Attacks/Excecao.h"
#include <regex>
#include "../Source/Attacks/PastaDoChefe.h"
#include "../Source/Materias.h"
#include "../Source/Orientacao.h"
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

    /**
     * OS CHEFES SAO OS QUE materias.json DIZ, e nao uma lista escrita aqui.
     *
     * Era uma lista a mao - e ela ficou para tras. O Thiago entrou no curso e
     * NUNCA foi conferido por nenhum dos sete testes que a usavam: transicoes,
     * quatro fases, conjunto de projeteis, animacoes. Trinta e duas verificacoes
     * deixavam de rodar, e nada dizia nada.
     *
     * Tirar o nome do chefe de dentro dos arquivos de ataque deixou materias.json
     * como a unica fonte de quem existe - entao e dali que a lista vem.
     */
    const std::vector<std::string>& Chefes() {
        static const std::vector<std::string> chefes = [] {
            std::set<std::string> nomes;
            for (const auto& m : Materias::LerMaterias(LerArquivo("materias.json")).materias) {
                if (!m.chefe.empty()) nomes.insert(m.chefe);
            }
            return std::vector<std::string>(nomes.begin(), nomes.end());
        }();
        return chefes;
    }

    /// As regras de um chefe, da pasta dele.
    RegrasLidas RegrasDoChefe(const std::string& chefe) {
        return LerRegras(LerArquivo(PastaDoChefe::Arquivo(chefe, PastaDoChefe::kRegras)));
    }

    /// As fases de um chefe, da pasta dele.
    FasesLidas FasesDoChefe(const std::string& chefe) {
        return LerFases(LerArquivo(PastaDoChefe::Arquivo(chefe, PastaDoChefe::kFases)));
    }

    /// Os projeteis de um chefe, da pasta dele.
    ProjeteisLidos ProjeteisDoChefe(const std::string& chefe) {
        return LerProjeteis(LerArquivo(PastaDoChefe::Arquivo(chefe, PastaDoChefe::kProjeteis)));
    }

}

// ---------------------------------------------------------------------------
// Cada arquivo, por si
// ---------------------------------------------------------------------------

TEST_CASE("Dados: Assets/Paths/formas.json nao tem problema nenhum") {
    const auto r = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));
    CHECK_MESSAGE(r.problemas.empty(), "problemas em formas.json:" << Juntar(r.problemas));
    CHECK(r.formas.size() > 0);
}

TEST_CASE("Dados: o regras.json de cada chefe nao tem problema nenhum") {
    for (const auto& chefe : Chefes()) {
        CAPTURE(chefe);
        const auto r = RegrasDoChefe(chefe);
        CHECK_MESSAGE(r.problemas.empty(), "problemas em " << chefe << "/regras.json:"
                                                           << Juntar(r.problemas));
        CHECK(r.conjuntos.size() > 0);
    }
}

TEST_CASE("Dados: o fases.json de cada chefe nao tem problema nenhum") {
    for (const auto& chefe : Chefes()) {
        CAPTURE(chefe);
        const auto r = FasesDoChefe(chefe);
        CHECK_MESSAGE(r.problemas.empty(), "problemas em " << chefe << "/fases.json:"
                                                           << Juntar(r.problemas));
    }
}

// ---------------------------------------------------------------------------
// Os chefes
// ---------------------------------------------------------------------------

TEST_CASE("Dados: todo chefe tem fases e as transicoes delas fecham") {
    // Sem a reserva em C++, um conjunto recusado aqui e um chefe parado em tela.
    for (const auto& chefe : Chefes()) {
        CAPTURE(chefe);
        const auto& fases = FasesDoChefe(chefe).fases;
        REQUIRE_MESSAGE(!fases.empty(), "nao ha fases em " << chefe << "/fases.json");

        const auto problemas = ValidarTransicoes(fases, chefe);
        CHECK_MESSAGE(problemas.empty(), "transicoes de \"" << chefe << "\":" << Juntar(problemas));
    }
}

TEST_CASE("Dados: todo chefe tem as quatro fases") {
    // Nao e exigencia do formato - um chefe pode ter tres. E exigencia DESTE
    // jogo: os cinco chefes atuais tem StateOne, Two, Three e Final, e perder
    // uma delas por um recorte errado seria dificil de notar jogando.
    for (const auto& chefe : Chefes()) {
        CAPTURE(chefe);
        std::set<std::string> nomes;
        for (const auto& f : FasesDoChefe(chefe).fases) nomes.insert(f.nome);
        CHECK(nomes.count("StateOne") == 1);
        CHECK(nomes.count("StateTwo") == 1);
        CHECK(nomes.count("StateThree") == 1);
        CHECK(nomes.count("StateFinal") == 1);
    }
}

TEST_CASE("Dados: toda fase tem de onde tirar pelo menos um ataque") {
    // LerFases ja descarta fase sem ataque, entao isto pega o descarte: a fase
    // some do conjunto e o chefe fica parado 17 segundos naquele trecho.
    //
    // "OU EMPRESTA DE OUTRA" entrou quando a fase que se divide passou a usar
    // "ataquesDe" - ela nao declara ataques de proposito, e a lista dela fica
    // vazia na camada pura (quem resolve e a ponte). O que o caso protege
    // continua sendo o mesmo: nenhuma fase deixa o chefe parado.
    //
    // Que o nome apontado EXISTE e assunto de ValidarTransicoes, e ha caso
    // proprio para ele em test_fases.cpp.
    for (const auto& chefe : Chefes()) {
        const auto& fases = FasesDoChefe(chefe).fases;
        for (const auto& f : fases) {
            CAPTURE(chefe);
            CAPTURE(f.nome);
            CAPTURE(f.ataquesDe);
            CHECK((f.ataques.size() >= 1 || !f.ataquesDe.empty()));
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
    for (const auto& chefe : Chefes()) {
        const auto regras = RegrasDoChefe(chefe);
        const auto& lista = FasesDoChefe(chefe).fases;
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
    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    auto conferir = [&](const std::string& conjunto, const DescricaoDeBehavior& d) {
        if (d.tipo != "Path") return;
        if (kFormasEmCodigo.count(d.forma) == 1) return;
        CAPTURE(conjunto);
        CAPTURE(d.forma);
        CHECK_MESSAGE(formas.formas.count(d.forma) == 1,
                      "a forma \"" << d.forma << "\" nao existe em formas.json");
    };

    for (const auto& chefe : Chefes()) {
      for (const auto& par : RegrasDoChefe(chefe).conjuntos) {
        const std::string nome = chefe + "/" + par.first;
        const auto& lista = par.second;
        for (const auto& r : lista) {
            if (r.temMotion) conferir(nome, r.motion);
            for (const auto& m : r.modifiers) conferir(nome, m);
        }
      }
    }
}

TEST_CASE("Dados: as formas citadas nas regras EM LINHA de fases.json tambem existem") {
    // Um ataque pode trazer as regras escritas nele mesmo, e elas citam formas do
    // mesmo jeito. O caminho e outro, o erro de digitacao e o mesmo.
    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    for (const auto& chefe : Chefes()) {
        const auto& lista = FasesDoChefe(chefe).fases;
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
    for (const auto& chefe : Chefes()) {
        const auto regras = RegrasDoChefe(chefe);
        const auto& lista = FasesDoChefe(chefe).fases;
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
    for (const auto& chefe : Chefes()) {
        const auto& lista = FasesDoChefe(chefe).fases;
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

TEST_CASE("Dados: o projeteis.json de cada chefe nao tem problema nenhum") {
    for (const auto& chefe : Chefes()) {
        CAPTURE(chefe);
        const auto r = ProjeteisDoChefe(chefe);
        CHECK_MESSAGE(r.problemas.empty(), "problemas em " << chefe << "/projeteis.json:"
                                                           << Juntar(r.problemas));
    }
}

TEST_CASE("Dados: todo chefe tem projeteis na pasta dele") {
    // Sem projetil, o chefe monta e anda, mas nenhum ataque encontra a fabrica
    // que pede: ele fica inofensivo, o que em jogo parece bug e nao erro de dados.
    for (const auto& chefe : Chefes()) {
        CAPTURE(chefe);
        CHECK_FALSE(ProjeteisDoChefe(chefe).projeteis.empty());
    }
}

TEST_CASE("Dados: todo projetil citado nas fases existe na pasta do chefe") {
    for (const auto& chefe : Chefes()) {
        const auto& disponiveis = ProjeteisDoChefe(chefe).projeteis;

        for (const auto& f : FasesDoChefe(chefe).fases) {
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
    for (const auto& chefe : Chefes()) {
        for (const auto& entrada : ProjeteisDoChefe(chefe).projeteis) {
            const std::string onde = chefe + "/" + entrada.first;
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
    for (const auto& chefe : Chefes()) {
        for (const auto& entrada : ProjeteisDoChefe(chefe).projeteis) {

            const std::string onde = chefe + "/" + entrada.first;
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

namespace {
    /// A largura e a altura de um PNG, lidas do cabecalho (IHDR): os quatro
    /// bytes de cada uma comecam no byte 16 e vem do mais alto para o mais
    /// baixo. Zero se o arquivo nao abrir ou nao for PNG.
    std::pair<int, int> TamanhoDoPng(const std::string& caminhoRelativo) {
        std::ifstream f(std::string(DPI_ASSETS_DIR) + "/" + caminhoRelativo, std::ios::binary);
        unsigned char b[24] = {};
        if (!f.read(reinterpret_cast<char*>(b), sizeof b)) return {0, 0};
        if (b[1] != 'P' || b[2] != 'N' || b[3] != 'G') return {0, 0};
        const auto u32 = [&](const int i) { return (b[i] << 24) | (b[i + 1] << 16) | (b[i + 2] << 8) | b[i + 3]; };
        return {u32(16), u32(20)};
    }
}

TEST_CASE("Dados: todo quadro de toda folha de projetil cabe na imagem") {
    // O DrawAnimatedComponent desenha o retangulo do quadro no tamanho que o
    // .json diz, e um quadro maior que a folha nao da erro: sai ESTICADO. Foi
    // assim que o diagrama do automato do Vladimir (1000 x 400) saiu com quadro
    // de 1000 x 1000 e apareceu duas vezes e meia mais alto, sem nada reclamar.
    for (const auto& chefe : Chefes()) {
        for (const auto& entrada : ProjeteisDoChefe(chefe).projeteis) {
            const std::string onde = chefe + "/" + entrada.first;
            CAPTURE(onde);
            const auto [largura, altura] = TamanhoDoPng(entrada.second.sprite);
            REQUIRE_MESSAGE(largura > 0, "nao li o PNG " << entrada.second.sprite);

            const auto atlas = LerJsonDeDados(LerArquivo(entrada.second.dados));
            REQUIRE(atlas.contains("frames"));
            for (const auto& item : atlas["frames"]) {
                // Lista ou objeto, como no teste de cima: o quadro e o mesmo.
                const auto& quadro = item.contains("frame") ? item["frame"] : item;
                REQUIRE(quadro.contains("x"));
                const int x = quadro["x"].get<int>(), y = quadro["y"].get<int>();
                const int w = quadro["w"].get<int>(), h = quadro["h"].get<int>();
                CAPTURE(x); CAPTURE(y); CAPTURE(w); CAPTURE(h);
                CHECK(x >= 0);
                CHECK(y >= 0);
                CHECK(x + w <= largura);
                CHECK(y + h <= altura);
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
        // (Os "Dados" do Julio sairam no 126, quando a ultima fase dele que os
        // usava virou o modelo em loop, o "Arduino" do Ricardo no 132, quando
        // a fase final virou o Blink - a arte dele virou a da Placa -, e a
        // "Estruturas" e a "Duplamente" do Salles no 146, depois que as listas
        // das fases dele viraram a pilha, a tabela hash e a arvore de busca.)
        // Os baloes sao o unico que difere em tres campos de uma vez, e os tres
        // eram de proposito: hitbox menor que o desenho (divisor 4), nao nascer na
        // posicao do chefe (a BaloonAttack escolhe o lado da tela) e morrer com
        // folga de dois sprites sem parcela de tela.
        {"andre",   "Baloes",   "Teachers/Projectiles/DPIBHAndreBaloon.png",
         2.0f, 90, "Red",    "largura", 4.0f, false, 2.0f,  0.0f, 3},
    };

    for (const auto& e : esperados) {
        CAPTURE(e.conjunto);
        CAPTURE(e.nome);

        const auto doChefe = ProjeteisDoChefe(e.conjunto);
        REQUIRE(doChefe.projeteis.count(e.nome) == 1);

        const auto& p = doChefe.projeteis.at(e.nome);
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
    struct AnimEsperada {
        const char* conjunto;
        const char* projetil;
        const char* animacao;
        std::vector<int> quadros;
    };

    const std::vector<AnimEsperada> esperadas = {
        {"salles",  "Capivara", "Normal",      {0}},
        {"salles",  "Capivara", "Homing",      {1}},
        // A folha dos baloes tem 24 quadros e as animacoes comecam no 6: os seis
        // primeiros sao de uma versao antiga e nenhuma animacao os usa.
        {"andre",   "Baloes",   "Red",         {6, 7, 8, 9, 10, 11}},
        {"andre",   "Baloes",   "Blue",        {12, 13, 14, 15, 16, 17}},
        {"andre",   "Baloes",   "Yellow",      {18, 19, 20, 21, 22, 23}},
    };

    for (const auto& e : esperadas) {
        CAPTURE(e.conjunto);
        CAPTURE(e.projetil);
        CAPTURE(e.animacao);

        const auto doChefe = ProjeteisDoChefe(e.conjunto);
        REQUIRE(doChefe.projeteis.count(e.projetil) == 1);

        const auto& anims = doChefe.projeteis.at(e.projetil).animacoes;

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

    int combinacoesConferidas = 0;

    for (const auto& chefe : Chefes()) {
        const auto regras = RegrasDoChefe(chefe);

        for (const auto& f : FasesDoChefe(chefe).fases) {
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

    // Hoje so uma combinacao destas existe: o diagrama da fase 1 da Maria
    // Lucia (oito pecas, 0,05s entre elas, o caminho com 0,5s). A que o criou
    // - a lista duplamente encadeada do Salles - saiu quando a fase 2 dele
    // virou a tabela hash. Se este numero cair a zero, o teste passou a nao
    // conferir nada - foi assim que eu quase deixei passar uma medicao
    // invalida antes.
    CHECK(combinacoesConferidas >= 1);
}

TEST_CASE("Dados: todo projetil e todo conjunto de regras de um chefe e usado por alguma fase dele") {

    // O ORFAO NAO DA ERRO NENHUM: um tipo de projeteis.json que nenhuma fase
    // pede so ocupa a pasta, e quem le o arquivo acredita que ele esta em jogo.
    // Foi assim que a "Estruturas" e a "Duplamente" do Salles ficaram na pasta
    // depois que as fases que as disparavam viraram a hash e a arvore - e com
    // elas um teste que trancava o giro de uma seta que ninguem mais via. Quem
    // tira a ultima fase que usa um tipo, tira o tipo junto (ou o teste diz
    // qual sobrou).
    for (const auto& chefe : Chefes()) {
        std::set<std::string> projeteis, conjuntos;
        for (const auto& f : FasesDoChefe(chefe).fases) {
            for (const auto& a : f.ataques) {
                projeteis.insert(a.projetil);
                if (!a.regrasNome.empty()) conjuntos.insert(a.regrasNome);
            }
        }
        for (const auto& [nome, _] : ProjeteisDoChefe(chefe).projeteis) {
            CAPTURE(chefe);
            CAPTURE(nome);
            CHECK_MESSAGE(projeteis.count(nome) == 1, "o projetil nao e disparado por fase nenhuma");
        }
        for (const auto& [nome, _] : RegrasDoChefe(chefe).conjuntos) {
            CAPTURE(chefe);
            CAPTURE(nome);
            CHECK_MESSAGE(conjuntos.count(nome) == 1, "o conjunto de regras nao e vestido por ataque nenhum");
        }
    }
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

    int pedidosConferidos = 0;

    for (const auto& chefe : Chefes()) {
        const auto regras = RegrasDoChefe(chefe);

        const auto doChefe = ProjeteisDoChefe(chefe);

        for (const auto& f : FasesDoChefe(chefe).fases) {
            for (const auto& a : f.ataques) {

                const auto projetil = doChefe.projeteis.find(a.projetil);
                if (projetil == doChefe.projeteis.end()) continue;   // outro teste cobre

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

    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    int paradasConferidas = 0;

    for (const auto& chefe : Chefes())
      for (const auto& conjunto : RegrasDoChefe(chefe).conjuntos) {
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

    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    int conferidas = 0;

    for (const auto& chefe : Chefes())
      for (const auto& conjunto : RegrasDoChefe(chefe).conjuntos) {
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

    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    int conferidos = 0;

    for (const auto& chefe : Chefes()) {
        const auto regras = RegrasDoChefe(chefe);

        for (const auto& f : FasesDoChefe(chefe).fases) {
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
        "INF110", "INF213", "INF250", "INF220", "INF340", "INF331",
        "INF420", "INF221", "INF321", "INF310", "TCC"
    };

    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    REQUIRE(l.Quantas() == static_cast<int>(esperadas.size()));

    std::set<std::string> noArquivo;
    for (const auto& m : l.materias) noArquivo.insert(m.codigo);
    CHECK(noArquivo == esperadas);
}

TEST_CASE("Dados: toda materia com chefe aponta para uma pasta de chefe que existe") {
    // O campo "chefe" e o NOME DA PASTA em Assets/Attacks/. E ele que liga a
    // materia aos tres arquivos de ataque dela.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));

    int comChefe = 0;
    for (const auto& m : l.materias) {
        if (m.chefe.empty()) continue;   // materia ainda sem chefe e normal
        CAPTURE(m.codigo);
        CAPTURE(m.chefe);
        CHECK_FALSE(FasesDoChefe(m.chefe).fases.empty());
        ++comChefe;
    }
    CHECK(comChefe == 11);  // hoje sao todas as onze: o Hugo, no TCC, foi o ultimo
}

TEST_CASE("Dados: todo chefe citado em materias.json tem fabrica registrada") {

    // Game::GetFactory procura a fabrica PELO NOME que esta no arquivo. Um nome
    // escrito errado aqui nao da erro de compilacao: a materia abre, nao acha
    // chefe nenhum e a fase volta sozinha para a selecao. Este teste transforma
    // isso numa falha de suite, que e onde da para ver.
    //
    // A lista espelha Game::InitializeBossFactory, como kFormasEmCodigo espelha
    // FormaPeloNome. Registrar um chefe novo la pede acrescenta-lo aqui.
    const std::set<std::string> kFabricasRegistradas = {"salles", "ricardo", "andre", "julio",
                                                        "thiago", "vladimir", "vitor",
                                                        "marialucia", "lucas", "luizinho", "hugo"};

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
    //   coluna 2 (INF250, INF220, INF340, INF331) abre ao passar em INF213;
    //   coluna 3 (INF420, INF221, INF321, INF310) abre com 2 aprovacoes na 2;
    //   TCC abre com 2 aprovacoes na coluna 3.
    const std::vector<std::string> col2 = {"INF250", "INF220", "INF340", "INF331"};
    const std::vector<std::string> col3 = {"INF420", "INF221", "INF321", "INF310"};

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

TEST_CASE("Dados: nenhum projetil do Salles gira com a direcao do movimento") {

    // Era o contrario: as listas encadeadas antigas eram uma caixa com SETA, e
    // precisavam girar para a corrente se ler como lista. Hoje todo projetil
    // dele se LE em pe - a chave, o bloco e o no levam o numero escrito, e a
    // capivara anda de lado, com um passo para cada direcao. Girado, o 42 de
    // uma chave que desce viraria um rabisco, e a capivara que volta andaria
    // de cabeca para baixo.
    const auto r = ProjeteisDoChefe("salles");
    REQUIRE(!r.projeteis.empty());
    for (const auto& [nome, p] : r.projeteis) {
        CAPTURE(nome);
        CHECK_FALSE(p.rotacionarComAVelocidade);
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

TEST_CASE("Dados: toda materia do curso tem nome de disciplina e professor") {

    // Sao opcionais no formato, para nao quebrar arquivo antigo - mas no curso
    // de verdade nenhuma pode ficar sem, senao a tela de selecao mostra uma
    // materia muda no meio das outras.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    REQUIRE(l.Quantas() > 0);

    for (const auto& m : l.materias) {
        CAPTURE(m.codigo);
        CHECK_FALSE(m.nomeCompleto.empty());
        CHECK_FALSE(m.professor.empty());

        // O nome da disciplina nao pode ser o codigo repetido: a tela mostra
        // os dois, um ao lado do outro.
        CHECK(m.nomeCompleto != m.codigo);
        CHECK(m.nomeCompleto != m.nome);
    }
}

TEST_CASE("Dados: toda materia com professor ja tem batalha - o TCC foi a ultima") {

    // ERA A MATERIA COM PROFESSOR E SEM BATALHA, e o teste existia para provar
    // que as duas coisas sao independentes: o TCC era do Hugo e nao tinha luta.
    // Desde o 134 ele tem. A independencia continua coberta, com um curso
    // escrito no proprio teste, em test_materias.cpp ("professor" e "chefe"
    // sao coisas diferentes); aqui fica o retrato do curso de hoje.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));

    int comProfessorSemChefe = 0;
    for (const auto& m : l.materias) {
        if (!m.professor.empty() && m.chefe.empty()) ++comProfessorSemChefe;
    }
    CHECK(comProfessorSemChefe == 0);
}

TEST_CASE("Dados: toda coluna do curso depende da anterior, entao a grade pode ser ligada") {

    // E o que autoriza a tela de selecao a desenhar o tronco entre as colunas.
    // Se alguem criar uma materia que exige algo de duas colunas atras, este
    // teste cai e a linha daquela coluna some sozinha - e e assim que tem de
    // ser, porque desenhar uma ligacao que nao existe e pior que nao desenhar.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    REQUIRE(l.QuantasColunas() == 5);

    CHECK_FALSE(l.ColunaDependeDaAnterior(0));
    for (int c = 1; c < l.QuantasColunas(); ++c) {
        CAPTURE(c);
        CHECK(l.ColunaDependeDaAnterior(c));
    }
}

TEST_CASE("Dados: as quatro fases do Vladimir anunciam alguma coisa no placar") {

    // CANARIO, e espelha os dados de proposito. O anuncio e a identidade do
    // chefe ("os outros surpreendem, e ele ANUNCIA"), e o jeito mais facil de
    // perde-lo e escrever a chave em portugues de verdade - "anuncio" com
    // acento - num arquivo que aceita campo desconhecido calado. O placar
    // apagaria naquela fase e nada mais quebraria.
    //
    // Se uma fase do Vladimir deixar de anunciar DE PROPOSITO, tire-a daqui.
    const auto fases = FasesDoChefe("vladimir").fases;
    REQUIRE(fases.size() == 4);
    for (const auto& f : fases) {
        CAPTURE(f.nome);
        CHECK_FALSE(f.anuncio.empty());
    }
}

TEST_CASE("Dados: o Programa de Intercambio do Vladimir vale nas quatro fases") {

    // CANARIO, e espelha os dados de proposito: o intercambio e a mecanica que
    // atravessa a luta dele inteira, e uma fase nova escrita sem o bloco
    // "intercambio" ficaria sem troca nenhuma, calada. Se uma fase deixar de
    // trocar DE PROPOSITO, tire-a daqui.
    const auto fases = FasesDoChefe("vladimir").fases;
    REQUIRE(fases.size() == 4);
    for (const auto& f : fases) {
        CAPTURE(f.nome);
        CHECK(f.intercambio.has_value());
    }
}

TEST_CASE("Dados: o intercambio do Vladimir troca as letras que o Simbolo tem, e deixa a fita no posto dela") {
    CHECK(PostoExiste(Alfabeto::kPostoDaFita));

    // AS DUAS LETRAS SAO ANIMACOES DO SIMBOLO: a troca muda a cara pelo nome, e
    // uma cara que o tipo nao tem deixaria a letra trocada com a cara antiga.
    const auto& tipos = ProjeteisDoChefe("vladimir").projeteis;
    REQUIRE(tipos.count("Simbolo") == 1);
    std::set<std::string> caras;
    for (const auto& a : tipos.at("Simbolo").animacoes) caras.insert(a.nome);
    CHECK(caras.count(Alfabeto::kA) == 1);
    CHECK(caras.count(Alfabeto::kB) == 1);

    // A CASA DA FITA NAO E LETRA EM VOO: toda regra da fita a poe no posto
    // dela, e o intercambio deixa a casa como esta (Alfabeto::Troca).
    const auto regras = RegrasDoChefe("vladimir");
    REQUIRE(regras.conjuntos.count("fita") == 1);
    for (const auto& r : regras.conjuntos.at("fita")) CHECK(r.posto == Alfabeto::kPostoDaFita);
}

TEST_CASE("Dados: o automato do Vladimir - gatilhos, posto e caras sao nomes que os dados tem") {
    for (const char* g : {Automato::kGatilhoDosEstados, Automato::kGatilhoDoDiagrama, Automato::kGatilhoDoA,
                          Automato::kGatilhoDoB, Automato::kGatilhoDeAceitar, Automato::kGatilhoDeRejeitar}) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }
    CHECK(PostoExiste(Automato::kPostoDosEstados));

    // AS CARAS DOS ESTADOS: o Vladimir as monta como "Q" + estado + "Aceso"
    // (Vladimir::CaraDoEstado). Espelhado aqui, porque o Vladimir nao entra nos
    // testes.
    const auto& tipos = ProjeteisDoChefe("vladimir").projeteis;
    REQUIRE(tipos.count("Estado") == 1);
    REQUIRE(tipos.count("Diagrama") == 1);
    std::set<std::string> caras;
    for (const auto& a : tipos.at("Estado").animacoes) caras.insert(a.nome);
    for (int q = 0; q < Automato::kEstados; ++q) {
        CHECK(caras.count("Q" + std::to_string(q)) == 1);
        CHECK(caras.count("Q" + std::to_string(q) + "Aceso") == 1);
    }
    bool temDiagrama = false;
    for (const auto& a : tipos.at("Diagrama").animacoes) temDiagrama = temDiagrama || a.nome == Automato::kCaraDoDiagrama;
    CHECK(temDiagrama);

    // TODO GATILHO DO AUTOMATO ESTA NA FASE DELE: um que faltasse deixaria o
    // professor lendo sem atirar, ou o mapa sem desenhar.
    std::set<std::string> naFase;
    for (const auto& f : FasesDoChefe("vladimir").fases) {
        if (f.nome != Automato::kFaseDoAutomato) continue;
        for (const auto& a : f.ataques) naFase.insert(a.gatilho);
    }
    for (const char* g : {Automato::kGatilhoDosEstados, Automato::kGatilhoDoDiagrama, Automato::kGatilhoDoA,
                          Automato::kGatilhoDoB, Automato::kGatilhoDeAceitar, Automato::kGatilhoDeRejeitar}) {
        CAPTURE(g);
        CHECK(naFase.count(g) == 1);
    }
}

TEST_CASE("Dados: o compilador do Vladimir - gatilhos, posto e caras sao nomes que os dados tem") {
    // TODO GATILHO DO COMPILADOR ESTA NA FASE DELE: um que faltasse deixaria
    // uma etapa sem atirar, ou a arvore sem nos.
    std::set<std::string> naFase;
    std::string projetilDoNo;
    for (const auto& f : FasesDoChefe("vladimir").fases) {
        if (f.nome != Compilador::kFaseDoCompilador) continue;
        for (const auto& a : f.ataques) {
            naFase.insert(a.gatilho);
            if (a.gatilho == Compilador::kGatilhoDoNo) projetilDoNo = a.projetil;
        }
    }
    for (const char* g : {Compilador::kGatilhoDoLexema, Compilador::kGatilhoDoA, Compilador::kGatilhoDoB,
                          Compilador::kGatilhoDoNo, Compilador::kGatilhoDaReducao, Compilador::kGatilhoDaInstrucao,
                          Compilador::kGatilhoDoStore}) {
        CAPTURE(g);
        CHECK(naFase.count(g) == 1);
    }
    CHECK(PostoExiste(Compilador::kPostoDaArvore));

    // AS CARAS QUE O VLADIMIR DA - a de cada token, de cada no e de cada
    // instrucao - sao animacoes do projetil em que caem. Uma que faltasse
    // deixaria a ficha com a cara errada, sem aviso.
    const auto& tipos = ProjeteisDoChefe("vladimir").projeteis;
    const auto carasDe = [&](const std::string& tipo) {
        std::set<std::string> caras;
        if (tipos.count(tipo) == 0) return caras;
        for (const auto& a : tipos.at(tipo).animacoes) caras.insert(a.nome);
        return caras;
    };
    const auto lexemas = carasDe("Lexema");
    const auto doNo = carasDe(projetilDoNo);
    for (const char* texto : {"x", "=", "+", "-", "*", ";", "1", "5", "9"}) {
        CAPTURE(texto);
        CHECK(lexemas.count(Compilador::CaraDoLexema(texto)) == 1);
    }
    const Compilador::Arvore a = Compilador::Sintatico(Compilador::Lexico("x = a - b * 7;"));
    for (int n = 0; n < static_cast<int>(a.nos.size()); ++n) CHECK(doNo.count(Compilador::CaraDoNo(a, n)) == 1);
    const auto instrucoes = carasDe("Instrucao");
    for (const auto& i : Compilador::Codigo(Compilador::Sintatico(Compilador::Lexico("x = a + b - 2 * a;")))) {
        CAPTURE(i);
        CHECK(instrucoes.count(Compilador::CaraDaInstrucao(i)) == 1);
    }
    // E NENHUMA FICHA SE CHAMA "A" OU "B": o intercambio troca as animacoes com
    // esses nomes, e uma ficha nao e letra.
    for (const auto& caras : {lexemas, instrucoes}) {
        CHECK(caras.count("A") == 0);
        CHECK(caras.count("B") == 0);
    }
}

TEST_CASE("Dados: a fita e o problema da parada do Vladimir estao nas fases que o C++ le") {
    // A FITA: a fase em que o Vladimir roda o castor tem os quatro gatilhos de
    // um passo, e a marca escrita nasce no posto em que ele a procura para
    // apagar - senao a marca apagada ficaria na fileira, e o "b" sairia de
    // uma casa que continua escrita.
    bool achouAFita = false;
    std::set<std::string> gatilhosDaFita;
    std::set<std::string> gatilhosDaParada;
    const auto regras = RegrasDoChefe("vladimir");
    for (const auto& f : FasesDoChefe("vladimir").fases) {
        for (const auto& a : f.ataques) {
            if (f.nome == Fita::kFaseDaFita) gatilhosDaFita.insert(a.gatilho);
            if (f.nome == Fita::kFaseDaFita && a.gatilho == Fita::kGatilhoDaEscrita) {
                achouAFita = true;
                REQUIRE(regras.conjuntos.count(a.regrasNome) == 1);
                for (const auto& r : regras.conjuntos.at(a.regrasNome)) CHECK(r.posto == Fita::kPostoDaFita);
            }
            if (f.nome == Parada::kFaseDaParada) gatilhosDaParada.insert(a.gatilho);
        }
    }
    CHECK(achouAFita);
    CHECK(PostoExiste(Fita::kPostoDaFita));
    for (const char* g : {Fita::kGatilhoDaEscrita, Fita::kGatilhoDoApagar, Fita::kGatilhoDoReler, Fita::kGatilhoDaParada}) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        CHECK(gatilhosDaFita.count(g) == 1);
    }

    // A PARADA: os tres gatilhos que o programa puxa, todos na fase dele.
    for (const char* g : {Parada::kGatilhoDaMetade, Parada::kGatilhoDoTriplo, Parada::kGatilhoDeParar}) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        CHECK(gatilhosDaParada.count(g) == 1);
    }
}

TEST_CASE("Dados: a capivara do Salles pode aparecer em toda fase dele, e e um abrigo que anda para os dois lados") {
    CHECK(GatilhoExiste(Capivara::kGatilho));

    // EM TODA FASE: o Salles sorteia a visita em qualquer uma, e uma fase sem o
    // ataque de gatilho faria o sorteio dar "vem" e nada aparecer.
    const auto& tipos = ProjeteisDoChefe("salles").projeteis;
    const auto fases = FasesDoChefe("salles").fases;
    REQUIRE(fases.size() == 4);
    for (const auto& f : fases) {
        CAPTURE(f.nome);
        int capivaras = 0;
        for (const auto& a : f.ataques) {
            if (a.gatilho != Capivara::kGatilho) continue;
            ++capivaras;
            REQUIRE(tipos.count(a.projetil) == 1);
            const auto& tipo = tipos.at(a.projetil);
            CHECK(tipo.abrigo);
            CHECK_FALSE(tipo.derrubavel);
            std::set<std::string> caras;
            for (const auto& anim : tipo.animacoes) caras.insert(anim.nome);
            CHECK(caras.count(Capivara::kAnimacaoDireita) == 1);
            CHECK(caras.count(Capivara::kAnimacaoEsquerda) == 1);
            // A TRAVESSIA ACABA DENTRO DA FASE: a que sai no ultimo instante
            // da janela atravessa o campo e as duas folgas antes do fim.
            REQUIRE(a.velocidade.has_value());
            const float travessia = (1200.0f + 2.0f * Capivara::kForaDaTela) / *a.velocidade;
            CHECK(Capivara::kAte + travessia <= f.duracao);
        }
        CHECK(capivaras == 1);
    }
}

TEST_CASE("Dados: a pilha do Salles - gatilhos, posto e caras sao nomes que os dados tem") {
    CHECK(GatilhoExiste(Pilha::kGatilhoDoPush));
    CHECK(GatilhoExiste(Pilha::kGatilhoDoEstouro));
    CHECK(PostoExiste(Pilha::kPostoDaPilha));

    // O BLOCO E DERRUBAVEL - e o pop do aluno, a tiro - e tem a cara de cada
    // digito que o programa empurra (Pilha::CaraDoBloco).
    const auto& tipos = ProjeteisDoChefe("salles").projeteis;
    const auto regras = RegrasDoChefe("salles");
    std::set<std::string> gatilhos;
    for (const auto& f : FasesDoChefe("salles").fases) {
        if (f.nome != Pilha::kFaseDaPilha) continue;
        for (const auto& a : f.ataques) {
            gatilhos.insert(a.gatilho);
            if (a.gatilho != Pilha::kGatilhoDoPush) continue;
            REQUIRE(tipos.count(a.projetil) == 1);
            const auto& bloco = tipos.at(a.projetil);
            CHECK(bloco.derrubavel);
            std::set<std::string> caras;
            for (const auto& anim : bloco.animacoes) caras.insert(anim.nome);
            for (int v = 0; v <= 9; ++v) CHECK(caras.count(Pilha::CaraDoBloco(v)) == 1);
            // E nasce no posto em que o Salles o procura.
            REQUIRE(regras.conjuntos.count(a.regrasNome) == 1);
            for (const auto& r : regras.conjuntos.at(a.regrasNome)) CHECK(r.posto == Pilha::kPostoDaPilha);
        }
    }
    CHECK(gatilhos.count(Pilha::kGatilhoDoPush) == 1);
    CHECK(gatilhos.count(Pilha::kGatilhoDoEstouro) == 1);
}

TEST_CASE("Dados: a tabela hash do Salles - gatilhos, postos e caras sao nomes que os dados tem") {
    for (const char* g : {Hash::kGatilhoDosBaldes, Hash::kGatilhoDaChave, Hash::kGatilhoDoPercurso, Hash::kGatilhoDoNulo}) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }
    CHECK(PostoExiste(Hash::kPostoDosBaldes));
    CHECK(PostoExiste(Hash::kPostoDasChaves));

    // TODA CHAVE QUE O PROGRAMA PODE SORTEAR TEM CARA, e todo balde do maior
    // tamanho tambem: uma cara que faltasse deixaria a chave com o numero errado.
    const auto& tipos = ProjeteisDoChefe("salles").projeteis;
    const auto regras = RegrasDoChefe("salles");
    std::map<std::string, std::string> projetilDoGatilho, regraDoGatilho;
    for (const auto& f : FasesDoChefe("salles").fases) {
        if (f.nome != Hash::kFaseDoHash) continue;
        for (const auto& a : f.ataques) {
            projetilDoGatilho[a.gatilho] = a.projetil;
            regraDoGatilho[a.gatilho] = a.regrasNome;
        }
    }
    const auto caras = [&](const std::string& tipo) {
        std::set<std::string> c;
        if (tipos.count(tipo)) for (const auto& a : tipos.at(tipo).animacoes) c.insert(a.nome);
        return c;
    };
    REQUIRE(projetilDoGatilho.count(Hash::kGatilhoDaChave) == 1);
    REQUIRE(projetilDoGatilho.count(Hash::kGatilhoDosBaldes) == 1);
    const auto dasChaves = caras(projetilDoGatilho[Hash::kGatilhoDaChave]);
    for (int k = 0; k <= Hash::kMaiorChave; ++k) CHECK(dasChaves.count(Hash::CaraDaChave(k)) == 1);
    const auto dosBaldes = caras(projetilDoGatilho[Hash::kGatilhoDosBaldes]);
    for (int i = 0; i < Hash::kTamanhos.back(); ++i) CHECK(dosBaldes.count(Hash::CaraDoBalde(i)) == 1);
    CHECK(tipos.at(projetilDoGatilho[Hash::kGatilhoDaChave]).derrubavel);
    CHECK(projetilDoGatilho.count(Hash::kGatilhoDoPercurso) == 1);
    CHECK(projetilDoGatilho.count(Hash::kGatilhoDoNulo) == 1);

    // E cada uma nasce no posto em que o Salles a procura.
    for (const auto& [gatilho, posto] : std::map<std::string, std::string>{
             {Hash::kGatilhoDaChave, Hash::kPostoDasChaves}, {Hash::kGatilhoDosBaldes, Hash::kPostoDosBaldes}}) {
        CAPTURE(gatilho);
        REQUIRE(regras.conjuntos.count(regraDoGatilho[gatilho]) == 1);
        for (const auto& r : regras.conjuntos.at(regraDoGatilho[gatilho])) CHECK(r.posto == posto);
    }
}

TEST_CASE("Dados: a arvore do Salles - gatilhos, posto e caras sao nomes que os dados tem") {
    for (const char* g : {Arvore::kGatilhoDaChave, Arvore::kGatilhoDaVisita, Arvore::kGatilhoDoNulo,
                          Arvore::kGatilhoDoAchado, Arvore::kGatilhoDaAresta}) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }
    CHECK(PostoExiste(Arvore::kPostoDaArvore));

    // O NO E DERRUBAVEL, tem a cara de toda chave que a arvore sorteia (as da
    // tabela hash, Hash::CaraDaChave), e nasce no posto da arvore.
    const auto& tipos = ProjeteisDoChefe("salles").projeteis;
    const auto regras = RegrasDoChefe("salles");
    std::set<std::string> gatilhos;
    for (const auto& f : FasesDoChefe("salles").fases) {
        if (f.nome != Arvore::kFaseDaArvore) continue;
        for (const auto& a : f.ataques) {
            gatilhos.insert(a.gatilho);
            if (a.gatilho != Arvore::kGatilhoDaChave) continue;
            REQUIRE(tipos.count(a.projetil) == 1);
            CHECK(tipos.at(a.projetil).derrubavel);
            std::set<std::string> caras;
            for (const auto& anim : tipos.at(a.projetil).animacoes) caras.insert(anim.nome);
            for (int k = 1; k <= Arvore::kMaiorChave; ++k) CHECK(caras.count(Hash::CaraDaChave(k)) == 1);
            REQUIRE(regras.conjuntos.count(a.regrasNome) == 1);
            for (const auto& r : regras.conjuntos.at(a.regrasNome)) CHECK(r.posto == Arvore::kPostoDaArvore);
        }
    }
    for (const char* g : {Arvore::kGatilhoDaChave, Arvore::kGatilhoDaVisita, Arvore::kGatilhoDoNulo,
                          Arvore::kGatilhoDoAchado, Arvore::kGatilhoDaAresta}) {
        CAPTURE(g);
        CHECK(gatilhos.count(g) == 1);
    }
}

TEST_CASE("Dados: o bubble sort do Salles - gatilhos, posto e caras sao nomes que os dados tem") {
    const std::vector<const char*> nomes = {Ordenacao::kGatilhoDoElemento, Ordenacao::kGatilhoDaTroca,
                                            Ordenacao::kGatilhoDoLugar};
    for (const char* g : nomes) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }
    CHECK(PostoExiste(Ordenacao::kPostoDoVetor));

    // O ELEMENTO E DERRUBAVEL, tem a cara de todo valor que o vetor sorteia, e
    // nasce no posto do vetor - e parado: quem o leva ao lugar e o Salles.
    const auto& tipos = ProjeteisDoChefe("salles").projeteis;
    const auto regras = RegrasDoChefe("salles");
    std::set<std::string> gatilhos;
    for (const auto& f : FasesDoChefe("salles").fases) {
        if (f.nome != Ordenacao::kFaseDaOrdenacao) continue;
        for (const auto& a : f.ataques) {
            gatilhos.insert(a.gatilho);
            if (a.gatilho != Ordenacao::kGatilhoDoElemento) continue;
            REQUIRE(tipos.count(a.projetil) == 1);
            CHECK(tipos.at(a.projetil).derrubavel);
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(0.0f));
            std::set<std::string> caras;
            for (const auto& anim : tipos.at(a.projetil).animacoes) caras.insert(anim.nome);
            for (int v = Ordenacao::kMenorValor; v <= Ordenacao::kMaiorValor; ++v) {
                CAPTURE(v);
                CHECK(caras.count(Ordenacao::CaraDoElemento(v)) == 1);
            }
            REQUIRE(regras.conjuntos.count(a.regrasNome) == 1);
            for (const auto& r : regras.conjuntos.at(a.regrasNome)) CHECK(r.posto == Ordenacao::kPostoDoVetor);
        }
    }
    for (const char* g : nomes) {
        CAPTURE(g);
        CHECK(gatilhos.count(g) == 1);
    }
}

TEST_CASE("Dados: a tabela do Thiago - toda fase a monta, e o SELECT e o produto tem os numeros de Banco.h") {
    for (const char* g : {Banco::kGatilhoDoCabecalho, Banco::kGatilhoDoDado, Banco::kGatilhoDoAviso,
                          Banco::kGatilhoDaSelecao, Banco::kGatilhoDaProjecao, Banco::kGatilhoDaCasa}) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }
    CHECK(PostoExiste(Banco::kPostoDaTabela));
    CHECK(PostoExiste(Banco::kPostoDoAviso));

    const auto& tipos = ProjeteisDoChefe("thiago").projeteis;
    const auto regras = RegrasDoChefe("thiago");
    auto postoDe = [&regras](const std::string& conjunto) {
        REQUIRE(regras.conjuntos.count(conjunto) == 1);
        std::string posto;
        for (const auto& r : regras.conjuntos.at(conjunto)) if (!r.posto.empty()) posto = r.posto;
        return posto;
    };

    for (const auto& f : FasesDoChefe("thiago").fases) {
        CAPTURE(f.nome);
        std::set<std::string> gatilhos;
        for (const auto& a : f.ataques) {
            gatilhos.insert(a.gatilho);
            if (a.gatilho == Banco::kGatilhoDoCabecalho || a.gatilho == Banco::kGatilhoDoDado) {
                // A TABELA NASCE PARADA, no posto dela, com a cara de tudo que
                // pode estar escrito numa casa.
                REQUIRE(a.velocidade.has_value());
                CHECK(*a.velocidade == doctest::Approx(0.0f));
                CHECK(postoDe(a.regrasNome) == Banco::kPostoDaTabela);
                REQUIRE(tipos.count(a.projetil) == 1);
                std::set<std::string> caras;
                for (const auto& anim : tipos.at(a.projetil).animacoes) caras.insert(anim.nome);
                for (int c = 0; c < Banco::kColunas; ++c) {
                    const auto col = static_cast<Banco::Coluna>(c);
                    if (a.gatilho == Banco::kGatilhoDoCabecalho) {
                        CHECK(caras.count(Banco::CaraDoCabecalho(col)) == 1);
                        continue;
                    }
                    for (const auto& r : Banco::Sortear(static_cast<std::uint32_t>(c) * 77u + 5u)) {
                        CAPTURE(Banco::Cara(r, col));
                        CHECK(caras.count(Banco::Cara(r, col)) == 1);
                    }
                }
                if (a.gatilho == Banco::kGatilhoDoDado) {
                    for (int v = 0; v <= 99; ++v) CHECK(caras.count("V" + std::to_string(v)) == 1);
                    for (const char* n : Banco::kNomes) CHECK(caras.count(n) == 1);
                    for (const char* c : Banco::kCursos) CHECK(caras.count(c) == 1);
                }
            }
        }
        // A FASE EM QUE A BATALHA COMECA E A QUE MONTA A TABELA: toda fase tem
        // os dois gatilhos.
        CHECK(gatilhos.count(Banco::kGatilhoDoCabecalho) == 1);
        CHECK(gatilhos.count(Banco::kGatilhoDoDado) == 1);

        // O MARCADOR E A CASA QUE ACENDE servem ao SELECT e ao produto
        // cartesiano: em qualquer fase, o marcador no posto do aviso, e a casa
        // com o numero de pedacos que o Thiago espalha nela.
        for (const auto& a : f.ataques) {
            if (a.gatilho == Banco::kGatilhoDoAviso) CHECK(postoDe(a.regrasNome) == Banco::kPostoDoAviso);
            if (a.gatilho == Banco::kGatilhoDaCasa) {
                REQUIRE(a.projeteis.has_value());
                CHECK(*a.projeteis == Banco::kPedacosNaLargura * Banco::kPedacosNaAltura);
            }
        }
        if (f.nome == Banco::kFaseDoProduto) {
            CHECK(gatilhos.count(Banco::kGatilhoDoAviso) == 1);
            CHECK(gatilhos.count(Banco::kGatilhoDaCasa) == 1);
        }
        if (f.nome == Transacao::kFaseDaTransacao) {
            // O UPDATE VARRE A LINHA, as escritas a travam, e o ROLLBACK a varre
            // do outro lado.
            for (const char* g : {Banco::kGatilhoDaSelecao, Transacao::kGatilhoDaEscrita,
                                  Transacao::kGatilhoDoDesfazer}) {
                CAPTURE(g);
                CHECK(GatilhoExiste(g));
                CHECK(gatilhos.count(g) == 1);
            }
            for (const auto& a : f.ataques) {
                if (a.gatilho != Transacao::kGatilhoDoDesfazer) continue;
                REQUIRE(a.consulta.has_value());
                CHECK(a.consulta->eixo == "Linha");
                REQUIRE(a.consulta->invertido.has_value());
                CHECK(*a.consulta->invertido);
                REQUIRE(a.consulta->linhas.has_value());
                CHECK(*a.consulta->linhas == Banco::kLinhas);
                REQUIRE(a.consulta->aviso.has_value());
                CHECK(*a.consulta->aviso == doctest::Approx(Banco::kAvisoDaLinha));
                REQUIRE(a.velocidade.has_value());
                CHECK(*a.velocidade == doctest::Approx(Banco::kVelocidadeDaLinha));
            }
        }
        if (f.nome == Banco::kFaseDaEscrita) {
            // O INSERT E O DELETE VARREM A LINHA, e o UPDATE acende a casa.
            for (const char* g : {Banco::kGatilhoDoAviso, Banco::kGatilhoDaSelecao, Banco::kGatilhoDaCasa}) {
                CAPTURE(g);
                CHECK(gatilhos.count(g) == 1);
            }
        }
        // A VARREDURA DA LINHA E A MESMA EM QUALQUER FASE que a use.
        for (const auto& a : f.ataques) {
            if (a.gatilho != Banco::kGatilhoDaSelecao) continue;
            REQUIRE(a.consulta.has_value());
            CHECK(a.consulta->eixo == "Linha");
            REQUIRE(a.consulta->linhas.has_value());
            CHECK(*a.consulta->linhas == Banco::kLinhas);
            REQUIRE(a.consulta->aviso.has_value());
            CHECK(*a.consulta->aviso == doctest::Approx(Banco::kAvisoDaLinha));
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(Banco::kVelocidadeDaLinha));
        }

        if (f.nome != Banco::kFaseDaSelecao) continue;
        for (const char* g : {Banco::kGatilhoDoAviso, Banco::kGatilhoDaSelecao, Banco::kGatilhoDaProjecao,
                              Banco::kGatilhoDaCasa}) {
            CAPTURE(g);
            CHECK(gatilhos.count(g) == 1);
        }
        for (const auto& a : f.ataques) {
            // A VARREDURA E A DA TABELA DE Banco.h: a mesma forma, e o aviso e a
            // velocidade que o Thiago escreve ao disparar - os daqui nao podem
            // dizer outra coisa.
            const bool linha = a.gatilho == Banco::kGatilhoDaSelecao;
            const bool coluna = a.gatilho == Banco::kGatilhoDaProjecao;
            if (!linha && !coluna) continue;
            REQUIRE(a.consulta.has_value());
            CHECK(a.consulta->eixo == (linha ? "Linha" : "Coluna"));
            if (linha) { REQUIRE(a.consulta->linhas.has_value()); CHECK(*a.consulta->linhas == Banco::kLinhas); }
            if (coluna) { REQUIRE(a.consulta->colunas.has_value()); CHECK(*a.consulta->colunas == Banco::kColunas); }
            REQUIRE(a.consulta->aviso.has_value());
            CHECK(*a.consulta->aviso == doctest::Approx(linha ? Banco::kAvisoDaLinha : Banco::kAvisoDaColuna));
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(linha ? Banco::kVelocidadeDaLinha : Banco::kVelocidadeDaColuna));
        }
    }
}

TEST_CASE("Dados: o diagrama do automato do Vladimir foi desenhado para os estados onde ele os poe") {
    // O DIAGRAMA E UMA FIGURA SO, gerada por Ferramentas/gerar_vladimir.py com os
    // centros dos estados escritos la. Se o C++ mudar um estado de lugar e o
    // gerador nao, as setas apontam para o vazio - e nada reclama.
    std::ifstream f(std::string(DPI_ASSETS_DIR) + "/../Ferramentas/gerar_vladimir.py");
    REQUIRE(f.is_open());
    std::ostringstream texto;
    texto << f.rdbuf();
    const std::string py = texto.str();

    std::smatch m;
    REQUIRE(std::regex_search(py, m, std::regex(R"(CENTROS = \[\((\d+), (\d+)\), \((\d+), (\d+)\), \((\d+), (\d+)\)\])")));
    for (int q = 0; q < Automato::kEstados; ++q) {
        CAPTURE(q);
        CHECK(std::stof(m[1 + 2 * q]) == doctest::Approx(Automato::kXDosEstados[static_cast<size_t>(q)]));
        CHECK(std::stof(m[2 + 2 * q]) == doctest::Approx(Automato::kYDosEstados[static_cast<size_t>(q)]));
    }
    REQUIRE(std::regex_search(py, m, std::regex(R"(DIAGRAMA_X0, DIAGRAMA_Y0 = (\d+), (\d+))")));
    const float x0 = std::stof(m[1]), y0 = std::stof(m[2]);
    REQUIRE(std::regex_search(py, m, std::regex(R"(DIAGRAMA_W, DIAGRAMA_H = (\d+), (\d+))")));
    CHECK(x0 + std::stof(m[1]) / 2.0f == doctest::Approx(Automato::kXDoDiagrama));
    CHECK(y0 + std::stof(m[2]) / 2.0f == doctest::Approx(Automato::kYDoDiagrama));
}

TEST_CASE("Dados: nenhuma regra de uma SprintAttack tem motion") {

    // A SPRINT E AS SUAS DUAS MOTIONS: as regras rodam DEPOIS da estrategia, e
    // uma "motion" numa regra substituiria a dela. A historia que perdesse a
    // motion nunca chegaria ao quadro, e o cursor - que nao acerta ninguem -
    // sairia voando pela tela. (A serpente do Luizinho antigo era o outro caso.)
    for (const auto& chefe : Chefes()) {
        const auto regras = RegrasDoChefe(chefe);
        for (const auto& f : FasesDoChefe(chefe).fases) {
            for (const auto& a : f.ataques) {
                if (a.estrategia != "SprintAttack") continue;
                std::vector<Regra> aplicaveis = a.regras;
                if (!a.regrasNome.empty() && regras.conjuntos.count(a.regrasNome)) {
                    const auto& r = regras.conjuntos.at(a.regrasNome);
                    aplicaveis.insert(aplicaveis.end(), r.begin(), r.end());
                }
                CAPTURE(chefe);
                CAPTURE(f.nome);
                for (const auto& r : aplicaveis) CHECK_FALSE(r.temMotion);
            }
        }
    }
}

TEST_CASE("Dados: toda fase de todo chefe dura Fase::kDuracao, e as etapas cobrem a fase inteira") {

    // A DURACAO MORA EM DOIS LUGARES: em cada fases.json e em Fase.h, de onde
    // os modulos que cortam a fase em pedacos a tiram (os comandos do Andre, as
    // etapas da cascata, os problemas da prova). Uma fase de outro tamanho
    // faria um deles acabar antes - ou depois - dela.
    //
    // E AS ETAPAS SOMAM A FASE: a leitura so recusa etapas que PASSAM da
    // fase. Quando as fases foram de 17 para 20 s, as etapas de 17 teriam
    // deixado 3 s no fim sem etapa nenhuma - sem placar da etapa e sem os
    // ataques dela.
    for (const auto& chefe : Chefes()) {
        for (const auto& f : FasesDoChefe(chefe).fases) {
            CAPTURE(chefe);
            CAPTURE(f.nome);
            CHECK(f.duracao == doctest::Approx(Fase::kDuracao));
            if (f.etapas.empty()) continue;
            float soma = 0.0f;
            for (const auto& e : f.etapas) soma += e.duracao;
            CHECK(soma == doctest::Approx(f.duracao));
        }
    }
}

TEST_CASE("Dados: a escada tem um degrau por etapa, do tamanho da etapa") {

    // A ESCADA E O DIAGRAMA DA CASCATA: um degrau por etapa. O movimento
    // conta os degraus pelo relogio ("a" segundos cada, "b" degraus), e as
    // etapas contam pelo delas - sao dois relogios dizendo a mesma coisa, e
    // so batem se os numeros baterem. Com um degrau mais curto que a etapa, a
    // professora chegaria na Manutencao enquanto o placar ainda diz Testes.
    for (const auto& chefe : Chefes()) {
        for (const auto& f : FasesDoChefe(chefe).fases) {
            if (f.movimento.tipo != "Escada") continue;
            CAPTURE(chefe);
            CAPTURE(f.nome);
            REQUIRE_FALSE(f.etapas.empty());
            CHECK(static_cast<int>(f.movimento.b) == static_cast<int>(f.etapas.size()));
            for (const auto& e : f.etapas) {
                CAPTURE(e.nome);
                CHECK(e.duracao == doctest::Approx(f.movimento.a));
            }
        }
    }
}

TEST_CASE("Dados: a cascata da Maria Lucia - etapas, gatilhos e caras sao nomes que os dados tem") {

    // A PROFESSORA CONTA AS ETAPAS PELO RELOGIO DELA (Cascata::EtapaNoTempo) e
    // dispara o gatilho de cada uma quando a etapa abre; quem deixa o disparo
    // passar e a janela da "etapa" do ataque, contada pelo relogio de
    // fases.json. Sao dois relogios dizendo a mesma coisa: com uma etapa de
    // outro nome ou de outra duracao, ou um gatilho na etapa errada, o disparo
    // cai fora da janela e nada nasce - os bugs nao viram divida, calados.
    const char* gatilhos[] = {Cascata::kGatilhoDaPlaca, Cascata::kGatilhoDoPedido, Cascata::kGatilhoDaClasse,
                              Cascata::kGatilhoDaMensagem, Cascata::kGatilhoDoCodigo, Cascata::kGatilhoDoBug,
                              Cascata::kGatilhoDaDivida};
    // A etapa de cada gatilho, na ordem acima ("" = a fase toda).
    const char* etapaDe[] = {"", Cascata::kEtapas[0], Cascata::kEtapas[1], Cascata::kEtapas[1],
                             Cascata::kEtapas[2], Cascata::kEtapas[3], Cascata::kEtapas[4]};
    for (const char* g : gatilhos) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }

    const auto lidas = FasesDoChefe("marialucia");
    const DescricaoDeFase* cascata = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Cascata::kFaseDaCascata) cascata = &f;
    REQUIRE(cascata != nullptr);

    REQUIRE(cascata->etapas.size() == Cascata::kEtapas.size());
    for (size_t k = 0; k < Cascata::kEtapas.size(); ++k) {
        CAPTURE(k);
        CHECK(cascata->etapas[k].nome == Cascata::kEtapas[k]);
        CHECK(cascata->etapas[k].duracao == doctest::Approx(Cascata::kDuracaoDaEtapa));
    }

    for (size_t i = 0; i < std::size(gatilhos); ++i) {
        CAPTURE(gatilhos[i]);
        int quantos = 0;
        for (const auto& a : cascata->ataques) {
            if (a.gatilho != gatilhos[i]) continue;
            ++quantos;
            CHECK(a.etapa == std::string(etapaDe[i]));
        }
        CHECK(quantos == 1);
    }

    // A CAIXA DE CADA ETAPA tem a cara apagada (P) e a acesa (A) que a
    // professora poe nela quando a etapa abre.
    const auto& tipos = ProjeteisDoChefe("marialucia").projeteis;
    std::string placa;
    for (const auto& a : cascata->ataques) if (a.gatilho == Cascata::kGatilhoDaPlaca) placa = a.projetil;
    REQUIRE(tipos.count(placa) == 1);
    std::set<std::string> caras;
    for (const auto& anim : tipos.at(placa).animacoes) caras.insert(anim.nome);
    for (size_t k = 0; k < Cascata::kEtapas.size(); ++k) {
        CHECK(caras.count("P" + std::to_string(k)) == 1);
        CHECK(caras.count("A" + std::to_string(k)) == 1);
    }
}

TEST_CASE("Dados: o fluxograma do Andre - gatilhos, velocidade do confete e caras sao nomes que os dados tem") {

    // O ANDRE PUXA OS GATILHOS e pinta os projeteis pelos nomes de
    // Condicional.h. Um cabecalho sem a cara do ramo ficaria com a do ramo
    // anterior - o aluno leria "if (n > 5)" num switch -, e um balao sem a do
    // numero mostraria o numero errado: a fase inteira e ler o numero.
    const char* gatilhos[] = {Condicional::kGatilhoDoRotulo, Condicional::kGatilhoDoBalao, Condicional::kGatilhoDoConfete};
    for (const char* g : gatilhos) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }

    const auto lidas = FasesDoChefe("andre");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Condicional::kFaseDoSe) fase = &f;
    REQUIRE(fase != nullptr);

    const auto& tipos = ProjeteisDoChefe("andre").projeteis;
    auto caras = [&tipos](const std::string& tipo) {
        std::set<std::string> nomes;
        REQUIRE(tipos.count(tipo) == 1);
        for (const auto& a : tipos.at(tipo).animacoes) nomes.insert(a.nome);
        return nomes;
    };

    std::map<std::string, std::string> projetilDe;
    for (const char* g : gatilhos) {
        CAPTURE(g);
        int quantos = 0;
        for (const auto& a : fase->ataques) {
            if (a.gatilho != g) continue;
            ++quantos;
            projetilDe[g] = a.projetil;
            // O CONFETE NASCE NA VELOCIDADE do Andre: ele o reposiciona pela
            // faixa e escreve a mesma velocidade de novo.
            if (a.gatilho == Condicional::kGatilhoDoConfete) {
                REQUIRE(a.velocidade.has_value());
                CHECK(*a.velocidade == doctest::Approx(Condicional::kVelocidadeDoConfete));
            }
        }
        CHECK(quantos == 1);
    }

    const auto rotulos = caras(projetilDe[Condicional::kGatilhoDoRotulo]);
    for (const auto& c : Condicional::Todos()) {
        for (int r = 0; r < Condicional::Ramos(c); ++r) {
            CAPTURE(Condicional::Rotulo(c, r));
            CHECK(rotulos.count(Condicional::CaraDoRotulo(c, r)) == 1);
            CHECK(rotulos.count(Condicional::CaraDoRotuloAceso(c, r)) == 1);
        }
    }
    const auto baloes = caras(projetilDe[Condicional::kGatilhoDoBalao]);
    const auto confetes = caras(projetilDe[Condicional::kGatilhoDoConfete]);
    for (int cor = 0; cor < Condicional::kCores; ++cor) {
        CHECK(confetes.count(Condicional::CaraDoConfete(cor)) == 1);
        for (int n = 0; n <= Condicional::kMaiorValor; ++n) {
            CAPTURE(Condicional::CaraDoBalao(n, cor));
            CHECK(baloes.count(Condicional::CaraDoBalao(n, cor)) == 1);
        }
    }
}

TEST_CASE("Dados: o vetor do chao do Andre - gatilhos, velocidade do cacho e caras sao nomes que os dados tem") {

    // O ANDRE ACENDE O INDICE da casa por onde o i passa e solta o cacho dela
    // pelos nomes de Laco.h. Um indice sem a cara acesa deixaria o aluno sem
    // o aviso; um cacho em outra velocidade chegaria ao chao antes do aviso
    // acabar - ou depois de o aluno ja ter voltado para a casa.
    const char* gatilhos[] = {Laco::kGatilhoDoIndice, Laco::kGatilhoDoCacho, Laco::kGatilhoDoDespejo};
    for (const char* g : gatilhos) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }

    const auto lidas = FasesDoChefe("andre");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Laco::kFaseDoLaco) fase = &f;
    REQUIRE(fase != nullptr);

    const auto& tipos = ProjeteisDoChefe("andre").projeteis;
    auto caras = [&tipos](const std::string& tipo) {
        std::set<std::string> nomes;
        REQUIRE(tipos.count(tipo) == 1);
        for (const auto& a : tipos.at(tipo).animacoes) nomes.insert(a.nome);
        return nomes;
    };

    std::map<std::string, std::string> projetilDe;
    for (const char* g : gatilhos) {
        CAPTURE(g);
        int quantos = 0;
        for (const auto& a : fase->ataques) {
            if (a.gatilho != g) continue;
            ++quantos;
            projetilDe[g] = a.projetil;
            if (a.gatilho == Laco::kGatilhoDoCacho) {
                REQUIRE(a.velocidade.has_value());
                CHECK(*a.velocidade == doctest::Approx(Laco::kVelocidadeDoCacho));
                REQUIRE(a.projeteis.has_value());
                CHECK(*a.projeteis == Laco::kBaloesPorCacho);
            }
        }
        CHECK(quantos == 1);
    }

    const auto indices = caras(projetilDe[Laco::kGatilhoDoIndice]);
    for (int i = 0; i < Laco::kCasas; ++i) {
        CAPTURE(i);
        CHECK(indices.count(Laco::CaraDoIndice(i)) == 1);
        CHECK(indices.count(Laco::CaraDoIndiceAceso(i)) == 1);
        CHECK(indices.count(Laco::CaraDoIndicePulado(i)) == 1);
        CHECK(indices.count(Laco::CaraDoIndiceErro(i)) == 1);
    }
    const auto bytes = caras(projetilDe[Laco::kGatilhoDoDespejo]);
    for (int k = 0; k < Laco::kBytes; ++k) CHECK(bytes.count(Laco::CaraDoByte(k)) == 1);
    const auto cachos = caras(projetilDe[Laco::kGatilhoDoCacho]);
    for (const char* cor : {"Red", "Blue", "Yellow"}) CHECK(cachos.count(cor) == 1);
}

TEST_CASE("Dados: as funcoes do Andre - gatilhos, leque e caras sao nomes que os dados tem") {

    // O ANDRE POE O CARTAO DE CADA FUNCAO e o valor que voa pelos nomes de
    // Funcao.h. Um cartao sem a cara da funcao mostraria a assinatura de
    // outra; um valor sem a cara do numero, o numero errado - e a fase inteira
    // e ler o que vai e o que volta.
    const char* gatilhos[] = {Laco::kGatilhoDoIndice, Laco::kGatilhoDoCacho, Funcao::kGatilhoDoCartao,
                              Funcao::kGatilhoDoValor, Funcao::kGatilhoDoLeque};
    for (const char* g : gatilhos) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }

    const auto lidas = FasesDoChefe("andre");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Funcao::kFaseDaFuncao) fase = &f;
    REQUIRE(fase != nullptr);

    const auto& tipos = ProjeteisDoChefe("andre").projeteis;
    auto caras = [&tipos](const std::string& tipo) {
        std::set<std::string> nomes;
        REQUIRE(tipos.count(tipo) == 1);
        for (const auto& a : tipos.at(tipo).animacoes) nomes.insert(a.nome);
        return nomes;
    };

    std::map<std::string, std::string> projetilDe;
    for (const char* g : gatilhos) {
        CAPTURE(g);
        int quantos = 0;
        for (const auto& a : fase->ataques) {
            if (a.gatilho != g) continue;
            ++quantos;
            projetilDe[g] = a.projetil;
            // O CACHO E O DA FASE 2, com a velocidade dela; o leque abre o que
            // Funcao.h diz.
            if (a.gatilho == Laco::kGatilhoDoCacho) {
                REQUIRE(a.velocidade.has_value());
                CHECK(*a.velocidade == doctest::Approx(Laco::kVelocidadeDoCacho));
            }
            if (a.gatilho == Funcao::kGatilhoDoLeque) {
                REQUIRE(a.angulo.has_value());
                CHECK(*a.angulo == doctest::Approx(Funcao::kAberturaDoLeque));
            }
        }
        CHECK(quantos == 1);
    }

    const auto cartoes = caras(projetilDe[Funcao::kGatilhoDoCartao]);
    for (int f = 0; f < Funcao::kFuncoes; ++f) {
        const auto nome = static_cast<Funcao::Nome>(f);
        CAPTURE(Funcao::Assinatura(nome));
        CHECK(cartoes.count(Funcao::CaraDoCartao(nome)) == 1);
        CHECK(cartoes.count(Funcao::CaraDoCartaoAceso(nome)) == 1);
    }
    const auto valores = caras(projetilDe[Funcao::kGatilhoDoValor]);
    for (int v = Funcao::kMenorValor; v <= Funcao::kMaiorValor; ++v) {
        CAPTURE(v);
        CHECK(valores.count(Funcao::CaraDoValor(v)) == 1);
    }
}

TEST_CASE("Dados: a prova do Andre roda cada problema com os mesmos ataques da fase de onde ele vem") {

    // A REPESCAGEM REUSA AS TRES MECANICAS: o Andre puxa nela os mesmos
    // gatilhos, e cada um tem de ser o MESMO ataque da fase de origem - outro
    // numero aqui (um confete mais rapido, um cacho de dois baloes) faria o
    // problema ser outra coisa que a fase que ele cobra.
    CHECK(GatilhoExiste(Prova::kGatilhoDoTrofeu));
    const auto lidas = FasesDoChefe("andre");
    auto achar = [&lidas](const std::string& nome) -> const DescricaoDeFase* {
        for (const auto& f : lidas.fases) if (f.nome == nome) return &f;
        return nullptr;
    };
    const DescricaoDeFase* prova = achar(Prova::kFaseDaProva);
    REQUIRE(prova != nullptr);

    std::set<std::string> fasesCobradas;
    for (const auto& p : Prova::kProva) {
        fasesCobradas.insert(p.tema == Prova::Tema::Condicional ? Condicional::kFaseDoSe
                           : p.tema == Prova::Tema::Laco ? Laco::kFaseDoLaco : Funcao::kFaseDaFuncao);
    }
    // O baloes(...) da funcao usa o vetor do chao, que e o da fase 2.
    for (const auto& origem : fasesCobradas) {
        const DescricaoDeFase* f = achar(origem);
        REQUIRE(f != nullptr);
        for (const auto& a : f->ataques) {
            CAPTURE(origem);
            CAPTURE(a.gatilho);
            REQUIRE_FALSE(a.gatilho.empty());
            int iguais = 0;
            for (const auto& b : prova->ataques) {
                if (b.gatilho != a.gatilho) continue;
                ++iguais;
                CHECK(b.estrategia == a.estrategia);
                CHECK(b.projetil == a.projetil);
                CHECK(b.projeteis == a.projeteis);
                CHECK(b.velocidade == a.velocidade);
                CHECK(b.angulo == a.angulo);
                CHECK(b.anguloCentral == a.anguloCentral);
                CHECK(b.regrasNome == a.regrasNome);
            }
            CHECK(iguais == 1);
        }
    }

    // O BALAO DO YES: a letra de cada problema, subindo na velocidade de
    // Prova.h.
    std::string projetil;
    for (const auto& b : prova->ataques) {
        if (b.gatilho != Prova::kGatilhoDoTrofeu) continue;
        projetil = b.projetil;
        REQUIRE(b.velocidade.has_value());
        CHECK(*b.velocidade == doctest::Approx(Prova::kVelocidadeDoTrofeu));
        REQUIRE(b.anguloCentral.has_value());
        CHECK(*b.anguloCentral == doctest::Approx(270.0f));
    }
    const auto& tipos = ProjeteisDoChefe("andre").projeteis;
    REQUIRE(tipos.count(projetil) == 1);
    std::set<std::string> caras;
    for (const auto& a : tipos.at(projetil).animacoes) caras.insert(a.nome);
    for (int k = 0; k < Prova::kProblemas; ++k) if (Prova::Aceito(k)) CHECK(caras.count(Prova::CaraDoTrofeu(k)) == 1);
    CHECK(prova->duracao >= Prova::kProva[Prova::kProblemas - 1].fim);
}

TEST_CASE("Dados: toda sprint dispara uma vez na etapa dela, e a revisao cai antes de a etapa acabar") {

    // O TIMEBOX E A ETAPA. A sprint dispara quando a etapa abre
    // (PrimeiroDisparoNaJanela - que NAO e o mesmo ponto para a etapa que abre
    // a fase), e a revisao vem "revisao" segundos depois. Se a revisao caisse
    // fora da etapa, o placar ja diria "Sprint 3" enquanto o que sobrou da 2
    // ainda esperava no quadro; e se o cooldown fosse menor que a etapa, uma
    // segunda leva de historias saia por cima da primeira.
    //
    // E A REVISAO DA ULTIMA SPRINT CAI DENTRO DA FASE: a primeira versao
    // revisava a sprint de oito historias aos 16,7 s, e o que sobrava dela
    // so chegava no aluno ja na fase seguinte. Pelo menos um segundo de fase
    // depois de cada revisao.
    bool achou = false;
    for (const auto& chefe : Chefes()) {
        for (const auto& f : FasesDoChefe(chefe).fases) {
            for (const auto& a : f.ataques) {
                if (a.estrategia != "SprintAttack") continue;
                CAPTURE(chefe);
                CAPTURE(f.nome);
                CAPTURE(a.etapa);
                REQUIRE_FALSE(a.etapa.empty());
                float de = 0.0f, ate = 0.0f;
                REQUIRE(JanelaDaEtapa(f.etapas, a.etapa, de, ate));

                const float disparo = PrimeiroDisparoNaJanela(de);
                const float revisao = (a.sprint && a.sprint->revisao) ? *a.sprint->revisao : Sprint::kRevisaoPadrao;
                CHECK(disparo + revisao <= ate + 1.0e-4f);
                CHECK(disparo + a.cooldown >= ate);
                CHECK(disparo + revisao + 1.0f <= f.duracao + 1.0e-4f);
                achou = true;
            }
        }
    }
    CHECK(achou);
}

TEST_CASE("Dados: o placar de cada sprint diz quantas historias ela tem") {

    // O PLANNING POKER ESTA ESCRITO DUAS VEZES: no anuncio da etapa ("Sprint 4
    // · 5 pontos") e nos "projeteis" do ataque dela. Mudar um sem o outro faria
    // o placar prometer cinco historias e o cursor arrastar tres.
    bool achou = false;
    for (const auto& chefe : Chefes()) {
        for (const auto& f : FasesDoChefe(chefe).fases) {
            for (const auto& a : f.ataques) {
                if (a.estrategia != "SprintAttack" || a.etapa.empty()) continue;
                for (const auto& e : f.etapas) {
                    if (e.nome != a.etapa) continue;
                    const auto ponto = e.anuncio.find("\xC2\xB7");   // o ponto do meio, em UTF-8
                    if (ponto == std::string::npos) continue;
                    CAPTURE(chefe);
                    CAPTURE(e.anuncio);
                    const int prometidas = std::atoi(e.anuncio.c_str() + ponto + 2);
                    REQUIRE(a.projeteis.has_value());
                    CHECK(prometidas == *a.projeteis);
                    achou = true;
                }
            }
        }
    }
    CHECK(achou);
}

TEST_CASE("Dados: o posto do quadro da sprint e um posto que a leitura aceita") {

    // O NOME ESTA EM DOIS LUGARES: o behavior da historia poe o projetil no
    // posto Sprint::kPostoDoQuadro, e a leitura de fases.json so aceita os
    // postos de PostoExiste. Trocar um sem o outro faria a leitura recusar a
    // PontosAttack da Maria Lucia - ou aceitar um nome que ninguem ocupa.
    CHECK(PostoExiste(Sprint::kPostoDoQuadro));
}

TEST_CASE("Dados: os gatilhos e o posto do Observer sao nomes que a leitura aceita") {

    // A MARIA LUCIA PUXA OS GATILHOS PELOS NOMES DE Observer.h, e os olhos
    // entram no posto pela regra "inscrever". Trocar um nome so de um lado faria
    // a leitura recusar o ataque - ou aceitar um gatilho que ninguem puxa.
    CHECK(GatilhoExiste(Observer::kGatilhoDaNotificacao));
    CHECK(GatilhoExiste(Observer::kGatilhoDaConsulta));
    CHECK(PostoExiste(Observer::kPostoDosObservadores));
}

TEST_CASE("Dados: o olho do Observer tem as caras que a professora poe nele") {

    // QUEM ANIMA O OLHO E A PROFESSORA, a cada quadro, pelos nomes de
    // Observer.h. Uma cara que o tipo nao tem e um SetAnimation que nao acha
    // nada - o olho para de olhar para o aluno, calado.
    const auto& tipos = ProjeteisDoChefe("marialucia").projeteis;
    REQUIRE(tipos.count("Observador") == 1);
    const auto& olho = tipos.at("Observador");
    for (const char* cara : { Observer::kOlhaEsquerda, Observer::kOlhaFrente, Observer::kOlhaDireita, Observer::kPisca }) {
        CAPTURE(cara);
        bool tem = false;
        for (const auto& a : olho.animacoes) if (a.nome == cara) tem = true;
        CHECK(tem);
    }
}

TEST_CASE("Dados: toda fase com ataque de notificacao tem o de consulta, e cada gatilho sai do posto de quem o puxa") {

    // O OBSERVER E O POLLING SAO A MESMA FASE: quem nao muda de estado tambem
    // leva aviso, so que mais devagar. Uma fase so com notificacao deixaria o
    // aluno parado sem aviso nenhum - a fase inteira resolvida ficando quieto.
    bool achou = false;
    for (const auto& chefe : Chefes()) {
        for (const auto& f : FasesDoChefe(chefe).fases) {
            bool notifica = false, consulta = false;
            for (const auto& a : f.ataques) {
                if (a.gatilho == Observer::kGatilhoDaNotificacao) notifica = true;
                if (a.gatilho == Observer::kGatilhoDaConsulta) consulta = true;
                // OS AVISOS SAEM DOS OLHOS, E AS FERRAMENTAS DOS AGENTES: quem
                // puxa cada gatilho e quem ocupa o posto dele. Um aviso do posto
                // errado sairia de ninguem, calado.
                if (!a.gatilho.empty() && a.estrategia == "PontosAttack") {
                    CAPTURE(chefe);
                    CAPTURE(f.nome);
                    CAPTURE(a.gatilho);
                    REQUIRE(a.pontos.has_value());
                    std::string esperado = Observer::kPostoDosObservadores;
                    if (a.gatilho == ReAct::kGatilhoDeAgir) esperado = ReAct::kPostoDosAgentes;
                    if (a.gatilho == Blink::kGatilhoDoLed) esperado = Blink::kPostoDasAcesas;
                    CHECK(a.pontos->posto == esperado);
                }
            }
            CAPTURE(chefe);
            CAPTURE(f.nome);
            CHECK(notifica == consulta);
            achou = achou || notifica;
        }
    }
    CHECK(achou);
}

TEST_CASE("Dados: toda palavra da frase de uma TokenAttack e uma animacao do token") {

    // A FRASE E UMA LISTA DE NOMES DE ANIMACAO, e a TokenAttack os pede um por
    // token. Uma palavra que o tipo nao tem e um SetAnimation que nao acha
    // nada: o token sai com a cara do anterior, e a frase muda calada.
    bool achou = false;
    for (const auto& chefe : Chefes()) {
        const auto tipos = ProjeteisDoChefe(chefe).projeteis;
        for (const auto& f : FasesDoChefe(chefe).fases) {
            for (const auto& a : f.ataques) {
                if (a.estrategia != "TokenAttack" || !a.tokens) continue;
                CAPTURE(chefe);
                CAPTURE(f.nome);
                REQUIRE(tipos.count(a.projetil) == 1);
                const auto& tipo = tipos.at(a.projetil);
                for (const auto& palavra : a.tokens->frase) {
                    CAPTURE(palavra);
                    bool tem = false;
                    for (const auto& anim : tipo.animacoes) if (anim.nome == palavra) tem = true;
                    CHECK(tem);
                    achou = true;
                }
            }
        }
    }
    CHECK(achou);
}

TEST_CASE("Dados: o gatilho e o posto dos agentes do Julio sao nomes que a leitura aceita") {
    CHECK(GatilhoExiste(ReAct::kGatilhoDeAgir));
    CHECK(PostoExiste(ReAct::kPostoDosAgentes));
}

TEST_CASE("Dados: as nuvens do Vitor - gatilhos, posto e caras sao nomes que os dados tem") {
    CHECK(GatilhoExiste(Nuvem::kGatilhoDasNuvens));
    CHECK(PostoExiste(Nuvem::kPostoDasNuvens));

    // QUEM TROCA A CARA DA NUVEM E O VITOR, pelos nomes de Nuvem.h. Uma cara
    // que o tipo nao tem e uma nuvem que para de avisar que a janela vai sair.
    const auto& tipos = ProjeteisDoChefe("vitor").projeteis;
    REQUIRE(tipos.count("Nuvem") == 1);
    for (const char* cara : { Nuvem::kCaraOciosa, Nuvem::kCaraProcessando }) {
        CAPTURE(cara);
        bool tem = false;
        for (const auto& a : tipos.at("Nuvem").animacoes) if (a.nome == cara) tem = true;
        CHECK(tem);
    }
}

TEST_CASE("Dados: a nuvem que o Ponto Extra derruba volta a tempo de mandar janela na fase dela") {

    // A NUVEM FORA DO AR volta kVoltaDasNuvens depois, e chove um ciclo de
    // processamento depois disso. Se as duas coisas juntas passassem da
    // metade da fase, um Ponto Extra no meio dela a deixaria sem chuva ate o
    // fim - o defeito que existia antes de a nuvem voltar.
    const auto lidas = FasesDoChefe("vitor");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Nuvem::kFaseDasNuvens) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(Nuvem::kVoltaDasNuvens + Tcp::kProcessa <= fase->duracao / 2.0f);
}

TEST_CASE("Dados: as nuvens do Vitor ficam presas a ele a fase inteira") {

    // A ANCORA DAS NUVENS COBRE A FASE: elas nascem em Tcp::kFormam e, solta
    // antes do fim, a nuvem cai pelo campo. Foi o que aconteceu quando as
    // fases passaram de 17 para 20 s e a ancora ficou nos 17.
    const auto lidas = FasesDoChefe("vitor");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Nuvem::kFaseDasNuvens) fase = &f;
    REQUIRE(fase != nullptr);
    std::string conjunto;
    for (const auto& a : fase->ataques) if (a.gatilho == Nuvem::kGatilhoDasNuvens) conjunto = a.regrasNome;
    REQUIRE(!conjunto.empty());
    const auto regras = RegrasDoChefe("vitor");
    REQUIRE(regras.conjuntos.count(conjunto) == 1);
    int ancoras = 0;
    for (const Regra& r : regras.conjuntos.at(conjunto)) {
        if (!r.temMotion || r.motion.tipo != "Ancorar") continue;
        ++ancoras;
        CHECK(r.motion.duracao >= fase->duracao - Tcp::kFormam);
    }
    CHECK(ancoras == 2);
}

TEST_CASE("Dados: o fork do Vitor e pela nuvem - a fase do fork vem logo depois da das nuvens, e se divide") {
    // O VITOR SO JUNTA AS NUVENS se a fase delas acaba NA do fork; em qualquer
    // outra ele as desfaz, e o fork volta a ser o de sempre sem ninguem notar.
    const auto lidas = FasesDoChefe("vitor");
    const DescricaoDeFase* nuvens = nullptr;
    const DescricaoDeFase* fork = nullptr;
    for (const auto& f : lidas.fases) {
        if (f.nome == Nuvem::kFaseDasNuvens) nuvens = &f;
        if (f.nome == Nuvem::kFaseDoFork) fork = &f;
    }
    REQUIRE(nuvens);
    REQUIRE(fork);
    CHECK(nuvens->proximo == Nuvem::kFaseDoFork);
    CHECK(fork->clone.existe);

    bool formaNuvens = false;
    for (const auto& a : nuvens->ataques) if (a.gatilho == Nuvem::kGatilhoDasNuvens) formaNuvens = true;
    CHECK(formaNuvens);
}

TEST_CASE("Dados: as portas do Ricardo - gatilho, postos e caras sao nomes que os dados tem") {
    CHECK(GatilhoExiste(Logica::kGatilhoDasPortas));
    CHECK(PostoExiste(Logica::kPostoDaPalavra));
    CHECK(PostoExiste(Logica::kPostoDasPortas));

    // QUEM TROCA A CARA DO BIT E DA PORTA E O RICARDO, pelos nomes de
    // Logica.h. Uma cara que o tipo nao tem e um bit que atravessa a porta e
    // continua mostrando o valor de antes.
    const auto& tipos = ProjeteisDoChefe("ricardo").projeteis;
    const auto tem = [&](const char* tipo, const std::string& cara) {
        if (tipos.count(tipo) == 0) return false;
        for (const auto& a : tipos.at(tipo).animacoes) if (a.nome == cara) return true;
        return false;
    };
    CHECK(tem("Bit", Logica::kAnimacaoDoZero));
    CHECK(tem("Bit", Logica::kAnimacaoDoUm));
    for (const auto op : { Logica::Operacao::E, Logica::Operacao::Ou,
                           Logica::Operacao::OuExclusivo, Logica::Operacao::Nao }) {
        for (const std::uint32_t b : { 0x0000u, 0xFFFFu }) {
            const std::string cara = Logica::AnimacaoDaPorta(Logica::Porta{op, b}, 0, Logica::kBits);
            CAPTURE(cara);
            CHECK(tem("Porta", cara));
        }
    }
}

TEST_CASE("Dados: a fase das portas do Ricardo - todo placar de etapa e uma porta, e toda porta admite palavra justa") {
    const auto lidas = FasesDoChefe("ricardo");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Logica::kFaseDasPortas) fase = &f;
    REQUIRE(fase);
    REQUIRE_FALSE(fase->etapas.empty());

    // O PLACAR E A PORTA: um anuncio que nao se le como porta deixaria as
    // portas fazendo a operacao da etapa anterior, com o placar dizendo outra.
    for (const auto& etapa : fase->etapas) {
        CAPTURE(etapa.anuncio);
        const auto porta = Logica::LerPorta(etapa.anuncio, Logica::kBits);
        REQUIRE(porta);

        // E UMA PORTA SEM PALAVRA JUSTA faria o sorteio devolver qualquer
        // coisa - um muro inteiro, ou nada.
        bool admite = false;
        for (std::uint32_t a = 0; a < (1u << Logica::kBits) && !admite; ++a) admite = Logica::Justa(*porta, a, Logica::kBits);
        CHECK(admite);
    }

    // A PALAVRA E A FILEIRA DE PORTAS TEM UM BIT POR COLUNA, as kBits colunas
    // em que o Ricardo divide a tela: com outro numero, o bit da coluna 3 da
    // palavra passaria pela porta da coluna 2.
    int palavras = 0, fileiras = 0;
    for (const auto& a : fase->ataques) {
        if (a.estrategia != "PalavraAttack") continue;
        CAPTURE(a.projetil);
        REQUIRE(a.projeteis);
        CHECK(*a.projeteis == Logica::kBits);
        if (a.gatilho == Logica::kGatilhoDasPortas) {
            ++fileiras;
            REQUIRE(a.palavra);
            CHECK(a.palavra->todosAtravessam);   // a porta nao machuca
            CHECK(a.palavra->altura);            // fica numa altura so
            CHECK((!a.velocidade || *a.velocidade == 0.0f));
        } else {
            ++palavras;
            CHECK((a.velocidade && *a.velocidade > 0.0f));
        }
    }
    CHECK(fileiras == 1);
    CHECK(palavras >= 1);
}

TEST_CASE("Dados: o clock do Ricardo - o placar de cada etapa e uma frequencia inteira, trocada num segundo inteiro") {
    CHECK(GatilhoExiste(Relogio::kGatilhoDoContador));
    CHECK(PostoExiste(Relogio::kPostoDosRegistradores));

    const auto lidas = FasesDoChefe("ricardo");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Relogio::kFaseDoRelogio) fase = &f;
    REQUIRE(fase);
    REQUIRE_FALSE(fase->etapas.empty());

    // O PLACAR E O CLOCK. E as contas do Ricardo - o leque a cada segundo
    // inteiro, que tem de ser borda de subida; a troca de frequencia, que tem
    // de cair numa borda - so fecham com frequencias inteiras trocadas em
    // segundos inteiros.
    float inicio = 0.0f;
    for (const auto& etapa : fase->etapas) {
        CAPTURE(etapa.anuncio);
        const auto f = Relogio::LerFrequencia(etapa.anuncio);
        REQUIRE(f);
        CHECK(*f >= 1.0f);
        CHECK(*f == std::floor(*f));
        CHECK(inicio == std::floor(inicio));
        inicio += etapa.duracao;
    }

    // O LEQUE DO CONTADOR: uma WaveAttack de gatilho, cujos bits nascem no posto
    // dos registradores - e com velocidade acima de 1 px/s: e dela que o
    // Ricardo tira a direcao de cada bit, e o RigidBody zera o x abaixo disso.
    const auto regras = RegrasDoChefe("ricardo");
    int leques = 0;
    for (const auto& a : fase->ataques) {
        if (a.gatilho != Relogio::kGatilhoDoContador) continue;
        ++leques;
        CHECK(a.estrategia == "WaveAttack");
        CHECK((a.velocidade && *a.velocidade > 1.0f));
        bool registra = false;
        if (regras.conjuntos.count(a.regrasNome)) {
            for (const auto& r : regras.conjuntos.at(a.regrasNome)) if (r.posto == Relogio::kPostoDosRegistradores) registra = true;
        }
        CHECK(registra);
    }
    CHECK(leques == 1);
}

TEST_CASE("Dados: o pipeline do Ricardo - toda instrucao que ataca tem o ataque na fase, e as caras existem") {
    CHECK(GatilhoExiste(Pipeline::kGatilhoDosEstagios));
    CHECK(GatilhoExiste(Pipeline::kGatilhoDaBusca));
    CHECK(PostoExiste(Pipeline::kPostoDosEstagios));
    CHECK(PostoExiste(Pipeline::kPostoDoPipeline));

    const auto lidas = FasesDoChefe("ricardo");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Pipeline::kFaseDoPipeline) fase = &f;
    REQUIRE(fase);
    const auto temGatilho = [&](const std::string& g) {
        for (const auto& a : fase->ataques) if (a.gatilho == g) return true;
        return false;
    };

    // UMA INSTRUCAO CUJO GATILHO A FASE NAO TEM chega ao estagio dela e nao
    // faz nada - calada, porque Disparar de um gatilho sem ataque e permitido.
    for (const auto& instrucao : Pipeline::Programa()) {
        for (int s = 0; s < Pipeline::kEstagios; ++s) {
            const std::string g = Pipeline::Gatilho(instrucao.tipo, s);
            if (g.empty()) continue;
            CAPTURE(g);
            CHECK(GatilhoExiste(g));
            CHECK(temGatilho(g));
        }
    }

    // A FILEIRA DE ESTAGIOS tem um por estagio, parada e toda enfeite; e cada
    // um, molduras e chips, nasce no posto em que o Ricardo o procura.
    const auto regras = RegrasDoChefe("ricardo");
    const auto poe = [&](const DescricaoDeAtaque& a, const char* posto) {
        if (!regras.conjuntos.count(a.regrasNome)) return false;
        for (const auto& r : regras.conjuntos.at(a.regrasNome)) if (r.posto == posto) return true;
        return false;
    };
    int fileiras = 0, buscas = 0;
    for (const auto& a : fase->ataques) {
        if (a.gatilho == Pipeline::kGatilhoDosEstagios) {
            ++fileiras;
            CHECK(a.estrategia == "PalavraAttack");
            REQUIRE(a.projeteis);
            CHECK(*a.projeteis == Pipeline::kEstagios);
            REQUIRE(a.palavra);
            CHECK(a.palavra->todosAtravessam);
            CHECK(poe(a, Pipeline::kPostoDosEstagios));
        }
        if (a.gatilho == Pipeline::kGatilhoDaBusca) {
            ++buscas;
            REQUIRE(a.projeteis);
            CHECK(*a.projeteis == 1);   // a busca traz UMA instrucao
            CHECK(poe(a, Pipeline::kPostoDoPipeline));
        }
    }
    CHECK(fileiras == 1);
    CHECK(buscas == 1);

    // AS CARAS: o nome de cada estagio, cada instrucao do programa e a bolha.
    const auto& tipos = ProjeteisDoChefe("ricardo").projeteis;
    const auto tem = [&](const char* tipo, const std::string& cara) {
        if (tipos.count(tipo) == 0) return false;
        for (const auto& a : tipos.at(tipo).animacoes) if (a.nome == cara) return true;
        return false;
    };
    for (int s = 0; s < Pipeline::kEstagios; ++s) CHECK(tem("Estagio", Pipeline::NomeDoEstagio(s)));
    for (const auto& instrucao : Pipeline::Programa()) CHECK(tem("Instrucao", Pipeline::Animacao(instrucao.tipo)));
    CHECK(tem("Instrucao", Pipeline::kAnimacaoDaBolha));

    // E O ALUNO FAZ BOLHA: a instrucao tem de ser derrubavel.
    REQUIRE(tipos.count("Instrucao") == 1);
    CHECK(tipos.at("Instrucao").derrubavel);
}

TEST_CASE("Dados: o Blink do Ricardo - o placar de cada etapa e um delay, e as placas sao as do posto que ele procura") {
    CHECK(GatilhoExiste(Blink::kGatilhoDoLed));
    CHECK(PostoExiste(Blink::kPostoDasPlacas));
    CHECK(PostoExiste(Blink::kPostoDasAcesas));

    const auto lidas = FasesDoChefe("ricardo");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Blink::kFaseDoBlink) fase = &f;
    REQUIRE(fase);
    REQUIRE_FALSE(fase->etapas.empty());

    // O PLACAR E O SKETCH: um anuncio sem delay deixaria as placas piscando no
    // de reserva, com o placar dizendo outro.
    for (const auto& etapa : fase->etapas) {
        CAPTURE(etapa.anuncio);
        CHECK(Blink::LerDelay(etapa.anuncio));
    }

    // AS PLACAS NASCEM NO POSTO "placas" - e dali que o Ricardo as acende -, e
    // o LED sai das acesas.
    const auto regras = RegrasDoChefe("ricardo");
    int placas = 0, leds = 0;
    for (const auto& a : fase->ataques) {
        if (a.gatilho == Blink::kGatilhoDoLed) {
            ++leds;
            REQUIRE(a.pontos);
            CHECK(a.pontos->posto == Blink::kPostoDasAcesas);
        }
        if (a.projetil == "Placa") {
            ++placas;
            bool posta = false;
            if (regras.conjuntos.count(a.regrasNome)) {
                for (const auto& r : regras.conjuntos.at(a.regrasNome)) if (r.posto == Blink::kPostoDasPlacas) posta = true;
            }
            CHECK(posta);
        }
    }
    CHECK(placas >= 1);
    CHECK(leds == 1);

    const auto& tipos = ProjeteisDoChefe("ricardo").projeteis;
    REQUIRE(tipos.count("Placa") == 1);
    std::set<std::string> caras;
    for (const auto& anim : tipos.at("Placa").animacoes) caras.insert(anim.nome);
    CHECK(caras.count(Blink::kAnimacaoApagada) == 1);
    CHECK(caras.count(Blink::kAnimacaoAcesa) == 1);
}

TEST_CASE("Dados: so o TCC pede orientacao, e quem abre o TCC sempre tem tres orientadores para escolher") {
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    std::vector<int> pedem;
    for (int i = 0; i < l.Quantas(); ++i) if (l.Por(i)->orientacao) pedem.push_back(i);
    REQUIRE(pedem.size() == 1);
    const int tcc = pedem.front();
    CHECK(l.CodigoDe(tcc) == "TCC");

    // TODA COMBINACAO DE APROVACOES nas outras materias, com a nota minima de
    // aprovacao: a que abre o TCC tem de deixar kCandidatos para escolher. Se o
    // curso mudar e der para chegar ao TCC com menos, a tela da escolha
    // mostraria uma lista curta - ou vazia.
    //
    // SO AS QUE DA PARA ALCANCAR JOGANDO: toda materia aprovada no conjunto
    // estava aberta por ele. Desbloqueada nao recorre (Materias.h), entao duas
    // aprovacoes soltas na coluna 3 abririam o TCC no papel - um progresso que
    // ninguem tem, porque a coluna 3 so abre depois da 2.
    std::vector<int> outras;
    for (int i = 0; i < l.Quantas(); ++i) if (i != tcc) outras.push_back(i);
    REQUIRE(outras.size() < 20);
    int abriram = 0;
    for (unsigned conjunto = 0; conjunto < (1u << outras.size()); ++conjunto) {
        Progresso p;
        for (size_t k = 0; k < outras.size(); ++k) {
            if (conjunto & (1u << k)) p.RegistrarNota(outras[k], Progresso::kNotaDeAprovacao);
        }
        bool alcancavel = true;
        for (size_t k = 0; k < outras.size(); ++k) {
            if ((conjunto & (1u << k)) && !l.Desbloqueada(outras[k], p)) alcancavel = false;
        }
        if (!alcancavel || !l.Desbloqueada(tcc, p)) continue;
        ++abriram;
        CAPTURE(conjunto);
        CHECK(static_cast<int>(Orientacao::Candidatos(l, {&p}).size()) == Orientacao::kCandidatos);
    }
    CHECK(abriram > 0);
}

// ---------------------------------------------------------------------------
// A DOCUMENTACAO NAO ESQUECE NOME NENHUM
//
// Os gatilhos e os postos sao listas fechadas: o leitor recusa um nome que nao
// conhece. Quem escreve dados novos aprende os nomes na tabela de
// Documentacao/arquivos-de-dados.md - e ela ficou parada nos do Ricardo
// enquanto o Vladimir e o Salles ganhavam trinta nomes. Este teste e o que faltava: cada nome que o motor aceita tem de
// aparecer, entre crases, na linha da tabela que fala dele.

namespace {
    std::vector<std::string> LinhasDaTabela(const std::string& campo) {
        std::ifstream f(std::string(DPI_ASSETS_DIR) + "/../Documentacao/arquivos-de-dados.md");
        std::vector<std::string> linhas;
        std::string linha;
        const std::string inicio = "| `" + campo + "` |";
        while (std::getline(f, linha)) {
            if (linha.rfind(inicio, 0) == 0) linhas.push_back(linha);
        }
        return linhas;
    }
}

TEST_CASE("Documentacao: a tabela de arquivos-de-dados.md cita todo gatilho que o motor aceita") {
    const auto linhas = LinhasDaTabela("gatilho");
    REQUIRE(linhas.size() == 1);
    for (const std::string& nome : NomesDosGatilhos()) {
        CAPTURE(nome);
        CHECK(linhas.front().find("`" + nome + "`") != std::string::npos);
    }
}

TEST_CASE("Documentacao: as duas linhas de posto em arquivos-de-dados.md citam todo posto que o motor aceita") {
    // Uma e a do ataque (de onde a PontosAttack atira), a outra a da regra (em
    // que posto o projetil entra ao nascer).
    const auto linhas = LinhasDaTabela("posto");
    REQUIRE(linhas.size() == 2);
    for (const std::string& linha : linhas) {
        for (const std::string& nome : NomesDosPostos()) {
            CAPTURE(nome);
            CHECK(linha.find("`" + nome + "`") != std::string::npos);
        }
    }
}

TEST_CASE("Dados: a chuva do escopo do Luizinho - gatilho, gota parada, raio e caras sao os de Escopo.h") {

    // O LUIZINHO PUXA O GATILHO e cobre a regiao com gotas pelas contas de
    // Escopo.h: a grade e feita para o raio kRaioDaGota, e uma gota maior
    // acertaria quem esta no buraco (a outra variavel), uma menor deixaria
    // fresta na regiao. A gota nasce PARADA - e o aviso - e quem a faz cair e
    // o professor, quando ela acende; uma velocidade aqui faria o aviso andar.
    CHECK(GatilhoExiste(Escopo::kGatilhoDaGota));

    const auto lidas = FasesDoChefe("luizinho");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Escopo::kFaseDoEscopo) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(fase->movimento.tipo == "Conduzido");

    std::string tipo;
    int quantos = 0;
    for (const auto& a : fase->ataques) {
        // A FASE E TODA DE GATILHO: um ataque de relogio choveria fora da busca.
        CHECK(!a.gatilho.empty());
        if (a.gatilho != Escopo::kGatilhoDaGota) continue;
        ++quantos;
        tipo = a.projetil;
        REQUIRE(a.velocidade.has_value());
        CHECK(*a.velocidade == doctest::Approx(0.0f));
    }
    CHECK(quantos == 1);

    const auto& tipos = ProjeteisDoChefe("luizinho").projeteis;
    REQUIRE(tipos.count(tipo) == 1);
    const auto& gota = tipos.at(tipo);
    std::set<std::string> caras;
    for (const auto& a : gota.animacoes) caras.insert(a.nome);
    for (const char nome : {'x', 'y'}) {
        CHECK(caras.count(Escopo::CaraDoAviso(nome)) == 1);
        CHECK(caras.count(Escopo::CaraDaGota(nome)) == 1);
    }

    // O COLISOR E O QUADRO DIVIDIDO PELO DIVISOR, vezes a escala.
    CHECK(gota.escala == doctest::Approx(1.0f));
    const auto atlas = LerJsonDeDados(LerArquivo(gota.dados));
    REQUIRE(atlas.contains("frames"));
    for (const auto& item : atlas["frames"]) {
        const auto& quadro = item.contains("frame") ? item["frame"] : item;
        const float lado = (gota.colisorDimensao == "altura") ? quadro["h"].get<float>() : quadro["w"].get<float>();
        CHECK(lado / gota.colisorDivisor == doctest::Approx(Escopo::kRaioDaGota));
    }
}

TEST_CASE("Dados: a coercao do Luizinho - gatilhos, velocidades e caras sao os de Coercao.h") {

    // O LUIZINHO PUXA OS GATILHOS e pinta cada chip pelo nome de Coercao.h.
    // Um chip sem a cara do numero mostraria o anel errado - a fase inteira e
    // ler a conta -, e a velocidade do arquivo e a do projetil: o da WaveAttack
    // nasce nela, e o professor so diz onde, quantos e para onde.
    const auto lidas = FasesDoChefe("luizinho");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Coercao::kFaseDosTipos) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(fase->movimento.tipo == "Conduzido");

    const std::map<std::string, float> velocidades = {
        {Coercao::kGatilhoDoOperador, 0.0f},
        {Coercao::kGatilhoDoOperando, 0.0f},
        {Coercao::kGatilhoDoAnel, Coercao::kVelocidadeDoAnel},
        {Coercao::kGatilhoDoTexto, Coercao::kVelocidadeDoTexto},
        {Coercao::kGatilhoDoErro, Coercao::kVelocidadeDoErro},
        {Coercao::kGatilhoDaCompilacao, Coercao::kVelocidadeDaCompilacao},
    };
    std::map<std::string, std::string> projetilDe;
    for (const auto& a : fase->ataques) CHECK(!a.gatilho.empty());
    for (const auto& [g, v] : velocidades) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        int quantos = 0;
        for (const auto& a : fase->ataques) {
            if (a.gatilho != g) continue;
            ++quantos;
            projetilDe[g] = a.projetil;
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(v));
        }
        CHECK(quantos == 1);
    }

    const auto& tipos = ProjeteisDoChefe("luizinho").projeteis;
    auto caras = [&tipos](const std::string& tipo) {
        std::set<std::string> nomes;
        REQUIRE(tipos.count(tipo) == 1);
        for (const auto& a : tipos.at(tipo).animacoes) nomes.insert(a.nome);
        return nomes;
    };

    const auto operandos = caras(projetilDe[Coercao::kGatilhoDoOperando]);
    const auto aneis = caras(projetilDe[Coercao::kGatilhoDoAnel]);
    const auto letras = caras(projetilDe[Coercao::kGatilhoDoTexto]);
    for (std::uint32_t s = 0; s < 200; ++s) {
        for (int v = 0; v < Coercao::kComandos; ++v) {
            const auto c = Coercao::Proximo(v, s * 2654435761u + static_cast<std::uint32_t>(v));
            const auto r = Coercao::Avaliar(c);
            CHECK(operandos.count(Coercao::CaraDoOperando(Coercao::Esquerdo(c))) == 1);
            CHECK(operandos.count(Coercao::CaraDoOperando(Coercao::Direito(c))) == 1);
            CHECK(operandos.count(Coercao::CaraDoOperando(r.esquerdoDepois)) == 1);
            CHECK(operandos.count(Coercao::CaraDoOperando(r.direitoDepois)) == 1);
            if (r.desfecho != Coercao::Desfecho::Valor) continue;
            if (r.valor.tipo == Coercao::Tipo::Numero) {
                CHECK(aneis.count(Coercao::CaraDoNumero(r.valor.numero)) == 1);
            } else {
                for (const char l : Coercao::Letras(r.valor)) CHECK(letras.count(Coercao::CaraDaLetra(l)) == 1);
            }
        }
    }

    const auto operadores = caras(projetilDe[Coercao::kGatilhoDoOperador]);
    for (const auto op : {Coercao::Operacao::Soma, Coercao::Operacao::Produto}) {
        CHECK(operadores.count(Coercao::CaraDoOperador(op, false)) == 1);
        CHECK(operadores.count(Coercao::CaraDoOperador(op, true)) == 1);
    }
    CHECK(caras(projetilDe[Coercao::kGatilhoDoErro]).count(Coercao::kCaraDoTypeError) == 1);
    CHECK(caras(projetilDe[Coercao::kGatilhoDaCompilacao]).count(Coercao::kCaraDaCompilacao) == 1);
}

TEST_CASE("Dados: o heap do Luizinho - gatilhos, velocidades, raio e caras sao os de Memoria.h") {

    // O LUIZINHO PUXA OS GATILHOS e pinta cada celula pelo estado do heap: uma
    // celula sem a cara de lixo deixaria o aluno sem saber o que o coletor vai
    // derrubar. A celula nasce PARADA - e o aviso - e o professor a deixa ali;
    // os cacos nascem na velocidade do arquivo, que e a de Memoria.h.
    const auto lidas = FasesDoChefe("luizinho");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Memoria::kFaseDaMemoria) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(fase->movimento.tipo == "Conduzido");
    CHECK(fase->proximo.empty());   // a memoria decide a batalha

    const std::map<std::string, float> velocidades = {
        {Memoria::kGatilhoDaCelula, 0.0f},
        {Memoria::kGatilhoDoPendente, Memoria::kVelocidadeDoPendente},
        {Memoria::kGatilhoDoEstouro, Memoria::kVelocidadeDoEstouro},
    };
    std::map<std::string, std::string> projetilDe;
    for (const auto& a : fase->ataques) CHECK(!a.gatilho.empty());
    for (const auto& [g, v] : velocidades) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        int quantos = 0;
        for (const auto& a : fase->ataques) {
            if (a.gatilho != g) continue;
            ++quantos;
            projetilDe[g] = a.projetil;
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(v));
        }
        CHECK(quantos == 1);
    }

    const auto& tipos = ProjeteisDoChefe("luizinho").projeteis;
    REQUIRE(tipos.count(projetilDe[Memoria::kGatilhoDaCelula]) == 1);
    const auto& celula = tipos.at(projetilDe[Memoria::kGatilhoDaCelula]);
    std::set<std::string> caras;
    for (const auto& a : celula.animacoes) caras.insert(a.nome);
    for (const char* c : {Memoria::kCaraDoAviso, Memoria::kCaraViva, Memoria::kCaraCrua, Memoria::kCaraDoProx,
                          Memoria::kCaraDoLixo, Memoria::kCaraMarcada, Memoria::kCaraQueVazou}) {
        CAPTURE(c);
        CHECK(caras.count(c) == 1);
    }
    for (const char* g : {Memoria::kGatilhoDoPendente, Memoria::kGatilhoDoEstouro}) {
        REQUIRE(tipos.count(projetilDe[g]) == 1);
        bool tem = false;
        for (const auto& a : tipos.at(projetilDe[g]).animacoes) tem = tem || a.nome == Memoria::kCaraDoEstilhaco;
        CHECK(tem);
    }

    // O COLISOR DA CELULA E A METADE DO LADO com que o alocador a encaixa.
    CHECK(celula.escala == doctest::Approx(1.0f));
    const auto atlas = LerJsonDeDados(LerArquivo(celula.dados));
    REQUIRE(atlas.contains("frames"));
    for (const auto& item : atlas["frames"]) {
        const auto& quadro = item.contains("frame") ? item["frame"] : item;
        const float lado = (celula.colisorDimensao == "altura") ? quadro["h"].get<float>() : quadro["w"].get<float>();
        CHECK(lado / celula.colisorDivisor == doctest::Approx(Memoria::kLadoDaCelula / 2.0f));
    }
}

TEST_CASE("Dados: a pilha de chamadas do Luizinho - gatilhos, velocidades e caras sao os de Excecao.h") {

    // O LUIZINHO PUXA OS GATILHOS e pinta a excecao com o nome do tipo: um
    // chip sem a cara do tipo diria IOException onde voa uma
    // NullPointerException, e a rodada inteira e ler o tipo contra os catch.
    const auto lidas = FasesDoChefe("luizinho");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Excecao::kFaseDasExcecoes) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(fase->movimento.tipo == "Conduzido");

    const std::map<std::string, float> velocidades = {
        {Excecao::kGatilhoDaExcecao, 0.0f},
        {Excecao::kGatilhoDoDestroco, Excecao::kVelocidadeDoDestroco},
        {Excecao::kGatilhoDoFinally, Excecao::kVelocidadeDoFinally},
    };
    std::map<std::string, std::string> projetilDe;
    for (const auto& a : fase->ataques) CHECK(!a.gatilho.empty());
    for (const auto& [g, v] : velocidades) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        int quantos = 0;
        for (const auto& a : fase->ataques) {
            if (a.gatilho != g) continue;
            ++quantos;
            projetilDe[g] = a.projetil;
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(v));
        }
        CHECK(quantos == 1);
    }

    const auto& tipos = ProjeteisDoChefe("luizinho").projeteis;
    auto caras = [&tipos](const std::string& tipo) {
        std::set<std::string> nomes;
        REQUIRE(tipos.count(tipo) == 1);
        for (const auto& a : tipos.at(tipo).animacoes) nomes.insert(a.nome);
        return nomes;
    };
    const auto excecoes = caras(projetilDe[Excecao::kGatilhoDaExcecao]);
    for (int k = 0; k < Excecao::kRodadas; ++k) {
        CHECK(excecoes.count(Excecao::CaraDaExcecao(Excecao::RodadaDaVez(k).lancada)) == 1);
    }
    CHECK(caras(projetilDe[Excecao::kGatilhoDoDestroco]).count(Excecao::kCaraDoDestroco) == 1);
    CHECK(caras(projetilDe[Excecao::kGatilhoDoFinally]).count(Excecao::kCaraDoFinally) == 1);
}

TEST_CASE("Dados: os semaforos do Vitor - o gatilho, a velocidade e a cara de cada thread sao os de Semaforo.h") {

    // O VITOR PUXA O GATILHO e pinta cada instrucao com o nome da coluna: uma
    // thread sem a cara dela no tipo sairia com a cor de outra, e o placar
    // ("fila: T3 T1") deixaria de apontar para uma coluna.
    const auto lidas = FasesDoChefe("vitor");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Semaforo::kFaseDosSemaforos) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(fase->movimento.tipo == "Conduzido");
    CHECK(GatilhoExiste(Semaforo::kGatilhoDaInstrucao));

    std::string tipo;
    int quantos = 0;
    for (const auto& a : fase->ataques) {
        if (a.gatilho != Semaforo::kGatilhoDaInstrucao) continue;
        ++quantos;
        tipo = a.projetil;
        // A WaveAttack escreve a velocidade do arquivo no primeiro quadro: e a
        // que a pilha conta para chegar a porta.
        REQUIRE(a.velocidade.has_value());
        CHECK(*a.velocidade == doctest::Approx(Semaforo::kVelocidadeDaInstrucao));
        CHECK(a.projeteis.value_or(0) == 1);
    }
    CHECK(quantos == 1);

    const auto& tipos = ProjeteisDoChefe("vitor").projeteis;
    REQUIRE(tipos.count(tipo) == 1);
    std::set<std::string> caras;
    for (const auto& a : tipos.at(tipo).animacoes) caras.insert(a.nome);
    for (int i = 0; i < Semaforo::kMaisThreads; ++i) CHECK(caras.count(Semaforo::Nome(i)) == 1);

    // AS CORRENTES CONTINUAM NA FASE: um ataque de relogio de um projetil que
    // acorrenta.
    bool corrente = false;
    for (const auto& a : fase->ataques) {
        if (!a.gatilho.empty() || tipos.count(a.projetil) == 0) continue;
        corrente = corrente || tipos.at(a.projetil).acorrenta.segundos > 0.0f;
    }
    CHECK(corrente);
}

TEST_CASE("Dados: o TCP do Vitor - gatilhos, velocidades e caras sao os de Tcp.h") {

    // A FASE DO TCP E A DAS NUVENS: sao elas que mandam a janela.
    CHECK(std::string(Tcp::kFaseDoTcp) == Nuvem::kFaseDasNuvens);
    const auto lidas = FasesDoChefe("vitor");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Tcp::kFaseDoTcp) fase = &f;
    REQUIRE(fase != nullptr);

    // A WaveAttack escreve a velocidade do arquivo no primeiro quadro, e o
    // Vitor so escolhe de onde e para onde: a velocidade e a de Tcp.h.
    const std::map<std::string, float> velocidades = {
        {Tcp::kGatilhoDoSegmento, Tcp::kVelocidadeDoSegmento},
        {Tcp::kGatilhoDoEncaminhado, Tcp::kVelocidadeDoEncaminhado},
        {Tcp::kGatilhoDoDescarte, Tcp::kVelocidadeDoDescarte},
    };
    std::map<std::string, std::string> projetilDe;
    for (const auto& [g, v] : velocidades) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        int quantos = 0;
        for (const auto& a : fase->ataques) {
            if (a.gatilho != g) continue;
            ++quantos;
            projetilDe[g] = a.projetil;
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(v));
            if (g == Tcp::kGatilhoDoDescarte) CHECK(a.projeteis.value_or(0) == Tcp::kAnelDoDescarte);
        }
        CHECK(quantos == 1);
    }
    // TODO ATAQUE DA FASE E DE GATILHO: o relogio dela e o RTT.
    for (const auto& a : fase->ataques) CHECK(!a.gatilho.empty());

    const auto& tipos = ProjeteisDoChefe("vitor").projeteis;
    auto caras = [&tipos](const std::string& tipo) {
        std::set<std::string> nomes;
        REQUIRE(tipos.count(tipo) == 1);
        for (const auto& a : tipos.at(tipo).animacoes) nomes.insert(a.nome);
        return nomes;
    };
    const auto doSegmento = caras(projetilDe[Tcp::kGatilhoDoSegmento]);
    CHECK(doSegmento.count(Tcp::kCaraDoNovo) == 1);
    CHECK(doSegmento.count(Tcp::kCaraDoRetransmitido) == 1);
    CHECK(caras(projetilDe[Tcp::kGatilhoDoEncaminhado]).count(Tcp::kCaraDoNovo) == 1);
    CHECK(caras(projetilDe[Tcp::kGatilhoDoDescarte]).count(Tcp::kCaraDoDescartado) == 1);
}

TEST_CASE("Dados: o jantar dos filosofos do Vitor - gatilhos e velocidades sao os de Filosofos.h") {

    const auto lidas = FasesDoChefe("vitor");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Filosofos::kFaseDoJantar) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(fase->movimento.tipo == "Conduzido");

    // A garfada sai na velocidade do arquivo; o garfo e a espera nascem
    // parados, e quem os move e o Vitor.
    const std::map<std::string, float> velocidades = {
        {Filosofos::kGatilhoDoGarfo, 0.0f},
        {Filosofos::kGatilhoDaGarfada, Filosofos::kVelocidadeDaGarfada},
        {Filosofos::kGatilhoDaEspera, 0.0f},
    };
    for (const auto& [g, v] : velocidades) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        int quantos = 0;
        for (const auto& a : fase->ataques) {
            if (a.gatilho != g) continue;
            ++quantos;
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(v));
            if (g == Filosofos::kGatilhoDaGarfada) CHECK(a.projeteis.value_or(0) == Filosofos::kGarfadasPorLeque);
        }
        CHECK(quantos == 1);
    }
    // TODO ATAQUE E DE GATILHO - e e por isso que o clone da fase 3 pode
    // emprestar esta fase: ele nao atira sozinho.
    for (const auto& a : fase->ataques) CHECK(!a.gatilho.empty());
    for (const auto& f : lidas.fases) {
        if (f.clone.existe) CHECK(f.clone.ataquesDe == Filosofos::kFaseDoJantar);
    }
}

TEST_CASE("Dados: a troca de mensagens do Vitor - gatilhos, velocidades e caras sao os de Mensagens.h") {

    // A FASE DAS MENSAGENS E A DO FORK: os dois processos sao os dois corpos.
    CHECK(std::string(Mensagens::kFaseDasMensagens) == Nuvem::kFaseDoFork);
    const auto lidas = FasesDoChefe("vitor");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Mensagens::kFaseDasMensagens) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(fase->movimento.tipo == "Conduzido");
    CHECK(fase->clone.existe);

    const std::map<std::string, float> velocidades = {
        {Mensagens::kGatilhoDoEnvelope, 0.0f},
        {Mensagens::kGatilhoDoProcessado, Mensagens::kVelocidadeDoProcessado},
        {Mensagens::kGatilhoDoEncontro, Mensagens::kVelocidadeDoEncontro},
    };
    std::map<std::string, std::string> projetilDe;
    for (const auto& [g, v] : velocidades) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        int quantos = 0;
        for (const auto& a : fase->ataques) {
            if (a.gatilho != g) continue;
            ++quantos;
            projetilDe[g] = a.projetil;
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(v));
            if (g == Mensagens::kGatilhoDoEncontro) CHECK(a.projeteis.value_or(0) == Mensagens::kAnelDoEncontro);
        }
        CHECK(quantos == 1);
    }
    for (const auto& a : fase->ataques) CHECK(!a.gatilho.empty());

    const auto& tipos = ProjeteisDoChefe("vitor").projeteis;
    const std::string envelope = projetilDe[Mensagens::kGatilhoDoEnvelope];
    REQUIRE(tipos.count(envelope) == 1);
    std::set<std::string> caras;
    for (const auto& a : tipos.at(envelope).animacoes) caras.insert(a.nome);
    CHECK(caras.count(Mensagens::kCaraDoEnvelope) == 1);
    CHECK(caras.count(Mensagens::kCaraDoEsperando) == 1);

    // O RESULTADO DA RPC E UM LEQUE com o do meio no aluno: as somas sao impares.
    for (int i = 0; i < Mensagens::kChamadas; ++i) CHECK(Mensagens::Soma(Mensagens::ChamadaDaVez(i)) % 2 == 1);
}
