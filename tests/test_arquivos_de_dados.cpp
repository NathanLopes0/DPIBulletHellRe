// Os arquivos de dados de verdade, os que o jogo carrega. Este arquivo e diferente dos outros da
// suite.

#include "doctest.h"
#include "../Source/Regras/Dificuldade.h"

#include "Ajudantes.h"

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
#include "../Source/Attacks/Dml.h"
#include "../Source/Attacks/Cascata.h"
#include "../Source/Attacks/BalaDePrata.h"
#include "../Source/Attacks/Decorator.h"
#include "../Source/Attacks/Scrum.h"
#include "../Source/Attacks/Tempera.h"
#include "../Source/Attacks/Condicional.h"
#include "../Source/Attacks/Laco.h"
#include "../Source/Attacks/Funcao.h"
#include "../Source/Attacks/Prova.h"
#include "../Source/Attacks/Amarracao.h"
#include "../Source/Attacks/Termina.h"
#include "../Source/Attacks/Coercao.h"
#include "../Source/Attacks/Excecao.h"
#include <regex>
#include "../Source/Attacks/PastaDoChefe.h"
#include "../Source/Curso/Materias.h"
#include "../Source/Curso/Orientacao.h"
#include "../Source/Entrada/Navegacao.h"
#include "../Source/Personagem/Personagens.h"
#include "../Source/Aluno/Progresso.h"
#include "../Source/Motor/JsonDeDados.h"
#include "../Source/Interface/Manual.h"

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

    /// Junta as frases de problema numa mensagem so, para a falha do teste dizer. O que esta errado
    /// em vez de so "esperava lista vazia".
    std::string Juntar(const std::vector<std::string>& problemas) {
        std::string s;
        for (const auto& p : problemas) s += "\n    - " + p;
        return s;
    }

    /// Os nomes de forma que existem em codigo e portanto nao precisam estar no
    /// arquivo. Espelha FormaPeloNome em RegrasDeAtaqueArquivo.cpp.
    const std::set<std::string> kFormasEmCodigo = {"Reta", ""};

    /// Os chefes sao os que materias.json diz, e nao uma lista escrita aqui.
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

// Cada arquivo, por si.

TEST_CASE("Dados: Assets/como-jogar.json nao tem problema nenhum") {
    const auto r = Manual::LerManual(LerArquivo("como-jogar.json"));
    CHECK_MESSAGE(r.problemas.empty(), "problemas em como-jogar.json:" << Juntar(r.problemas));
    CHECK(r.paginas.size() > 0);

    // O gabinete tambem tem o que ler: uma pagina so de teclas ficaria vazia la (Manual::Linhas). A
    // das mencoes e escrita pelo jogo, e nao tem linhas.
    for (const auto& p : r.paginas) {
        CAPTURE(p.titulo);
        CHECK((p.mencoes || !Manual::Linhas(p, true).empty()));
        for (const auto& linha : p.linhas) {
            CAPTURE(linha);
            for (const char* tecla : {"Esc ", "Espa\xc3\xa7o", "W A S D", "setas"}) {
                CHECK(linha.find(tecla) == std::string::npos);
            }
        }

        // So o que a fonte desenha.
        std::vector<std::string> todas = Manual::Linhas(p, false);
        todas.push_back(p.titulo);
        for (const auto& linha : todas) {
            CAPTURE(linha);
            CHECK(linha.find_first_of("#$%&*+/;=@[\\]^_`{}~") == std::string::npos);
        }
    }
}

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

// Os chefes.

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
    // Nao e exigencia do formato - um chefe pode ter tres.
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
    // LerFases ja descarta fase sem ataque, entao isto pega o descarte.
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

// As referencias entre os arquivos. E aqui que moram os erros que o jogo nao denuncia em tela.

TEST_CASE("Dados: todo conjunto de regras citado em fases.json existe em regras.json") {
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
    // Um ataque pode trazer as regras escritas nele mesmo, e elas citam formas do mesmo jeito.
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

// Coerencia interna dos ataques.

TEST_CASE("Dados: uma regra por indice nao aponta para fora da rajada") {
    // Se diminuir para 2, a regra do indice 2 nunca se aplica.
    for (const auto& chefe : Chefes()) {
        const auto regras = RegrasDoChefe(chefe);
        const auto& lista = FasesDoChefe(chefe).fases;
        for (const auto& f : lista) {
            for (const auto& a : f.ataques) {

                // Quantos projeteis o ataque dispara.
                const int n = a.projeteis.value_or(10);

                std::vector<Regra> aplicaveis = a.regras;
                if (!a.regrasNome.empty() && regras.conjuntos.count(a.regrasNome)) {
                    const auto& r = regras.conjuntos.at(a.regrasNome);
                    aplicaveis.insert(aplicaveis.end(), r.begin(), r.end());
                }

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
    // LerFases ja recusa isso, entao este teste confere que nenhum ataque real caiu na armadilha.
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

// Os tipos de projetil. Um erro aqui e da mesma familia dos tres que abrem este arquivo: nao quebra
// o jogo, deixa-o silenciosamente errado.

TEST_CASE("Dados: o projeteis.json de cada chefe nao tem problema nenhum") {
    for (const auto& chefe : Chefes()) {
        CAPTURE(chefe);
        const auto r = ProjeteisDoChefe(chefe);
        CHECK_MESSAGE(r.problemas.empty(), "problemas em " << chefe << "/projeteis.json:"
                                                           << Juntar(r.problemas));
    }
}

TEST_CASE("Dados: todo chefe tem projeteis na pasta dele") {
    // Sem projetil, o chefe monta e anda, mas nenhum ataque encontra a fabrica que pede.
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
    // Este teste pega o erro que passou anos escondido na fabrica da capivara.
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
    // O DrawAnimatedComponent nao reclama de indice fora da lista de quadros.
    for (const auto& chefe : Chefes()) {
        for (const auto& entrada : ProjeteisDoChefe(chefe).projeteis) {

            const std::string onde = chefe + "/" + entrada.first;
            CAPTURE(onde);

            const auto atlas = LerJsonDeDados(LerArquivo(entrada.second.dados));
            REQUIRE(atlas.contains("frames"));

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
    /// A largura e a altura de um PNG, lidas do cabecalho (ihdr): os quatro bytes de cada uma
    /// comecam no byte 16 e vem do mais alto para o mais baixo.
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

// A tranca da migracao. Os valores abaixo foram lidos das seis fabricas em C++ antes de elas serem
// apagadas.

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
    struct AnimEsperada {
        const char* conjunto;
        const char* projetil;
        const char* animacao;
        std::vector<int> quadros;
    };

    const std::vector<AnimEsperada> esperadas = {
        {"salles",  "Capivara", "Normal",      {0}},
        {"salles",  "Capivara", "Homing",      {1}},
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

// O acoplamento entre a onda e o caminho. Quando um ataque e WaveAttack e as regras dele tem um
// Path com atraso, os dois numeros nao sao independentes.

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

    // Hoje so uma combinacao destas existe.
    CHECK(combinacoesConferidas >= 1);
}

TEST_CASE("Dados: todo projetil e todo conjunto de regras de um chefe e usado por alguma fase dele") {

    // Quem tira a ultima fase que usa um tipo, tira o tipo junto (ou o teste diz qual sobrou).
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

    CHECK(pedidosConferidos >= 1);
}

TEST_CASE("Dados: uma parada de caminho aponta para um ponto que a forma tem") {

    // A parada ("pararNoPonto") e o indice de um waypoint.

    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));

    int paradasConferidas = 0;

    for (const auto& chefe : Chefes())
      for (const auto& conjunto : RegrasDoChefe(chefe).conjuntos) {
        for (const auto& r : conjunto.second) {

            if (!r.temMotion || r.motion.pararNoPonto < 0) continue;

            const std::string onde = conjunto.first + ", forma \"" + r.motion.forma + "\"";
            CAPTURE(onde);
            CAPTURE(r.motion.pararNoPonto);

            // Uma forma que vem do codigo (Reta) nao esta no arquivo.
            const auto forma = formas.formas.find(r.motion.forma);
            if (forma == formas.formas.end()) continue;

            const int pontos = static_cast<int>(forma->second.size());
            CAPTURE(pontos);

            // A exigencia e pararNoPonto < pontos - 1, nao < pontos: parar no.
            CHECK_MESSAGE(r.motion.pararNoPonto < pontos - 1,
                "a parada aponta para o ponto " << r.motion.pararNoPonto
                << " de uma forma com " << pontos << " pontos: ou o projetil nunca"
                << " chega nele, ou para no ultimo e o caminho acaba logo em seguida"
                << " - de um jeito ou de outro a parada nao faz o que se espera");

            ++paradasConferidas;
        }
    }

    // A fase 2 do Salles tem exatamente uma.
    CHECK(paradasConferidas >= 1);
}

// Familias de forma. Uma familia e uma entrada de formas.json que vale por varias formas, uma por
// projetil - "divisoes": 3 numa arvore vira oito folhas.

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

            CHECK_MESSAGE(formas.formas.count(r.motion.formaPorIndice) == 0,
                "\"" << r.motion.formaPorIndice << "\" e uma forma simples, nao uma familia"
                << " - a regra deveria usar \"forma\"");

            ++conferidas;
        }
    }

    CHECK(conferidas >= 1);
}

TEST_CASE("Dados: um ataque que usa familia dispara exatamente o tamanho dela") {

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
    // Uma familia de uma forma so e uma forma simples escrita de um jeito complicado.
    const auto formas = PathShapes::LerFormas(LerArquivo("Paths/formas.json"));
    for (const auto& fam : formas.familias) {
        CAPTURE(fam.first);
        CHECK(fam.second.size() >= 2);
    }
}

// As materias, e a prova de que o arquivo reproduz o C++ que ele substitui.

TEST_CASE("Dados: Assets/materias.json nao tem problema nenhum") {
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    CHECK_MESSAGE(l.problemas.empty(), "problemas em materias.json:" << Juntar(l.problemas));
    CHECK(l.Quantas() > 0);
}

TEST_CASE("Dados: as materias do curso estao la, com os codigos esperados") {

    // Esta lista nao e a ordem, e sim o conjunto.
    const std::set<std::string> esperadas = {
        "INF110", "INF213", "INF250", "INF220", "INF340", "INF331",
        "INF420", "INF221", "INF321", "INF310", "TCC", "INF332", "INF451", "INF425", "INF429"
    };

    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    REQUIRE(l.Quantas() == static_cast<int>(esperadas.size()));

    std::set<std::string> noArquivo;
    for (const auto& m : l.materias) noArquivo.insert(m.codigo);
    CHECK(noArquivo == esperadas);
}

TEST_CASE("Dados: toda materia com chefe aponta para uma pasta de chefe que existe") {
    // O campo "chefe" e o nome da pasta em Assets/Attacks/.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));

    int comChefe = 0;
    for (const auto& m : l.materias) {
        if (m.chefe.empty()) continue;   // materia ainda sem chefe e normal
        CAPTURE(m.codigo);
        CAPTURE(m.chefe);
        CHECK_FALSE(FasesDoChefe(m.chefe).fases.empty());
        ++comChefe;
    }
    CHECK(comChefe == 15);  // Hoje sao todas as quinze: o Michel, na optativa INF 429, foi o ultimo.
}

TEST_CASE("Dados: todo chefe citado em materias.json tem fabrica registrada") {

    // Game::GetFactory procura a fabrica pelo nome que esta no arquivo.
    const std::set<std::string> kFabricasRegistradas = {"salles", "ricardo", "andre", "julio",
                                                        "thiago", "vladimir", "vitor",
                                                        "marialucia", "lucas", "luizinho", "hugo",
                                                        "arroyo", "goulart", "daniel", "michel"};

    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));

    for (const auto& m : l.materias) {
        if (m.chefe.empty()) continue;
        CHECK_MESSAGE(kFabricasRegistradas.count(m.chefe) == 1,
                      "a materia " << m.codigo << " pede o chefe \"" << m.chefe
                      << "\", que nao tem fabrica em Game::InitializeBossFactory");
    }
}

TEST_CASE("Dados: a progressao do curso e a que o arquivo diz que e") {

    // Este teste nasceu comparando materias.json com a regra que vivia em Game::IsStageUnlocked.
    const std::vector<std::string> col2 = {"INF250", "INF220", "INF340", "INF331", "INF332"};
    const std::vector<std::string> col3 = {"INF420", "INF221", "INF321", "INF310", "INF451"};

    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    REQUIRE(l.Quantas() == 15);

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
        // As optativas pedem ouro e a compra: com 80 em tudo, nunca abrem (o teste delas vem abaixo).
        return false;
    };

    // Varre todas as combinacoes de aprovacao das treze obrigatorias: 8192 estados.
    int comparacoes = 0;
    for (int mascara = 0; mascara < (1 << 13); ++mascara) {

        Progresso p;
        for (int bit = 0; bit < 13; ++bit) {
            if (mascara & (1 << bit)) p.RegistrarNota(bit, 80.0f, Dificuldade::Nivel::Normal);
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

    CHECK(comparacoes == (1 << 13) * 15);

    // As optativas nao pedem materia alem da INF 213, nem uma a outra: pedem mencoes e a compra.
    // A INF 425: uma mencao de ouro.
    const int ds = l.IndiceDe("INF425");
    REQUIRE(ds >= 0);
    CHECK(l.Por(ds)->optativa);
    Progresso p;
    p.RegistrarNota(l.IndiceDe("INF110"), 93.0f, Dificuldade::Nivel::Normal);
    CHECK_FALSE(l.RegraCumprida(ds, p));          // ouro, mas sem a INF 213
    p.RegistrarNota(l.IndiceDe("INF213"), 80.0f, Dificuldade::Nivel::Normal);
    CHECK(l.RegraCumprida(ds, p));
    CHECK_FALSE(l.Desbloqueada(ds, p));           // falta comprar
    CHECK(l.PrecoDe(ds) > 0);
    p.RegistrarAtividade(Materias::ChaveDaCompra("INF425"));
    CHECK(l.Desbloqueada(ds, p));

    // A INF 429, o MaVIlab: duas mencoes de ouro, uma no Dificil, e a compra - mais cara. Sem a INF
    // 425: ela nao entra na conta.
    const int mv = l.IndiceDe("INF429");
    REQUIRE(mv >= 0);
    CHECK(l.Por(mv)->optativa);
    CHECK(l.PrecoDe(mv) > l.PrecoDe(ds));
    CHECK_FALSE(l.RegraCumprida(mv, p));          // um ouro so, e nada no Dificil
    p.RegistrarNota(l.IndiceDe("INF250"), 95.0f, Dificuldade::Nivel::Normal);
    CHECK_FALSE(l.RegraCumprida(mv, p));          // dois ouros, e ainda nada no Dificil
    p.RegistrarNota(l.IndiceDe("INF340"), 70.0f, Dificuldade::Nivel::Dificil);
    CHECK(l.RegraCumprida(mv, p));
    CHECK_FALSE(p.Aprovado(ds));                  // e a INF 425 nem foi feita
    CHECK_FALSE(l.Desbloqueada(mv, p));           // falta comprar
    p.RegistrarAtividade(Materias::ChaveDaCompra("INF429"));
    CHECK(l.Desbloqueada(mv, p));
}

TEST_CASE("Dados: personagens.json e lido sem nenhum problema") {

    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    CHECK_MESSAGE(c.problemas.empty(), "problemas em personagens.json:" << Juntar(c.problemas));
    CHECK_FALSE(c.categorias.empty());
}

TEST_CASE("Dados: as categorias que a tela de criacao precisa existem") {

    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    for (const std::string& id : {"pele", "cabelo", "camisa", "calca", "sapato", "mochila"}) {
        CHECK_MESSAGE(c.Por(id) != nullptr, "falta a categoria \"" << id << "\"");
    }
}

TEST_CASE("Dados: o sapato e a mochila novos nao mudam a personagem de quem ja tinha ficha") {

    // Uma ficha de antes nao tem escolha de sapato nem de mochila, e cai no padrao (Resolver): o
    // padrao tem de ser o que ela ja mostrava.
    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    // O sapato era uma camada fixa grafite (74, 70, 92): o padrao e o tenis nessa cor.
    const Personagens::Categoria* sapato = c.Por("sapato");
    REQUIRE(sapato != nullptr);
    CHECK(sapato->pecas.front().id == "tenis");
    CHECK(sapato->cores.front().r == 74);
    CHECK(sapato->cores.front().g == 70);
    CHECK(sapato->cores.front().b == 92);
    CHECK(c.fixas.empty());

    // E ninguem ganha uma mochila sem escolher.
    const Personagens::Categoria* mochila = c.Por("mochila");
    REQUIRE(mochila != nullptr);
    CHECK(mochila->pecas.front().id == "nenhuma");

    // O resto da ficha de antes continua o mesmo: as pecas e as cores antigas seguem no catalogo.
    const Personagens::Aparencia antiga = c.Resolver(c.predefinidas.front().aparencia);
    CHECK(antiga.Por("sapato")->peca == "tenis");
    CHECK(antiga.Por("mochila")->peca == "nenhuma");
    for (const char* cabelo : {"curto", "longo", "coque", "espetado", "raspado"}) {
        CHECK(c.Por("cabelo")->PecaPor(cabelo) != nullptr);
    }
}

TEST_CASE("Dados: ha o que escolher em toda categoria, e o cabelo tem mais que todas") {
    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));
    for (const auto& cat : c.categorias) {
        CAPTURE(cat.id);
        // A pele e uma peca so (o corpo): escolhe-se a cor.
        CHECK((cat.pecas.size() > 1 || cat.cores.size() > 1));
        CHECK(cat.cores.size() >= 8);
    }
    CHECK(c.Por("cabelo")->pecas.size() >= 15);
    CHECK(c.Por("cabelo")->cores.size() >= 15);
}

TEST_CASE("Dados: toda arte citada no catalogo existe em disco") {

    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    std::vector<std::string> artes;
    for (const auto& cat : c.categorias) {
        for (const auto& p : cat.pecas) artes.push_back(p.arte);
    }
    for (const auto& f : c.fixas) artes.push_back(f.arte);
    if (c.formatura.Existe()) artes.push_back(c.formatura.arte);

    REQUIRE_FALSE(artes.empty());

    for (const auto& arte : artes) {
        // O catalogo guarda o caminho sem extensao; quem carrega acrescenta.
        for (const char* ext : {".png", ".json"}) {
            const std::string caminho = std::string(DPI_ASSETS_DIR) + "/" + arte + ext;
            std::ifstream a(caminho, std::ios::binary);
            CHECK_MESSAGE(a.is_open(), "arte citada no catalogo e ausente em disco: " << caminho);
        }
    }
}

TEST_CASE("Dados: toda peca tem quatro quadros de 64x64") {

    // Nao e detalhe de arte.
    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));

    std::vector<std::string> artes;
    for (const auto& cat : c.categorias) {
        for (const auto& p : cat.pecas) artes.push_back(p.arte);
    }
    for (const auto& f : c.fixas) artes.push_back(f.arte);
    if (c.formatura.Existe()) artes.push_back(c.formatura.arte);

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

    const std::vector<Personagens::Camada> camadas = c.Camadas(Ajudantes::Padrao(c));

    // Uma por categoria, mais as fixas.
    CHECK(camadas.size() == c.categorias.size() + c.fixas.size());

    for (size_t i = 1; i < camadas.size(); ++i) {
        CHECK_MESSAGE(camadas[i - 1].ordem < camadas[i].ordem,
                      "duas camadas com a mesma ordem de desenho");
    }
}

TEST_CASE("Dados: o capelo e de quem passou no TCC, e vai por cima de tudo") {

    const Personagens::Catalogo c = Personagens::LerCatalogo(LerArquivo("personagens.json"));
    REQUIRE(c.formatura.Existe());

    // A materia e uma que existe - o TCC, o fim do curso.
    const auto materias = Materias::LerMaterias(LerArquivo("materias.json"));
    CHECK(c.formatura.materia == "TCC");
    CHECK(materias.IndiceDe(c.formatura.materia) >= 0);

    // Por cima de toda categoria: o cabelo e a mochila nao cobrem o capelo.
    for (const auto& cat : c.categorias) {
        CAPTURE(cat.id);
        CHECK(cat.ordem < c.formatura.ordem);
    }

    // Quem se formou tem uma camada a mais, a ultima, e ela nao e tingida.
    const Personagens::Aparencia padrao = Ajudantes::Padrao(c);
    const std::vector<Personagens::Camada> sem = c.Camadas(padrao);
    const std::vector<Personagens::Camada> com = c.Camadas(padrao, true);
    REQUIRE(com.size() == sem.size() + 1);
    CHECK(com.back().arte == c.formatura.arte);
    CHECK_FALSE(com.back().tingida);
}

TEST_CASE("Dados: nenhum projetil do Salles gira com a direcao do movimento") {

    // Hoje todo projetil dele se le em pe.
    const auto r = ProjeteisDoChefe("salles");
    REQUIRE(!r.projeteis.empty());
    for (const auto& [nome, p] : r.projeteis) {
        CAPTURE(nome);
        CHECK_FALSE(p.rotacionarComAVelocidade);
    }
}

// A navegacao contra o materias.json de verdade. Os testes de test_navegacao.cpp provam a regra
// contra uma grade escrita a mao.

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

    // O defeito relatado.
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

    // O defeito que ninguem tinha visto ainda.
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

    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    REQUIRE(l.Quantas() > 0);

    for (const auto& m : l.materias) {
        CAPTURE(m.codigo);
        CHECK_FALSE(m.nomeCompleto.empty());
        CHECK_FALSE(m.professor.empty());

        // O nome da disciplina nao pode ser o codigo repetido.
        CHECK(m.nomeCompleto != m.codigo);
        CHECK(m.nomeCompleto != m.nome);
    }
}

TEST_CASE("Dados: toda materia com professor ja tem batalha - o TCC foi a ultima") {

    // Desde o 134 ele tem.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));

    int comProfessorSemChefe = 0;
    for (const auto& m : l.materias) {
        if (!m.professor.empty() && m.chefe.empty()) ++comProfessorSemChefe;
    }
    CHECK(comProfessorSemChefe == 0);
}

TEST_CASE("Dados: toda coluna do curso depende da anterior, entao a grade pode ser ligada") {

    // E o que autoriza a tela de selecao a desenhar o tronco entre as colunas.
    // As optativas ficam de fora: estao em outra pagina, e nao se ligam ao TCC.
    const auto l = Materias::LerMaterias(LerArquivo("materias.json"));
    REQUIRE(l.QuantasColunas() == 7);
    REQUIRE(l.QuantasColunasObrigatorias() == 5);

    CHECK_FALSE(l.ColunaDependeDaAnterior(0));
    for (int c = 1; c < l.QuantasColunasObrigatorias(); ++c) {
        CAPTURE(c);
        CHECK(l.ColunaDependeDaAnterior(c));
    }
}

TEST_CASE("Dados: as quatro fases do Vladimir anunciam alguma coisa no placar") {

    // Canario, e espelha os dados.
    const auto fases = FasesDoChefe("vladimir").fases;
    REQUIRE(fases.size() == 4);
    for (const auto& f : fases) {
        CAPTURE(f.nome);
        CHECK_FALSE(f.anuncio.empty());
    }
}

TEST_CASE("Dados: o Programa de Intercambio do Vladimir vale nas quatro fases") {

    // Se uma fase deixar de trocar, tire-a daqui.
    const auto fases = FasesDoChefe("vladimir").fases;
    REQUIRE(fases.size() == 4);
    for (const auto& f : fases) {
        CAPTURE(f.nome);
        CHECK(f.intercambio.has_value());
    }
}

TEST_CASE("Dados: o intercambio do Vladimir troca as letras que o Simbolo tem, e deixa a fita no posto dela") {
    CHECK(PostoExiste(Alfabeto::kPostoDaFita));

    const auto& tipos = ProjeteisDoChefe("vladimir").projeteis;
    REQUIRE(tipos.count("Simbolo") == 1);
    std::set<std::string> caras;
    for (const auto& a : tipos.at("Simbolo").animacoes) caras.insert(a.nome);
    CHECK(caras.count(Alfabeto::kA) == 1);
    CHECK(caras.count(Alfabeto::kB) == 1);

    // A casa da fita nao e letra em voo.
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

    // As caras dos estados: o Vladimir as monta como "Q" + estado + "Aceso" (Vladimir::CaraDoEstado).
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

    // Todo gatilho do automato esta na fase dele.
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
    // Todo gatilho do compilador esta na fase dele.
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

    // As caras que o Vladimir da - a de cada token, de cada no e de cada instrucao.
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
    for (const auto& i : Ajudantes::Codigo(Compilador::Sintatico(Compilador::Lexico("x = a + b - 2 * a;")))) {
        CAPTURE(i);
        CHECK(instrucoes.count(Compilador::CaraDaInstrucao(i)) == 1);
    }
    // E nenhuma ficha se chama "A" ou "B".
    for (const auto& caras : {lexemas, instrucoes}) {
        CHECK(caras.count("A") == 0);
        CHECK(caras.count("B") == 0);
    }
}

TEST_CASE("Dados: a fita e o problema da parada do Vladimir estao nas fases que o C++ le") {
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

    // A parada: os tres gatilhos que o programa puxa, todos na fase dele.
    for (const char* g : {Parada::kGatilhoDaMetade, Parada::kGatilhoDoTriplo, Parada::kGatilhoDeParar}) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        CHECK(gatilhosDaParada.count(g) == 1);
    }
}

TEST_CASE("Dados: a capivara do Salles pode aparecer em toda fase dele, e e um abrigo que anda para os dois lados") {
    CHECK(GatilhoExiste(Capivara::kGatilho));

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
            // A travessia acaba dentro da fase.
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

    // O bloco e derrubavel - e o pop do aluno, a tiro.
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

TEST_CASE("Dados: o Salles do Normal e mais leve que o do Dificil na pilha e no hash") {
    // A pilha cai com menos tiros no Normal.
    const auto lidos = ProjeteisDoChefe("salles");
    const auto& bloco = lidos.projeteis.at("Bloco");
    CHECK(NoNivel(bloco, Dificuldade::Nivel::Normal).resistencia == 2);
    CHECK(NoNivel(bloco, Dificuldade::Nivel::Dificil).resistencia == 3);

    // E no hash ele vai aos baldes mais devagar que o aluno anda (300), nos dois modos.
    bool achou = false;
    for (const auto& f : FasesDoChefe("salles").fases) {
        if (f.nome != Hash::kFaseDoHash) continue;
        achou = true;
        CHECK(f.movimento.tipo == "Conduzido");
        CHECK(NoNivel(f.movimento, Dificuldade::Nivel::Normal).a == doctest::Approx(220.0f));
        CHECK(NoNivel(f.movimento, Dificuldade::Nivel::Dificil).a == doctest::Approx(280.0f));
    }
    CHECK(achou);
}

TEST_CASE("Dados: a tabela hash do Salles - gatilhos, postos e caras sao nomes que os dados tem") {
    for (const char* g : {Hash::kGatilhoDosBaldes, Hash::kGatilhoDaChave, Hash::kGatilhoDoPercurso, Hash::kGatilhoDoNulo}) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
    }
    CHECK(PostoExiste(Hash::kPostoDosBaldes));
    CHECK(PostoExiste(Hash::kPostoDasChaves));

    // Toda chave que o programa pode sortear tem cara, e todo balde do maior tamanho tambem.
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
                // A tabela nasce parada, no posto dela, com a cara de tudo que pode estar escrito numa casa.
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
        // A fase em que a batalha comeca e a que monta a tabela: toda fase tem os dois gatilhos.
        CHECK(gatilhos.count(Banco::kGatilhoDoCabecalho) == 1);
        CHECK(gatilhos.count(Banco::kGatilhoDoDado) == 1);

        // O marcador e a casa que acende servem ao select e ao produto cartesiano.
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
        auto carasDe = [&tipos](const std::string& projetil) {
            REQUIRE(tipos.count(projetil) == 1);
            std::set<std::string> caras;
            for (const auto& anim : tipos.at(projetil).animacoes) caras.insert(anim.nome);
            return caras;
        };
        if (f.nome == Banco::kFaseDaEscrita) {
            // A fase 2 nao varre: o INSERT arremessa, o DELETE risca e derruba, o UPDATE marca e chuta.
            for (const char* g : {Banco::kGatilhoDoAviso, Transacao::kGatilhoDoRascunho, Dml::kGatilhoDoCaco}) {
                CAPTURE(g);
                CHECK(GatilhoExiste(g));
                CHECK(gatilhos.count(g) == 1);
            }
            for (const auto& a : f.ataques) {
                CAPTURE(a.gatilho);
                CHECK(a.estrategia != "ConsultaAttack");
                if (a.gatilho == Transacao::kGatilhoDoRascunho) CHECK(carasDe(a.projetil).count(Dml::kCaraDoRisco) == 1);
            }
        }
        for (const auto& a : f.ataques) {
            if (a.gatilho != Dml::kGatilhoDoCaco) continue;
            // Os cacos sobem em leque, mais no Dificil; sobem o bastante para cobrir o chao, e quem
            // sai dele - meio aluno acima - escapa.
            for (const auto nivel : {Dificuldade::Nivel::Normal, Dificuldade::Nivel::Dificil}) {
                const DescricaoDeAtaque n = NoNivel(a, nivel);
                REQUIRE(n.projeteis.has_value());
                REQUIRE(n.velocidade.has_value());
                CHECK(*n.projeteis >= 3);
                REQUIRE(n.anguloCentral.has_value());
                CHECK(*n.anguloCentral == doctest::Approx(270.0f));
                const float subida = Dml::Subida(*n.velocidade, Dml::DegrauDo(nivel).gravidade);
                CHECK(subida > 30.0f);
                CHECK(subida < 120.0f);
            }
            CHECK(*NoNivel(a, Dificuldade::Nivel::Dificil).projeteis > *NoNivel(a, Dificuldade::Nivel::Normal).projeteis);
        }
        if (f.nome == Transacao::kFaseDaTransacao) {
            // O COMMIT varre linha e coluna, risca, derruba e chuta; o selo e o que o aluno derruba.
            for (const char* g : {Banco::kGatilhoDaSelecao, Banco::kGatilhoDaProjecao, Transacao::kGatilhoDoRascunho,
                                  Transacao::kGatilhoDoSelo, Dml::kGatilhoDoCaco}) {
                CAPTURE(g);
                CHECK(GatilhoExiste(g));
                CHECK(gatilhos.count(g) == 1);
            }
            for (const auto& a : f.ataques) {
                if (a.gatilho == Transacao::kGatilhoDoRascunho) {
                    const auto caras = carasDe(a.projetil);
                    for (int g = 0; g < Transacao::kMaximo; ++g) {
                        CHECK(caras.count(Transacao::CaraDoRascunho(g, false)) == 1);
                        CHECK(caras.count(Transacao::CaraDoRascunho(g, true)) == 1);
                    }
                }
                if (a.gatilho == Transacao::kGatilhoDoSelo) {
                    const auto caras = carasDe(a.projetil);
                    for (int g = 0; g < Transacao::kMaximo; ++g) {
                        for (int id = 1; id <= Transacao::kIds; ++id) {
                            CHECK(caras.count(Transacao::CaraDoSelo(g, id, false)) == 1);
                            CHECK(caras.count(Transacao::CaraDoSelo(g, id, true)) == 1);
                        }
                    }
                    // O selo cai a tiro, e no Dificil aguenta mais.
                    const auto& selo = tipos.at(a.projetil);
                    CHECK(selo.derrubavel);
                    REQUIRE(selo.resistenciaNoDificil.has_value());
                    CHECK(selo.resistencia > 1);
                    CHECK(*selo.resistenciaNoDificil > selo.resistencia);
                }
                if (a.gatilho == Banco::kGatilhoDoDado) {
                    // O valor sujo do UPDATE preparado.
                    const auto caras = carasDe(a.projetil);
                    for (int v = 0; v <= 99; ++v) CHECK(caras.count(Banco::CaraSuja(v)) == 1);
                }
            }
        }
        // A varredura da linha e a mesma em qualquer fase que a use.
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
            // A varredura e a da tabela de Banco.h.
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
    // Se o C++ mudar um estado de lugar e o gerador nao, as setas apontam para o vazio.
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

TEST_CASE("Dados: toda fase de todo chefe dura Fase::kDuracao (o TCC, kDuracaoDoTcc), e as etapas cobrem a fase inteira") {

    // Uma fase de outro tamanho faria um deles acabar antes - ou depois - dela.
    for (const auto& chefe : Chefes()) {
        for (const auto& f : FasesDoChefe(chefe).fases) {
            CAPTURE(chefe);
            CAPTURE(f.nome);
            // O TCC E a excecao declarada (Fase::kDuracaoDoTcc): a batalha maior.
            CHECK(f.duracao == doctest::Approx(chefe == "hugo" ? Fase::kDuracaoDoTcc : Fase::kDuracao));
            if (f.etapas.empty()) continue;
            float soma = 0.0f;
            for (const auto& e : f.etapas) soma += e.duracao;
            CHECK(soma == doctest::Approx(f.duracao));
        }
    }
}

TEST_CASE("Dados: a tempera do Arroyo tem uma etapa por esfriamento, com anuncio") {

    // O placar da tempera escreve na etapa da metade em que a fase esta.
    const DescricaoDeFase* final = nullptr;
    const auto lidas = FasesDoChefe("arroyo");
    for (const auto& f : lidas.fases) if (f.nome == Tempera::kFase) final = &f;
    REQUIRE(final != nullptr);
    REQUIRE(final->etapas.size() == static_cast<size_t>(Tempera::kEtapas));
    for (const auto& e : final->etapas) {
        CHECK(e.duracao == doctest::Approx(Tempera::kReaquecimento));
        CHECK_FALSE(e.anuncio.empty());
    }
    CHECK(Tempera::kTempoDoAnuncio + Tempera::kAntesDoReaquecimento < Tempera::kReaquecimento);
}

TEST_CASE("Dados: a escada tem um degrau por etapa, do tamanho da etapa") {

    // A escada e o diagrama da cascata: um degrau por etapa.
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

TEST_CASE("Dados: a Maria Lucia - cada gatilho que ela puxa esta na fase dela, e cada cara que ela poe existe") {

    // A fase de cada gatilho, como o C++ os puxa (MariaLucia.cpp).
    const std::vector<std::pair<std::string, const char*>> gatilhos = {
        {Cascata::kFaseDaCascata, Cascata::kGatilhoDaPlaca}, {Cascata::kFaseDaCascata, Cascata::kGatilhoDaQueda},
        {Cascata::kFaseDaCascata, Cascata::kGatilhoDoPedido}, {Cascata::kFaseDaCascata, Cascata::kGatilhoDoCusto},
        {Cascata::kFaseDaCascata, Cascata::kGatilhoDoEstouro},
        {Scrum::kFase, Scrum::kGatilhoDaHistoria}, {Scrum::kFase, Scrum::kGatilhoDoCursor},
        {Scrum::kFase, Scrum::kGatilhoDoTitulo}, {Scrum::kFase, Scrum::kGatilhoDoTrabalho},
        {Scrum::kFase, Scrum::kGatilhoDaEntrega}, {Scrum::kFase, Scrum::kGatilhoDaDaily},
        {Observer::kFaseDosPadroes, Observer::kGatilhoDoObservador}, {Observer::kFaseDosPadroes, Observer::kGatilhoDaNotificacao},
        {Observer::kFaseDosPadroes, Observer::kGatilhoDaConsulta}, {Observer::kFaseDosPadroes, Decorator::kGatilhoDaBala},
        {Observer::kFaseDosPadroes, Decorator::kGatilhoDaDivisao},
        {"StateFinal", BalaDePrata::kGatilhoDoLobisomem}, {"StateFinal", BalaDePrata::kGatilhoDoUivo},
        {"StateFinal", BalaDePrata::kGatilhoDaBala}, {"StateFinal", BalaDePrata::kGatilhoDoEstilhaco},
    };
    const auto lidas = FasesDoChefe("marialucia");
    const auto& tipos = ProjeteisDoChefe("marialucia").projeteis;
    auto caras = [&](const std::string& tipo) {
        std::set<std::string> c;
        if (tipos.count(tipo)) for (const auto& a : tipos.at(tipo).animacoes) c.insert(a.nome);
        return c;
    };
    std::map<std::string, std::string> projetilDoGatilho;
    for (const auto& [fase, g] : gatilhos) {
        CAPTURE(fase);
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        int quantos = 0;
        for (const auto& f : lidas.fases) {
            if (f.nome != fase) continue;
            for (const auto& a : f.ataques) if (a.gatilho == g) { ++quantos; projetilDoGatilho[g] = a.projetil; }
        }
        CHECK(quantos == 1);
    }

    // As etapas da cascata sao as de Cascata.h, e cada artefato tem a cara da etapa dele.
    const DescricaoDeFase* cascata = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Cascata::kFaseDaCascata) cascata = &f;
    REQUIRE(cascata != nullptr);
    REQUIRE(cascata->etapas.size() == Cascata::kEtapas.size());
    for (size_t k = 0; k < Cascata::kEtapas.size(); ++k) {
        CHECK(cascata->etapas[k].nome == Cascata::kEtapas[k]);
        CHECK(cascata->etapas[k].duracao == doctest::Approx(Cascata::kDuracaoDaEtapa));
        for (const char* g : {Cascata::kGatilhoDaQueda, Cascata::kGatilhoDoPedido}) {
            CHECK(caras(projetilDoGatilho[g]).count(Cascata::CaraDaEtapa(static_cast<int>(k))) == 1);
        }
        const auto placa = caras(projetilDoGatilho[Cascata::kGatilhoDaPlaca]);
        CHECK(placa.count("P" + std::to_string(k)) == 1);
        CHECK(placa.count("A" + std::to_string(k)) == 1);
    }

    // O Scrum: as sprints sao as etapas, e as caras dos pontos, do cursor e dos titulos existem.
    const DescricaoDeFase* scrum = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Scrum::kFase) scrum = &f;
    REQUIRE(scrum != nullptr);
    REQUIRE(scrum->etapas.size() == static_cast<size_t>(Scrum::kSprints) + 2);
    CHECK(scrum->etapas.front().duracao == doctest::Approx(Scrum::kBacklog));
    for (int k = 1; k <= Scrum::kSprints; ++k) CHECK(scrum->etapas[static_cast<size_t>(k)].duracao == doctest::Approx(Scrum::kSprint));
    for (const int p : Scrum::kBaralho) CHECK(caras(projetilDoGatilho[Scrum::kGatilhoDaHistoria]).count(Scrum::Cara(p)) == 1);
    for (const char* t : Scrum::kTitulos) CHECK(caras(projetilDoGatilho[Scrum::kGatilhoDoTitulo]).count(t) == 1);
    for (const char* c : {"Cursor", "Clique"}) CHECK(caras(projetilDoGatilho[Scrum::kGatilhoDoCursor]).count(c) == 1);

    // Os padroes: o Observer e o Decorator sao as duas etapas, e o componente tem uma cara por camada.
    const DescricaoDeFase* padroes = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Observer::kFaseDosPadroes) padroes = &f;
    REQUIRE(padroes != nullptr);
    REQUIRE(padroes->etapas.size() == 2u);
    CHECK(padroes->etapas[0].duracao == doctest::Approx(Observer::kFimDoObserver));
    for (int c = 0; c <= Decorator::kCamadas; ++c) {
        CHECK(caras(projetilDoGatilho[Decorator::kGatilhoDaBala]).count(Decorator::Cara(c)) == 1);
        CHECK(caras(projetilDoGatilho[Decorator::kGatilhoDaDivisao]).count(Decorator::Cara(c)) == 1);
    }
    for (const char* c : {"Lobo", "Uivo"}) CHECK(caras(projetilDoGatilho[BalaDePrata::kGatilhoDoLobisomem]).count(c) == 1);
}

TEST_CASE("Dados: o fluxograma do Andre - gatilhos, velocidade do confete e caras sao nomes que os dados tem") {

    // O Andre puxa os gatilhos e pinta os projeteis pelos nomes de Condicional.h.
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
            // O confete nasce na velocidade do Andre.
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
            CAPTURE(r);
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

    // O Andre acende o indice da casa por onde o i passa e solta o cacho dela pelos nomes de Laco.h.
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

    // O Andre poe o cartao de cada funcao e o valor que voa pelos nomes de Funcao.h.
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
            // O cacho e o da fase 2, com a velocidade dela; o leque abre o que Funcao.h diz.
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
        CAPTURE(f);
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

    // A repescagem reusa as tres mecanicas.
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

    // O balao do yes: a letra de cada problema, subindo na velocidade de Prova.h.
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

TEST_CASE("Dados: os gatilhos e o posto do Observer sao nomes que a leitura aceita") {

    // Trocar um nome so de um lado faria a leitura recusar o ataque.
    CHECK(GatilhoExiste(Observer::kGatilhoDaNotificacao));
    CHECK(GatilhoExiste(Observer::kGatilhoDaConsulta));
    CHECK(PostoExiste(Observer::kPostoDosObservadores));
}

TEST_CASE("Dados: o olho do Observer tem as caras que a professora poe nele") {

    // Quem anima o olho e a professora, a cada quadro, pelos nomes de Observer.h.
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

    // O observer e o polling sao a mesma fase.
    bool achou = false;
    for (const auto& chefe : Chefes()) {
        for (const auto& f : FasesDoChefe(chefe).fases) {
            bool notifica = false, consulta = false;
            for (const auto& a : f.ataques) {
                if (a.gatilho == Observer::kGatilhoDaNotificacao) notifica = true;
                if (a.gatilho == Observer::kGatilhoDaConsulta) consulta = true;
                // Os avisos saem dos olhos, e as ferramentas dos agentes.
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

    // A frase e uma lista de nomes de animacao, e a TokenAttack os pede um por token.
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

    // Quem troca a cara da nuvem e o Vitor, pelos nomes de Nuvem.h.
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

    const auto lidas = FasesDoChefe("vitor");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Nuvem::kFaseDasNuvens) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(Nuvem::kVoltaDasNuvens + Tcp::kProcessa <= fase->duracao / 2.0f);
}

TEST_CASE("Dados: as nuvens do Vitor ficam presas a ele a fase inteira") {

    // A ancora das nuvens cobre a fase.
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
    // O Vitor so junta as nuvens se a fase delas acaba na do fork.
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

    // Quem troca a cara do bit e da porta e o Ricardo, pelos nomes de Logica.h.
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

    for (const auto& etapa : fase->etapas) {
        CAPTURE(etapa.anuncio);
        const auto porta = Logica::LerPorta(etapa.anuncio, Logica::kBits);
        REQUIRE(porta);

        // E uma porta sem palavra justa faria o sorteio devolver qualquer coisa.
        bool admite = false;
        for (std::uint32_t a = 0; a < (1u << Logica::kBits) && !admite; ++a) admite = Logica::Justa(*porta, a, Logica::kBits);
        CHECK(admite);
    }

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

    // O placar e o clock.
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

    // O leque do contador: uma WaveAttack de gatilho, cujos bits nascem no posto dos registradores.
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

    // Uma instrucao cujo gatilho a fase nao tem chega ao estagio dela e nao faz nada.
    for (const auto& instrucao : Pipeline::Programa()) {
        for (int s = 0; s < Pipeline::kEstagios; ++s) {
            const std::string g = Pipeline::Gatilho(instrucao.tipo, s);
            if (g.empty()) continue;
            CAPTURE(g);
            CHECK(GatilhoExiste(g));
            CHECK(temGatilho(g));
        }
    }

    // A fileira de estagios tem um por estagio, parada e toda enfeite.
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
            CHECK(*a.projeteis == 1);   // A busca traz uma instrucao.
            CHECK(poe(a, Pipeline::kPostoDoPipeline));
        }
    }
    CHECK(fileiras == 1);
    CHECK(buscas == 1);

    // As caras: o nome de cada estagio, cada instrucao do programa e a bolha.
    const auto& tipos = ProjeteisDoChefe("ricardo").projeteis;
    const auto tem = [&](const char* tipo, const std::string& cara) {
        if (tipos.count(tipo) == 0) return false;
        for (const auto& a : tipos.at(tipo).animacoes) if (a.nome == cara) return true;
        return false;
    };
    for (int s = 0; s < Pipeline::kEstagios; ++s) CHECK(tem("Estagio", Pipeline::NomeDoEstagio(s)));
    for (const auto& instrucao : Pipeline::Programa()) CHECK(tem("Instrucao", Pipeline::Animacao(instrucao.tipo)));
    CHECK(tem("Instrucao", Pipeline::kAnimacaoDaBolha));

    // E o aluno faz bolha: a instrucao tem de ser derrubavel.
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

    for (const auto& etapa : fase->etapas) {
        CAPTURE(etapa.anuncio);
        CHECK(Blink::LerDelay(etapa.anuncio));
    }

    // As placas nascem no posto "placas" - e dali que o Ricardo as acende -, e o led sai das acesas.
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

    // Toda combinacao de aprovacoes nas outras materias, com a nota minima de aprovacao.
    std::vector<int> outras;
    for (int i = 0; i < l.Quantas(); ++i) if (i != tcc) outras.push_back(i);
    REQUIRE(outras.size() < 20);
    int abriram = 0;
    for (unsigned conjunto = 0; conjunto < (1u << outras.size()); ++conjunto) {
        Progresso p;
        for (size_t k = 0; k < outras.size(); ++k) {
            if (conjunto & (1u << k)) p.RegistrarNota(outras[k], Progresso::kNotaDeAprovacao, Dificuldade::Nivel::Normal);
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

// A documentacao nao esquece nome nenhum. Os gatilhos e os postos sao listas fechadas: o leitor
// recusa um nome que nao conhece.

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
    const auto linhas = LinhasDaTabela("posto");
    REQUIRE(linhas.size() == 2);
    for (const std::string& linha : linhas) {
        for (const std::string& nome : NomesDosPostos()) {
            CAPTURE(nome);
            CHECK(linha.find("`" + nome + "`") != std::string::npos);
        }
    }
}

TEST_CASE("Dados: as amarracoes do Luizinho - o gatilho da resposta, a velocidade e uma cara por valor de x") {
    CHECK(GatilhoExiste(Amarracao::kGatilhoDaResposta));

    const auto lidas = FasesDoChefe("luizinho");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Amarracao::kFaseDasAmarracoes) fase = &f;
    REQUIRE(fase != nullptr);
    CHECK(fase->movimento.tipo == "Conduzido");

    std::string tipo;
    for (const auto& a : fase->ataques) {
        // A fase e toda de gatilho: quem responde e o bloco que a busca achou.
        CHECK(!a.gatilho.empty());
        if (a.gatilho != Amarracao::kGatilhoDaResposta) continue;
        tipo = a.projetil;
        REQUIRE(a.velocidade.has_value());
        CHECK(*a.velocidade == doctest::Approx(Amarracao::kVelocidade));
    }
    const auto& tipos = ProjeteisDoChefe("luizinho").projeteis;
    REQUIRE(tipos.count(tipo) == 1);
    std::set<std::string> caras;
    for (const auto& a : tipos.at(tipo).animacoes) caras.insert(a.nome);
    for (int v = Amarracao::kMenorValor; v <= Amarracao::kMaiorValor; ++v) CHECK(caras.count("X" + std::to_string(v)) == 1);
}

TEST_CASE("Dados: a coercao do Luizinho - gatilhos, velocidades e caras sao os de Coercao.h") {

    // O Luizinho puxa os gatilhos e pinta cada chip pelo nome de Coercao.h.
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

TEST_CASE("Dados: o Termina do Luizinho - os gatilhos da volta e da contradicao, e a velocidade da espiral") {
    const auto lidas = FasesDoChefe("luizinho");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Termina::kFaseDoTermina) fase = &f;
    REQUIRE(fase != nullptr);

    std::map<std::string, std::string> projetilDe;
    for (const auto& a : fase->ataques) {
        CHECK(!a.gatilho.empty());
        projetilDe[a.gatilho] = a.projetil;
        if (a.gatilho == Termina::kGatilhoDaVolta) {
            // A velocidade do arquivo e a que a camada pura usa.
            REQUIRE(a.velocidade.has_value());
            CHECK(*a.velocidade == doctest::Approx(Termina::kVelocidade));
        }
        // Os cacos apontam para onde voam, pela regra.
        if (a.gatilho == Termina::kGatilhoDaContradicao) CHECK(a.regrasNome == "contradicao");
    }
    const auto& tipos = ProjeteisDoChefe("luizinho").projeteis;
    for (const char* g : {Termina::kGatilhoDaVolta, Termina::kGatilhoDaContradicao}) {
        CAPTURE(g);
        CHECK(GatilhoExiste(g));
        REQUIRE(projetilDe.count(g) == 1);
        CHECK(tipos.count(projetilDe[g]) == 1);
    }
}

TEST_CASE("Dados: a pilha de chamadas do Luizinho - gatilhos, velocidades e caras sao os de Excecao.h") {

    // O Luizinho puxa os gatilhos e pinta a excecao com o nome do tipo.
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

    // O Vitor puxa o gatilho e pinta cada instrucao com o nome da coluna.
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
        // A WaveAttack escreve a velocidade do arquivo no primeiro quadro.
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

    // As correntes continuam na fase: um ataque de relogio de um projetil que acorrenta.
    bool corrente = false;
    for (const auto& a : fase->ataques) {
        if (!a.gatilho.empty() || tipos.count(a.projetil) == 0) continue;
        corrente = corrente || tipos.at(a.projetil).acorrenta.segundos > 0.0f;
    }
    CHECK(corrente);
}

TEST_CASE("Dados: o TCP do Vitor - gatilhos, velocidades e caras sao os de Tcp.h") {

    // A fase do TCP E a das nuvens: sao elas que mandam a janela.
    CHECK(std::string(Tcp::kFaseDoTcp) == Nuvem::kFaseDasNuvens);
    const auto lidas = FasesDoChefe("vitor");
    const DescricaoDeFase* fase = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == Tcp::kFaseDoTcp) fase = &f;
    REQUIRE(fase != nullptr);

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
    // Todo ataque da fase e de gatilho: o relogio dela e o RTT.
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

    // A garfada sai na velocidade do arquivo.
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
    // Todo ataque e de gatilho - e e por isso que o clone da fase 3 pode emprestar esta fase.
    for (const auto& a : fase->ataques) CHECK(!a.gatilho.empty());
    for (const auto& f : lidas.fases) {
        if (f.clone.existe) CHECK(f.clone.ataquesDe == Filosofos::kFaseDoJantar);
    }
}

TEST_CASE("Dados: a troca de mensagens do Vitor - gatilhos, velocidades e caras sao os de Mensagens.h") {

    // A fase das mensagens e a do fork: os dois processos sao os dois corpos.
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

    // O resultado da rpc e um leque com o do meio no aluno: as somas sao impares.
    for (int i = 0; i < Mensagens::kChamadas; ++i) CHECK(Mensagens::Soma(Mensagens::ChamadaDaVez(i)) % 2 == 1);
}
