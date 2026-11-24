//
// As fases 1 e 2 do Lucas (INF 321 - Programacao Web): a <div> que as regras CSS e JavaScript mudam.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// O professor poe uma <div> no campo - pequena e previsivel, quicando nas bordas - e joga regras
/// nela. Cada regra e um projetil que voa para um ponto do campo: o tiro do aluno a derruba; se
/// acertar o aluno, machuca como qualquer tiro e ainda pesa nele; e se acertar a <div>, muda a
/// <div>, e as mudancas se somam. Na fase 1 as regras sao de CSS (a forma: largura, altura, a
/// grade, o display: none...); na fase 2, de JavaScript (o comportamento: o setInterval que atira,
/// o onclick que revida, o mousemove que segue, o cloneNode...). Umas pioram a <div>, outras a
/// aliviam - e o aluno escolhe o que derrubar, ou se foca no professor.
namespace Dom {

    constexpr const char* kFaseDoCss = "StateOne";
    constexpr const char* kFaseDoJs = "StateTwo";

    struct Campo {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
    };

    // ---- A <div> ----

    constexpr float kLarguraInicial = 72.0f;
    constexpr float kAlturaInicial = 48.0f;
    constexpr float kMenorLado = 28.0f;
    constexpr float kMaiorLargura = 360.0f;
    constexpr float kMaiorAltura = 240.0f;
    constexpr float kMaiorRapidez = 420.0f;
    /// display: grid: as quatro caixas, com este vao entre elas.
    constexpr float kVaoDaGrade = 18.0f;
    /// cloneNode: nunca mais que isto na tela.
    constexpr int kMaisDivs = 3;

    struct Div {
        Vector2 centro;
        Vector2 velocidade;
        float largura = kLarguraInicial;
        float altura = kAlturaInicial;
        bool redonda = false;
        bool grade = false;
        float opacidade = 1.0f;
        /// display: none e div.remove(): some ate este instante.
        float escondidaAte = -1.0f;
        /// position: fixed: parada ate este instante.
        float paradaAte = -1.0f;
        // O comportamento (JavaScript).
        float intervalo = 0.0f;        ///< setInterval: um anel a cada tantos segundos (0: nenhum)
        float proximoTiro = 0.0f;
        bool revida = false;           ///< onclick: o tiro do aluno nela volta como leque
        float proximoRevide = 0.0f;
        bool segue = false;            ///< mousemove: vai atras do aluno
        float teleporteACada = 0.0f;   ///< Math.random(): salta para um lugar sorteado
        float proximoTeleporte = 0.0f;
        float bombaEm = -1.0f;         ///< setTimeout: explode neste instante
        /// Quantas regras ja a mudaram.
        int mudancas = 0;
    };

    /// A <div> nova: no alto do campo, indo em diagonal a 'rapidez'.
    Div Nova(const Campo& c, float rapidez, Variacao::Sorteio& s);

    [[nodiscard]] bool Visivel(const Div& d, float agora);

    /// As caixas que ela ocupa agora: uma, ou as quatro da grade. Escondida, nenhuma.
    struct Caixa {
        Vector2 centro;
        float largura = 0.0f;
        float altura = 0.0f;
        bool redonda = false;
    };
    std::vector<Caixa> Caixas(const Div& d, float agora);

    /// Se um circulo de 'raio' em 'p' toca a <div>.
    bool Toca(const Div& d, Vector2 p, float raio, float agora);

    /// Os pontos em que ficam as pecas que machucam, 'passo' pixels uma da outra, cobrindo cada
    /// caixa (a redonda, so o circulo).
    std::vector<Vector2> Pecas(const Div& d, float agora, float passo);

    /// Anda 'dt': quica nas bordas, vai atras do aluno (mousemove), salta (Math.random()), para
    /// (position: fixed).
    void Andar(Div& d, float dt, float agora, Vector2 aluno, const Campo& c, Variacao::Sorteio& s);

    // ---- As regras ----

    enum class Efeito {
        // CSS: a forma.
        Largura, Altura, Padding, Redonda, Rapida, Some, Estreita, Grade, Opaca, Fixa,
        // JavaScript: o comportamento.
        Intervalo, Revide, Segue, Aleatoria, Bomba, Remove, Clone, Acelera
    };
    constexpr Efeito kDoCss[] = {Efeito::Largura, Efeito::Altura, Efeito::Padding, Efeito::Redonda, Efeito::Rapida,
                                 Efeito::Some, Efeito::Estreita, Efeito::Grade, Efeito::Opaca, Efeito::Fixa};
    constexpr Efeito kDoJs[] = {Efeito::Intervalo, Efeito::Revide, Efeito::Segue, Efeito::Aleatoria,
                                Efeito::Bomba, Efeito::Remove, Efeito::Clone, Efeito::Acelera};

    [[nodiscard]] bool EhJs(Efeito e);
    /// As que aliviam a <div> (deixar passar compensa): display: none, width: 50%, position: fixed e
    /// div.remove().
    [[nodiscard]] bool Ajuda(Efeito e);
    /// O texto da regra, como se escreve: "width: 200%;", "setInterval(atirar, 1500);".
    const char* Texto(Efeito e);

    /// Uma regra sorteada da linguagem: as que aliviam saem com metade da chance das outras.
    Efeito Sortear(bool js, Variacao::Sorteio& s);

    /// Muda a <div> pela regra, somando ao que ja havia. Devolve true quando a batalha tem de clonar
    /// a <div> (cloneNode).
    bool Aplicar(Div& d, Efeito e, float agora);

    constexpr float kSome = 4.0f;             ///< display: none
    constexpr float kRemovida = 5.0f;         ///< div.remove(), ate o proximo render
    constexpr float kFixa = 4.0f;             ///< position: fixed
    constexpr float kPavio = 3.0f;            ///< setTimeout(explodir, 3000)
    constexpr float kEntreRevides = 0.6f;     ///< o onclick nao revida todo tiro

    /// O que a <div> atira agora - e anda os relogios dela: o setTimeout que explode, ou o anel do
    /// setInterval.
    enum class Disparo { Nenhum, Anel, Explosao };
    Disparo Atirar(Div& d, float agora);
    /// O onclick: o tiro do aluno acertou a <div>; true se ela revida agora.
    bool Revidar(Div& d, float agora);

    /// O que a regra faz no aluno que ela acerta, alem de machucar: o CSS pesa (ele anda devagar um
    /// pouco), e o JavaScript trava (o alert() que para tudo, por um instante).
    constexpr float kPesoDoCss = 1.5f;
    constexpr float kFatorDoCss = 0.6f;
    constexpr float kTravaDoJs = 0.7f;
    constexpr float kFatorDoJs = 0.2f;

    /// Para onde a regra voa: as vezes direto na <div>, as vezes num ponto sorteado da metade de
    /// baixo do campo - "aleatoriamente".
    constexpr float kChanceDeMirarADiv = 0.65f;
    Vector2 Destino(const Campo& c, const Div* alvo, Variacao::Sorteio& s);

    /// A regra mirada na <div> vai atras dela, virando no maximo kGiroDaRegra graus por segundo:
    /// faz a curva que o aluno ve chegando, e da tempo de derruba-la no caminho. Depois de
    /// kCurvaDaRegra segundos ela segue reta - a que errou nao fica rodando a <div> para sempre.
    constexpr float kGiroDaRegra = 120.0f;
    constexpr float kCurvaDaRegra = 4.0f;
    Vector2 Curvar(Vector2 velocidade, Vector2 de, Vector2 para, float giroPorSegundo, float dt);

    /// O ritmo de cada modo.
    struct Degrau {
        float calma = 3.0f;               ///< a <div> sozinha no comeco da fase
        float entreLances = 2.0f;         ///< entre um lance de regras e o seguinte
        int regrasPorLance = 2;
        float velocidadeDasRegras = 170.0f;
        float rapidezDaDiv = 110.0f;
        int tirosDoAnel = 10;             ///< setInterval
        int tirosDaExplosao = 14;         ///< setTimeout
        int tirosDoRevide = 3;            ///< onclick
        float entreTags = 2.0f;           ///< o leque do proprio professor
        int tagsPorLeque = 6;
        /// Na fase 2, quantas regras saem de CSS ainda (o resto e JavaScript).
        float chanceDeCssNoJs = 0.25f;
    };
    Degrau DegrauDo(Dificuldade::Nivel nivel);
}
