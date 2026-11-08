// Progresso academico: o recorde por materia e o ponto de retomada.
// Camada pura: sem Game, sem SDL.

#include "doctest.h"
#include "../Source/Regras/Dificuldade.h"
#include "../Source/Aluno/Progresso.h"
#include "Ajudantes.h"

// ---------------------------------------------------------------------------
// Materia nunca jogada
// ---------------------------------------------------------------------------

TEST_CASE("Progresso: materia nunca jogada tem recorde zero") {
    const Progresso p;
    CHECK(p.MelhorNota(99) == doctest::Approx(0.f));
    CHECK_FALSE(p.Aprovado(99));
}

TEST_CASE("Progresso: consultar NAO cria registro") {
    // O getter antigo usava operator[], que inseria uma entrada zero a cada
    // leitura: mutacao dentro de uma consulta, e o mapa crescia com materias
    // que ninguem jogou.
    Progresso p;
    for (int m = 0; m < 50; ++m) {
        (void)p.MelhorNota(m);
        (void)p.Aprovado(m);
        (void)Ajudantes::UltimaNota(p, m);
    }
    CHECK(p.QuantasRegistradas() == 0);
}

TEST_CASE("Progresso: materia nunca jogada nao tem ultima nota") {
    // Zero, e nao kNotaInicial: a ultima nota e um fato sobre o passado do aluno,
    // nao o ponto de partida da proxima batalha. Toda fase comeca em kNotaInicial,
    // e isso agora e decisao de Battle::Load, nao daqui.
    const Progresso p;
    CHECK(Ajudantes::UltimaNota(p, 1) == doctest::Approx(0.0f));
}

// ---------------------------------------------------------------------------
// O recorde so sobe
// ---------------------------------------------------------------------------

TEST_CASE("Progresso: o recorde sobe quando a nota e melhor") {
    Progresso p;
    p.RegistrarNota(1, 55.f, Dificuldade::Nivel::Normal);
    CHECK(p.MelhorNota(1) == doctest::Approx(55.f));
    p.RegistrarNota(1, 80.f, Dificuldade::Nivel::Normal);
    CHECK(p.MelhorNota(1) == doctest::Approx(80.f));
}

TEST_CASE("Progresso: uma tentativa ruim NAO apaga o recorde") {
    // Era o bug visivel na tela de selecao: a variavel se chamava highScore mas
    // mostrava a ULTIMA nota, entao jogar mal baixava o recorde exibido.
    Progresso p;
    p.RegistrarNota(1, 85.f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(1, 45.f, Dificuldade::Nivel::Normal);
    CHECK(p.MelhorNota(1) == doctest::Approx(85.f));
}

TEST_CASE("Progresso: uma tentativa ruim NAO re-tranca uma materia aprovada") {
    // Consequencia da anterior, e a mais grave: como as regras de desbloqueio
    // leem o recorde, apagar o recorde re-trancava a coluna seguinte.
    Progresso p;
    p.RegistrarNota(1, 70.f, Dificuldade::Nivel::Normal);
    CHECK(p.Aprovado(1));
    p.RegistrarNota(1, 41.f, Dificuldade::Nivel::Normal);
    CHECK(p.Aprovado(1));
}

TEST_CASE("Progresso: a retomada acompanha a ULTIMA nota, nao o recorde") {
    // As duas coisas que o mapa antigo confundia, agora separadas: o recorde e
    // um registro, a retomada e onde a proxima batalha comeca.
    Progresso p;
    p.RegistrarNota(1, 85.f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(1, 50.f, Dificuldade::Nivel::Normal);
    CHECK(p.MelhorNota(1) == doctest::Approx(85.f));
    CHECK(Ajudantes::UltimaNota(p, 1) == doctest::Approx(50.f));
}

TEST_CASE("Progresso: a ultima nota e a ultima mesmo, sem piso") {
    // Tinha piso de kNotaInicial enquanto a batalha comecava daqui. Agora que ela
    // sempre comeca em 40, um piso aqui so mentiria sobre o que o aluno tirou - e
    // a planilha do professor exporta este numero.
    Progresso p;
    p.RegistrarNota(1, 10.f, Dificuldade::Nivel::Normal);
    CHECK(Ajudantes::UltimaNota(p, 1) == doctest::Approx(10.f));
    CHECK(p.MelhorNota(1) == doctest::Approx(10.f));
}

// ---------------------------------------------------------------------------
// Aprovacao
// ---------------------------------------------------------------------------

TEST_CASE("Progresso: a nota de aprovacao e inclusiva") {
    Progresso p;
    p.RegistrarNota(1, Progresso::kNotaDeAprovacao - 0.1f, Dificuldade::Nivel::Normal);
    CHECK_FALSE(p.Aprovado(1));
    p.RegistrarNota(1, Progresso::kNotaDeAprovacao, Dificuldade::Nivel::Normal);
    CHECK(p.Aprovado(1));
}

TEST_CASE("Progresso: conta as aprovadas de uma lista") {
    Progresso p;
    p.RegistrarNota(1, 70.f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(2, 45.f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(3, 61.f, Dificuldade::Nivel::Normal);
    CHECK(p.QuantasAprovadas({1, 2, 3}) == 2);
    CHECK(p.QuantasAprovadas({2}) == 0);
    CHECK(p.QuantasAprovadas({}) == 0);
    CHECK(p.QuantasAprovadas({1, 99}) == 1);   // 99 nunca jogada
}

// ---------------------------------------------------------------------------
// As regras de desbloqueio, expressas sobre o progresso
// ---------------------------------------------------------------------------

TEST_CASE("Desbloqueio: a coluna 3 exige duas aprovacoes na coluna 2") {
    const std::vector<int> col2 = {2, 3, 4, 5};
    Progresso p;

    CHECK(p.QuantasAprovadas(col2) < 2);
    p.RegistrarNota(2, 70.f, Dificuldade::Nivel::Normal);
    CHECK(p.QuantasAprovadas(col2) < 2);
    p.RegistrarNota(3, 65.f, Dificuldade::Nivel::Normal);
    CHECK(p.QuantasAprovadas(col2) >= 2);

    // E continua desbloqueada depois de uma tentativa ruim numa delas.
    p.RegistrarNota(2, 42.f, Dificuldade::Nivel::Normal);
    CHECK(p.QuantasAprovadas(col2) >= 2);
}

// ---------------------------------------------------------------------------
// AS BATALHAS EM DUPLA (requisitos-coop, D9 e D10)
//
// O recorde de dupla e um numero SEPARADO do de solo: sao duas competicoes, e
// o ranking precisa delas separadas. Mas o MAPA DO CURSO nao separa nada -
// passar e passar, tenha sido com quem for.
// ---------------------------------------------------------------------------

TEST_CASE("Progresso: a nota de dupla nao entra no recorde de solo") {

    // ESTE E O TESTE QUE PROTEGE OS DOIS RANKINGS. Se uma nota de co-op caisse
    // no recorde de solo, o ranking de solo passaria a conter notas feitas
    // acompanhado - e as duas listas deixariam de significar coisas diferentes.
    Progresso p;
    p.RegistrarNota(3, 50.0f, Dificuldade::Nivel::Normal);
    p.RegistrarNotaDeDupla(3, 80.0f, Dificuldade::Nivel::Normal, "90112");

    CHECK(p.Recorde(3, Progresso::Modo::Solo, Dificuldade::Nivel::Normal).nota == doctest::Approx(50.0f));
    CHECK(p.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).nota == doctest::Approx(80.0f));
}

TEST_CASE("Progresso: e o contrario tambem - a nota de solo nao entra na de dupla") {

    Progresso p;
    p.RegistrarNotaDeDupla(3, 50.0f, Dificuldade::Nivel::Normal, "90112");
    p.RegistrarNota(3, 80.0f, Dificuldade::Nivel::Normal);

    CHECK(p.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).nota == doctest::Approx(50.0f));
    CHECK(p.Recorde(3, Progresso::Modo::Solo, Dificuldade::Nivel::Normal).nota == doctest::Approx(80.0f));
}

TEST_CASE("Progresso: passar e passar, sozinho ou acompanhado") {

    // D10. Quem passou em dupla destranca a proxima materia igual a quem passou
    // sozinho - e MelhorNota, que e o que o mapa do curso le, devolve a maior
    // das duas.
    Progresso emDupla;
    emDupla.RegistrarNotaDeDupla(3, 75.0f, Dificuldade::Nivel::Normal, "90112");

    CHECK(emDupla.Aprovado(3));
    CHECK(emDupla.MelhorNota(3) == doctest::Approx(75.0f));
    CHECK(emDupla.Recorde(3, Progresso::Modo::Solo, Dificuldade::Nivel::Normal).nota == doctest::Approx(0.0f));

    Progresso sozinho;
    sozinho.RegistrarNota(3, 75.0f, Dificuldade::Nivel::Normal);

    CHECK(sozinho.Aprovado(3));
    CHECK(sozinho.MelhorNota(3) == doctest::Approx(75.0f));
}

TEST_CASE("Progresso: o recorde de dupla so sobe, e o parceiro acompanha a nota") {

    // O PARCEIRO NAO PODE FICAR PARA TRAS. Se o nome ficasse do primeiro
    // recorde e a nota subisse com outra pessoa, o ranking de duplas mostraria
    // uma dupla que nunca fez aquele numero.
    Progresso p;
    p.RegistrarNotaDeDupla(3, 60.0f, Dificuldade::Nivel::Normal, "90112");
    CHECK(p.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).parceiro == "90112");

    // Melhor, com outra pessoa: a nota E o nome mudam.
    p.RegistrarNotaDeDupla(3, 85.0f, Dificuldade::Nivel::Normal, "77777");
    CHECK(p.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).nota == doctest::Approx(85.0f));
    CHECK(p.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).parceiro == "77777");

    // Pior, com uma terceira: nada muda.
    p.RegistrarNotaDeDupla(3, 40.0f, Dificuldade::Nivel::Normal, "11111");
    CHECK(p.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).nota == doctest::Approx(85.0f));
    CHECK(p.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).parceiro == "77777");
}

TEST_CASE("Progresso: a retomada e a data sao da MATERIA, nao do modo") {

    // "Quando este aluno jogou esta materia pela ultima vez" e a mesma pergunta
    // tenha ele jogado sozinho ou acompanhado, e o professor quer a resposta
    // das duas. Por isso a batalha em dupla tambem move a retomada.
    Progresso p;
    p.RegistrarNota(3, 70.0f, Dificuldade::Nivel::Normal, "2026-10-01 10:00:00");
    p.RegistrarNotaDeDupla(3, 20.0f, Dificuldade::Nivel::Normal, "90112", "2026-10-14 15:00:00");

    CHECK(Ajudantes::UltimaNota(p, 3) == doctest::Approx(20.0f));

    const auto entradas = p.Entradas();
    REQUIRE(entradas.size() == 1);
    CHECK(entradas[0].quando == "2026-10-14 15:00:00");
}

TEST_CASE("Progresso: a data do recorde de dupla e a do RECORDE, e nao a ultima") {

    // Sao duas perguntas diferentes, e por isso dois campos: "quando" responde
    // "quem jogou esta semana", e "quandoDaDupla" data a linha do ranking.
    Progresso p;
    p.RegistrarNotaDeDupla(3, 85.0f, Dificuldade::Nivel::Normal, "90112", "2026-10-01 10:00:00");
    p.RegistrarNotaDeDupla(3, 30.0f, Dificuldade::Nivel::Normal, "77777", "2026-10-14 15:00:00");

    const auto entradas = p.Entradas();
    REQUIRE(entradas.size() == 1);

    CHECK(entradas[0].De(Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).quando == "2026-10-01 10:00:00");   // o recorde
    CHECK(entradas[0].quando == "2026-10-14 15:00:00");          // a ultima vez
}

TEST_CASE("Progresso: parceiro visitante e um parceiro vazio, e isso nao e erro") {

    // D7 permite visitante no co-op. O recorde vale para o progresso e para o
    // desbloqueio; a dupla e que fica de fora do ranking, por nao ter nome.
    Progresso p;
    p.RegistrarNotaDeDupla(3, 90.0f, Dificuldade::Nivel::Normal, "");

    CHECK(p.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).nota == doctest::Approx(90.0f));
    CHECK(p.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).parceiro.empty());
    CHECK(p.Aprovado(3));
}

TEST_CASE("Progresso: guardar e repor conserva os seis campos") {

    // Entradas() e Restaurar() sao a ponte com o disco. Eram tres campos e
    // passaram a seis; um que nao atravessasse se perderia em silencio no
    // primeiro save.
    Progresso antes;
    antes.RegistrarNota(3, 55.0f, Dificuldade::Nivel::Normal, "2026-10-01 10:00:00");
    antes.RegistrarNotaDeDupla(3, 88.0f, Dificuldade::Nivel::Normal, "90112", "2026-10-02 11:00:00");

    Progresso depois;
    depois.Restaurar(antes.Entradas());

    CHECK(depois.Recorde(3, Progresso::Modo::Solo, Dificuldade::Nivel::Normal).nota == doctest::Approx(55.0f));
    CHECK(depois.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).nota == doctest::Approx(88.0f));
    CHECK(depois.Recorde(3, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).parceiro == "90112");
    CHECK(Ajudantes::UltimaNota(depois, 3) == doctest::Approx(88.0f));

    const auto e = depois.Entradas();
    REQUIRE(e.size() == 1);
    CHECK(e[0].quando == "2026-10-02 11:00:00");
    CHECK(e[0].De(Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).quando == "2026-10-02 11:00:00");
}

TEST_CASE("Progresso: materia nunca jogada nao tem recorde de dupla nem parceiro") {

    const Progresso p;
    CHECK(p.Recorde(7, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).nota == doctest::Approx(0.0f));
    CHECK(p.Recorde(7, Progresso::Modo::Dupla, Dificuldade::Nivel::Normal).parceiro.empty());

    // E CONSULTAR NAO CRIA REGISTRO. O getter antigo usava operator[] e fazia o
    // mapa crescer com materias nunca jogadas.
    CHECK(p.QuantasRegistradas() == 0);
}

// ---------------------------------------------------------------------------
// Os recordes por nivel (Normal e Dificil)
// ---------------------------------------------------------------------------

TEST_CASE("Progresso: cada nivel tem o proprio recorde, e passar vale em qualquer um") {
    using Dificuldade::Nivel;
    using Modo = Progresso::Modo;
    Progresso p;
    p.RegistrarNota(0, 72.0f, Nivel::Dificil);

    CHECK(p.Recorde(0, Modo::Solo, Nivel::Dificil).nota == doctest::Approx(72.0f));
    CHECK(p.Recorde(0, Modo::Solo, Nivel::Normal).nota == 0.0f);     // o Normal nao ganha a nota
    CHECK(p.MelhorNota(0) == doctest::Approx(72.0f));               // o mapa ve a maior de todas
    CHECK(p.Aprovado(0));                                           // passar no Dificil e passar

    // UMA NOTA MENOR NO OUTRO NIVEL NAO MEXE NESTE.
    p.RegistrarNota(0, 50.0f, Nivel::Normal);
    CHECK(p.Recorde(0, Modo::Solo, Nivel::Dificil).nota == doctest::Approx(72.0f));
    CHECK(p.Recorde(0, Modo::Solo, Nivel::Normal).nota == doctest::Approx(50.0f));
}

TEST_CASE("Progresso: a dupla do Dificil guarda o proprio parceiro") {
    using Dificuldade::Nivel;
    using Modo = Progresso::Modo;
    Progresso p;
    p.RegistrarNotaDeDupla(1, 80.0f, Nivel::Normal, "11111", "2026-11-01 10:00:00");
    p.RegistrarNotaDeDupla(1, 65.0f, Nivel::Dificil, "22222", "2026-11-02 10:00:00");

    CHECK(p.Recorde(1, Modo::Dupla, Nivel::Normal).parceiro == "11111");
    CHECK(p.Recorde(1, Modo::Dupla, Nivel::Dificil).parceiro == "22222");
    CHECK(p.Recorde(1, Modo::Dupla, Nivel::Dificil).quando == "2026-11-02 10:00:00");
    CHECK(p.Recorde(1, Modo::Solo, Nivel::Dificil).nota == 0.0f);

    // A RETOMADA E UMA SO: a ultima tentativa, em qualquer modo e nivel.
    const auto entradas = p.Entradas();
    REQUIRE(entradas.size() == 1u);
    CHECK(entradas[0].retomada == doctest::Approx(65.0f));
}
