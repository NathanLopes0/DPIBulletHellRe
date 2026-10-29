//
// A repescagem do Andre: a prova no BOCA.
//

#include "../../doctest.h"

#include <cstdint>
#include <set>
#include <string>

#include "../../../Source/Gameplay/Variacao.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "../../../Source/Attacks/Andre/Condicional.h"
#include "../../../Source/Attacks/Engine/Fase.h"
#include "../../../Source/Attacks/Andre/Funcao.h"
#include "../../../Source/Attacks/Andre/Laco.h"
#include "../../../Source/Attacks/Andre/Prova.h"

using namespace Prova;

TEST_CASE("Prova: os problemas vem em ordem, um depois do outro, dentro da fase") {
    for (int k = 0; k < kProblemas; ++k) {
        const auto& p = kProva[static_cast<std::size_t>(k)];
        CAPTURE(p.letra);
        CHECK(p.letra == 'A' + k);
        CHECK(p.fim > p.inicio + kAbertura);
        if (k > 0) CHECK(p.inicio == doctest::Approx(kProva[static_cast<std::size_t>(k - 1)].fim));
        CHECK(ProblemaEm(p.inicio) == k);
        CHECK(ProblemaEm(p.fim - 0.01f) == k);
        CHECK(p.quantos >= 1);
    }
    CHECK(kProva[0].inicio == doctest::Approx(0.0f));
    CHECK(kProva[kProblemas - 1].fim <= Fase::kDuracao);
    CHECK(ProblemaEm(kProva[kProblemas - 1].fim) == -1);
}

TEST_CASE("Prova: cada problema cabe no tempo dele - o que ele roda acaba antes de ele fechar") {
    for (int k = 0; k < kProblemas; ++k) {
        const auto& p = kProva[static_cast<std::size_t>(k)];
        CAPTURE(p.letra);
        const float tempo = p.fim - p.inicio - kAbertura;
        switch (p.tema) {
            case Tema::Condicional: {
                // Um comando inteiro, com o ultimo balao estourado.
                CHECK(p.quantos == 1);
                CHECK(Condicional::kPorComando <= tempo + 1.0e-4f);
                const auto forma = Condicional::Proximo(p.vez, 1u).forma;
                for (std::uint32_t s = 0; s < 50; ++s) CHECK(Condicional::Proximo(p.vez, s).forma == forma);
                break;
            }
            case Tema::Laco: {
                float pior = 0.0f;
                for (std::uint32_t s = 0; s < 200; ++s) {
                    const auto prog = Laco::Proximo(p.vez, s * 7919u);
                    pior = std::max(pior, Laco::Duracao(prog, Laco::Execucao(prog, s)));
                }
                CHECK(Laco::kPrimeiro + pior <= tempo + 1.0e-4f);
                break;
            }
            case Tema::Funcao: {
                // O pior caminho de uma chamada com funcao de dentro: o
                // professor numa ponta e o cartao de dentro do outro lado, ida,
                // execucao, volta, ida ate o de fora e execucao.
                const float largura = 1200.0f;
                const float dx = Funcao::kXMaximo - largura * Funcao::kXDoLado;
                const float dy = Funcao::kYDoCartaoDeDentro - Funcao::kYDoProfessor;
                const float lado = std::sqrt(dx * dx + dy * dy) / Funcao::kVelocidadeDoValor;
                const float ax = Funcao::kXMaximo - largura / 2.0f;
                const float ay = Funcao::kYDoCartao - Funcao::kYDoProfessor;
                const float fora = std::sqrt(ax * ax + ay * ay) / Funcao::kVelocidadeDoValor;
                const float chamada = lado + Funcao::kExecutando + lado + fora + Funcao::kExecutando;
                CHECK(Funcao::TemInterna(Funcao::Proximo(p.vez, 1u)));
                CHECK(Funcao::kPrimeiro + static_cast<float>(p.quantos) * chamada +
                      static_cast<float>(p.quantos - 1) * Funcao::kEntreComandos <= tempo + 1.0e-4f);
                break;
            }
        }
    }
}

TEST_CASE("Prova: o placar abre cada problema com o veredito do anterior, e cada um tem o seu balao") {
    CHECK(Abertura(0) == "Problema A: if / else");
    CHECK(Abertura(1).rfind("A: YES!", 0) == 0);
    CHECK(Abertura(1).find("Problema B") != std::string::npos);
    CHECK(Veredito(3) == "Problema D: YES");
    // O ULTIMO CAPOTA: o laco que passa do fim do vetor, e o juiz nao aceita.
    CHECK(Veredito(4) == "Problema E: NO - Runtime error");
    CHECK_FALSE(Aceito(4));
    CHECK(CaraDoTrofeu(4).empty());
    CHECK(Laco::Proximo(kProva[4].vez, 1u).forma == Laco::Forma::ForaDoVetor);
    CHECK(Abertura(4).rfind("D: YES!", 0) == 0);
    std::set<std::string> trofeus;
    int aceitos = 0;
    for (int k = 0; k < kProblemas; ++k) {
        if (!Aceito(k)) continue;
        ++aceitos;
        trofeus.insert(CaraDoTrofeu(k));
    }
    CHECK(aceitos == 4);
    CHECK(static_cast<int>(trofeus.size()) == aceitos);
    CHECK(CaraDoTrofeu(-1).empty());
    CHECK(Abertura(kProblemas).empty());
}

TEST_CASE("Prova: a prova da luta - os quatro primeiros em qualquer ordem, o que capota no fim, a letra do lugar") {
    std::set<std::string> ordens;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        CAPTURE(semente);
        Variacao::Sorteio s(semente);
        const Lista prova = Embaralhada(s);
        std::string nomes;
        std::multiset<std::string> todos;
        for (int k = 0; k < kProblemas; ++k) {
            const auto& p = prova[static_cast<std::size_t>(k)];
            CHECK(p.letra == 'A' + k);
            if (k == 0) CHECK(p.inicio == doctest::Approx(0.0f));
            else CHECK(p.inicio == doctest::Approx(prova[static_cast<std::size_t>(k - 1)].fim));
            CHECK(ProblemaEm(p.inicio, prova) == k);
            CHECK(ProblemaEm(p.fim - 0.01f, prova) == k);
            todos.insert(p.nome);
            nomes += p.nome;
            nomes += "|";
            // CADA PROBLEMA LEVA O PROPRIO TEMPO para onde vai.
            bool achou = false;
            for (const auto& q : kProva) {
                if (std::string(q.nome) != p.nome) continue;
                achou = true;
                CHECK(p.fim - p.inicio == doctest::Approx(q.fim - q.inicio));
                CHECK(p.tema == q.tema);
                CHECK(p.vez == q.vez);
            }
            CHECK(achou);
        }
        CHECK(todos.size() == static_cast<size_t>(kProblemas));
        std::set<std::string> distintos(todos.begin(), todos.end());
        CHECK(distintos.size() == static_cast<size_t>(kProblemas));
        CHECK_FALSE(Aceito(kProblemas - 1, prova));                 // o NO fecha a prova
        for (int k = 0; k + 1 < kProblemas; ++k) CHECK(Aceito(k, prova));
        CHECK(prova[kProblemas - 1].fim == doctest::Approx(kProva[kProblemas - 1].fim));
        CHECK(Abertura(1, prova).rfind(std::string("A: YES!  Problema B: ") + prova[1].nome, 0) == 0);
        ordens.insert(nomes);
    }
    CHECK(ordens.size() == 24u);
}
