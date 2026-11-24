//
// As fases 1 e 2 do Lucas (INF 321 - Programacao Web): a <div> que as regras CSS e JavaScript mudam.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// O professor poe uma <div> no campo - pequena e previsivel, quicando pela parte de cima - e joga
/// regras nela. A <div> ja e perigo sozinha: o corpo dela machuca, e o conteudo transborda (o
/// overflow), gotas que caem da borda de baixo de tempos em tempos. Cada regra e um projetil que voa
/// para um ponto do campo: o tiro do aluno a derruba; se acertar o aluno, machuca como qualquer tiro
/// e ainda pesa nele; e se acertar a <div>, muda a <div>, e as mudancas se somam - sempre para pior.
/// Na fase 1 as regras sao de CSS (a forma: largura, altura, a grade, o z-index que a deixa descer);
/// na fase 2 a MESMA <div> (document.querySelector) recebe as de JavaScript (o comportamento: o
/// setInterval que atira, o onclick que revida, o mousemove que segue, o cloneNode...). A <div> nunca
/// some e nunca e destruida: o aluno escolhe o que derrubar, ou se foca no professor.
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
    constexpr float kMaiorLargura = 360.0f;
    constexpr float kMaiorAltura = 240.0f;
    /// Nunca mais rapida que 80% do aluno (Player::kVelocidade, 300): a <div> que o segue e alcancavel
    /// de fugir.
    constexpr float kMaiorRapidez = 220.0f;
    /// display: grid: as quatro caixas, com este vao entre elas.
    constexpr float kVaoDaGrade = 18.0f;
    /// cloneNode: nunca mais que isto na tela.
    constexpr int kMaisDivs = 2;
    /// Sem o z-index, a <div> fica nesta fracao de cima do campo; com ele, desce ate o aluno.
    constexpr float kFaixaDaDiv = 0.62f;
    /// animation: o tamanho vai e volta ate esta fracao a mais, uma vez a cada kPeriodoDoPulso.
    constexpr float kPulso = 0.3f;
    constexpr float kPeriodoDoPulso = 1.2f;
    /// overflow: scroll encurta o intervalo do transbordo, ate este piso.
    constexpr float kMenorTransbordo = 1.8f;

    struct Div {
        Vector2 centro;
        Vector2 velocidade;
        float largura = kLarguraInicial;
        float altura = kAlturaInicial;
        // A forma (CSS).
        bool redonda = false;          ///< border-radius: o transbordo sai para todos os lados
        bool grade = false;            ///< display: grid: quatro caixas
        bool pulsa = false;            ///< animation: cresce e encolhe
        bool centraliza = false;       ///< margin: 0 auto: fica em cima do aluno
        bool desce = false;            ///< z-index: anda pelo campo inteiro
        float entreTransbordos = 2.8f; ///< o overflow: as gotas a cada tantos segundos
        float proximoTransbordo = 0.0f;
        // O comportamento (JavaScript).
        float intervalo = 0.0f;        ///< setInterval: um anel a cada tantos segundos (0: nenhum)
        float proximoTiro = 0.0f;
        float entreLeques = 0.0f;      ///< forEach: um leque mirado no aluno a cada tantos segundos
        float proximoLeque = 0.0f;
        bool revida = false;           ///< onclick: o tiro do aluno nela volta como leque
        float proximoRevide = 0.0f;
        bool segue = false;            ///< mousemove: vai atras do aluno
        bool sobre = false;            ///< onmouseover: com o aluno embaixo, ela despeja
        float proximaQueda = 0.0f;
        float teleporteACada = 0.0f;   ///< Math.random(): salta para um lugar sorteado
        float proximoTeleporte = 0.0f;
        float bombaEm = -1.0f;         ///< setTimeout: estoura neste instante (e continua la)
        float recuoAte = -1.0f;        ///< depois de acertar o aluno, ela se afasta ate aqui
        /// Quantas regras ja a mudaram.
        int mudancas = 0;
    };

    /// A <div> nova: no alto do campo, indo em diagonal a 'rapidez', transbordando a cada
    /// 'entreTransbordos' segundos (o primeiro, depois de um intervalo inteiro).
    Div Nova(const Campo& c, float rapidez, float entreTransbordos, float agora, Variacao::Sorteio& s);

    /// O tamanho de agora: o da regra, vezes o pulso da animation.
    [[nodiscard]] float Escala(const Div& d, float agora);

    /// As caixas que ela ocupa agora: uma, ou as quatro da grade. Sempre ha pelo menos uma.
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

    /// Anda 'dt': quica nas bordas da faixa dela (o campo inteiro, com o z-index), vai atras do
    /// aluno (mousemove), fica em cima dele (margin: 0 auto), salta (Math.random()).
    void Andar(Div& d, float dt, float agora, Vector2 aluno, const Campo& c, Variacao::Sorteio& s);

    /// O corpo dela acertou o aluno: por kRecuo segundos ela se afasta dele (sem seguir nem
    /// centralizar), em vez de ficar colada e acertar de novo assim que ele piscar.
    constexpr float kRecuo = 1.2f;
    void Recuar(Div& d, Vector2 aluno, float agora);

    // ---- As regras ----

    /// Todas pioram a <div> para o aluno: nenhuma a esconde, encolhe ou para.
    enum class Efeito {
        // CSS: a forma.
        Largura, Altura, Padding, Redonda, Rapida, Grade, Pulsa, Centraliza, Transborda, Desce,
        // JavaScript: o comportamento.
        Intervalo, Leque, Revide, Segue, Sobre, Aleatoria, Bomba, Clone, Acelera
    };
    constexpr Efeito kDoCss[] = {Efeito::Largura, Efeito::Altura, Efeito::Padding, Efeito::Redonda, Efeito::Rapida,
                                 Efeito::Grade, Efeito::Pulsa, Efeito::Centraliza, Efeito::Transborda, Efeito::Desce};
    constexpr Efeito kDoJs[] = {Efeito::Intervalo, Efeito::Leque, Efeito::Revide, Efeito::Segue, Efeito::Sobre,
                                Efeito::Aleatoria, Efeito::Bomba, Efeito::Clone, Efeito::Acelera};

    [[nodiscard]] bool EhJs(Efeito e);
    /// O texto da regra, como se escreve: "width: 200%;", "setInterval(atirar, 1500);".
    const char* Texto(Efeito e);

    /// Se a regra ja nao mudaria nada nesta <div>: o border-radius numa redonda, a largura no teto...
    [[nodiscard]] bool Inutil(const Div& d, Efeito e);

    /// Uma regra sorteada da linguagem. Com 'alvo', evita as que nao mudariam nada nele: o aluno
    /// ve uma regra nova quase toda vez.
    Efeito Sortear(bool js, Variacao::Sorteio& s, const Div* alvo = nullptr);

    /// Muda a <div> pela regra, somando ao que ja havia. Devolve true quando a batalha tem de clonar
    /// a <div> (cloneNode).
    bool Aplicar(Div& d, Efeito e, float agora);

    constexpr float kPavio = 3.0f;            ///< setTimeout(explodir, 3000)
    constexpr float kEntreRevides = 1.0f;     ///< o onclick nao revida todo tiro
    constexpr float kEntreQuedas = 0.9f;      ///< o onmouseover despeja de tanto em tanto

    /// O que a <div> atira neste quadro - e anda os relogios dela.
    struct Tiros {
        bool transbordo = false;   ///< o overflow (sempre)
        bool anel = false;         ///< setInterval
        bool leque = false;        ///< forEach
        bool explosao = false;     ///< setTimeout
        bool queda = false;        ///< onmouseover: o aluno esta embaixo dela
    };
    Tiros Atirar(Div& d, float agora, Vector2 aluno);
    /// O onclick: o tiro do aluno acertou a <div>; true se ela revida agora.
    bool Revidar(Div& d, float agora);

    /// O transbordo: cada gota sai da borda de baixo de cada caixa (na grade, so das duas de baixo),
    /// uma a cada kEspacoDasGotas pixels, e cai; da redonda, sai da metade de baixo, em leque. Entre
    /// as gotas sempre ha vao para o aluno passar. 'rumo' tem tamanho 1.
    struct Gota {
        Vector2 onde;
        Vector2 rumo;
    };
    constexpr float kEspacoDasGotas = 48.0f;
    std::vector<Gota> Gotas(const Div& d, float agora);
    /// Quanto antes do transbordo a borda de baixo acende - o aviso.
    constexpr float kAvisoDoTransbordo = 0.4f;

    /// O que a regra faz no aluno que ela acerta, alem de machucar: o CSS pesa (ele anda devagar um
    /// pouco), e o JavaScript trava (o alert() que para tudo, por um instante).
    constexpr float kPesoDoCss = 1.5f;
    constexpr float kFatorDoCss = 0.6f;
    constexpr float kTravaDoJs = 0.7f;
    constexpr float kFatorDoJs = 0.2f;

    /// Para onde a regra voa: as vezes direto na <div>, as vezes num ponto sorteado da metade de
    /// baixo do campo - "aleatoriamente". Os primeiros lances de cada fase vao todos na <div>: o
    /// aluno ve a mecanica antes de ter de escolher.
    constexpr float kChanceDeMirarADiv = 0.75f;
    constexpr int kLancesDeDemonstracao = 2;
    Vector2 Destino(const Campo& c, const Div* alvo, Variacao::Sorteio& s);

    /// A regra mirada na <div> vai atras dela, virando no maximo kGiroDaRegra graus por segundo:
    /// faz a curva que o aluno ve chegando, e da tempo de derruba-la no caminho. Depois de
    /// kCurvaDaRegra segundos ela segue reta - a que errou nao fica rodando a <div> para sempre.
    constexpr float kGiroDaRegra = 120.0f;
    constexpr float kCurvaDaRegra = 4.0f;
    Vector2 Curvar(Vector2 velocidade, Vector2 de, Vector2 para, float giroPorSegundo, float dt);

    /// O ritmo de cada modo.
    struct Degrau {
        float calma = 2.0f;               ///< a <div> sozinha no comeco da fase
        float entreLances = 2.3f;         ///< entre um lance de regras e o seguinte
        int regrasPorLance = 3;
        float velocidadeDasRegras = 170.0f;
        float rapidezDaDiv = 100.0f;
        float entreTransbordos = 3.2f;    ///< o overflow da <div> nova
        float velocidadeDasGotas = 110.0f;
        int tirosDoAnel = 8;              ///< setInterval
        int tirosDaExplosao = 12;         ///< setTimeout
        int tirosDoRevide = 3;            ///< onclick
        int tirosDoLeque = 3;             ///< forEach
        float velocidadeDaQueda = 270.0f; ///< onmouseover
        float entreTags = 2.3f;           ///< o leque do proprio professor
        int tagsPorLeque = 5;
        /// Na fase 2, quantas regras saem de CSS ainda (o resto e JavaScript).
        float chanceDeCssNoJs = 0.2f;
    };
    Degrau DegrauDo(Dificuldade::Nivel nivel);
}
