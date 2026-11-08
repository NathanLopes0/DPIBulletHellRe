//
// Para onde a selecao vai quando alguem aperta uma seta.
//

#include "Navegacao.h"

#include <algorithm>

namespace Navegacao {

namespace {

    /// As colunas que tem algum botao. Uma coluna vazia nao e destino de nada:
    /// parar numa deixaria a selecao em lugar nenhum, sem nada aceso na tela.
    std::vector<size_t> ColunasComBotao(const Grade& grade) {
        std::vector<size_t> quais;
        for (size_t c = 0; c < grade.size(); ++c) {
            if (!grade[c].empty()) quais.push_back(c);
        }
        return quais;
    }

    /// Para qual linha da coluna de destino a selecao vai.
    size_t LinhaDestino(const size_t linhaAtual, const size_t tamanhoAtual,
                        const size_t tamanhoDestino) {

        if (tamanhoDestino <= 1) return 0;

        // Saindo de uma coluna de um item so, a linha atual e sempre 0 e nao
        // significa nada - aquele botao fica centrado na tela, nao no topo.
        // Mandar para a linha 0 jogaria a selecao la para cima, longe de onde o
        // olho estava. O meio e o que fica perto.
        if (tamanhoAtual <= 1) return (tamanhoDestino - 1) / 2;

        // Mesma linha quando ela existe do outro lado; a ultima quando nao.
        return std::min(linhaAtual, tamanhoDestino - 1);
    }

    size_t AoLado(const Grade& grade, const size_t atual, const int passo) {

        size_t coluna = 0, linha = 0;
        if (!Onde(grade, atual, coluna, linha)) return atual;

        const std::vector<size_t> comBotao = ColunasComBotao(grade);
        if (comBotao.size() <= 1) return atual;

        // Onde a coluna atual esta na lista das que tem botao. Andar por esta
        // lista, e nao pelos indices de coluna, e o que faz a seta pular uma
        // coluna vazia em vez de travar nela.
        const auto aqui = std::find(comBotao.begin(), comBotao.end(), coluna);
        if (aqui == comBotao.end()) return atual;

        const auto quantas = static_cast<int>(comBotao.size());
        const auto ondeEstou = static_cast<int>(aqui - comBotao.begin());

        // A volta e dos dois lados: da primeira coluna para a ultima e vice-versa.
        const int ondeVou = ((ondeEstou + passo) % quantas + quantas) % quantas;

        const std::vector<size_t>& destino = grade[comBotao[static_cast<size_t>(ondeVou)]];
        return destino[LinhaDestino(linha, grade[coluna].size(), destino.size())];
    }
}

bool Onde(const Grade& grade, const size_t atual, size_t& coluna, size_t& linha) {

    for (size_t c = 0; c < grade.size(); ++c) {
        for (size_t l = 0; l < grade[c].size(); ++l) {
            if (grade[c][l] == atual) { coluna = c; linha = l; return true; }
        }
    }
    return false;
}

size_t Cima(const Grade& grade, const size_t atual) {

    size_t coluna = 0, linha = 0;
    if (!Onde(grade, atual, coluna, linha)) return atual;

    const std::vector<size_t>& minha = grade[coluna];
    if (minha.size() <= 1) return atual;

    return (linha == 0) ? minha.back() : minha[linha - 1];
}

size_t Baixo(const Grade& grade, const size_t atual) {

    size_t coluna = 0, linha = 0;
    if (!Onde(grade, atual, coluna, linha)) return atual;

    const std::vector<size_t>& minha = grade[coluna];
    if (minha.size() <= 1) return atual;

    return (linha + 1 >= minha.size()) ? minha.front() : minha[linha + 1];
}

size_t Esquerda(const Grade& grade, const size_t atual) { return AoLado(grade, atual, -1); }
size_t Direita (const Grade& grade, const size_t atual) { return AoLado(grade, atual, +1); }

}
