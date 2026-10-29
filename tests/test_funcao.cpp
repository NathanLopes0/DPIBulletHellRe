//
// As funcoes da fase 3 do Andre.
//

#include "doctest.h"

#include <cstdint>
#include <set>
#include <string>

#include "../Source/Variacao.h"

#include <set>

#include "../Source/Attacks/Funcao.h"
#include "../Source/Attacks/Laco.h"

using namespace Funcao;

TEST_CASE("Funcao: o vetor do chao e o mesmo da fase 2") {
    CHECK(kCasas == Laco::kCasas);
    CHECK(CasaDoX(50.0f, 1200.0f) == 0);
    CHECK(CasaDoX(1150.0f, 1200.0f) == kCasas - 1);
    CHECK(CasaDoX(-30.0f, 1200.0f) == 0);
    CHECK(CasaDoX(1300.0f, 1200.0f) == kCasas - 1);
    // A casa do aluno e a casa em cujo meio o cacho sobe.
    for (int i = 0; i < kCasas; ++i) CHECK(CasaDoX(Laco::XDaCasa(i, 1200.0f), 1200.0f) == i);
}

TEST_CASE("Funcao: o retorno da de dentro vira o argumento da de fora") {
    const Comando dobro{Forma::BaloesDoDobro, 3, 11};
    CHECK(TemInterna(dobro));
    CHECK(Interna(dobro) == Nome::Dobro);
    CHECK(Externa(dobro) == Nome::Baloes);
    CHECK(ArgumentosDaInterna(dobro) == std::vector<int>{3});
    CHECK(Retorno(dobro) == 6);
    CHECK(ArgumentosDaExterna(dobro, 0) == std::vector<int>{6, 11});
    CHECK(Placar(dobro) == "baloes(dobro(3), 11);");
    CHECK(PlacarComValores(dobro, 0) == "baloes(6, 11);");

    // A DIVISAO DE INTEIROS: meio(3, 8) e 5, nao 5,5.
    const Comando meio{Forma::LequeDoMeio, 3, 8};
    CHECK(Retorno(meio) == 5);
    CHECK(Placar(meio) == "leque(meio(3, 8));");
    CHECK(PlacarComValores(meio, 0) == "leque(5);");

    const Comando simples{Forma::Baloes, 2, 5};
    CHECK_FALSE(TemInterna(simples));
    CHECK(Interna(simples) == Externa(simples));
    CHECK(PlacarComValores(simples, 7) == Placar(simples));
}

TEST_CASE("Funcao: por valor a funcao recebe a casa do aluno na hora da chamada; por referencia, um endereco") {
    const Comando valor{Forma::PorValor, 0, 0};
    CHECK(ArgumentosDaExterna(valor, 6) == std::vector<int>{5, 7});
    CHECK(PlacarComValores(valor, 6) == "aluno = 6: baloes(5, 7);");
    // Na ponta, a copia sai do vetor, e o baloes corta.
    CHECK(ArgumentosDaExterna(valor, 0) == std::vector<int>{-1, 1});
    CHECK(CasasDosBaloes(-1, 1) == std::vector<int>{0, 1});
    CHECK(CasasDosBaloes(10, 12) == std::vector<int>{10, 11});

    const Comando referencia{Forma::PorReferencia, 0, 0};
    CHECK(Externa(referencia) == Nome::Segue);
    CHECK(ArgumentosDaExterna(referencia, 6).empty());
    CHECK(Placar(referencia).find("&aluno") != std::string::npos);
}

TEST_CASE("Funcao: o baloes de tras para a frente nao roda") {
    CHECK(CasasDosBaloes(2, 5) == std::vector<int>{2, 3, 4, 5});
    CHECK(CasasDosBaloes(6, 2).empty());
}

TEST_CASE("Funcao: todo comando sorteado deixa casa livre, e todo valor que voa tem cara") {
    std::set<int> formas;
    for (int vez = 0; vez < 3 * kFormas; ++vez) {
        for (std::uint32_t s = 0; s < 80; ++s) {
            const Comando c = Proximo(vez, s * 2654435761u + static_cast<std::uint32_t>(vez));
            CAPTURE(Placar(c));
            formas.insert(static_cast<int>(c.forma));
            for (const int v : ArgumentosDaInterna(c)) {
                CHECK(v >= kMenorValor);
                CHECK(v <= kMaiorValor);
            }
            for (int casa = 0; casa < kCasas; ++casa) {
                for (const int v : ArgumentosDaExterna(c, casa)) {
                    CHECK(v >= kMenorValor);
                    CHECK(v <= kMaiorValor);
                }
            }
            if (Externa(c) == Nome::Baloes && c.forma != Forma::PorValor) {
                const auto args = ArgumentosDaExterna(c, 0);
                const auto casas = CasasDosBaloes(args[0], args[1]);
                // Sobe ao menos um cacho, e nunca o vetor inteiro.
                CHECK_FALSE(casas.empty());
                CHECK(static_cast<int>(casas.size()) < kCasas);
            }
            if (Externa(c) == Nome::Leque) {
                const int n = ArgumentosDaExterna(c, 0)[0];
                CHECK(n >= 3);
                CHECK(n <= 7);
            }
        }
    }
    CHECK(static_cast<int>(formas.size()) == kFormas);
    // A fase segue a ordem: primeiro a chamada simples, por ultimo a referencia.
    CHECK(Proximo(0, 1u).forma == Forma::Baloes);
    CHECK(Proximo(kFormas - 1, 1u).forma == Forma::PorReferencia);
}

TEST_CASE("Funcao: cada funcao tem as suas caras, e cada valor a dele") {
    std::set<std::string> caras;
    for (int f = 0; f < kFuncoes; ++f) {
        const auto nome = static_cast<Nome>(f);
        caras.insert(CaraDoCartao(nome));
        caras.insert(CaraDoCartaoAceso(nome));
    }
    CHECK(static_cast<int>(caras.size()) == 2 * kFuncoes);
    std::set<std::string> valores;
    for (int v = kMenorValor; v <= kMaiorValor; ++v) valores.insert(CaraDoValor(v));
    CHECK(static_cast<int>(valores.size()) == kMaiorValor - kMenorValor + 1);
}

TEST_CASE("Funcao: o degrau - o Normal e o de sempre, o Dificil segue mais de perto") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.entreSegue == doctest::Approx(kEntreSegue));
    CHECK(n.rapidez == doctest::Approx(1.0f));
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entreSegue < n.entreSegue);
    CHECK(d.rapidez > n.rapidez);
}

TEST_CASE("Funcao: em qualquer ordem da luta, as seis chamadas aparecem e o baloes abre") {
    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        Variacao::Sorteio s(semente);
        const auto ordem = Variacao::Ordem(s, kFormas);
        CHECK(Proximo(Variacao::NaOrdem(ordem, 0), semente).forma == Forma::Baloes);
        std::set<int> formas;
        for (int vez = 0; vez < kFormas; ++vez) formas.insert(static_cast<int>(Proximo(Variacao::NaOrdem(ordem, vez), semente).forma));
        CHECK(formas.size() == static_cast<size_t>(kFormas));
    }
}
