//
// O modo de dificuldade e o arquivo de opcoes. Camadas puras.
//

#include "doctest.h"

#include <set>
#include <string>

#include "../Source/Dificuldade.h"
#include "../Source/Configuracao.h"

using Dificuldade::Nivel;

TEST_CASE("Dificuldade: cada nivel tem nome, descricao e uma chave que volta a ele") {
    std::set<std::string> chaves, nomes;
    for (const Nivel n : Dificuldade::kNiveis) {
        CAPTURE(static_cast<int>(n));
        CHECK(std::string(Dificuldade::Nome(n)).size() > 0);
        CHECK(std::string(Dificuldade::Descricao(n)).size() > 0);
        chaves.insert(Dificuldade::Chave(n));
        nomes.insert(Dificuldade::Nome(n));

        Nivel lido = (n == Nivel::Normal) ? Nivel::Dificil : Nivel::Normal;
        REQUIRE(Dificuldade::DeChave(Dificuldade::Chave(n), lido));
        CHECK(lido == n);

        // A CHAVE E ASCII: o arquivo pode ser editado em qualquer editor.
        for (const char c : std::string(Dificuldade::Chave(n))) CHECK(static_cast<unsigned char>(c) < 128);
    }
    CHECK(chaves.size() == static_cast<size_t>(Dificuldade::kQuantos));
    CHECK(nomes.size() == static_cast<size_t>(Dificuldade::kQuantos));
    CHECK(std::string(Dificuldade::Nome(Nivel::Dificil)) == "DIF\xc3\x8d" "CIL");
}

TEST_CASE("Dificuldade: chave desconhecida nao mexe no nivel") {
    Nivel n = Nivel::Dificil;
    CHECK_FALSE(Dificuldade::DeChave("dif\xc3\xad" "cil", n));   // com acento nao e a chave
    CHECK_FALSE(Dificuldade::DeChave("DIFICIL", n));
    CHECK_FALSE(Dificuldade::DeChave("", n));
    CHECK(n == Nivel::Dificil);
}

TEST_CASE("Dificuldade: andar para qualquer lado troca, e da a volta") {
    CHECK(Dificuldade::Mover(Nivel::Normal, 1) == Nivel::Dificil);
    CHECK(Dificuldade::Mover(Nivel::Normal, -1) == Nivel::Dificil);
    CHECK(Dificuldade::Mover(Nivel::Dificil, 1) == Nivel::Normal);
    CHECK(Dificuldade::Mover(Nivel::Dificil, -1) == Nivel::Normal);
    CHECK(Dificuldade::Mover(Nivel::Dificil, -3) == Nivel::Normal);
    CHECK(Dificuldade::Mover(Nivel::Normal, 2) == Nivel::Normal);
    CHECK(Dificuldade::Mover(Nivel::Normal, 0) == Nivel::Normal);
}

TEST_CASE("Dificuldade: Escolher devolve o valor do nivel") {
    CHECK(Dificuldade::Escolher(Nivel::Normal, 10, 13) == 10);
    CHECK(Dificuldade::Escolher(Nivel::Dificil, 10, 13) == 13);
    CHECK(Dificuldade::Escolher(Nivel::Dificil, 0.4f, 0.3f) == doctest::Approx(0.3f));
}

TEST_CASE("Configuracao: o padrao e o Normal, e o arquivo vai e volta") {
    CHECK(Configuracao::Dados{}.dificuldade == Nivel::Normal);
    for (const Nivel n : Dificuldade::kNiveis) {
        Configuracao::Dados d;
        d.dificuldade = n;
        const auto lida = Configuracao::Ler(Configuracao::Serializar(d));
        CHECK(lida.problemas.empty());
        CHECK(lida.dados.dificuldade == n);
    }
}

TEST_CASE("Configuracao: arquivo vazio, comentario e linha em branco dao o padrao, sem problema") {
    for (const char* texto : {"", "\n\n", "# so comentario\n", "   \r\n\t\r\n"}) {
        CAPTURE(texto);
        const auto lida = Configuracao::Ler(texto);
        CHECK(lida.problemas.empty());
        CHECK(lida.dados.dificuldade == Nivel::Normal);
    }
}

TEST_CASE("Configuracao: aceita espaco em volta e fim de linha do Windows") {
    const auto lida = Configuracao::Ler("# x\r\n  dificuldade =  dificil  \r\n");
    CHECK(lida.problemas.empty());
    CHECK(lida.dados.dificuldade == Nivel::Dificil);
}

TEST_CASE("Configuracao: o que nao se entende vira problema, e o resto vale") {
    SUBCASE("valor desconhecido fica no padrao") {
        const auto lida = Configuracao::Ler("dificuldade=impossivel\n");
        REQUIRE(lida.problemas.size() == 1);
        CHECK(lida.problemas[0].find("linha 1") != std::string::npos);
        CHECK(lida.dados.dificuldade == Nivel::Normal);
    }
    SUBCASE("chave desconhecida nao derruba a conhecida") {
        const auto lida = Configuracao::Ler("volume=7\ndificuldade=dificil\n");
        REQUIRE(lida.problemas.size() == 1);
        CHECK(lida.problemas[0].find("volume") != std::string::npos);
        CHECK(lida.dados.dificuldade == Nivel::Dificil);
    }
    SUBCASE("linha sem igual") {
        const auto lida = Configuracao::Ler("dificil\n");
        REQUIRE(lida.problemas.size() == 1);
        CHECK(lida.dados.dificuldade == Nivel::Normal);
    }
    SUBCASE("a ultima linha de uma chave repetida vale") {
        const auto lida = Configuracao::Ler("dificuldade=dificil\ndificuldade=normal\n");
        CHECK(lida.problemas.empty());
        CHECK(lida.dados.dificuldade == Nivel::Normal);
    }
}

TEST_CASE("Configuracao: o volume, a tela cheia e a hitbox vao e voltam, e o padrao e o jogo de sempre") {
    const Configuracao::Dados padrao;
    CHECK(padrao.volumeMusica == Configuracao::kVolumeMaximo);
    CHECK(padrao.volumeEfeitos == Configuracao::kVolumeMaximo);
    CHECK_FALSE(padrao.telaCheia);
    CHECK_FALSE(padrao.mostrarHitbox);

    Configuracao::Dados d;
    d.dificuldade = Nivel::Dificil;
    d.volumeMusica = 0;
    d.volumeEfeitos = 7;
    d.telaCheia = true;
    d.mostrarHitbox = true;
    const auto lida = Configuracao::Ler(Configuracao::Serializar(d));
    CHECK(lida.problemas.empty());
    CHECK(lida.dados.dificuldade == Nivel::Dificil);
    CHECK(lida.dados.volumeMusica == 0);
    CHECK(lida.dados.volumeEfeitos == 7);
    CHECK(lida.dados.telaCheia);
    CHECK(lida.dados.mostrarHitbox);

    // O ARQUIVO DE ANTES, so com a dificuldade, continua valendo: o resto
    // fica no padrao, sem problema.
    const auto antigo = Configuracao::Ler("dificuldade=dificil\n");
    CHECK(antigo.problemas.empty());
    CHECK(antigo.dados.volumeMusica == Configuracao::kVolumeMaximo);
    CHECK_FALSE(antigo.dados.telaCheia);
}

TEST_CASE("Configuracao: volume fora da faixa e sim/nao estranho viram problema e ficam no padrao") {
    const auto lida = Configuracao::Ler("volume_musica=11\nvolume_efeitos=-1\ntela_cheia=talvez\nhitbox=1\n");
    CHECK(lida.problemas.size() == 4u);
    CHECK(lida.dados.volumeMusica == Configuracao::kVolumeMaximo);
    CHECK(lida.dados.volumeEfeitos == Configuracao::kVolumeMaximo);
    CHECK_FALSE(lida.dados.telaCheia);
    CHECK_FALSE(lida.dados.mostrarHitbox);
}

TEST_CASE("Configuracao: o volume anda preso entre mudo e o maximo, e o fator vai de 0 a 1") {
    CHECK(Configuracao::MoverVolume(5, 1) == 6);
    CHECK(Configuracao::MoverVolume(0, -1) == 0);
    CHECK(Configuracao::MoverVolume(Configuracao::kVolumeMaximo, 1) == Configuracao::kVolumeMaximo);
    CHECK(Configuracao::FatorDoVolume(0) == 0.0f);
    CHECK(Configuracao::FatorDoVolume(Configuracao::kVolumeMaximo) == 1.0f);
    CHECK(Configuracao::FatorDoVolume(5) == doctest::Approx(0.5f));
}
