// A matricula do aluno.

#include "doctest.h"

#include <string>

#include "../Source/Aluno/Matricula.h"

using Matricula::Validar;
using Matricula::Digitar;
using Matricula::Apagar;
using Matricula::Erro;

// O que e uma matricula valida.

TEST_CASE("Matricula: as duas matriculas reais do enunciado passam") {
    // 89384 tem cinco digitos, 115849 tem seis.
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
    // Do ponto de vista de quem digitou, um espaco no meio e o mesmo caso de uma letra.
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

// Forma canonica: o mesmo aluno tem de cair sempre na mesma ficha.

TEST_CASE("Matricula: zeros a esquerda nao criam um aluno novo") {
    // E a razao de existir uma forma canonica.
    CHECK(Validar("089384").canonica == "89384");
    CHECK(Validar("0089384").canonica == "89384");
    CHECK(Validar("89384").canonica == "89384");
    CHECK(Validar("089384").canonica == Validar("89384").canonica);
}

TEST_CASE("Matricula: zeros a esquerda contam para o limite de seis digitos") {
    // "0089384" tem sete caracteres mas cinco digitos significativos.
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
    // Decisao minha, nao regra da UFV.
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

// Mensagens.

TEST_CASE("Matricula: cada erro tem uma frase propria, e o acerto nao tem frase") {
    CHECK(Matricula::MensagemDeErro(Erro::Nenhum).empty());

    const Erro erros[] = {Erro::Vazia, Erro::NaoSoNumeros, Erro::LongaDemais, Erro::Zero};
    for (const Erro e : erros) {
        CAPTURE(static_cast<int>(e));
        CHECK_FALSE(Matricula::MensagemDeErro(e).empty());
    }

    // E as frases sao diferentes entre si.
    CHECK(Matricula::MensagemDeErro(Erro::Vazia) != Matricula::MensagemDeErro(Erro::NaoSoNumeros));
    CHECK(Matricula::MensagemDeErro(Erro::LongaDemais) != Matricula::MensagemDeErro(Erro::Zero));
}

// Digitacao: a regra vive aqui, e nao na tela.

TEST_CASE("Matricula: digitar um numero acrescenta") {
    CHECK(Digitar("", '8') == "8");
    CHECK(Digitar("893", '8') == "8938");
}

TEST_CASE("Matricula: tecla que nao e numero nao muda nada") {
    // Devolve o texto igual em vez de um bool.
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
    // A propriedade que liga as duas metades deste arquivo.
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

// Exibicao.

TEST_CASE("Matricula: ParaExibir devolve a canonica") {
    CHECK(Matricula::ParaExibir("89384") == "89384");
    CHECK(Matricula::ParaExibir(Validar("089384").canonica) == "89384");
}
