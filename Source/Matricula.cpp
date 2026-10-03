//
// A matricula do aluno: o que e uma matricula valida e como ela e digitada.
//

#include "Matricula.h"

namespace {

    bool EhDigito(const char c) { return c >= '0' && c <= '9'; }

    bool EhEspaco(const char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
    }

    /// Tira espaco so das PONTAS. Espaco no meio continua sendo um caractere
    /// invalido - ver o comentario em Matricula::Validar.
    std::string Aparar(const std::string& s) {
        size_t ini = 0;
        while (ini < s.size() && EhEspaco(s[ini])) ++ini;

        size_t fim = s.size();
        while (fim > ini && EhEspaco(s[fim - 1])) --fim;

        return s.substr(ini, fim - ini);
    }

    /// Remove zeros a esquerda. "000" vira vazio de proposito: quem chama usa isso
    /// para distinguir "so zeros" de uma matricula de verdade.
    std::string SemZerosAEsquerda(const std::string& s) {
        size_t i = 0;
        while (i < s.size() && s[i] == '0') ++i;
        return s.substr(i);
    }
}

namespace Matricula {

Resultado Validar(const std::string& digitada) {

    Resultado r;

    const std::string texto = Aparar(digitada);

    if (texto.empty()) {
        r.erro = Erro::Vazia;
        return r;
    }

    for (const char c : texto) {
        if (!EhDigito(c)) {
            r.erro = Erro::NaoSoNumeros;
            return r;
        }
    }

    // Os zeros saem ANTES da conta de tamanho, e nao depois: "0089384" tem sete
    // caracteres e cinco digitos significativos, e e uma matricula que existe -
    // recusar seria recusar por causa de enfeite. Ja "01234567" continua longa
    // demais, porque sobram sete digitos de verdade.
    const std::string canonica = SemZerosAEsquerda(texto);

    if (canonica.empty()) {
        r.erro = Erro::Zero;
        return r;
    }

    if (canonica.size() > static_cast<size_t>(kMaximoDeDigitos)) {
        r.erro = Erro::LongaDemais;
        return r;
    }

    r.valida = true;
    r.canonica = canonica;
    r.erro = Erro::Nenhum;
    return r;
}

std::string MensagemDeErro(const Erro erro) {
    // As frases dizem o que FAZER, e nao so o que esta errado: quem le isto esta
    // na frente da maquina querendo jogar, nao depurando.
    switch (erro) {
        case Erro::Nenhum:       return "";
        case Erro::Vazia:        return "Digite sua matricula para continuar.";
        case Erro::NaoSoNumeros: return "A matricula tem so numeros, sem letras nem espacos.";
        case Erro::LongaDemais:  return "A matricula tem no maximo seis numeros.";
        // NAO diz "essa matricula nao existe": o jogo nao tem a lista de alunos e
        // nao tem como saber disso. O que ele sabe e a regra de formato.
        case Erro::Zero:         return "A matricula nao pode ser so zeros.";
    }
    return "";
}

std::string Digitar(const std::string& atual, const char tecla) {

    if (!EhDigito(tecla)) return atual;

    // O limite e sobre o que esta na tela. Quem digita nao tem como escrever zeros
    // a esquerda alem do sexto caractere, entao a digitacao e um pouco mais
    // restrita que a validacao - e tudo bem: todo texto digitavel e valido, que e
    // a propriedade conferida em tests/test_matricula.cpp.
    if (atual.size() >= static_cast<size_t>(kMaximoDeDigitos)) return atual;

    return atual + tecla;
}

std::string Apagar(const std::string& atual) {
    if (atual.empty()) return atual;
    return atual.substr(0, atual.size() - 1);
}

std::string ParaExibir(const std::string& canonica) {
    return canonica;
}

}
