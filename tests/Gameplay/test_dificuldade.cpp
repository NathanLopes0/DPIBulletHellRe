//
// O modo de dificuldade e o arquivo de opcoes. Camadas puras.
//

#include "../doctest.h"

#include <set>
#include <string>

#include "../../Source/Gameplay/Dificuldade.h"
#include "../../Source/Data/Configuracao.h"

using Dificuldade::Nivel;

TEST_CASE("Dificuldade: cada nivel tem nome, descricao e uma chave so dele") {
    std::set<std::string> chaves, nomes;
    for (const Nivel n : Dificuldade::kNiveis) {
        CAPTURE(static_cast<int>(n));
        CHECK(std::string(Dificuldade::Nome(n)).size() > 0);
        CHECK(std::string(Dificuldade::Descricao(n)).size() > 0);
        chaves.insert(Dificuldade::Chave(n));
        nomes.insert(Dificuldade::Nome(n));

        // A CHAVE E ASCII: o CSV abre igual em qualquer editor.
        for (const char c : std::string(Dificuldade::Chave(n))) CHECK(static_cast<unsigned char>(c) < 128);
    }
    CHECK(chaves.size() == static_cast<size_t>(Dificuldade::kQuantos));
    CHECK(nomes.size() == static_cast<size_t>(Dificuldade::kQuantos));
    CHECK(std::string(Dificuldade::Nome(Nivel::Dificil)) == "DIF\xc3\x8d" "CIL");
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

TEST_CASE("Configuracao: a dificuldade nao e opcao da maquina") {
    // ELA E DA PARTIDA (Game::NivelDeDificuldade): o arquivo nao a escreve, e a
    // linha de um arquivo antigo e uma opcao desconhecida como outra qualquer -
    // vira aviso no log, e o resto do arquivo vale.
    CHECK(Configuracao::Serializar(Configuracao::Dados{}).find("dificuldade") == std::string::npos);
    const auto antigo = Configuracao::Ler("dificuldade=dificil\nhitbox=sim\n");
    CHECK(antigo.problemas.size() == 1u);
    CHECK(antigo.dados.mostrarHitbox);
}

TEST_CASE("Configuracao: arquivo vazio, comentario e linha em branco dao o padrao, sem problema") {
    for (const char* texto : {"", "\n\n", "# so comentario\n", "   \r\n\t\r\n"}) {
        CAPTURE(texto);
        const auto lida = Configuracao::Ler(texto);
        CHECK(lida.problemas.empty());
        CHECK(lida.dados.volumeMusica == Configuracao::kVolumeMaximo);
    }
}

TEST_CASE("Configuracao: aceita espaco em volta e fim de linha do Windows") {
    const auto lida = Configuracao::Ler("# x\r\n  volume_musica =  3  \r\n");
    CHECK(lida.problemas.empty());
    CHECK(lida.dados.volumeMusica == 3);
}

TEST_CASE("Configuracao: o que nao se entende vira problema, e o resto vale") {
    SUBCASE("chave desconhecida nao derruba a conhecida") {
        const auto lida = Configuracao::Ler("volume=7\nvolume_efeitos=4\n");
        REQUIRE(lida.problemas.size() == 1);
        CHECK(lida.problemas[0].find("volume") != std::string::npos);
        CHECK(lida.problemas[0].find("linha 1") != std::string::npos);
        CHECK(lida.dados.volumeEfeitos == 4);
    }
    SUBCASE("linha sem igual") {
        const auto lida = Configuracao::Ler("dificil\n");
        REQUIRE(lida.problemas.size() == 1);
    }
    SUBCASE("a ultima linha de uma chave repetida vale") {
        const auto lida = Configuracao::Ler("volume_musica=2\nvolume_musica=5\n");
        CHECK(lida.problemas.empty());
        CHECK(lida.dados.volumeMusica == 5);
    }
}

TEST_CASE("Configuracao: o volume, a tela cheia e a hitbox vao e voltam, e o padrao e o jogo de sempre") {
    const Configuracao::Dados padrao;
    CHECK(padrao.volumeMusica == Configuracao::kVolumeMaximo);
    CHECK(padrao.volumeEfeitos == Configuracao::kVolumeMaximo);
    CHECK_FALSE(padrao.telaCheia);
    CHECK_FALSE(padrao.mostrarHitbox);

    Configuracao::Dados d;
    d.volumeMusica = 0;
    d.volumeEfeitos = 7;
    d.telaCheia = true;
    d.mostrarHitbox = true;
    const auto lida = Configuracao::Ler(Configuracao::Serializar(d));
    CHECK(lida.problemas.empty());
    CHECK(lida.dados.volumeMusica == 0);
    CHECK(lida.dados.volumeEfeitos == 7);
    CHECK(lida.dados.telaCheia);
    CHECK(lida.dados.mostrarHitbox);

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

TEST_CASE("Dificuldade: no Dificil a invencibilidade depois de um acerto e bem mais curta") {
    using Dificuldade::Nivel;
    CHECK(Dificuldade::Invencibilidade(Nivel::Normal) == doctest::Approx(1.2f));
    CHECK(Dificuldade::Invencibilidade(Nivel::Dificil) <= Dificuldade::Invencibilidade(Nivel::Normal) / 2.0f);
    // AINDA PISCA: abaixo de um piscar (0,1 s) o aluno nem veria que foi acertado.
    CHECK(Dificuldade::Invencibilidade(Nivel::Dificil) >= 0.3f);
}

#include "../../Source/Gameplay/Acerto.h"

TEST_CASE("Acerto: o mesmo causador nao acerta de novo antes de 1,2 s, mas outro acerta") {
    Acerto::Memoria m;
    int tiroA = 0, tiroB = 0;
    m.Lembrar(&tiroA, 0.5f);
    CHECK(m.Lembra(&tiroA, 0.6f));
    CHECK_FALSE(m.Lembra(&tiroB, 0.6f));   // outro tiro acerta no ritmo do modo
    m.Avancar(0.6f);
    CHECK(m.Lembra(&tiroA, 1.1f));         // a parede que continua em cima dele
    m.Avancar(Acerto::kMesmoCausador);
    CHECK_FALSE(m.Lembra(&tiroA, 3.0f));   // passado o tempo, acerta de novo
    CHECK(m.Quantos() == 0u);
}

TEST_CASE("Acerto: o pool recicla o endereco, e o tiro novo nao herda a memoria do velho") {
    Acerto::Memoria m;
    int tiro = 0;
    m.Lembrar(&tiro, 2.0f);
    m.Avancar(0.2f);
    // O MESMO ENDERECO, mas com 0,1 s de vida: morreu e voltou como outro tiro.
    CHECK_FALSE(m.Lembra(&tiro, 0.1f));
    // O CORPO DO CHEFE nao envelhece: idade 0 sempre.
    int corpo = 0;
    m.Lembrar(&corpo, 0.0f);
    CHECK(m.Lembra(&corpo, 0.0f));
    CHECK_FALSE(m.Lembra(nullptr, 0.0f));
}
