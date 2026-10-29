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

std::string ParceiroDe(const std::vector<Dados>& sessoes, const size_t posicao) {

    // So uma DUPLA tem parceiro. Ver o cabecalho para os tres casos vazios.
    if (sessoes.size() != 2 || posicao >= sessoes.size()) return {};

    const std::string& outra = sessoes[1 - posicao].matricula;

    // O MESMO TESTE DE Game::ProgressoEGravado: por Matricula::Validar, e nao
    // por comparacao com o rotulo "visitante". Assim qualquer valor que nao
    // seja uma matricula de verdade - inclusive a string vazia do jogo
    // recem-aberto - cai do lado certo, e sem uma segunda lista de valores
    // especiais para alguem esquecer de atualizar.
    return Matricula::Validar(outra).valida ? outra : std::string{};
}

Passo Proximo(const Modo modo, const size_t posicaoQueTerminou) {
    return posicaoQueTerminou + 1 < Quantos(modo) ? Passo::OutroJogador : Passo::Jogar;
}

} // namespace Sessao
