//
// Os tipos de projetil de cada chefe, lidos de texto JSON.
//

#pragma once

#include <map>
#include <string>
#include <vector>

/**
 * @brief Uma animacao registrada no projetil: um nome e os quadros da folha.
 *
 * Os quadros sao INDICES na lista "frames" do atlas do sprite, na ordem em que
 * devem tocar. Repetir um indice e legitimo (ida e volta), e por isso a lista
 * nao e um conjunto.
 */
struct AnimacaoDeProjetil {
    std::string nome;
    std::vector<int> quadros;
};

/**
 * @brief Tudo que distingue um tipo de projetil de outro.
 *
 * Esta struct existe porque as seis fabricas de projetil do jogo eram seis
 * copias do MESMO codigo - pool, Acquire, Release, Prewarm, cerca de cem linhas
 * cada - diferindo apenas nos campos abaixo. Cada copia era uma chance de
 * esquecer uma linha, e tres delas esqueceram (uma nao escolhia animacao
 * inicial, uma errou o caminho do atlas, duas sombreavam os membros da base).
 *
 * Com a descricao em dados, acrescentar um tipo de projetil a um chefe passou a
 * ser uma entrada em Assets/Attacks/projeteis.json - nenhum arquivo .cpp novo.
 */
struct DescricaoDeProjetil {

    /// Caminhos RELATIVOS a pasta Assets. Quem resolve para caminho absoluto e a
    /// ponte, porque so ela conhece Caminhos::Asset.
    std::string sprite;
    std::string dados;

    /// Multiplica o sprite E o raio do colisor (CircleColliderComponent usa
    /// GetScale), entao mexer aqui mexe na hitbox - nao e so aparencia.
    float escala = 1.0f;

    /// Ordem de desenho do DrawAnimatedComponent. O padrao 100 e o da propria
    /// classe; cinco das seis fabricas passavam 90 e uma omitia, e essa
    /// diferenca de camada entre os projeteis do Andre e os outros e de verdade,
    /// entao e descrita em vez de uniformizada por conta.
    int ordemDeDesenho = 100;

    std::vector<AnimacaoDeProjetil> animacoes;

    /// Qual animacao o projetil mostra ao nascer. NUNCA pode ficar vazia: um
    /// projetil criado pelo Prewarm vai direto para o pool e pode ser desenhado
    /// antes de qualquer ataque escolher a animacao dele. Quando o arquivo nao
    /// diz, o leitor resolve para a PRIMEIRA animacao da lista.
    std::string animacaoInicial;

    /// Qual medida do sprite vira o raio do colisor: "largura" ou "altura".
    /// Parece detalhe, mas o projetil de lista duplamente encadeada e mais largo
    /// que alto, e usar a largura dele daria uma hitbox bem maior que o desenho.
    std::string colisorDimensao = "largura";

    /// O raio e dimensao/divisor. Os baloes do Andre usam 4 (hitbox menor que o
    /// desenho, de proposito: o balao e grande e seria injusto).
    float colisorDivisor = 2.0f;

    /// Se o projetil nasce na posicao do dono. Quatro fabricas faziam isso e
    /// duas nao, e a diferenca nao e visivel em jogo porque toda strategy chama
    /// SetPosition logo depois - mas e comportamento existente, e migracao nao e
    /// hora de mudar comportamento.
    bool posicionarNoDono = true;

    /// Quanta folga o projetil tem alem da borda antes de morrer, em multiplos do
    /// tamanho do sprite e em fracao da tela (margem = sprite*emSprites +
    /// tela/divisorDeTela; divisor 0 remove a parcela de tela).
    ///
    /// Os padroes sao a regra geral do jogo. Os baloes do Andre usam 2 e 0, e
    /// eram o unico motivo pelo qual existia uma subclasse de projetil so para
    /// eles - a diferenca era de numero, nao de comportamento.
    float margemEmSprites = 1.0f;
    float margemDivisorDeTela = 12.0f;
};

/**
 * @brief O resultado da leitura: um mapa de conjuntos e a lista de problemas.
 *
 * Os conjuntos sao indexados pelo nome do chefe, igual a fases.json, e dentro de
 * cada um os projeteis pelo nome que o campo "projetil" de um ataque usa. Com
 * isso dois chefes podem ter um projetil "Normal" sem colidir.
 */
struct ProjeteisLidos {
    std::map<std::string, std::map<std::string, DescricaoDeProjetil>> conjuntos;
    std::vector<std::string> problemas;
};

/**
 * @brief Interpreta o texto de projeteis.json.
 *
 * Nunca lanca e nunca abre arquivo: recebe o texto e devolve o que conseguiu ler
 * mais a lista do que estava errado. Uma entrada com problema GRAVE (sem sprite,
 * sem animacao) e descartada, porque um projetil sem desenho nao tem como
 * aparecer; um problema LEVE (divisor invalido, dimensao desconhecida) vira
 * problema relatado e o campo volta ao padrao.
 */
ProjeteisLidos LerProjeteis(const std::string& textoJson);

/// Se o nome serve para DescricaoDeProjetil::colisorDimensao.
bool DimensaoDeColisorExiste(const std::string& nome);
