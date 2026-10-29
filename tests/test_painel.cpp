// O painel de controle do gabinete. Camada pura.
//
// O que estes testes protegem nao e a aparencia do rodape: e o ACORDO entre o
// que a tela anuncia e o que o jogo escuta. A selecao de fases escrevia "T
// trocar usuario" e escutava o T - num gabinete que nao tem tecla T. A acao era
// impossivel, e o rodape ensinava a fazer o impossivel.
//
// Por isso os testes insistem em tres coisas: o rodape sai do BOTAO, dois
// controles nunca compartilham tecla, e o nome da tecla que aparece escrito no
// computador e o nome da tecla que o jogo le.
//
// COM AS DUAS POSICOES, a conferencia de teclas repetidas passou a valer para o
// painel INTEIRO, e nao para os tres botoes de um lado. E ela que impede o
// manche de um jogador de ser o botao do outro - era exatamente esse o estado
// anterior, com o W A S D servindo de atalho para as setas do jogador 1.

#include "doctest.h"

#include "../Source/Input/Painel.h"

#include <string>
#include <vector>

namespace {

/// Um teclado zerado do tamanho que a SDL entrega, com uma tecla apertada.
std::vector<Uint8> TecladoCom(const SDL_Scancode tecla) {
    std::vector<Uint8> teclado(SDL_NUM_SCANCODES, 0);
    teclado[tecla] = 1;
    return teclado;
}

/// TODAS AS TECLAS DO PAINEL, as duas posicoes juntas: quatro do manche e tres
/// botoes de cada lado. As listas vem de Painel.h de proposito - um kTodos
/// local aqui seria uma segunda copia dos enumeradores, e foi assim que a lista
/// de chefes do teste de dados ficou sem o Thiago.
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

    // ESTE E O TESTE DO CO-OP. Uma tecla repetida entre posicoes faria os dois
    // jogadores andarem juntos, ou o tiro de um disparar o do outro.
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

    // Eram a mesma coisa, e por isso a segunda posicao do gabinete nao tinha
    // manche. O rodape ja dizia "SETAS" enquanto o jogo tambem aceitava W A S D.
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

    // O gabinete tem os botoes 1, 2 e 3 dos dois lados. Quem precisa dizer de
    // quem e o botao diz pela posicao, nao por um nome diferente.
    CHECK(std::string(Painel::Nome(Painel::Botao::Um))   == "BOTAO 1");
    CHECK(std::string(Painel::Nome(Painel::Botao::Dois)) == "BOTAO 2");
    CHECK(std::string(Painel::Nome(Painel::Botao::Tres)) == "BOTAO 3");
}

TEST_CASE("Painel: o nome da tecla nao e vazio nem repetido, em nenhuma posicao") {

    // Ele aparece escrito na tela no computador. Dois controles com o mesmo
    // nome de tecla mandariam quem esta testando apertar a tecla errada.
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

    // Varre o painel inteiro: para cada tecla, um teclado com ela apertada tem
    // de acender UM controle. Com a tabela anterior, apertar o W acendia o
    // CIMA do jogador 1 - que e justamente o que nao pode mais acontecer.
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

    // A cena recebe o ponteiro da SDL e o repassa. Se ele chegar nulo, o jogo
    // nao pode cair - e muito menos agir como se tudo estivesse apertado.
    for (const Painel::Jogador quem : Painel::kJogadores) {
        for (const Painel::Botao b : Painel::kBotoes)     CHECK(Painel::Apertado(nullptr, quem, b) == false);
        for (const Painel::Direcao d : Painel::kDirecoes) CHECK(Painel::Apertado(nullptr, quem, d) == false);
    }
}

TEST_CASE("Painel: o rodape anuncia o NOME do botao que a cena escuta") {

    // ESTE E O TESTE QUE IMPORTA. O item do rodape e construido a partir do
    // botao, entao nao existe caminho para escrever um nome e escutar outro.
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

    // Quem esta testando no teclado nao tem como saber qual e o botao 2. E a
    // tecla escrita tem de ser a MESMA que Apertado le - se fossem duas fontes,
    // o rodape mandaria apertar uma tecla que nao faz nada.
    //
    // Com duas posicoes ha um segundo jeito de errar: anunciar a tecla do
    // jogador 1 num rodape que o jogador 2 esta lendo.
    for (const Painel::Jogador quem : Painel::kJogadores) {
        for (const Painel::Botao b : Painel::kBotoes) {
            const std::string linha = Painel::Rodape({{quem, b, "jogar"}}, Painel::Jeito::ComATecla);

            CHECK(linha.find(Painel::Nome(b)) != std::string::npos);
            CHECK(linha.find(std::string("(") + Painel::NomeDaTecla(quem, b) + ")") != std::string::npos);

            const std::vector<Uint8> teclado = TecladoCom(Painel::Tecla(quem, b));
            CHECK(Painel::Apertado(teclado.data(), quem, b));

            // E a tecla do OUTRO lado nao aparece nesta linha.
            const auto outro = quem == Painel::Jogador::Um ? Painel::Jogador::Dois : Painel::Jogador::Um;
            CHECK(linha.find(std::string("(") + Painel::NomeDaTecla(outro, b) + ")") == std::string::npos);
        }
    }
}

TEST_CASE("Painel: no gabinete a tecla NAO aparece") {

    // La nao ha teclado. Dizer "ESPACO" para quem tem um botao na mao e ruido.
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

    // E o manche do jogador 2 anuncia o manche DELE.
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

    // AMARRA AS DUAS COLUNAS DA TABELA. O nome da tecla e escrito na tela e o
    // scancode e o que o jogo le; sem esta conferencia, trocar o botao 2 de B
    // para C e esquecer o nome faria a tela mandar apertar a tecla errada - e
    // nada reclamaria, porque os dois campos continuariam preenchidos.
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

    // "SETAS" e "WASD" sao nomes de CONJUNTO, e nao de uma tecla; o que o teste
    // pode conferir e que o nome do jogador 2 lista as quatro letras que ele le.
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
