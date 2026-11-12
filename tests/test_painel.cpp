// O painel de controle do gabinete.

#include "doctest.h"

#include "../Source/Entrada/Painel.h"

#include <string>
#include <vector>

namespace {

/// Um teclado zerado do tamanho que a SDL entrega, com uma tecla apertada.
std::vector<Uint8> TecladoCom(const SDL_Scancode tecla) {
    std::vector<Uint8> teclado(SDL_NUM_SCANCODES, 0);
    teclado[tecla] = 1;
    return teclado;
}

/// Todas as teclas do painel, as duas posicoes juntas: quatro do manche e tres botoes de cada lado.
std::vector<SDL_Scancode> TodasAsTeclas() {
    std::vector<SDL_Scancode> teclas;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        for (const Painel::Direcao d : Painel::kDirecoes) teclas.push_back(Painel::Tecla(quem, d));
        for (const Painel::Botao b : Painel::kBotoes)     teclas.push_back(Painel::Tecla(quem, b));
    }
    return teclas;
}

} // namespace

TEST_CASE("Painel: o gabinete tem duas posicoes, e nenhuma tecla serve as duas") {

    // Este e o teste do co-op.
    const std::vector<SDL_Scancode> teclas = TodasAsTeclas();

    CHECK(teclas.size() == 14);   // (4 do manche + 3 botoes) x 2 posicoes

    for (size_t i = 0; i < teclas.size(); ++i) {
        CHECK(teclas[i] != SDL_SCANCODE_UNKNOWN);
        for (size_t j = i + 1; j < teclas.size(); ++j) {
            CHECK(teclas[i] != teclas[j]);
        }
    }
}

TEST_CASE("Painel: as setas sao do jogador 1 e o W A S D e do jogador 2") {

    // O rodape ja dizia "SETAS" enquanto o jogo tambem aceitava W A S D.
    CHECK(Painel::Tecla(Painel::Jogador::Um, Painel::Direcao::Cima)     == SDL_SCANCODE_UP);
    CHECK(Painel::Tecla(Painel::Jogador::Um, Painel::Direcao::Baixo)    == SDL_SCANCODE_DOWN);
    CHECK(Painel::Tecla(Painel::Jogador::Um, Painel::Direcao::Esquerda) == SDL_SCANCODE_LEFT);
    CHECK(Painel::Tecla(Painel::Jogador::Um, Painel::Direcao::Direita)  == SDL_SCANCODE_RIGHT);

    CHECK(Painel::Tecla(Painel::Jogador::Dois, Painel::Direcao::Cima)     == SDL_SCANCODE_W);
    CHECK(Painel::Tecla(Painel::Jogador::Dois, Painel::Direcao::Baixo)    == SDL_SCANCODE_S);
    CHECK(Painel::Tecla(Painel::Jogador::Dois, Painel::Direcao::Esquerda) == SDL_SCANCODE_A);
    CHECK(Painel::Tecla(Painel::Jogador::Dois, Painel::Direcao::Direita)  == SDL_SCANCODE_D);
}

TEST_CASE("Painel: cada botao tem um nome so dele, e o nome NAO depende da posicao") {

    // O gabinete tem os botoes 1, 2 e 3 dos dois lados.
    CHECK(std::string(Painel::Nome(Painel::Botao::Um))   == "BOTAO 1");
    CHECK(std::string(Painel::Nome(Painel::Botao::Dois)) == "BOTAO 2");
    CHECK(std::string(Painel::Nome(Painel::Botao::Tres)) == "BOTAO 3");
}

TEST_CASE("Painel: o nome da tecla nao e vazio nem repetido, em nenhuma posicao") {

    // Ele aparece escrito na tela no computador.
    std::vector<std::string> nomes;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        nomes.emplace_back(Painel::NomeDoManche(quem));
        for (const Painel::Botao b : Painel::kBotoes) nomes.emplace_back(Painel::NomeDaTecla(quem, b));
    }

    for (size_t i = 0; i < nomes.size(); ++i) {
        CHECK(nomes[i].empty() == false);
        for (size_t j = i + 1; j < nomes.size(); ++j) {
            CHECK(nomes[i] != nomes[j]);
        }
    }
}

TEST_CASE("Painel: Apertado responde pela tecla daquele controle, e so por ela") {

    // Com a tabela anterior, apertar o W acendia o cima do jogador 1.
    for (const Painel::Jogador dono : Painel::kJogadores) {
        for (const Painel::Direcao mandada : Painel::kDirecoes) {
            const std::vector<Uint8> teclado = TecladoCom(Painel::Tecla(dono, mandada));

            for (const Painel::Jogador quem : Painel::kJogadores) {
                for (const Painel::Direcao d : Painel::kDirecoes) {
                    CHECK(Painel::Apertado(teclado.data(), quem, d)
                          == (quem == dono && d == mandada));
                }
                for (const Painel::Botao b : Painel::kBotoes) {
                    CHECK(Painel::Apertado(teclado.data(), quem, b) == false);
                }
            }
        }

        for (const Painel::Botao apertado : Painel::kBotoes) {
            const std::vector<Uint8> teclado = TecladoCom(Painel::Tecla(dono, apertado));

            for (const Painel::Jogador quem : Painel::kJogadores) {
                for (const Painel::Botao b : Painel::kBotoes) {
                    CHECK(Painel::Apertado(teclado.data(), quem, b)
                          == (quem == dono && b == apertado));
                }
                for (const Painel::Direcao d : Painel::kDirecoes) {
                    CHECK(Painel::Apertado(teclado.data(), quem, d) == false);
                }
            }
        }
    }
}

TEST_CASE("Painel: teclado ausente nao e botao nem manche apertado") {

    // A cena recebe o ponteiro da SDL e o repassa.
    for (const Painel::Jogador quem : Painel::kJogadores) {
        for (const Painel::Botao b : Painel::kBotoes)     CHECK(Painel::Apertado(nullptr, quem, b) == false);
        for (const Painel::Direcao d : Painel::kDirecoes) CHECK(Painel::Apertado(nullptr, quem, d) == false);
    }
}

TEST_CASE("Painel: o rodape anuncia o NOME do botao que a cena escuta") {

    // Este e o teste que importa.
    for (const Painel::Botao b : Painel::kBotoes) {
        const std::string linha = Painel::Rodape({{Painel::Jogador::Um, b, "fazer alguma coisa"}},
                                                 Painel::Jeito::SoOPainel);

        CHECK(linha.find(Painel::Nome(b)) != std::string::npos);
        CHECK(linha.find("fazer alguma coisa") != std::string::npos);

        // E nao anuncia os outros.
        for (const Painel::Botao outro : Painel::kBotoes) {
            if (outro == b) continue;
            CHECK(linha.find(Painel::Nome(outro)) == std::string::npos);
        }
    }
}

TEST_CASE("Painel: no computador o rodape diz TAMBEM a tecla, e e a tecla DAQUELA posicao") {

    // Quem esta testando no teclado nao tem como saber qual e o botao 2.
    for (const Painel::Jogador quem : Painel::kJogadores) {
        for (const Painel::Botao b : Painel::kBotoes) {
            const std::string linha = Painel::Rodape({{quem, b, "jogar"}}, Painel::Jeito::ComATecla);

            CHECK(linha.find(Painel::Nome(b)) != std::string::npos);
            CHECK(linha.find(std::string("(") + Painel::NomeDaTecla(quem, b) + ")") != std::string::npos);

            const std::vector<Uint8> teclado = TecladoCom(Painel::Tecla(quem, b));
            CHECK(Painel::Apertado(teclado.data(), quem, b));

            // E a tecla do outro lado nao aparece nesta linha.
            const auto outro = quem == Painel::Jogador::Um ? Painel::Jogador::Dois : Painel::Jogador::Um;
            CHECK(linha.find(std::string("(") + Painel::NomeDaTecla(outro, b) + ")") == std::string::npos);
        }
    }
}

TEST_CASE("Painel: no gabinete a tecla NAO aparece") {

    // La nao ha teclado.
    const std::string linha = Painel::Rodape({{Painel::Jogador::Um, Painel::Botao::Um, "jogar"}},
                                             Painel::Jeito::SoOPainel);

    CHECK(linha.find("ESPACO") == std::string::npos);
    CHECK(linha.find("(") == std::string::npos);
    CHECK(linha == std::string("BOTAO 1") + Painel::kEntreNomeEAcao + "jogar");
}

TEST_CASE("Painel: o rodape junta os itens com o mesmo espacamento") {

    const std::string linha = Painel::Rodape({{Painel::Jogador::Um, Painel::Botao::Um, "jogar"},
                                              {Painel::Jogador::Um, Painel::Botao::Dois, "voltar"}},
                                             Painel::Jeito::SoOPainel);

    CHECK(linha == std::string("BOTAO 1") + Painel::kEntreNomeEAcao + "jogar"
                 + Painel::kEntreAcoes
                 + "BOTAO 2" + Painel::kEntreNomeEAcao + "voltar");
}

TEST_CASE("Painel: o manche entra no rodape sem ser botao, e tem tecla propria") {

    const std::string semTecla = Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher"),
                                                 {Painel::Jogador::Um, Painel::Botao::Um, "digitar"}},
                                                Painel::Jeito::SoOPainel);
    CHECK(semTecla.find("MANCHE") == 0);
    CHECK(semTecla.find("BOTAO 1") != std::string::npos);
    CHECK(semTecla.find("SETAS") == std::string::npos);

    const std::string comTecla = Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher")},
                                                Painel::Jeito::ComATecla);
    CHECK(comTecla.find("(SETAS)") != std::string::npos);

    // E o manche do jogador 2 anuncia o manche dele.
    const std::string doDois = Painel::Rodape({Painel::Manche(Painel::Jogador::Dois, "escolher")},
                                              Painel::Jeito::ComATecla);
    CHECK(doDois.find("(WASD)") != std::string::npos);
    CHECK(doDois.find("SETAS") == std::string::npos);
}

TEST_CASE("Painel: rodape vazio e linha vazia, e um item so nao leva separador") {

    CHECK(Painel::Rodape({}, Painel::Jeito::SoOPainel).empty());
    CHECK(Painel::Rodape({}, Painel::Jeito::ComATecla).empty());

    const std::string um = Painel::Rodape({{Painel::Jogador::Um, Painel::Botao::Tres, "ranking"}},
                                          Painel::Jeito::SoOPainel);
    CHECK(um == std::string("BOTAO 3") + Painel::kEntreNomeEAcao + "ranking");
    CHECK(um.find(Painel::kEntreAcoes) == std::string::npos);
}

TEST_CASE("Painel: tecla de letra se chama pela propria letra") {

    // Amarra as duas colunas da tabela.
    for (const Painel::Jogador quem : Painel::kJogadores) {
        for (const Painel::Botao b : Painel::kBotoes) {
            const SDL_Scancode tecla = Painel::Tecla(quem, b);
            if (tecla < SDL_SCANCODE_A || tecla > SDL_SCANCODE_Z) continue;

            const std::string esperado(1, static_cast<char>('A' + (tecla - SDL_SCANCODE_A)));
            CHECK(std::string(Painel::NomeDaTecla(quem, b)) == esperado);
        }
    }
}

TEST_CASE("Painel: o nome do manche diz as teclas dele") {

    // "SETAS" e "WASD" sao nomes de conjunto, e nao de uma tecla.
    CHECK(std::string(Painel::NomeDoManche(Painel::Jogador::Um)) == "SETAS");

    const std::string dois = Painel::NomeDoManche(Painel::Jogador::Dois);
    for (const Painel::Direcao d : Painel::kDirecoes) {
        const SDL_Scancode tecla = Painel::Tecla(Painel::Jogador::Dois, d);
        REQUIRE(tecla >= SDL_SCANCODE_A);
        REQUIRE(tecla <= SDL_SCANCODE_Z);

        const char letra = static_cast<char>('A' + (tecla - SDL_SCANCODE_A));
        CHECK(dois.find(letra) != std::string::npos);
    }
}
