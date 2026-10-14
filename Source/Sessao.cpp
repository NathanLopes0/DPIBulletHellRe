//
// Quem esta jogando, de cada lado do painel.
//

#include "Sessao.h"

#include "Matricula.h"

namespace Sessao {

size_t Quantos(const Modo modo) {
    switch (modo) {
        case Modo::Um:   return 1;
        case Modo::Dois: return 2;
    }
    return 1;
}

bool JaEstaEmUso(const std::vector<Dados>& sessoes, const size_t posicao,
                 const std::string& matricula) {

    // Visitante nao grava, entao nao colide com ninguem - nem consigo mesmo.
    if (matricula.empty() || matricula == Matricula::kVisitante) return false;

    for (size_t i = 0; i < sessoes.size(); ++i) {
        if (i == posicao) continue;
        if (sessoes[i].matricula == matricula) return true;
    }
    return false;
}

Passo Proximo(const Modo modo, const size_t posicaoQueTerminou) {
    return posicaoQueTerminou + 1 < Quantos(modo) ? Passo::OutroJogador : Passo::Jogar;
}

} // namespace Sessao
