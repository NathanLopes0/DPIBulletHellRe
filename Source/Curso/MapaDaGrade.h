//
// Onde fica cada coluna da grade na selecao de fases, e em que pagina.
//
#pragma once

#include <vector>

/// A grade e um mapa mais largo que a tela: as obrigatorias ocupam a primeira pagina, exatamente como
/// sempre ocuparam, e as optativas continuam o corredor para a direita, uma pagina por vez. Puro: so
/// contas de posicao; quem move os botoes e a StageSelect.
namespace MapaDaGrade {

    constexpr float kLarguraDoBotao = 128.0f;

    /// O x de uma coluna, ja na pagina dela (a pagina p comeca em p * largura da tela).
    struct Coluna {
        float x = 0.0f;
        int pagina = 0;
    };

    /// As 'total' colunas numa tela de 'largura'. As 'obrigatorias' primeiras dividem a tela entre as
    /// margens (1/12 de cada lado), como antes. As outras sao as optativas: vao para as paginas
    /// seguintes, nos lugares das colunas 1 em diante da primeira - o lugar da coluna 0 fica livre,
    /// porque e onde o cartao da materia esta.
    std::vector<Coluna> Colunas(int obrigatorias, int total, float largura);

    /// Quantas optativas cabem numa pagina.
    int PorPagina(int obrigatorias);

    /// Quantas paginas o mapa tem (pelo menos uma).
    int Paginas(const std::vector<Coluna>& colunas);

    /// O deslocamento horizontal que mostra a pagina 'pagina'.
    float DeslocamentoDa(int pagina, float largura);

    /// Um passo da camera rumo ao alvo: chega depressa no comeco e devagar no fim, e encosta quando
    /// falta menos de meio pixel.
    float Aproximar(float atual, float alvo, float deltaTime);

    /// Quanto da distancia a camera anda por segundo (a taxa da exponencial).
    constexpr float kRapidez = 9.0f;

    // ---- As ligacoes que acendem ----

    /// Um pedaco das ligacoes entre duas colunas: o tronco (botao -1), ou o ramo de um botao ate ele.
    /// 'tronco' e o numero do par de colunas, igual no tronco e nos ramos dele.
    struct Trecho {
        int tronco = 0;
        int botao = -1;
    };

    /// Um trecho que acende, e quantos segundos depois da troca de foco.
    struct Aceso {
        size_t trecho = 0;
        float atraso = 0.0f;
    };

    /// O caminho que acende quando o foco vai de 'anterior' para 'novo' (anterior -1: o primeiro
    /// foco, ao abrir a tela). Se os dois tocam o mesmo tronco, a luz corre de um ao outro: o ramo
    /// de quem saiu, o tronco, e o ramo de quem chegou, um 'passo' depois do outro. Se nao, acendem
    /// as ligacoes do novo, do ramo para o tronco. Um tronco que os dois tocam dos dois lados (a
    /// mesma coluna) e o da direita, rumo ao que vem depois.
    std::vector<Aceso> Caminho(const std::vector<Trecho>& trechos, int anterior, int novo, float passo);

    /// Os segundos entre um trecho acender e o seguinte.
    constexpr float kPassoDaLuz = 0.07f;
}
