//
// A INF 216 do Lucas Ferreira: as quatro provas que mexem nos sistemas do proprio jogo.
//

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Plataforma.h"
#include "../Regras/Variacao.h"

/// A optativa secreta nao tem materia para ensinar: o professor de Projeto e Implementacao de Jogos
/// Digitais pega o jogo que o aluno ja conhece e mexe nele. Cada prova e um sistema:
///   1. a interface (HudHostil): o professor reescreve a sua nota para 15; depois a barra da nota -
///      a da HUD, de verdade - sai do rodape e varre o campo, cheia ate a nota do aluno; e por fim
///      o primeiro inimigo, um fantasma de fliperama gigante, com a IA dele a mostra (IaDoInimigo) -
///      ele volta na fase das regras;
///   2. a fisica (Gravidade): a prova inteira vira jogo de plataforma, e o professor escolhe para que
///      lado a gravidade puxa - cada lado com o seu desafio;
///   3. as regras (RegrasTrocadas): a nota (acertar tira, os tiros dao), o campo (o tiro que erra
///      volta), o tempo (o FPS: o que foi feito sem deltaTime corre com ele) e o estado (o save que
///      foge ou persegue, e o quick load);
///   4. o lancamento (Lancamento, a repescagem): a hitbox descola, o jogo trava, os bugs marcham, os
///      creditos sobem.
/// Aqui ficam as contas; quem poe os tiros no campo e Ferreira.cpp.
namespace JogosDigitais {

    /// As fases dele duram isto, o dobro das dos outros professores (Fase::kDuracao) - menos a das
    /// regras, que tem a etapa do FPS e dura RegrasTrocadas::kDuracaoDaFase (DuracaoDa).
    constexpr float kDuracaoDaFase = 40.0f;
    /// Quanto dura a fase de nome 'fase' (StateOne, StateTwo, StateThree, StateFinal).
    float DuracaoDa(const std::string& fase);
    /// O que cada acerto nele paga do ganho de sempre (Boss::FatorDoGanho): com o dobro do tempo, o
    /// aluno acerta o dobro, e a nota andaria o dobro. Assim ela anda no ritmo das outras materias -
    /// e o perigo dura o dobro.
    constexpr float kFatorDoGanho = 0.55f;

    /// O campo, sem SDL: o retangulo em que o aluno anda (Battle::GetPlayfieldBounds).
    struct Campo {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
    };

    /// A etapa em 't' segundos de fase, para as duracoes 'd'.
    template <std::size_t N>
    int Etapa(const std::array<float, N>& d, const float t) {
        float fim = 0.0f;
        for (std::size_t k = 0; k < N; ++k) {
            fim += d[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(N) - 1;
    }

    /// Os segundos desde que a etapa de 't' comecou.
    template <std::size_t N>
    float NaEtapa(const std::array<float, N>& d, const float t) {
        float inicio = 0.0f;
        const int etapa = Etapa(d, t);
        for (int k = 0; k < etapa; ++k) inicio += d[static_cast<std::size_t>(k)];
        return t - inicio < 0.0f ? 0.0f : t - inicio;
    }

    /// O rumo de 'eu' ate 'alvo', de tamanho 1 (zero, se ja chegou).
    Vector2 Rumo(Vector2 eu, Vector2 alvo);
    /// Para onde foge quem esta em 'eu' de quem esta em 'de', sem sair de 'c' ('margem' das bordas):
    /// para longe; na parede, escorrega por ela; encurralado num canto, sai pela parede em que o
    /// perseguidor esta mais longe - e passa por ele, se ele nao fechar o caminho. Tamanho 1.
    Vector2 Fugir(Vector2 eu, Vector2 de, const Campo& c, float margem);
    /// 'p' preso dentro de 'c', a 'mx' das bordas dos lados e a 'my' das de cima e de baixo.
    Vector2 Prender(Vector2 p, const Campo& c, float mx, float my);
}

// ---- Fase 1: a interface ----

namespace HudHostil {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateOne";
    constexpr std::array<const char*, 4> kEtapas = {"Abertura", "BarraSobe", "BarraJoga", "Inimigo"};
    constexpr std::array<float, 4> kDuracoes = {4.5f, 11.0f, 11.0f, 13.5f};
    constexpr int kEtapaDaAbertura = 0;
    constexpr int kEtapaDaBarraDeitada = 1;
    constexpr int kEtapaDaBarraDePe = 2;
    constexpr int kEtapaDoInimigo = 3;
    constexpr const char* kGatilhoDaBarra = "barra";
    constexpr const char* kGatilhoDoLeque = "leque";
    /// O inimigo (IaDoInimigo): cada bloco do corpo, cada olho e o power-up.
    constexpr const char* kGatilhoDoInimigo = "inimigo";
    constexpr const char* kGatilhoDoOlho = "olho";
    constexpr const char* kGatilhoDoPoder = "poder";

    // ---- A abertura: a nota editada ----

    /// Antes de qualquer tiro, o professor mostra quem manda no jogo: tira o numero da sua nota da
    /// placa da HUD, leva ate ele, reescreve - o numero desce ate kNotaDaAbertura - e o devolve. A
    /// materia comeca com 15, e so entao a luta comeca. Nada machuca na abertura.
    constexpr float kNotaDaAbertura = 15.0f;
    constexpr float kTremeNaPlaca = 0.6f;   ///< o numero treme na placa: ele vai sair
    constexpr float kVaiAteEle = 0.9f;
    constexpr float kReescreve = 1.5f;
    constexpr float kVoltaAPlaca = 0.8f;
    /// Quando o numero chega de volta a placa: dali em diante a nota e a nova.
    constexpr float kNotaEditada = kTremeNaPlaca + kVaiAteEle + kReescreve + kVoltaAPlaca;
    /// Perto do professor o numero fica deste tamanho (na placa, 1).
    constexpr float kTamanhoNoProfessor = 2.4f;
    enum class Momento { NaPlaca, Indo, Reescrevendo, Voltando, Devolvido };
    struct NumeroDaAbertura {
        Momento momento = Momento::NaPlaca;
        Vector2 onde;
        float valor = 0.0f;     ///< o que o numero mostra agora
        float tamanho = 1.0f;
    };
    /// O numero 'te' segundos depois do comeco da prova: sai da 'placa', vai ate 'professor' (para
    /// baixo dele) por um arco, desce de 'nota' a kNotaDaAbertura e volta.
    NumeroDaAbertura NumeroEm(float te, Vector2 placa, Vector2 professor, float nota);

    // ---- A barra da nota: a de verdade ----

    /// A barra da HUD sai do lugar: o professor a arranca do rodape e varre o campo com ela - cheia
    /// ate a nota do aluno (e ela continua sendo a nota: acertar o professor a enche, apanhar a
    /// esvazia). So o pedaco vazio deixa passar; com 100, nao ha vazio. O que machuca e o desenho
    /// dela mesma: embaixo do cheio vao segmentos sem cara, de kSegmento px.
    constexpr float kSegmento = 28.0f;
    /// Deitada: ela treme no rodape, e arrancada (voa ate o alto do campo, passando pelo aluno sem
    /// machucar) e respira antes de descer.
    constexpr float kTremorDaBarra = 0.7f;
    constexpr float kArrancadaDaBarra = 0.8f;
    constexpr float kRespiroDaBarra = 0.3f;
    /// De pe: o giro (de deitada a de pe, ja num canto) e, no fim, a volta para o rodape.
    constexpr float kGiroDaBarra = 0.9f;
    constexpr float kVoltaDaBarra = 1.0f;
    constexpr float kPausaDaBarra = 0.5f;
    /// A distancia das pontas da varredura ate a borda do campo: ali o aluno nao chega
    /// (Player::PrenderNoCampo), e a barra descansa sem pegar ninguem.
    constexpr float kRecuoDaBarra = 22.0f;

    struct Degrau {
        int passadasDeitada = 3;
        int passadasDePe = 4;
        float entreLeques = 1.25f;
        int tirosDoLeque = 3;
        float velocidade = 230.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// Quanto dura cada passada, deitada e de pe: as passadas e as pausas enchem a etapa.
    float IdaDeitada(const Degrau& g);
    float IdaDePe(const Degrau& g);

    /// Quantos dos 'segmentos' estao cheios com a nota 'nota' (0 a 100).
    int Cheios(float nota, int segmentos);
    /// Se o segmento 'k' esta cheio; a barra 'espelhada' enche a partir do fim.
    bool Cheio(int k, int segmentos, float nota, bool espelhada);
    /// A varredura 't' segundos depois de ela comecar: 'onde' vai de 0 (a ponta de onde ela saiu) a 1
    /// (a outra), indo e voltando, cada passada em 'ida' segundos. A barra espelha a cada passada, ja
    /// no descanso antes dela: quem passou por um lado tem de passar pelo outro na volta.
    struct Varredura {
        float onde = 0.0f;
        int passada = 0;
        bool parada = true;
        bool espelhada = false;
    };
    Varredura NaVarredura(float t, float ida);

    /// Onde a barra esta e como: o centro, o comprimento, o angulo (graus, horario, como o SDL: 0 e
    /// deitada, -90 de pe enchendo de baixo) e se ela enche do outro fim.
    struct PoseDaBarra {
        Vector2 centro;
        float comprimento = 0.0f;
        float angulo = 0.0f;
        bool espelhada = false;
        /// Se o cheio machuca agora: nao no tremor, nem no voo, nem no giro.
        bool perigo = false;
        /// Ainda no rodape (o tremor), ou de volta a ele.
        bool noEncaixe = false;
        /// De 0 a 1: o tremor no rodape antes da arrancada.
        float tremor = 0.0f;
        /// O eixo da barra, do comeco ao fim (o comeco e onde ela enche, sem espelho).
        Vector2 eixo{1.0f, 0.0f};
    };
    /// A barra 'te' segundos depois de a etapa 'etapa' comecar (kEtapaDaBarraDeitada ou
    /// kEtapaDaBarraDePe), no campo 'c'. 'encaixe' e o centro dela no rodape, e 'noEncaixe' o
    /// comprimento que ela tem la.
    PoseDaBarra Pose(int etapa, float te, const JogosDigitais::Campo& c, Vector2 encaixe, float noEncaixe,
                     const Degrau& g);
    /// O centro do segmento 'k' de 'segmentos' numa barra nesta pose.
    Vector2 CentroDoSegmento(const PoseDaBarra& p, int k, int segmentos);

    // ---- A brincadeira: o Ponto Extra que limpa a tela leva a barra junto ----

    /// A onda do Ponto Extra dissolve o que e do chefe, e a barra em campo e um golpe dele: ela se
    /// quebra, os cacos voam por kQuebraDaBarra segundos enquanto a nota escorre, e no fim a nota e
    /// zero e a luta acaba - com kFraseDaNotaDestruida na tela do resultado.
    constexpr float kQuebraDaBarra = 1.6f;
    inline constexpr const char* kFraseDaNotaDestruida = "Voc\xc3\xaa destruiu a sua pr\xc3\xb3pria nota.";
    /// Os cacos de cada segmento, e a gravidade que os derruba (px/s^2).
    constexpr int kCacosPorSegmento = 3;
    constexpr float kGravidadeDosCacos = 520.0f;
    /// A nota escorre na primeira parte da quebra; no resto, os cacos terminam de cair.
    constexpr float kEscorreAte = 0.6f;

    struct Caco {
        Vector2 onde;
        Vector2 velocidade;
        /// Graus por segundo.
        float giro = 0.0f;
        float lado = 0.0f;
        /// Do pedaco cheio da barra (a cor da nota), ou do vazio.
        bool cheio = false;
    };
    /// Os cacos da barra nesta pose, com 'segmentos' segmentos e 'mostrada' de nota: espalhados ao
    /// longo dela, saindo dos dois lados e para longe do meio. Sem sorteio: a mesma barra quebra igual.
    std::vector<Caco> Cacos(const PoseDaBarra& p, int segmentos, float mostrada);
    /// Onde o caco esta 't' segundos depois da quebra.
    Vector2 OndeDoCaco(const Caco& c, float t);
    /// A nota 't' segundos depois da quebra, de 'antes' ate quase zero: so no fim (QuebraAcabou) ela
    /// e zero, para a luta nao acabar antes de os cacos cairem.
    float NotaNaQuebra(float antes, float t);
    [[nodiscard]] inline bool QuebraAcabou(const float t) { return t >= kQuebraDaBarra; }
}

// ---- Fase 1, a ultima etapa: a IA do inimigo ----

namespace IaDoInimigo {

    /// O primeiro inimigo do jogo, gigante: um fantasma de fliperama - a cupula, os olhos e a barra
    /// ondulando embaixo - feito de blocos da textura que faltou (o xadrez magenta e preto que o motor
    /// poe no lugar da arte que nao carregou). Ele persegue o aluno, e a IA dele e uma maquina de
    /// estados que se ve (o placar diz o estado, e cada troca tem o seu aviso). Ele aparece no fim da
    /// fase 1 e volta na das regras, nas tres primeiras etapas: na inversao vira moeda e foge, e no
    /// FPS anda sem deltaTime (RegrasTrocadas::SemDeltaTime). O diagrama:
    ///   NASCENDO -> PATRULHA: ele ronda os cantos do campo; o circulo e o raio de visao dele;
    ///   PATRULHA -> ALERTA: o aluno entrou no raio (ou a ronda cansou): um "!", e ele treme parado;
    ///   ALERTA -> PERSEGUE: vai atras do aluno mais perto, cada vez mais rapido;
    ///   PERSEGUE -> PATRULHA: o aluno ficou longe (fora do raio de perder) por kParaPerder: "?";
    ///     ou ele acabou de acertar o aluno - e recua para o canto mais longe, sem enxerga-lo por
    ///     kRecuo, como todo inimigo justo faz (sem isso, encostado no aluno, ele o veria na hora e o
    ///     acertaria de novo a cada fim de invencivel);
    ///   PATRULHA, ALERTA ou PERSEGUE -> ATORDOADO: um tiro no olho, o ponto fraco;
    ///   qualquer um, vivo -> FUGINDO: o aluno pegou o power-up - os blocos viram moeda, ele foge, e
    ///     cada bloco que o aluno encosta e nota; no fim do power-up ele pisca e volta a perseguir;
    ///   FUGINDO -> DERROTADO: o aluno pegou o ultimo bloco (sem bloco nenhum - o Ponto Extra tambem
    ///     os desfaz -, ele e derrotado em qualquer estado); DERROTADO -> NASCENDO: o respawn.
    enum class Estado { Nascendo, Patrulha, Alerta, Persegue, Atordoado, Fugindo, Derrotado };
    /// O nome que o placar e a etiqueta em cima dele mostram.
    const char* Nome(Estado e);

    /// O corpo, de cima para baixo: '#' e um bloco, 'O' um olho, '.' nada. A ultima linha e a barra do
    /// fantasma: quatro pes, que ondulam (Onda).
    constexpr int kColunas = 7;
    constexpr int kLinhas = 7;
    constexpr std::array<const char*, kLinhas> kCorpo = {"..###..", ".#####.", "#O###O#", "#######",
                                                        "#######", "#######", "#.#.#.#"};
    /// O lado de um bloco, em px (o sprite e desse tamanho).
    constexpr float kBloco = 20.0f;
    constexpr float kMeiaLargura = kBloco * static_cast<float>(kColunas) / 2.0f;
    constexpr float kMeiaAltura = kBloco * static_cast<float>(kLinhas) / 2.0f;
    /// O lugar de cada bloco e de cada olho, em relacao ao centro dele.
    std::vector<Vector2> LugaresDosBlocos();
    std::vector<Vector2> LugaresDosOlhos();
    /// Se o bloco em 'lugar' (de LugaresDosBlocos) e um pe da barra.
    bool NaBarra(Vector2 lugar);
    /// O fantasma flutua: a barra ondula - o pe da coluna 'coluna' sobe ou desce kOnda px, e os pes
    /// vizinhos andam ao contrario - e o corpo inteiro sobe e desce kFlutua px, devagar.
    constexpr float kOnda = 4.0f;
    constexpr float kFlutua = 3.0f;
    float Onda(int coluna, float relogio);
    float Flutua(float relogio);
    /// A coluna (0 a kColunas - 1) do bloco em 'lugar'.
    int Coluna(Vector2 lugar);

    /// Os tempos (segundos).
    constexpr float kNascendo = 0.9f;     ///< os blocos se juntam no lugar, sem machucar
    constexpr float kAlerta = 0.5f;       ///< o "!": ele treme parado, e ai corre
    constexpr float kRondaCansa = 3.0f;   ///< a patrulha que nao viu ninguem vira alerta assim mesmo
    constexpr float kParaPerder = 1.3f;   ///< longe por tanto tempo, e ele desiste: "?"
    constexpr float kRespawn = 2.2f;
    constexpr float kRecuo = 2.2f;        ///< depois de acertar o aluno, a ronda nao o ve por tanto tempo
    constexpr float kPiscaNoFim = 1.0f;   ///< o fim do power-up: os blocos piscam entre moeda e textura
    /// O que vale: cada bloco pego, o tiro no olho e o inimigo inteiro.
    constexpr float kPremioPorBloco = 0.25f;
    constexpr float kPremioPorOlho = 0.5f;
    constexpr float kPremioPorDerrotar = 2.0f;
    /// A folga do toque num bloco ou no power-up, alem do raio do aluno.
    constexpr float kToque = 14.0f;
    /// Onde o corpo pode andar: o campo sem a faixa de cima (a do professor e do placar).
    constexpr float kTopoDaArena = 170.0f;

    struct Degrau {
        float ronda = 100.0f;         ///< px/s na patrulha
        float persegue = 130.0f;      ///< px/s quando comeca a perseguir
        float aceleracao = 8.0f;      ///< a mais por segundo de perseguicao
        float maxima = 180.0f;        ///< o teto (o aluno anda a 300)
        float foge = 190.0f;          ///< fugindo: o aluno alcanca
        float raioDeVisao = 260.0f;
        float raioDePerder = 400.0f;
        float susto = 4.5f;           ///< quanto o power-up dura
        float atordoado = 2.0f;
        float primeiroPoder = 3.2f;   ///< segundos de etapa ate o primeiro power-up
        float entrePoderes = 2.5f;    ///< do fim de um susto ao power-up seguinte
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// O que ele percebe num quadro.
    struct Percepcao {
        float distancia = 1e9f;   ///< do centro dele ao aluno mais perto
        bool pegouOPoder = false;
        bool olhoAtingido = false;
        bool acertouOAluno = false;   ///< o corpo dele acabou de acertar o aluno
        int blocos = 0;           ///< os que sobram no corpo
        /// Quanto o susto dura, se nao e o do power-up: a inversao das regras manda o dela.
        float susto = 0.0f;
    };
    struct Ia {
        Estado estado = Estado::Nascendo;
        float noEstado = 0.0f;    ///< segundos no estado de agora
        float longe = 0.0f;       ///< segundos com o aluno fora do raio de perder
        float susto = 0.0f;       ///< o que falta do power-up
        float rapidez = 0.0f;     ///< a da perseguicao, que cresce
        float recuo = 0.0f;       ///< o que falta do recuo: a ronda ainda nao ve o aluno
    };
    /// Um passo de 'dt' segundos: as transicoes do diagrama acima. Devolve se o estado mudou.
    bool Passo(Ia& ia, const Percepcao& p, float dt, const Degrau& g);
    /// Se os blocos machucam neste estado: fugindo sao moeda; nascendo e derrotado, ainda nao estao la.
    bool Machuca(Estado e);
    /// A que velocidade ele anda agora (0: parado).
    float Rapidez(const Ia& ia, const Degrau& g);
    /// Se os blocos, fugindo, estao com a cara de textura neste instante: o aviso do fim.
    bool Pisca(const Ia& ia, float relogio);

    /// O centro dele sem o corpo sair da arena.
    Vector2 NaArena(Vector2 centro, const JogosDigitais::Campo& c);
    /// Os cantos da ronda, na ordem em que ele os percorre.
    std::array<Vector2, 4> Ronda(const JogosDigitais::Campo& c);
    /// O canto da ronda mais longe de 'aluno': onde ele nasce.
    int Ninho(const JogosDigitais::Campo& c, Vector2 aluno);
    /// Onde o power-up aparece: longe do inimigo - do outro lado do campo, com um pouco de sorteio.
    Vector2 LugarDoPoder(Variacao::Sorteio& s, const JogosDigitais::Campo& c, Vector2 inimigo);
}

// ---- Fase 2: a fisica ----

namespace Gravidade {

    constexpr const char* kFase = "StateTwo";
    /// Os lados sao sorteados (Ordem): a etapa diz a vez, e nao o lado.
    constexpr std::array<const char*, 4> kEtapas = {"Lado1", "Lado2", "Lado3", "Lado4"};
    constexpr std::array<float, 4> kDuracoes = {10.0f, 10.0f, 10.0f, 10.0f};
    constexpr const char* kGatilhoDaCaixa = "caixa";
    constexpr const char* kGatilhoDoEspinho = "espinho";
    constexpr const char* kGatilhoDaLanca = "lanca";
    constexpr const char* kGatilhoDoBloco = "bloco";
    constexpr const char* kGatilhoDaFagulha = "fagulha";
    constexpr const char* kGatilhoDaMoeda = "moeda";
    constexpr const char* kGatilhoDoTiro = "tiro";

    /// A prova inteira e jogo de plataforma (Plataforma.h), e o professor escolhe para que lado a
    /// gravidade puxa. Cada etapa e um lado: a seta aparece no comeco, a gravidade vira em kVirada e
    /// arremessa o aluno no chao novo, e o desafio do lado vai de kComecaODesafio a kAcabaODesafio.
    constexpr float kVirada = 0.9f;
    constexpr float kComecaODesafio = 1.5f;
    constexpr float kAcabaODesafio = 9.2f;

    /// O desafio de cada lado: no chao de sempre, pular as pilhas que deslizam; na parede esquerda, as
    /// lancas em duas alturas (pule a de baixo, fique para a de cima); no teto, a chuva de blocos (que
    /// estilhacam no chao em fagulhas) e moedas; na parede direita, a esteira - o chao anda ate o
    /// moedor, e os buracos de espinhos vem com ele.
    enum class Desafio { Pulos, Lancas, Chuva, Esteira };
    Desafio DesafioDo(Plataforma::Lado l);
    /// A ordem dos lados: o primeiro e sempre o chao de sempre, para ensinar o pulo; os outros tres,
    /// sorteados.
    std::array<Plataforma::Lado, 4> Ordem(Variacao::Sorteio& s);
    /// O lado oposto: o "ceu", onde o professor fica.
    Plataforma::Lado Oposto(Plataforma::Lado l);

    // ---- O campo visto do chao ----
    //
    // Todo desafio e escrito em (u, h): 'u' ao longo do chao (Plataforma::AoLongo) e 'h' para longe
    // dele, com h = 0 na linha em que o centro do aluno fica quando esta no chao. 'chao' e a distancia
    // entre essa linha e o lado do campo (Player::PrenderNoCampo: o tamanho do desenho).

    /// O comprimento do chao de 'l'.
    float Comprimento(Plataforma::Lado l, const JogosDigitais::Campo& c);
    /// A distancia da linha do aluno no chao ate a linha do lado oposto.
    float Profundidade(Plataforma::Lado l, const JogosDigitais::Campo& c, float chao);
    /// O ponto da tela em (u, h).
    Vector2 Ponto(Plataforma::Lado l, const JogosDigitais::Campo& c, float chao, float u, float h);
    /// E a volta: (u, h) de um ponto da tela.
    Vector2 NoChao(Plataforma::Lado l, const JogosDigitais::Campo& c, float chao, Vector2 p);

    /// As pecas do chao (caixas, espinhos, blocos) tem kPeca px. Os pes do aluno ficam 'pes' px
    /// abaixo do centro dele (meio desenho): e ai que o chao se desenha e as pilhas se apoiam.
    constexpr float kPeca = 28.0f;
    /// A altura do centro da peca 'j' (0 e a de baixo) de uma pilha apoiada no chao.
    float AlturaDaPeca(int j, float pes);

    // ---- Os pulos (o chao de sempre) ----

    /// Uma pilha de caixas de colisao que desliza pelo chao: 'altura' caixas de alto e 'largura' de
    /// largo, vinda de uma ponta ('sentido' +1 anda para u maior).
    struct Pilha {
        float quando = 0.0f;
        float sentido = 1.0f;
        float velocidade = 0.0f;
        int altura = 2;
        int largura = 1;
    };

    // ---- As lancas (a parede esquerda) ----

    /// Uma lanca corre rente ao chao em uma de duas alturas: a de baixo pega quem fica (pule), a de
    /// cima pega quem pula (fique). O aviso aparece na ponta de onde ela vem, kAvisoDaLanca antes.
    constexpr float kLancaBaixa = -4.0f;
    constexpr float kLancaAlta = 52.0f;
    constexpr float kAvisoDaLanca = 0.5f;
    struct Lanca {
        float quando = 0.0f;   ///< quando ela entra no campo
        float sentido = 1.0f;
        float h = kLancaBaixa;
        float velocidade = 0.0f;
    };

    // ---- A chuva (o teto) ----

    /// Um bloco (ou uma moeda) cai do ceu em 'u'; a sombra dele aparece no chao na hora. O bloco
    /// estilhaca no chao em duas fagulhas, que correm pelo chao, uma para cada lado; a moeda paga a
    /// quem a pega.
    struct Queda {
        float quando = 0.0f;
        float u = 0.0f;
        bool moeda = false;
    };
    constexpr float kPremioDaMoedaDaChuva = 0.6f;
    constexpr int kClasseDaMoedaDaChuva = 7;

    // ---- A esteira (a parede direita) ----

    /// O chao anda: quem fica parado e levado ate o moedor, na ponta de baixo da esteira. Os buracos
    /// - fileiras de espinhos - vem com o chao, e quem anda contra ele tem de pula-los.
    struct Buraco {
        float quando = 0.0f;
        int espinhos = 2;
    };
    /// Do moedor: onde o meio dele fica, contado da ponta do chao.
    constexpr float kRecuoDoMoedor = 70.0f;
    constexpr int kPecasDoMoedor = 9;

    struct Degrau {
        float entrePilhasDe = 0.85f;
        float entrePilhasAte = 1.25f;
        float pilhaDe = 300.0f;
        float pilhaAte = 360.0f;
        float entreLancasDe = 0.6f;
        float entreLancasAte = 0.85f;
        float lanca = 540.0f;
        float entreQuedasDe = 0.55f;
        float entreQuedasAte = 0.85f;
        float queda = 360.0f;
        float fagulha = 300.0f;
        float esteira = 130.0f;
        float entreBuracosDe = 0.9f;
        float entreBuracosAte = 1.4f;
        /// O professor, do ceu: um leque mirado no aluno de tanto em tanto.
        float entreLeques = 2.2f;
        int tirosDoLeque = 3;
        float velocidade = 200.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// O que cada desafio poe no campo, em segundos da etapa (de kComecaODesafio a kAcabaODesafio).
    std::vector<Pilha> Pulos(Variacao::Sorteio& s, const Degrau& g);
    std::vector<Lanca> Lancas(Variacao::Sorteio& s, const Degrau& g);
    std::vector<Queda> Chuva(Variacao::Sorteio& s, const Degrau& g, float comprimento);
    std::vector<Buraco> Buracos(Variacao::Sorteio& s, const Degrau& g);

    /// Se o aluno a 'h' (do centro) e pego por uma peca de raio 'raio' a 'hPeca', na mesma 'u': a
    /// conta que diz o que pular e o que nao (o aluno tem o raio 'raioDoAluno').
    bool Pega(float h, float hPeca, float raio, float raioDoAluno);
}

// ---- Fase 3: as regras ----

namespace RegrasTrocadas {

    constexpr const char* kFase = "StateThree";
    constexpr std::array<const char*, 4> kEtapas = {"Inversao", "FogoAmigo", "Fps", "SaveLoad"};
    /// A fase das regras e a mais longa: o FPS pede tempo para ser entendido, e o save, para ser pego.
    constexpr std::array<float, 4> kDuracoes = {10.0f, 10.0f, 16.0f, 12.0f};
    constexpr float kDuracaoDaFase = 48.0f;
    constexpr int kEtapaDaInversao = 0;
    constexpr int kEtapaDoFogoAmigo = 1;
    constexpr int kEtapaDoFps = 2;
    constexpr int kEtapaDoSave = 3;
    /// O primeiro inimigo volta nas etapas antes desta (o save fica sem ele).
    constexpr int kInimigoAte = kEtapaDoSave;
    constexpr const char* kGatilhoDoAnel = "anel";
    constexpr const char* kGatilhoDoTiro = "tiro";
    constexpr const char* kGatilhoDoRetorno = "retorno";
    constexpr const char* kGatilhoDoDisquete = "disquete";
    /// O FPS: o tiro vermelho, feito sem deltaTime, e o azul, com ele. O primeiro inimigo, que volta
    /// nesta fase, usa os gatilhos da fase 1 (HudHostil::kGatilhoDoInimigo e kGatilhoDoOlho).
    constexpr const char* kGatilhoDoVermelho = "vermelho";
    constexpr const char* kGatilhoDoAzul = "azul";

    // ---- A inversao: a regra da nota ----

    /// As regras da nota viram do avesso por um tempo: o acerto no professor TIRA nota, e o tiro dele
    /// vira moeda - passa pelo aluno e DA nota. Depois elas voltam, com um aviso curto: as moedas que
    /// ainda estao no campo viram tiros de novo.
    enum class Regra { Aviso, Invertida, Voltando, Normal };
    constexpr float kComecaAInversao = 1.2f;
    constexpr float kVoltaEm = 7.8f;
    constexpr float kVoltou = 9.0f;
    Regra RegraEm(float t);
    /// Se o tiro do professor e moeda (passa e paga), e se o acerto no professor cobra.
    bool TiroEMoeda(Regra r);
    bool AcertoCobra(Regra r);
    /// Na volta, a moeda pisca com a cara de tiro: o aviso.
    bool MoedaPisca(Regra r, float t);
    /// A classe da moeda e do aluno enquanto ela vale (BossProjectile::Classe).
    constexpr int kClasseDaMoeda = 6;
    constexpr float kPremioDaMoeda = 0.45f;
    constexpr float kMultaPorAcerto = 1.3f;
    constexpr float kToqueDaMoeda = 10.0f;

    // ---- O fogo amigo: a regra do campo ----

    /// O campo da a volta: o tiro do aluno que sai por cima (o que errou o professor) volta por
    /// baixo, no mesmo x, e machuca; o tiro do professor que sai por baixo reaparece em cima, uma vez.
    constexpr float kAvisoDoFogoAmigo = 1.3f;
    constexpr float kVelocidadeDoRetorno = 330.0f;
    constexpr float kFaixaDeSaida = 30.0f;
    bool SaiuPorCima(float y, float topo);
    bool SaiuPorBaixo(float y, float fundo);
    Vector2 OndeVolta(float x, float fundo);

    // ---- O FPS: a regra do tempo ----

    /// O jogo perde o limite de FPS, e o professor atira de dois jeitos. O VERMELHO foi programado sem
    /// deltaTime: anda tantos px por quadro, gira tantos graus por quadro, dispara a cada tantos
    /// quadros - e entao corre com o FPS (o dobro de quadros, o dobro da velocidade: o desenho e o
    /// mesmo, so o tempo dele muda). O AZUL usa o deltaTime: tantos px por segundo, em qualquer FPS. O
    /// FPS e do aluno: segurar o foco liga o modo economia, e o jogo cai para kFpsMinimo - o vermelho
    /// em camera lenta, mas o aluno devagar (o foco) para as paredes azuis; solto, o FPS sobe ate o
    /// teto do degrau, e o vermelho dispara. O primeiro inimigo, que volta nesta fase, tambem anda
    /// sem deltaTime.
    constexpr float kFpsDeReferencia = 60.0f;   ///< o FPS em que o vermelho anda o que esta escrito nele
    constexpr float kFpsMinimo = 30.0f;
    /// Os segundos para o FPS ir de um extremo ao outro (na escala log): cai rapido, sobe devagar.
    constexpr float kQuedaDoFps = 0.35f;
    constexpr float kSubidaDoFps = 1.1f;
    /// O aviso do comeco: o contador aparece, e o vermelho so comeca depois.
    constexpr float kComecaOFps = 1.4f;
    /// O FPS que o jogo procura com 'focados' (de 0 a 1) dos alunos segurando o foco: ninguem, o
    /// 'teto'; todos, kFpsMinimo; no meio, a media geometrica.
    float FpsAlvo(float focados, float teto);
    /// O FPS 'fps' um passo de 'dt' segundos mais perto de 'alvo'.
    float PassoDoFps(float fps, float alvo, float teto, float dt);
    /// Soltar o foco nao acelera o jogo na hora: por kFolgaAoSoltar o FPS segura onde esta, e so
    /// depois volta a subir - o respiro de quem solta o foco para correr. Apertar vale na hora.
    constexpr float kFolgaAoSoltar = 0.4f;
    struct Folga {
        float focados = 0.0f;   ///< a fracao de focados que ainda vale
        float ha = 0.0f;        ///< ha quanto tempo menos alunos seguram o foco
    };
    /// A fracao de focados que o FPS deve seguir, com 'agora' focados neste quadro.
    float FocadosComFolga(Folga& f, float agora, float dt);
    /// E quem solta o foco ganha um impulso: kImpulso segundos a kFatorDoImpulso da velocidade -
    /// o arranque para atravessar o campo ate o vao da parede azul. So vale para quem segurou o foco
    /// por kFocoParaImpulso: tocar o foco sem parar nao da impulso sem fim.
    constexpr float kImpulso = 0.6f;
    constexpr float kFatorDoImpulso = 1.7f;
    constexpr float kFocoParaImpulso = 0.3f;
    struct Arranque {
        float focado = 0.0f;    ///< ha quanto tempo o foco esta seguro
    };
    /// Um quadro do foco de um aluno: se ele soltou agora, depois de segurar o bastante - o impulso.
    bool Impulsiona(Arranque& a, bool focadoAgora, float dt);
    /// O quanto anda, neste FPS, quem foi feito sem deltaTime: 1 a kFpsDeReferencia.
    float SemDeltaTime(float fps);
    /// A altura de 'fps' no grafico do contador: 0 em kFpsMinimo, 1 no 'teto' (escala log).
    float NoGrafico(float fps, float teto);
    /// O contador e de sete segmentos - a (o de cima), b (cima a direita), c (baixo a direita), d (o
    /// de baixo), e (baixo a esquerda), f (cima a esquerda) e g (o do meio) -, e escreve os digitos e
    /// as letras F, P e S; o resto fica apagado.
    std::array<bool, 7> Segmentos(char c);
    /// A parede azul: uma fileira de tiros de borda a borda com um vao de kVaoDaParede px, que desce
    /// com deltaTime. Os tiros ficam juntos o bastante para o aluno nao passar entre dois (so pelo
    /// vao). O vao novo fica a pelo menos 'longe' do anterior - focado, nao se chega a tempo: e
    /// preciso soltar o foco e correr -, e a no maximo 'perto', o que o aluno sem foco alcanca antes
    /// de a parede chegar nele (VaoAlcancavel).
    constexpr float kVaoDaParede = 120.0f;
    constexpr float kEntreTirosDaParede = 26.0f;
    float VaoDaParede(Variacao::Sorteio& s, float anterior, float de, float ate, float longe, float perto);
    /// A distancia maxima entre dois vaos seguidos: o que o aluno anda a 'velocidade' em 85% do
    /// intervalo entre as paredes (nunca menos que 'longe' e um pouco).
    float VaoAlcancavel(float velocidade, float entreParedes, float longe);
    /// A velocidade do aluno sem o foco (Player::kVelocidade); com ele, a metade.
    constexpr float kAlunoSemFoco = 300.0f;

    // ---- O quick save: a regra do estado ----

    /// O professor salva a nota do aluno num disquete - e o disquete fica no campo, vivo, comparando a
    /// nota dele com a de agora. Se a do save e MAIOR (o aluno perdeu nota desde o save), ele vale
    /// mais: foge do aluno. Se e MENOR (o aluno ganhou), ele persegue. Encostar e o quick load: o
    /// aluno volta para onde estava no save, com a nota do save - para o bem e para o mal.
    enum class Humor { Parado, Foge, Persegue };
    /// Ate esta diferenca de nota, ele fica parado.
    constexpr float kEmpate = 0.5f;
    Humor HumorDoSave(float salva, float agora);
    /// A que velocidade ele anda: quanto maior a diferenca, mais rapido - ate o teto, que fica bem
    /// abaixo da do aluno (fugindo, ele e alcancavel; perseguindo, da para escapar).
    float RapidezDoSave(float salva, float agora);
    constexpr float kSaveDevagar = 90.0f;
    constexpr float kSavePorPonto = 14.0f;   ///< px/s a mais por ponto de diferenca
    constexpr float kSaveRapido = 215.0f;
    /// E a fuga e vencivel: o disquete encurralado - num canto da area (a kNoCanto das duas paredes),
    /// com o aluno a menos de kPertoDoCanto - trava por kEncurralado, tremendo, antes de escorregar
    /// pela parede, e o canto so volta a prende-lo kRecargaDoCanto depois. E o tiro do aluno o faz
    /// engasgar (o drive lendo): cada tiro que o acerta o para por kEngasgo mais, ate kEngasgoMaximo
    /// desde o comeco do engasgo - fugindo ou perseguindo -, e depois de um engasgo ele anda
    /// kImuneAoTiro sem parar: atirar sem parar faz o save andar aos trancos, e o aluno ganha terreno.
    constexpr float kNoCanto = 48.0f;
    constexpr float kPertoDoCanto = 200.0f;
    constexpr float kEncurralado = 1.0f;
    constexpr float kRecargaDoCanto = 1.6f;
    constexpr float kEngasgo = 0.15f;
    constexpr float kEngasgoMaximo = 0.45f;
    constexpr float kImuneAoTiro = 0.5f;
    /// Se 'eu' esta num canto de 'area' com o aluno em 'aluno' perto.
    bool Encurralado(Vector2 eu, Vector2 aluno, const JogosDigitais::Campo& area);
    struct Fuga {
        float travado = 0.0f;   ///< o que falta do canto
        float recarga = 0.0f;   ///< o que falta para o canto prender de novo
        float engasgo = 0.0f;   ///< o que falta do engasgo
        float engasgado = 0.0f; ///< quanto o engasgo de agora ja durou
        float imune = 0.0f;     ///< o que falta para o tiro voltar a engasga-lo
    };
    /// O passo do save: parado se engasgado ou encurralado; senao foge pela parede, ou vai atras do
    /// aluno. 'tiros': os tiros do aluno que o acertaram neste quadro. Devolve o deslocamento.
    Vector2 PassoDoSave(Fuga& f, Humor h, Vector2 eu, Vector2 aluno, const JogosDigitais::Campo& area, float rapidez,
                        int tiros, float dt);
    /// Se o save treme agora (engasgado ou encurralado): o aviso de que e a hora de pega-lo.
    bool Treme(const Fuga& f);
    /// O raio do disquete (o sprite tem 34 px): encostar e ficar a menos disto mais o raio do aluno.
    constexpr float kRaioDoSave = 17.0f;
    bool Encostou(Vector2 save, Vector2 aluno, float raioDoAluno);
    /// O save recem-feito ainda esta gravando: por este tempo, encostar nele nao carrega.
    constexpr float kGravando = 0.8f;
    /// Quando sai um save: o primeiro logo no comeco da etapa, e um novo kEntreSaves depois de cada
    /// load - enquanto ainda houver tempo para ele andar. 'ultimoLoad' < 0: nenhum ainda.
    constexpr float kPrimeiroSave = 1.0f;
    constexpr float kEntreSaves = 1.4f;
    constexpr float kUltimoSave = 7.6f;
    bool HoraDeSalvar(float te, float ultimoLoad);

    struct Degrau {
        // A inversao e o fogo amigo.
        float entreAneis = 1.0f;
        int tirosDoAnel = 14;
        float velocidade = 190.0f;
        float entreLeques = 1.5f;
        int tirosDoLeque = 5;
        float entreTiros = 0.85f;   ///< no fogo amigo: os leques que dao a volta
        // O save/load.
        float entreAneisDoMotor = 1.15f;
        int tirosDoAnelDoMotor = 12;
        float velocidadeDoMotor = 175.0f;
        float entreLequesDoMotor = 1.6f;
        int tirosDoLequeDoMotor = 3;
        // O FPS: a espiral vermelha (sem deltaTime, em quadros de kFpsDeReferencia) e as paredes azuis.
        float fpsMaximo = 144.0f;          ///< sem foco, o FPS sobe ate aqui
        float entreEspirais = 0.12f;       ///< um par de tiros a cada (a 60 FPS)
        float giroDaEspiral = 17.0f;       ///< graus a cada par
        float velocidadeVermelha = 120.0f; ///< px/s a 60 FPS
        float entreParedes = 2.3f;
        float velocidadeAzul = 205.0f;     ///< px/s, em qualquer FPS
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}

// ---- A repescagem: o lancamento ----

namespace Lancamento {

    constexpr const char* kFase = "StateFinal";
    constexpr std::array<const char*, 4> kEtapas = {"Hitbox", "Crash", "Frota", "Creditos"};
    /// A frota e a etapa mais longa: e preciso tempo para zera-la, e de novo.
    constexpr std::array<float, 4> kDuracoes = {9.0f, 9.0f, 16.0f, 10.0f};
    constexpr float kDuracaoDaFase = 44.0f;
    constexpr const char* kGatilhoDoAnel = "anel";
    constexpr const char* kGatilhoDoTiro = "tiro";
    constexpr const char* kGatilhoDoSprite = "sprite";
    constexpr const char* kGatilhoDoColisor = "colisor";
    constexpr const char* kGatilhoDoBotao = "botao";
    constexpr const char* kGatilhoDoEstilhaco = "estilhaco";
    constexpr const char* kGatilhoDoBug = "bug";
    constexpr const char* kGatilhoDaQueda = "queda";
    constexpr const char* kGatilhoDoCredito = "credito";

    // ---- A hitbox ----

    /// O jogo sai com o bug de colisao: o desenho de cada tiro vira enfeite, e a hitbox dele descola -
    /// fica a Descolamento(t) do desenho, com o circulo do modo debug em volta. O descolamento abre em
    /// kAbreODescolamento segundos e gira kGiroDoDescolamento graus por segundo.
    constexpr float kDescolamento = 56.0f;
    constexpr float kAbreODescolamento = 2.2f;
    constexpr float kGiroDoDescolamento = 38.0f;
    Vector2 Descolamento(float t);

    // ---- A travada ----

    /// O jogo "para de responder": os tiros congelam e uma caixa de dialogo abre, com dois botoes que
    /// se derrubam a tiro. ESPERAR destrava do jeito que estava (e vale nota); FECHAR estilhaca cada
    /// tiro congelado; e quem nao escolhe em kEspera segundos ve o jogo voltar acelerado.
    constexpr std::array<float, 2> kTravadas = {1.2f, 5.8f};
    constexpr float kEspera = 3.0f;
    /// A travada em curso em 't' (0 ou 1), ou -1.
    int TravadaEm(float t);
    enum class Escolha { Nenhuma, Esperar, Fechar };
    /// Por quanto a velocidade dos tiros e multiplicada na volta.
    float FatorDaVolta(Escolha e);
    /// De que lado o botao ESPERAR fica na travada 'k' (0: esquerda; 1: direita): troca de uma para a outra.
    int LadoDoEsperar(int k);
    constexpr float kPremioPorEsperar = 2.0f;

    // ---- A frota de bugs ----

    /// O jogo troca de genero: uma frota de bugs marcha de um lado ao outro, desce a cada batida na
    /// borda, e solta tiros. Cada bug cai com um tiro do aluno. E o professor e dono do jogo:
    ///   - enquanto houver bug, ele esta em GOD MODE - o tiro nele nao conta (Boss::AceitaDano), e
    ///     cada bug segura o god mode por um fio: ignorar a frota nao adianta;
    ///   - a cada 'hotfix' segundos ele recompila um bug morto no lugar dele (o lugar pisca
    ///     kAvisoDoHotfix antes, com o fio dele ate la): matar devagar nao adianta;
    ///   - a frota anda mais rapido quanto menos bugs sobram, e a cada onda;
    ///   - zerada a frota, o god mode cai por 'janela' segundos, e cada acerto nele vale
    ///     kFatorDaJanela; depois vem a onda seguinte.
    /// O Ponto Extra guardado (a onda que limpa a tela) desfaz a frota inteira de uma vez: e a hora de
    /// gasta-lo.
    constexpr int kFileiras = 3;
    constexpr int kColunasDaFrota = 9;
    constexpr float kEspacoX = 74.0f;
    constexpr float kEspacoY = 50.0f;
    constexpr float kPassoDaFrota = 30.0f;
    constexpr float kDescidaDaFrota = 56.0f;
    constexpr float kChegadaDaFrota = 0.9f;
    struct Marcha {
        float dx = 0.0f;
        int descidas = 0;
    };
    /// Onde a frota esta depois de 'passos' passos, com 'folga' px livres de cada lado.
    Marcha MarchaEm(int passos, float folga);
    Vector2 LugarNaFrota(int fileira, int coluna, Vector2 centroDoTopo, const Marcha& m);
    /// O bug vale pouco: ele e o obstaculo, e o premio e a janela.
    constexpr float kPremioPorBug = 0.15f;
    constexpr float kPremioDaFrotaLimpa = 3.0f;
    constexpr float kAvisoDoHotfix = 0.45f;
    constexpr float kFatorDaJanela = 2.0f;
    /// O intervalo entre dois passos da frota com 'vivos' de 'total' bugs na onda 'onda' (0 e a
    /// primeira): a frota cheia anda no 'base'; a de um bug so, a 40% dele; cada onda, 80% da anterior.
    float EntrePassos(float base, int vivos, int total, int onda);
    /// O lugar vazio que o hotfix recompila: um ao acaso entre os 'vazios' (indices); -1 sem nenhum.
    int LugarDoHotfix(const std::vector<int>& vazios, Variacao::Sorteio& s);

    // ---- Os creditos ----

    /// Os creditos sobem do rodape: cada linha e uma parede de blocos com um vao de kVaoDosCreditos
    /// blocos, fora da faixa do meio - onde o texto fica.
    constexpr float kBloco = 28.0f;
    constexpr int kVaoDosCreditos = 5;
    constexpr int kFaixaDoTexto = 7;
    constexpr std::array<const char*, 8> kCreditos = {
        "INF 216  \xc2\xb7  UM JOGO SOBRE FAZER JOGOS", "GAME DESIGN  \xc2\xb7  Lucas Ferreira",
        "PROGRAMA\xc3\x87\xc3\x83O  \xc2\xb7  a turma inteira",        "ARTE  \xc2\xb7  16 x 16 pixels",
        "TRILHA SONORA  \xc2\xb7  tan00.wav",            "QA  \xc2\xb7  voc\xc3\xaa, agora",
        "BUGS  \xc2\xb7  todos de prop\xc3\xb3sito",            "OBRIGADO POR JOGAR"};
    /// O primeiro bloco do vao de uma linha de 'blocos' blocos.
    int VaoDaLinha(int blocos, Variacao::Sorteio& s);
    bool NoVao(int bloco, int inicio);

    struct Degrau {
        float entreAneis = 0.95f;
        int tirosDoAnel = 16;
        float velocidade = 185.0f;
        float entrePassos = 0.3f;
        float entreQuedas = 0.42f;
        float velocidadeDaQueda = 230.0f;
        float entreLinhas = 1.15f;
        float subida = 78.0f;
        // A frota: o hotfix, a janela sem god mode, e o leque com que ele protege a frota.
        float hotfix = 1.2f;
        float janela = 3.5f;
        float entreLequesDaFrota = 1.7f;
        int tirosDoLequeDaFrota = 3;
        // A hitbox.
        float entreAneisDaHitbox = 1.15f;
        int tirosDoAnelDaHitbox = 12;
        float velocidadeDaHitbox = 175.0f;
        float entreLequesDaHitbox = 1.6f;
        int tirosDoLequeDaHitbox = 3;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
