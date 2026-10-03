// A matricula do aluno. Camada pura: nao abre arquivo, nao sobe SDL, so texto.
//
// Escrito ANTES da implementacao, de proposito: um teste escrito depois tende a
// descrever o que o codigo faz, e nao o que ele deveria fazer. Os casos abaixo
// sairam da conversa sobre o formato ("so numeros, ate 6, podem ter menos") e das
// duas matriculas reais que me deram de exemplo.

#include "doctest.h"

#include <string>

#include "../Source/Matricula.h"

using Matricula::Validar;
using Matricula::Digitar;
using Matricula::Apagar;
using Matricula::Erro;

// ---------------------------------------------------------------------------
// O que e uma matricula valida
// ---------------------------------------------------------------------------

TEST_CASE("Matricula: as duas matriculas reais do enunciado passam") {
    // 89384 tem cinco digitos, 115849 tem seis. Sao as que me deram como exemplo,
    // e estao aqui para que nenhuma regra futura as recuse por acidente.
    const auto a = Validar("89384");
    CHECK(a.valida);
    CHECK(a.canonica == "89384");
    CHECK(a.erro == Erro::Nenhum);

    const auto b = Validar("115849");
    CHECK(b.valida);
    CHECK(b.canonica == "115849");
}

TEST_CASE("Matricula: de um a seis digitos") {
    CHECK(Validar("1").valida);
    CHECK(Validar("12").valida);
    CHECK(Validar("123456").valida);
}

TEST_CASE("Matricula: sete digitos nao") {
    const auto r = Validar("1234567");
    CHECK_FALSE(r.valida);
    CHECK(r.erro == Erro::LongaDemais);
}

TEST_CASE("Matricula: vazia nao") {
    CHECK_FALSE(Validar("").valida);
    CHECK(Validar("").erro == Erro::Vazia);
}

TEST_CASE("Matricula: so espacos conta como vazia, nao como caractere invalido") {
    // A mensagem importa: quem apertou espaco sem querer precisa ler "digite sua
    // matricula", e nao "so numeros".
    const auto r = Validar("   ");
    CHECK_FALSE(r.valida);
    CHECK(r.erro == Erro::Vazia);
}

TEST_CASE("Matricula: letra, pontuacao e sinal nao") {
    for (const char* ruim : {"8938a", "abc", "89.384", "-89384", "+1", "89384!"}) {
        CAPTURE(ruim);
        const auto r = Validar(ruim);
        CHECK_FALSE(r.valida);
        CHECK(r.erro == Erro::NaoSoNumeros);
    }
}

TEST_CASE("Matricula: espaco NO MEIO nao") {
    // Do ponto de vista de quem digitou, um espaco no meio e o mesmo caso de uma
    // letra: tem coisa que nao e numero ali.
    const auto r = Validar("89 384");
    CHECK_FALSE(r.valida);
    CHECK(r.erro == Erro::NaoSoNumeros);
}

TEST_CASE("Matricula: espaco antes e depois e aparado") {
    // Colar de algum lugar costuma trazer espaco junto.
    CHECK(Validar("  89384  ").valida);
    CHECK(Validar("  89384  ").canonica == "89384");
    CHECK(Validar("\t89384\n").canonica == "89384");
}

// ---------------------------------------------------------------------------
// Forma canonica: o mesmo aluno tem de cair sempre na mesma ficha
// ---------------------------------------------------------------------------

TEST_CASE("Matricula: zeros a esquerda nao criam um aluno novo") {
    // E a razao de existir uma forma canonica. Sem isto, o aluno que um dia digita
    // 089384 e no outro 89384 teria duas fichas e perderia o progresso.
    CHECK(Validar("089384").canonica == "89384");
    CHECK(Validar("0089384").canonica == "89384");
    CHECK(Validar("89384").canonica == "89384");
    CHECK(Validar("089384").canonica == Validar("89384").canonica);
}

TEST_CASE("Matricula: zeros a esquerda contam para o limite de seis digitos") {
    // "0089384" tem sete caracteres mas cinco digitos significativos. Aceito:
    // quem digitou escreveu uma matricula que existe, so com enfeite na frente.
    const auto r = Validar("0089384");
    CHECK(r.valida);
    CHECK(r.canonica == "89384");
}

TEST_CASE("Matricula: sete digitos significativos continuam longos demais") {
    // O complemento do caso acima: o limite vale sobre o numero, nao sobre o texto.
    const auto r = Validar("01234567");
    CHECK_FALSE(r.valida);
    CHECK(r.erro == Erro::LongaDemais);
}

TEST_CASE("Matricula: so zeros e recusada") {
    // DECISAO MINHA, nao regra da UFV: a canonica de "000" seria "0", e matricula
    // zero nao identifica ninguem. Aceitar faria "apertei zero sem querer" virar
    // uma ficha indistinguivel de uma real. Se existir matricula 0, este e o unico
    // teste a tirar.
    for (const char* z : {"0", "00", "000000"}) {
        CAPTURE(z);
        const auto r = Validar(z);
        CHECK_FALSE(r.valida);
        CHECK(r.erro == Erro::Zero);
    }
}

TEST_CASE("Matricula: zero no MEIO ou no fim e normal") {
    CHECK(Validar("10203").valida);
    CHECK(Validar("10203").canonica == "10203");
    CHECK(Validar("89380").canonica == "89380");
}

TEST_CASE("Matricula: o visitante nunca e uma matricula valida") {
    // Se fosse, ele colidiria com um aluno e dividiriam a mesma ficha.
    CHECK_FALSE(Validar(Matricula::kVisitante).valida);
}

// ---------------------------------------------------------------------------
// Mensagens
// ---------------------------------------------------------------------------

TEST_CASE("Matricula: cada erro tem uma frase propria, e o acerto nao tem frase") {
    CHECK(Matricula::MensagemDeErro(Erro::Nenhum).empty());

    const Erro erros[] = {Erro::Vazia, Erro::NaoSoNumeros, Erro::LongaDemais, Erro::Zero};
    for (const Erro e : erros) {
        CAPTURE(static_cast<int>(e));
        CHECK_FALSE(Matricula::MensagemDeErro(e).empty());
    }

    // E as frases sao DIFERENTES entre si: quatro erros com a mesma frase nao
    // ajudam mais que um bool.
    CHECK(Matricula::MensagemDeErro(Erro::Vazia) != Matricula::MensagemDeErro(Erro::NaoSoNumeros));
    CHECK(Matricula::MensagemDeErro(Erro::LongaDemais) != Matricula::MensagemDeErro(Erro::Zero));
}

// ---------------------------------------------------------------------------
// Digitacao: a regra vive aqui, e nao na tela
// ---------------------------------------------------------------------------

TEST_CASE("Matricula: digitar um numero acrescenta") {
    CHECK(Digitar("", '8') == "8");
    CHECK(Digitar("893", '8') == "8938");
}

TEST_CASE("Matricula: tecla que nao e numero nao muda nada") {
    // Devolve o texto igual em vez de um bool: assim a tela so escreve o que
    // recebeu, sem decidir nada.
    CHECK(Digitar("893", 'a') == "893");
    CHECK(Digitar("893", ' ') == "893");
    CHECK(Digitar("893", '.') == "893");
    CHECK(Digitar("893", '\n') == "893");
}

TEST_CASE("Matricula: nao da para digitar alem do sexto digito") {
    // E o que impede o aluno de escrever uma matricula invalida: ele nem consegue.
    CHECK(Digitar("123456", '7') == "123456");
    CHECK(Digitar("12345", '6') == "123456");
}

TEST_CASE("Matricula: apagar tira o ultimo, e vazio continua vazio") {
    CHECK(Apagar("8938") == "893");
    CHECK(Apagar("8") == "");
    CHECK(Apagar("") == "");
}

TEST_CASE("Matricula: o que da para digitar e sempre aceito pela validacao") {
    // A propriedade que liga as duas metades deste arquivo: se a digitacao so
    // deixa entrar o que vale, nao existe texto alcancavel pelo teclado que a
    // validacao recuse por formato. Sobram so os dois casos que o teclado nao
    // evita - nao digitar nada, e digitar so zeros.
    std::string texto;
    const char* sequencia = "a8 9.3b8\n4!";   // mistura de boas e ruins
    for (const char* c = sequencia; *c; ++c) {
        texto = Digitar(texto, *c);
        if (texto.empty()) continue;
        const auto r = Validar(texto);
        CAPTURE(texto);
        CHECK(r.valida);
    }
    CHECK(texto == "89384");
}

TEST_CASE("Matricula: digitar seis digitos quaisquer sempre da matricula valida") {
    std::string texto;
    for (char c = '1'; c <= '6'; ++c) texto = Digitar(texto, c);
    CHECK(texto == "123456");
    CHECK(Validar(texto).valida);
}

// ---------------------------------------------------------------------------
// Exibicao
// ---------------------------------------------------------------------------

TEST_CASE("Matricula: ParaExibir devolve a canonica") {
    CHECK(Matricula::ParaExibir("89384") == "89384");
    CHECK(Matricula::ParaExibir(Validar("089384").canonica) == "89384");
}
