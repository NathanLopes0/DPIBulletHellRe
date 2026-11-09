//
// As transacoes da fase 3 do Thiago.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <set>

#include "../Source/Attacks/Transacao.h"

using Transacao::Operacao;
using Transacao::Tipo;

namespace {
    bool PorLinha(const Tipo t) { return t == Tipo::SelectLinha || t == Tipo::Update || t == Tipo::Delete; }

    int Contar(const std::vector<Operacao>& ops, const Tipo t) {
        return static_cast<int>(std::count_if(ops.begin(), ops.end(), [t](const Operacao& o) { return o.tipo == t; }));
    }

    /// O COMMIT na mesma ordem do Thiago: o que muda no lugar, depois o que tira linha, depois o que poe.
    void Aplicar(Banco::Registros& regs, const std::vector<Operacao>& ops) {
        std::vector<int> deletes;
        for (const Operacao& o : ops) {
            const int l = Transacao::LinhaDe(regs, o.mat);
            if (o.tipo == Tipo::Update && l >= 0) regs[static_cast<size_t>(l)].nota = o.nota;
            if (o.tipo == Tipo::Delete && l >= 0) deletes.push_back(l);
        }
        std::sort(deletes.rbegin(), deletes.rend());
        for (const int l : deletes) regs.erase(regs.begin() + l);
        for (const Operacao& o : ops) {
            if (o.tipo == Tipo::Insert && static_cast<int>(regs.size()) < Banco::kLinhas) regs.push_back(o.novo);
        }
    }
}

TEST_CASE("Transacao: a linha de uma matricula, e -1 para quem nao esta la") {
    const auto regs = Banco::Sortear(7u);
    for (size_t l = 0; l < regs.size(); ++l) CHECK(Transacao::LinhaDe(regs, regs[l].mat) == static_cast<int>(l));
    CHECK(Transacao::LinhaDe(regs, 5) == -1);
}

TEST_CASE("Transacao: as casas do rascunho de cada operacao") {
    const auto regs = Banco::Sortear(11u);
    const int nota = static_cast<int>(Banco::Coluna::Nota);

    Operacao linha{Tipo::SelectLinha, regs[2].mat};
    const auto l = Transacao::Casas(regs, linha, 4);
    REQUIRE(l.size() == static_cast<size_t>(Banco::kColunas));
    for (const auto& c : l) CHECK(c.linha == 2);

    Operacao coluna;
    coluna.tipo = Tipo::SelectColuna;
    coluna.coluna = 3;
    const auto c = Transacao::Casas(regs, coluna, 4);
    REQUIRE(c.size() == static_cast<size_t>(Banco::kLinhas));
    for (const auto& casa : c) CHECK(casa.coluna == 3);

    Operacao update{Tipo::Update, regs[1].mat};
    const auto u = Transacao::Casas(regs, update, 4);
    REQUIRE(u.size() == 1);
    CHECK(u[0] == Banco::Casa{1, nota});

    Operacao apaga{Tipo::Delete, regs[0].mat};
    CHECK(Transacao::Casas(regs, apaga, 4).size() == static_cast<size_t>(Banco::kColunas));

    // O INSERT cai na linha vazia; sem ela, nao tem rascunho.
    Operacao poe;
    poe.tipo = Tipo::Insert;
    const auto i = Transacao::Casas(regs, poe, 3);
    REQUIRE(i.size() == static_cast<size_t>(Banco::kColunas));
    for (const auto& casa : i) CHECK(casa.linha == 3);
    CHECK(Transacao::Casas(regs, poe, Banco::kLinhas).empty());

    // Matricula que nao esta na tabela: nada.
    CHECK(Transacao::Casas(regs, Operacao{Tipo::Update, 5}, 4).empty());
}

TEST_CASE("Transacao: a mesma sorte da a mesma operacao") {
    const auto regs = Banco::Sortear(3u);
    for (std::uint32_t s = 0; s < 50; ++s) {
        Operacao a, b;
        REQUIRE(Transacao::Proxima(regs, {}, s, a));
        REQUIRE(Transacao::Proxima(regs, {}, s, b));
        CHECK(a.tipo == b.tipo);
        CHECK(a.mat == b.mat);
        CHECK(a.coluna == b.coluna);
        CHECK(a.nota == b.nota);
    }
}

TEST_CASE("Transacao: com a tabela cheia nao ha INSERT, e o UPDATE muda a nota") {
    const auto regs = Banco::Sortear(21u);
    std::set<Tipo> vistos;
    for (std::uint32_t s = 0; s < 400; ++s) {
        Operacao op;
        REQUIRE(Transacao::Proxima(regs, {}, s, op));
        vistos.insert(op.tipo);
        CHECK(op.tipo != Tipo::Insert);
        if (op.tipo == Tipo::Update) {
            const int l = Transacao::LinhaDe(regs, op.mat);
            REQUIRE(l >= 0);
            CHECK(op.nota != regs[static_cast<size_t>(l)].nota);
            CHECK(op.nota >= 0);
            CHECK(op.nota <= 99);
        }
    }
    // Todos os outros aparecem.
    for (const Tipo t : {Tipo::SelectLinha, Tipo::SelectColuna, Tipo::Update, Tipo::Delete}) CHECK(vistos.count(t) == 1);
}

TEST_CASE("Transacao: transacoes abertas nunca brigam por linha, nunca esvaziam a tabela, e sempre sobra lugar") {
    std::mt19937 sorte(2024u);
    for (int luta = 0; luta < 60; ++luta) {
        Banco::Registros regs = Banco::Sortear(static_cast<std::uint32_t>(luta) * 31u + 1u);
        Transacao::Plano plano;
        plano.chanceDeSelect = 0.25f + 0.2f * static_cast<float>(luta % 5) / 4.0f;
        std::vector<std::vector<Operacao>> abertas(2);

        for (int passo = 0; passo < 300; ++passo) {
            const int qual = static_cast<int>(sorte() % 2u);
            auto& tx = abertas[static_cast<size_t>(qual)];
            const bool fecha = tx.size() >= 3 || (sorte() % 4u == 0u && !tx.empty());
            if (fecha) {
                Aplicar(regs, tx);
                tx.clear();
                continue;
            }
            std::vector<Operacao> pendentes;
            for (const auto& a : abertas) pendentes.insert(pendentes.end(), a.begin(), a.end());
            Operacao op;
            if (!Transacao::Proxima(regs, pendentes, sorte(), op, plano)) continue;
            tx.push_back(op);
            pendentes.push_back(op);

            // A linha preparada fica travada: nenhuma outra operacao pendente a usa.
            std::set<int> mats;
            for (const Operacao& o : pendentes) {
                if (!PorLinha(o.tipo)) continue;
                CHECK(Transacao::LinhaDe(regs, o.mat) >= 0);
                CHECK(mats.insert(o.mat).second);
            }
            // O DELETE deixa linhas, e o INSERT tem onde cair.
            CHECK(static_cast<int>(regs.size()) - Contar(pendentes, Tipo::Delete) >= Transacao::kMenosLinhas);
            CHECK(Contar(pendentes, Tipo::Insert) <= 1);
            CHECK(static_cast<int>(regs.size()) + Contar(pendentes, Tipo::Insert) <= Banco::kLinhas);
            // As varreduras.
            CHECK(Contar(pendentes, Tipo::SelectLinha) <= Transacao::kVarredurasDeLinha);
            CHECK(Contar(pendentes, Tipo::SelectColuna) <= Transacao::kVarredurasDeColuna);
            CHECK(Transacao::SobraLugar(regs, pendentes));
        }
        CHECK(static_cast<int>(regs.size()) >= Transacao::kMenosLinhas);
        CHECK(static_cast<int>(regs.size()) <= Banco::kLinhas);
    }
}

TEST_CASE("Transacao: sem lugar, nenhuma varredura cobre a tabela inteira") {
    const auto regs = Banco::Sortear(5u);
    std::vector<Operacao> todas;
    for (const auto& r : regs) todas.push_back(Operacao{Tipo::SelectLinha, r.mat});
    CHECK_FALSE(Transacao::SobraLugar(regs, todas));
    todas.pop_back();
    CHECK(Transacao::SobraLugar(regs, todas));
}

TEST_CASE("Transacao: os placares e as caras") {
    Operacao u{Tipo::Update, 42};
    u.nota = 12;
    CHECK(Transacao::PlacarDaOperacao(3, u) == "T3: UPDATE SET nota = 12 WHERE mat = 42");
    CHECK(Transacao::PlacarDaOperacao(3, Operacao{Tipo::Delete, 42}) == "T3: DELETE WHERE mat = 42");
    CHECK(Transacao::PlacarDaOperacao(3, Operacao{Tipo::SelectLinha, 42}) == "T3: SELECT * WHERE mat = 42");
    Operacao c;
    c.tipo = Tipo::SelectColuna;
    c.coluna = static_cast<int>(Banco::Coluna::Nota);
    CHECK(Transacao::PlacarDaOperacao(3, c) == "T3: SELECT nota");
    Operacao i;
    i.tipo = Tipo::Insert;
    i.novo.nome = 4;
    CHECK(Transacao::PlacarDaOperacao(3, i) == "T3: INSERT INTO alunos: Eva");
    CHECK(Transacao::PlacarDoBegin(3) == "BEGIN T3");
    CHECK(Transacao::PlacarDoCommit(3) == "T3: COMMIT");
    CHECK(Transacao::PlacarDoRollback(3) == "T3: ROLLBACK");

    CHECK(Transacao::CaraDoSelo(0, 3, false) == "Ciano3");
    CHECK(Transacao::CaraDoSelo(1, 9, true) == "MagentaPressa9");
    CHECK(Transacao::CaraDoRascunho(0, false) == "Ciano");
    CHECK(Transacao::CaraDoRascunho(1, true) == "MagentaRiscado");
}

TEST_CASE("Transacao: o selo novo cai num lugar livre, e com o tempo em todos") {
    std::set<int> vistos;
    for (std::uint32_t s = 0; s < 200; ++s) {
        const int p = Transacao::PosicaoDoSelo({1}, s);
        CHECK(p != 1);
        CHECK(p >= 0);
        CHECK(p < static_cast<int>(Transacao::kPosicoesDoSelo.size()));
        vistos.insert(p);
        vistos.insert(Transacao::PosicaoDoSelo({}, s));
    }
    CHECK(vistos.size() == Transacao::kPosicoesDoSelo.size());
    // O selo nunca nasce embaixo do professor, que le do meio.
    for (const float x : Transacao::kPosicoesDoSelo) CHECK(std::abs(x - 0.5f) > 0.1f);
}

TEST_CASE("Transacao: a chance do SELECT de cada luta fica em volta da de sempre") {
    float menor = 1.0f, maior = 0.0f;
    for (std::uint32_t s = 0; s < 200; ++s) {
        Variacao::Sorteio sorteio(s);
        const Transacao::Plano p = Transacao::Sortear(sorteio);
        menor = std::min(menor, p.chanceDeSelect);
        maior = std::max(maior, p.chanceDeSelect);
    }
    CHECK(menor >= 0.25f);
    CHECK(maior <= 0.45f);
    CHECK(menor < Transacao::Plano{}.chanceDeSelect);
    CHECK(maior > Transacao::Plano{}.chanceDeSelect);
}

TEST_CASE("Transacao: o degrau do Normal e o de sempre, e o do Dificil prepara mais com menos tempo") {
    const Transacao::Degrau n = Transacao::DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.novaTransacao == Transacao::kNovaTransacao);
    CHECK(n.operacoes == Transacao::kOperacoes);
    CHECK(n.entreOperacoes == Transacao::kEntreOperacoes);
    CHECK(n.ateOCommit == Transacao::kAteOCommit);

    const Transacao::Degrau d = Transacao::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.novaTransacao < n.novaTransacao);
    CHECK(d.operacoes > n.operacoes);
    CHECK(d.entreOperacoes < n.entreOperacoes);
    CHECK(d.ateOCommit < n.ateOCommit);

    // Do rascunho da ultima operacao ao COMMIT da tempo de atravessar uma casa da tabela.
    constexpr float kAndar = 300.0f;
    for (const Transacao::Degrau& g : {n, d}) CHECK(g.ateOCommit > 240.0f / kAndar);
}
