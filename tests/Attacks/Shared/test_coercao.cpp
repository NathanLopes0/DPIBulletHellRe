//
// Os sistemas de tipos da fase 2 do Luizinho.
//

#include "../../doctest.h"

#include <cmath>
#include <set>
#include <algorithm>
#include <cstdint>
#include <vector>

#include "../../../Source/Attacks/Shared/Coercao.h"

using namespace Coercao;

// A IGUALDADE DE VALORES e so do teste: o jogo nunca compara dois.
namespace Coercao {
    bool operator==(const Valor& a, const Valor& b) {
        if (a.tipo != b.tipo) return false;
        return a.tipo == Tipo::Numero ? a.numero == b.numero : a.texto == b.texto;
    }
}

namespace {
    constexpr Forma kTodas[kFormas] = {Forma::JsSoma, Forma::PhpSoma, Forma::PythonSoma,
                                       Forma::JsProduto, Forma::PythonRepete, Forma::HaskellSoma};

    /// Os angulos de um leque da WaveAttack: de central - abertura / 2, um
    /// passo de abertura / (n - 1) por projetil.
    std::vector<float> Leque(const int n, const float central, const float abertura) {
        std::vector<float> a;
        const float passo = n > 1 ? abertura / static_cast<float>(n - 1) : 0.0f;
        for (int i = 0; i < n; ++i) a.push_back(central - abertura / 2.0f + static_cast<float>(i) * passo);
        return a;
    }
}

TEST_CASE("Coercao: a mesma soma em quatro linguagens") {
    const Comando js{Forma::JsSoma, 3, 4};
    const Resultado rjs = Avaliar(js);
    CHECK(rjs.desfecho == Desfecho::Valor);
    CHECK(rjs.valor == Texto("34"));
    CHECK(rjs.coagidoEsquerdo);           // o 3 vira "3"
    CHECK_FALSE(rjs.coagidoDireito);
    CHECK(rjs.esquerdoDepois == Texto("3"));

    const Comando php{Forma::PhpSoma, 3, 4};
    const Resultado rphp = Avaliar(php);
    CHECK(rphp.valor == Numero(7));
    CHECK(rphp.coagidoDireito);           // o "4" vira 4
    CHECK_FALSE(rphp.coagidoEsquerdo);
    CHECK(rphp.direitoDepois == Numero(4));

    const Resultado rpy = Avaliar({Forma::PythonSoma, 3, 4});
    CHECK(rpy.desfecho == Desfecho::ErroDeExecucao);
    CHECK_FALSE(rpy.coagidoEsquerdo);
    CHECK_FALSE(rpy.coagidoDireito);

    const Resultado rhs = Avaliar({Forma::HaskellSoma, 3, 4});
    CHECK(rhs.desfecho == Desfecho::ErroDeCompilacao);

    // As quatro escrevem a MESMA expressao.
    CHECK(Escrito(Esquerdo(js)) == "3");
    CHECK(Escrito(Direito(js)) == "\"4\"");
    for (const Forma f : {Forma::PhpSoma, Forma::PythonSoma, Forma::HaskellSoma}) {
        CHECK(Escrito(Esquerdo({f, 3, 4})) == "3");
        CHECK(Escrito(Direito({f, 3, 4})) == "\"4\"");
    }
}

TEST_CASE("Coercao: o * do JavaScript converte os dois, e o do Python repete o texto") {
    const Resultado r = Avaliar({Forma::JsProduto, 6, 2});
    CHECK(r.valor == Numero(12));
    CHECK(r.coagidoEsquerdo);
    CHECK(r.coagidoDireito);
    CHECK(r.esquerdoDepois == Numero(6));
    CHECK(r.direitoDepois == Numero(2));
    CHECK(Escrito(Esquerdo({Forma::JsProduto, 6, 2})) == "\"6\"");

    const Resultado p = Avaliar({Forma::PythonRepete, 3, 4});
    CHECK(p.valor == Texto("3333"));
    CHECK_FALSE(p.coagidoEsquerdo);       // nada e convertido: a operacao existe
    CHECK_FALSE(p.coagidoDireito);
    CHECK(Escrito(Esquerdo({Forma::PythonRepete, 3, 4})) == "\"3\"");
    CHECK(Escrito(Direito({Forma::PythonRepete, 3, 4})) == "4");
}

TEST_CASE("Coercao: o placar diz a linguagem, como ela tipa, e depois o resultado") {
    const Comando js{Forma::JsSoma, 3, 4};
    CHECK(Placar(js) == "JavaScript \xc2\xb7 tipagem fraca\n3 + \"4\"");
    CHECK(PlacarComResultado(js) == "JavaScript \xc2\xb7 tipagem fraca\n3 + \"4\"  \xe2\x86\x92  \"34\"");
    CHECK(PlacarComResultado({Forma::PhpSoma, 3, 4}).find("\xe2\x86\x92  7") != std::string::npos);
    CHECK(PlacarComResultado({Forma::PythonSoma, 3, 4}).find("TypeError") != std::string::npos);
    CHECK(Placar({Forma::PythonSoma, 3, 4}).find("din\xc3\xa2mica") != std::string::npos);
    CHECK(Placar({Forma::HaskellSoma, 3, 4}).find("est\xc3\xa1tica") != std::string::npos);
    CHECK(PlacarComResultado({Forma::HaskellSoma, 3, 4}).find("compila\xc3\xa7\xc3\xa3o") != std::string::npos);

    for (const Forma f : kTodas) {
        const Comando c{f, 3, 4};
        // Duas linhas: a linguagem em cima, a expressao embaixo.
        CHECK(Placar(c).find('\n') != std::string::npos);
        CHECK(PlacarComResultado(c).rfind(Placar(c), 0) == 0);
    }
}

TEST_CASE("Coercao: o sorteio da numeros que o anel e a palavra comportam") {
    std::set<Forma> vistas;
    for (std::uint32_t s = 0; s < 400; ++s) {
        for (int v = 0; v < kComandos; ++v) {
            const Comando c = Proximo(v, s * 2654435761u + static_cast<std::uint32_t>(v));
            vistas.insert(c.forma);
            CHECK(c.a >= 1);
            CHECK(c.a <= 9);
            const Resultado r = Avaliar(c);
            if (r.desfecho != Desfecho::Valor) continue;
            if (r.valor.tipo == Tipo::Numero) {
                // UM ANEL DE 7 A 16: menos e pouco, e o chip tem a cara do numero.
                CHECK(r.valor.numero >= 7);
                CHECK(r.valor.numero <= 16);
                CHECK(r.valor.numero <= kMaiorNumero);
            } else {
                CHECK(r.valor.texto.size() >= 2u);
                CHECK(r.valor.texto.size() <= 5u);
            }
        }
    }
    CHECK(vistas.size() == static_cast<size_t>(kFormas));
    // A ordem: a fraca primeiro, e a estatica - a que protege - por ultimo.
    CHECK(Proximo(0, 1).forma == Forma::JsSoma);
    CHECK(Proximo(kComandos - 1, 1).forma == Forma::HaskellSoma);
}

TEST_CASE("Coercao: o anel poe um chip na direcao do aluno, com n par ou impar") {
    for (int n = 2; n <= 20; ++n) {
        CAPTURE(n);
        for (const float alvo : {90.0f, 37.0f, 140.0f}) {
            const auto angulos = Leque(n, CentroDoAnel(n, alvo), AberturaDoAnel(n));
            bool acertou = false;
            for (const float a : angulos) acertou = acertou || std::fabs(a - alvo) < 1e-3f;
            CHECK(acertou);
            // E O ANEL E CHEIO: o ultimo chip fica a um passo do primeiro.
            const float volta = angulos.back() - angulos.front() + 360.0f / static_cast<float>(n);
            CHECK(volta == doctest::Approx(360.0f));
        }
    }
}

TEST_CASE("Coercao: a palavra fica de frente para quem a recebe, com as aspas nas pontas") {
    const auto letras = Letras(Texto("34"));
    CHECK(letras == std::vector<char>{'"', '3', '4', '"'});
    CHECK(CaraDaLetra('"') == "Aspas");
    CHECK(CaraDaLetra('7') == "Letra7");

    // Descendo, a palavra se le da esquerda para a direita.
    const auto d = DesviosDaPalavra(4, 90.0f);
    REQUIRE(d.size() == 4u);
    for (size_t i = 1; i < d.size(); ++i) {
        CHECK(d[i].x > d[i - 1].x);
        CHECK(d[i].y == doctest::Approx(d[i - 1].y));
        CHECK(d[i].x - d[i - 1].x == doctest::Approx(kEntreLetras));
    }
    CHECK(d.front().x == doctest::Approx(-d.back().x));

    // Na diagonal, cada letra fica perpendicular ao caminho.
    for (const auto& p : DesviosDaPalavra(3, 45.0f)) {
        const float rad = 45.0f * 3.14159265f / 180.0f;
        CHECK(p.x * std::cos(rad) + p.y * std::sin(rad) == doctest::Approx(0.0f).epsilon(1e-4));
    }
}

TEST_CASE("Coercao: as caras dos chips") {
    CHECK(CaraDoOperando(Numero(7)) == "Numero7");
    CHECK(CaraDoOperando(Texto("4")) == "Texto4");
    CHECK(CaraDoNumero(16) == "Numero16");
    CHECK(CaraDoOperador(Operacao::Soma, false) == "Mais");
    CHECK(CaraDoOperador(Operacao::Produto, true) == "VezesErro");
}

TEST_CASE("Coercao: o operador fica em cima do aluno, dentro do campo") {
    CHECK(OndeFicaOOperador(600.0f, 1200.0f).x == doctest::Approx(600.0f));
    CHECK(OndeFicaOOperador(10.0f, 1200.0f).x == doctest::Approx(kMargemDoOperador));
    CHECK(OndeFicaOOperador(1190.0f, 1200.0f).x == doctest::Approx(1200.0f - kMargemDoOperador));
    CHECK(OndeFicaOOperador(600.0f, 1200.0f).y == doctest::Approx(kYDoOperador));
}

TEST_CASE("Coercao: o relogio da fase") {
    CHECK(kPrimeiro + static_cast<float>(kComandos) * kPorComando == doctest::Approx(Fase::kDuracao));
    // A conta roda bem antes de o comando seguinte abrir: o resultado tem
    // tempo de chegar ao aluno.
    CHECK(kConta + 1.0f < kPorComando);
    CHECK(kChecagem < kVoo);
    CHECK(kCoercao > 0.0f);
    CHECK(kCoercao < 1.0f);
}

TEST_CASE("Coercao: a ordem da luta embaralha as seis do meio, com o JavaScript abrindo e o Haskell fechando") {
    std::set<std::vector<int>> vistas;
    for (std::uint32_t semente = 0; semente < 3000; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDosTipos);
        const auto ordem = OrdemDosComandos(s);
        REQUIRE(ordem.size() == static_cast<size_t>(kComandos));
        std::vector<int> ordenada = ordem;
        std::sort(ordenada.begin(), ordenada.end());
        CHECK(ordenada == std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7});
        CHECK(OrdemDaFase(ordem.front()) == Forma::JsSoma);
        CHECK(OrdemDaFase(ordem.back()) == Forma::HaskellSoma);
        // AS MESMAS OITO CONTAS de sempre, so em outra ordem.
        std::multiset<int> formas, deSempre;
        for (int k = 0; k < kComandos; ++k) {
            formas.insert(static_cast<int>(Proximo(Variacao::NaOrdem(ordem, k), semente).forma));
            deSempre.insert(static_cast<int>(OrdemDaFase(k)));
        }
        CHECK(formas == deSempre);
        vistas.insert(ordem);
    }
    // 6! = 720 ordens; em 3000 sementes faltam, em media, 720 * e^(-4,2), umas 11.
    CHECK(vistas.size() > 680u);
}

TEST_CASE("Coercao: o degrau do Normal e o de sempre, e o do Dificil voa mais curto e sai mais rapido") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.voo == kVoo);
    CHECK(n.rapidez == 1.0f);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.voo < n.voo);
    CHECK(d.voo > 0.0f);
    CHECK(d.rapidez > 1.0f);
    // A troca de chip ainda acontece NO VOO: kCoercao e uma fracao dele.
    CHECK(kCoercao < 1.0f);
}

