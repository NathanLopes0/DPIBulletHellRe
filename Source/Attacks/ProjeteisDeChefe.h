//
// Os tipos de projetil de cada chefe, lidos de texto JSON.
//

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"

/// Uma animacao registrada no projetil: um nome e os quadros da folha.
struct AnimacaoDeProjetil {
    std::string nome;
    std::vector<int> quadros;
};

/// Tudo que distingue um tipo de projetil de outro.
struct DescricaoDeProjetil {

    /// Caminhos relativos a pasta Assets. Quem resolve para caminho absoluto e a ponte, porque so
    /// ela conhece Caminhos::Asset.
    std::string sprite;
    std::string dados;

    /// Multiplica o sprite E o raio do colisor (CircleColliderComponent usa
    /// GetScale), entao mexer aqui mexe na hitbox - nao e so aparencia.
    float escala = 1.0f;

    /// Ordem de desenho do DrawAnimatedComponent.
    int ordemDeDesenho = 100;

    std::vector<AnimacaoDeProjetil> animacoes;

    /// Qual animacao o projetil mostra ao nascer.
    std::string animacaoInicial;

    /// Qual medida do sprite vira o raio do colisor: "largura" ou "altura".
    std::string colisorDimensao = "largura";

    /// O raio e dimensao/divisor. Os baloes do Andre usam 4 (hitbox menor que o desenho: o balao e
    /// grande e seria injusto).
    float colisorDivisor = 2.0f;

    /// Se o projetil nasce na posicao do dono.
    bool posicionarNoDono = true;

    /// Os padroes sao a regra geral do jogo.
    float margemEmSprites = 1.0f;
    float margemDivisorDeTela = 12.0f;

    /// Se o sprite gira para acompanhar a direcao em que o projetil viaja. Existe para as listas do
    /// Salles: a arte delas e uma caixa com uma seta.
    bool rotacionarComAVelocidade = false;

    /// O que acontece com o aluno quando este projetil acerta.
    struct Acorrentamento {
        float segundos = 0.0f;              ///< 0 = nao acorrenta
        float fatorDeVelocidade = 1.0f;     ///< multiplica a velocidade do aluno
        float multiplicadorDaPerda = 1.0f;  ///< 1 = a perda de nota de sempre
    };

    Acorrentamento acorrenta;

    /// O tiro do aluno pode derrubar este projetil? O cursor e os workers do Lucas, e os bugs, as
    /// historias e as janelas da Maria Lucia.
    bool derrubavel = false;

    /// Quantos tiros derrubam este projetil, se ele for derrubavel.
    int resistencia = 1;

    /// O bloco "dificil": a resistencia do modo Dificil, quando ele a troca.
    std::optional<int> resistenciaNoDificil;

    /// Este projetil e um abrigo: os projeteis do chefe que encostam nele somem, ele nao machuca o
    /// aluno, e o tiro do aluno some nele.
    bool abrigo = false;
};

/// O resultado da leitura: os projeteis de um chefe e a lista de problemas. Indexados pelo nome que
/// o campo "projetil" de um ataque usa.
struct ProjeteisLidos {
    std::map<std::string, DescricaoDeProjetil> projeteis;
    std::vector<std::string> problemas;
};

/// Interpreta o texto do projeteis.json de um chefe.
ProjeteisLidos LerProjeteis(const std::string& textoJson);

/// O projetil no modo: no Dificil, com o que o bloco "dificil" troca; no Normal (ou sem bloco),
/// igual.
DescricaoDeProjetil NoNivel(const DescricaoDeProjetil& p, Dificuldade::Nivel n);

/// Se o nome serve para DescricaoDeProjetil::colisorDimensao.
bool DimensaoDeColisorExiste(const std::string& nome);
