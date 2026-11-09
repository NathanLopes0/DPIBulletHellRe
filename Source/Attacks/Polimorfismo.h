//
// A fase 1 do Luizinho: o polimorfismo.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A mesma chamada, f.desenhar(), e cada objeto desenha a sua forma: o circulo um circulo, o
/// quadrado um quadrado. Quem decide e o tipo do objeto (o dinamico), e nao o da variavel - que, em
/// "Forma f = sortear()", so se conhece na hora da chamada.
namespace Polimorfismo {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDasFormas = "StateOne";
    constexpr const char* kGatilhoDoTraco = "traco";

    enum class Classe { Circulo, Quadrado, Triangulo, Estrela };
    constexpr int kClasses = 4;

    /// "Circulo", "Quadrado", "Triangulo", "Estrela": o nome da classe, que e tambem a cara do
    /// traco dela.
    const char* Nome(Classe c);

    /// Os vertices da forma, centrada em zero e com o vertice mais longe a 1. O circulo nao tem.
    std::vector<Vector2> Vertices(Classe c);

    /// O contorno: 'pontos' pontos espalhados por igual pelo perimetro, na escala de Vertices.
    /// Cada ponto, andando na direcao dele a uma velocidade proporcional a distancia ao centro,
    /// faz a forma crescer sem deixar de ser ela.
    std::vector<Vector2> Contorno(Classe c, int pontos);

    enum class Comando { Instancia, Laco, TipoDinamico };

    struct Chamada {
        Comando comando = Comando::Instancia;
        std::vector<Classe> objetos;   ///< na ordem em que o metodo e chamado
    };

    /// O peso de cada comando nesta luta (varia em volta do padrao).
    struct Plano {
        float pesoDaInstancia = 0.3f;
        float pesoDoLaco = 0.35f;
        float pesoDoDinamico = 0.35f;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// O proximo comando, com as classes sorteadas: um objeto na instancia e no tipo dinamico,
    /// 'noLaco' no laco, sem dois iguais seguidos.
    Chamada Proxima(std::uint32_t sorteio, int noLaco, const Plano& p = Plano{});

    /// O codigo no placar: "new Estrela().desenhar();", "for (Forma f : formas) f.desenhar();",
    /// "Forma f = sortear();" e, na linha de baixo, "f.desenhar();" - e, revelado, o tipo dinamico
    /// no comentario.
    std::string Placar(const Chamada& c, bool revelado);

    /// O rotulo do objeto 'k' em campo: "new Estrela()" na instancia, "Estrela" no laco e
    /// "Forma f = ?" no tipo dinamico, ate a chamada revelar a classe.
    std::string Rotulo(const Chamada& c, int k, bool revelado);

    /// Onde os 'n' objetos ficam: sorteados na faixa entre 'topo' e 'fundo', dentro da 'largura'
    /// menos kMargem dos lados, e a pelo menos kDistancia uns dos outros (quando cabe).
    std::vector<Vector2> Lugares(int n, std::uint32_t sorteio, float largura, float topo, float fundo);
    constexpr float kMargem = 140.0f;
    constexpr float kDistancia = 230.0f;
    /// A faixa dos objetos, medida do alto do campo: o fundo dela fica a mais de 200 px do chao.
    constexpr float kTopoDaFaixa = 240.0f;
    constexpr float kFundoDaFaixa = 540.0f;

    /// O relogio de um comando: o objeto sai do professor (kVooDoNew), espera kAntesDaChamada, o
    /// tipo dinamico ainda leva kRevelacao para mostrar a classe, o contorno se desenha em kTraco e
    /// so entao a forma parte. No laco, uma chamada a cada kEntreChamadas.
    constexpr float kPrimeiro = 1.0f;
    constexpr float kVooDoNew = 0.5f;
    constexpr float kAntesDaChamada = 0.35f;
    constexpr float kRevelacao = 0.6f;
    constexpr float kTraco = 0.45f;
    constexpr float kEntreChamadas = 0.55f;

    /// A forma: o contorno a kRaio do objeto, kPontos pontos, e o ponto mais longe a kVelocidade.
    constexpr float kRaio = 34.0f;
    constexpr int kPontos = 26;
    constexpr float kVelocidade = 190.0f;

    /// Quanto um ponto do contorno vive, do nascimento: o miolo da estrela anda devagar, e sem
    /// prazo ficaria vagando pelo campo.
    constexpr float kVidaDoTraco = 3.4f;
    constexpr int kNoLaco = 3;

    /// De um comando ao proximo: sorteado entre os dois, contando do comeco do anterior.
    constexpr float kMenorPasso = 2.0f;
    constexpr float kMaiorPasso = 2.6f;

    /// Quanto o comando dura, do comeco a forma da ultima chamada partir.
    float Duracao(const Chamada& c, float entreChamadas = kEntreChamadas);

    /// O que o Dificil muda: comandos mais seguidos, um objeto a mais no laco, formas mais cheias e
    /// mais rapidas - e que giram enquanto crescem, 'giro' graus por segundo, cada uma para um lado.
    struct Degrau {
        float menorPasso = kMenorPasso;
        float maiorPasso = kMaiorPasso;
        int noLaco = kNoLaco;
        int pontos = kPontos;
        float velocidade = kVelocidade;
        float entreChamadas = kEntreChamadas;
        float giro = 0.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
