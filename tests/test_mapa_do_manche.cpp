// O painel quando o encoder do gabinete e joystick.

#include "doctest.h"

#include "../Source/Entrada/MapaDoManche.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

/// Um joystick de encoder comum: dois eixos, um direcional e dez botoes, tudo solto.
MapaDoManche::Leitura Solto() {
    MapaDoManche::Leitura l;
    l.eixos = {0, 0};
    l.hats = {0};
    l.botoes.assign(10, 0);
    return l;
}

/// As teclas que a leitura aperta para 'quem', num teclado zerado.
std::vector<Uint8> Teclado(const MapaDoManche::Leitura& l, const MapaDoManche::Posicao& p, const Painel::Jogador quem) {
    std::vector<Uint8> teclado(SDL_NUM_SCANCODES, 0);
    MapaDoManche::Apertar(l, p, 0.5f, quem, teclado.data(), static_cast<int>(teclado.size()));
    return teclado;
}

int Apertadas(const std::vector<Uint8>& teclado) {
    int n = 0;
    for (const Uint8 t : teclado) n += t != 0;
    return n;
}

} // namespace

TEST_CASE("MapaDoManche: o padrao e um joystick por posicao") {
    const MapaDoManche::Configuracao c = MapaDoManche::Padrao();
    CHECK(c.posicoes[0].dispositivo == 0);
    CHECK(c.posicoes[1].dispositivo == 1);
    CHECK(c.posicoes[0].botoes == std::array<int, 3>{0, 1, 2});
    CHECK(c.zonaMorta == doctest::Approx(0.5f));
}

TEST_CASE("MapaDoManche: solto, nada e apertado") {
    const MapaDoManche::Posicao p;
    CHECK(Apertadas(Teclado(Solto(), p, Painel::Jogador::Um)) == 0);
}

TEST_CASE("MapaDoManche: o eixo empurra o manche so depois da zona morta") {
    const MapaDoManche::Posicao p;
    MapaDoManche::Leitura l = Solto();
    l.eixos[0] = 10000;   // menos da metade do curso
    CHECK_FALSE(MapaDoManche::Empurrado(l, p, 0.5f, Painel::Direcao::Direita));
    l.eixos[0] = 32767;
    CHECK(MapaDoManche::Empurrado(l, p, 0.5f, Painel::Direcao::Direita));
    CHECK_FALSE(MapaDoManche::Empurrado(l, p, 0.5f, Painel::Direcao::Esquerda));
    l.eixos[1] = -32768;  // para cima, como o SDL manda
    CHECK(MapaDoManche::Empurrado(l, p, 0.5f, Painel::Direcao::Cima));

    const std::vector<Uint8> t = Teclado(l, p, Painel::Jogador::Um);
    CHECK(t[Painel::Tecla(Painel::Jogador::Um, Painel::Direcao::Direita)] == 1);
    CHECK(t[Painel::Tecla(Painel::Jogador::Um, Painel::Direcao::Cima)] == 1);
    CHECK(Apertadas(t) == 2);
}

TEST_CASE("MapaDoManche: inverter troca o lado do eixo") {
    MapaDoManche::Posicao p;
    p.inverterY = true;
    MapaDoManche::Leitura l = Solto();
    l.eixos[1] = 32767;
    CHECK(MapaDoManche::Empurrado(l, p, 0.5f, Painel::Direcao::Cima));
    CHECK_FALSE(MapaDoManche::Empurrado(l, p, 0.5f, Painel::Direcao::Baixo));
}

TEST_CASE("MapaDoManche: o direcional digital tambem move, e na diagonal aperta os dois") {
    const MapaDoManche::Posicao p;
    MapaDoManche::Leitura l = Solto();
    l.hats[0] = 0x04 | 0x08;   // baixo e esquerda
    const std::vector<Uint8> t = Teclado(l, p, Painel::Jogador::Dois);
    CHECK(t[Painel::Tecla(Painel::Jogador::Dois, Painel::Direcao::Baixo)] == 1);
    CHECK(t[Painel::Tecla(Painel::Jogador::Dois, Painel::Direcao::Esquerda)] == 1);
    CHECK(Apertadas(t) == 2);
}

TEST_CASE("MapaDoManche: os botoes viram os botoes do painel, de quem e a posicao") {
    MapaDoManche::Posicao p;
    p.botoes = {4, 5, 6};
    MapaDoManche::Leitura l = Solto();
    l.botoes[5] = 1;
    CHECK(MapaDoManche::Apertado(l, p, Painel::Botao::Dois));
    CHECK_FALSE(MapaDoManche::Apertado(l, p, Painel::Botao::Um));
    const std::vector<Uint8> t = Teclado(l, p, Painel::Jogador::Dois);
    CHECK(t[Painel::Tecla(Painel::Jogador::Dois, Painel::Botao::Dois)] == 1);
    CHECK(t[Painel::Tecla(Painel::Jogador::Um, Painel::Botao::Dois)] == 0);
}

TEST_CASE("MapaDoManche: o manche que chega como botoes") {
    MapaDoManche::Posicao p;
    p.eixoX = p.eixoY = p.hat = MapaDoManche::kNenhum;
    p.direcoes = {6, 7, 8, 9};
    MapaDoManche::Leitura l = Solto();
    l.botoes[8] = 1;
    CHECK(MapaDoManche::Empurrado(l, p, 0.5f, Painel::Direcao::Esquerda));
    CHECK_FALSE(MapaDoManche::Empurrado(l, p, 0.5f, Painel::Direcao::Cima));
}

TEST_CASE("MapaDoManche: o que o joystick nao tem nao aperta nada") {
    MapaDoManche::Posicao p;
    p.eixoX = 7;
    p.hat = 3;
    p.botoes = {20, 21, 22};
    MapaDoManche::Leitura l;   // sem eixo, sem direcional, sem botao
    CHECK(Apertadas(Teclado(l, p, Painel::Jogador::Um)) == 0);
}

TEST_CASE("MapaDoManche: o joystick nunca solta o teclado de verdade") {
    std::vector<Uint8> teclado(SDL_NUM_SCANCODES, 0);
    teclado[SDL_SCANCODE_RETURN] = 1;
    MapaDoManche::Apertar(Solto(), MapaDoManche::Posicao{}, 0.5f, Painel::Jogador::Um, teclado.data(),
                          static_cast<int>(teclado.size()));
    CHECK(teclado[SDL_SCANCODE_RETURN] == 1);
}

TEST_CASE("MapaDoManche: ler o arquivo") {
    std::vector<std::string> problemas;
    const MapaDoManche::Configuracao c = MapaDoManche::Ler(R"({
        // comentario, como nos outros arquivos de dados
        "zonaMorta": 0.3,
        "jogadores": [
            {"dispositivo": 0, "eixoX": 2, "inverterY": true, "botoes": [3, 4, 5]},
            {"dispositivo": 0, "eixoX": 3, "eixoY": 4, "hat": -1, "direcoes": [10, 11, 12, 13]}
        ]
    })", &problemas);
    CHECK(problemas.empty());
    CHECK(c.zonaMorta == doctest::Approx(0.3f));
    CHECK(c.posicoes[0].eixoX == 2);
    CHECK(c.posicoes[0].eixoY == 1);          // o que falta fica no padrao
    CHECK(c.posicoes[0].inverterY);
    CHECK(c.posicoes[0].botoes == std::array<int, 3>{3, 4, 5});
    CHECK(c.posicoes[1].dispositivo == 0);    // um encoder so para as duas posicoes
    CHECK(c.posicoes[1].hat == MapaDoManche::kNenhum);
    CHECK(c.posicoes[1].direcoes == std::array<int, 4>{10, 11, 12, 13});
}

TEST_CASE("MapaDoManche: o arquivo com erro nao derruba nada") {
    std::vector<std::string> problemas;
    MapaDoManche::Configuracao c = MapaDoManche::Ler("{ isto nao e json", &problemas);
    CHECK(problemas.size() == 1);
    CHECK(c.posicoes[1].dispositivo == 1);

    problemas.clear();
    c = MapaDoManche::Ler(R"({"zonaMorta": 2, "jogadores": [{"botoes": [1, 2], "eixoX": "x"}]})", &problemas);
    CHECK(problemas.size() == 3);
    CHECK(c.zonaMorta == doctest::Approx(0.5f));
    CHECK(c.posicoes[0].botoes == std::array<int, 3>{0, 1, 2});
    CHECK(c.posicoes[0].eixoX == 0);
}

TEST_CASE("MapaDoManche: o Assets/painel.json do jogo se le sem problema e e o padrao") {
    std::ifstream arquivo(std::string(DPI_ASSETS_DIR) + "/painel.json");
    REQUIRE(arquivo.good());
    std::stringstream texto;
    texto << arquivo.rdbuf();
    std::vector<std::string> problemas;
    const MapaDoManche::Configuracao c = MapaDoManche::Ler(texto.str(), &problemas);
    CHECK(problemas.empty());
    const MapaDoManche::Configuracao padrao = MapaDoManche::Padrao();
    for (size_t i = 0; i < 2; ++i) {
        CHECK(c.posicoes[i].dispositivo == padrao.posicoes[i].dispositivo);
        CHECK(c.posicoes[i].botoes == padrao.posicoes[i].botoes);
        CHECK(c.posicoes[i].eixoX == padrao.posicoes[i].eixoX);
        CHECK(c.posicoes[i].eixoY == padrao.posicoes[i].eixoY);
        CHECK(c.posicoes[i].hat == padrao.posicoes[i].hat);
        CHECK(c.posicoes[i].direcoes == padrao.posicoes[i].direcoes);
    }
}
