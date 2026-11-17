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
///   1. a interface (HudHostil): um golpe no primeiro quadro, para assustar; depois a barra da nota -
///      a da HUD, de verdade - sai do rodape e varre o campo, cheia ate a nota do aluno; e as
///      janelas de dica abrem em volta dele;
///   2. a fisica (Gravidade): a prova inteira vira jogo de plataforma, e o professor escolhe para que
///      lado a gravidade puxa - cada lado com o seu desafio;
///   3. as regras (RegrasTrocadas): a nota (acertar tira, os tiros dao), o campo (o tiro que erra
///      volta), o tempo (o lag) e o estado (o quick save e o quick load);
///   4. o lancamento (Lancamento, a repescagem): a hitbox descola, o jogo trava, os bugs marcham, os
///      creditos sobem.
/// Aqui ficam as contas; quem poe os tiros no campo e Ferreira.cpp.
namespace JogosDigitais {

    /// Toda fase dele dura isto: o dobro das dos outros professores (Fase::kDuracao).
    constexpr float kDuracaoDaFase = 40.0f;
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
}

// ---- Fase 1: a interface ----

namespace HudHostil {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateOne";
    constexpr std::array<const char*, 4> kEtapas = {"Abertura", "BarraSobe", "BarraJoga", "Janelas"};
    constexpr std::array<float, 4> kDuracoes = {3.0f, 11.5f, 11.5f, 14.0f};
    constexpr int kEtapaDaAbertura = 0;
    constexpr int kEtapaDaBarraDeitada = 1;
    constexpr int kEtapaDaBarraDePe = 2;
    constexpr int kEtapaDasJanelas = 3;
    constexpr const char* kGatilhoDaAbertura = "abertura";
    constexpr const char* kGatilhoDaBarra = "barra";
    constexpr const char* kGatilhoDoLeque = "leque";
    constexpr const char* kGatilhoDaJanela = "janela";
    constexpr const char* kGatilhoDeFechar = "fechar";

    // ---- A abertura: o susto ----

    /// Antes de qualquer aviso, o golpe do primeiro quadro: tres aneis rapidos fecham sobre o aluno,
    /// cada um com um vao so - sempre do lado de cima, porque embaixo e o fim do campo. Quem nao corre
    /// para o vao na hora e pego. O anel some quando fecha: ninguem e pego pela volta dele.
    struct AnelDaAbertura {
        float quando = 0.0f;       ///< segundos depois do comeco da prova
        float raio = 0.0f;
        float velocidade = 0.0f;   ///< px/s, para dentro
        int tiros = 0;
        int vao = 0;               ///< quantos tiros faltam, juntos
    };
    constexpr int kAneisDaAbertura = 3;
    std::array<AnelDaAbertura, kAneisDaAbertura> AneisDaAbertura(Dificuldade::Nivel n);
    /// O angulo do tiro 'i' de um anel de 'tiros', em graus: 0 e a direita, -90 e para cima.
    float AnguloNoAnel(int i, int tiros);
    /// O meio do vao, em graus: ate kAberturaDoVao de 'rumo' - o lado em que ha campo, do aluno ao
    /// meio dele; no comeco, para cima (entre kVaoDe e kVaoAte) -, e longe do vao do anel anterior
    /// ('anterior' abaixo de -900: nenhum).
    constexpr float kAberturaDoVao = 60.0f;
    constexpr float kVaoDe = -90.0f - kAberturaDoVao;
    constexpr float kVaoAte = -90.0f + kAberturaDoVao;
    float VaoDaAbertura(Variacao::Sorteio& s, float anterior, float rumo = -90.0f);
    /// Se o tiro 'i' falta: os 'largura' tiros mais perto de 'vao' graus.
    bool NoVao(int i, int tiros, float vao, int largura);
    /// Em quantos segundos o anel fecha (chega ao centro).
    float Fechamento(const AnelDaAbertura& a);

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
        float entreJanelas = 2.6f;
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

    // ---- As janelas ----

    /// Uma janela de dica abre em volta do aluno: a moldura machuca, e ela so fecha com um tiro no X.
    constexpr float kLarguraDaJanela = 220.0f;
    constexpr float kAlturaDaJanela = 140.0f;
    constexpr float kPecaDaJanela = 20.0f;
    /// O X fica na barra de titulo, kRecuoDoFechar para dentro do canto direito: da para mirar nele de
    /// dentro da janela.
    constexpr float kRecuoDoFechar = 40.0f;
    constexpr float kAvisoDaJanela = 0.75f;
    constexpr int kJanelasAbertas = 4;
    constexpr float kPremioPorFechar = 0.8f;
    /// As pecas da moldura da janela centrada em 'centro' (sem a do X).
    std::vector<Vector2> Moldura(Vector2 centro);
    Vector2 BotaoDeFechar(Vector2 centro);
    /// O centro da janela que abre em volta de 'aluno', sem sair do retangulo dado.
    Vector2 CentroDaJanela(Vector2 aluno, float esquerda, float topo, float direita, float fundo);
    constexpr std::array<const char*, 6> kDicas = {"Dica 1/97: desvie!", "Atirar d\xc3\xa1 nota!",     "Aceita os cookies?",
                                                   "Feche no X",         "Avalie: 5 estrelas?", "Compre a DLC do TCC"};
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
    constexpr std::array<const char*, 4> kEtapas = {"Inversao", "FogoAmigo", "Lag", "SaveLoad"};
    constexpr std::array<float, 4> kDuracoes = {10.0f, 10.0f, 10.0f, 10.0f};
    constexpr const char* kGatilhoDoAnel = "anel";
    constexpr const char* kGatilhoDoTiro = "tiro";
    constexpr const char* kGatilhoDoRetorno = "retorno";
    constexpr const char* kGatilhoDoDisquete = "disquete";

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

    // ---- O lag: a regra do tempo ----

    /// Os tiros so andam de kQuadroDoLag em kQuadroDoLag segundos, aos saltos: um circulo mostra onde
    /// cada um vai estar no proximo quadro.
    constexpr float kQuadroDoLag = 0.4f;
    int QuadroEm(float t);
    /// Onde esta, 'quadros' quadros depois, o tiro que estava em 'p0' com a velocidade 'v'.
    Vector2 NoQuadro(Vector2 p0, Vector2 v, int quadros);

    // ---- O quick save e o quick load: a regra do estado ----

    /// O professor salva o jogo e, segundos depois, carrega: o aluno volta para onde estava, os tiros
    /// voltam para onde estavam, e a nota volta a ser a do save - nada do que aconteceu no meio conta,
    /// para o bem e para o mal.
    enum class Evento { Nada, Salvar, Carregar };
    constexpr std::array<float, 2> kSaves = {1.0f, 5.4f};
    constexpr std::array<float, 2> kLoads = {4.4f, 9.2f};
    /// O evento que cai depois de 'antes' e ate 'agora'.
    Evento EventoEntre(float antes, float agora);
    /// De 0 a 1, quanto do caminho entre o save e o load dele ja passou em 't'; -1 fora dele.
    float RumoAoLoad(float t);

    struct Degrau {
        // A inversao e o fogo amigo.
        float entreAneis = 1.0f;
        int tirosDoAnel = 14;
        float velocidade = 190.0f;
        float entreLeques = 1.5f;
        int tirosDoLeque = 5;
        float entreTiros = 0.85f;   ///< no fogo amigo: os leques que dao a volta
        // O lag e o save/load.
        float entreAneisDoMotor = 1.15f;
        int tirosDoAnelDoMotor = 12;
        float velocidadeDoMotor = 175.0f;
        float entreLequesDoMotor = 1.6f;
        int tirosDoLequeDoMotor = 3;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}

// ---- A repescagem: o lancamento ----

namespace Lancamento {

    constexpr const char* kFase = "StateFinal";
    constexpr std::array<const char*, 4> kEtapas = {"Hitbox", "Crash", "Frota", "Creditos"};
    constexpr std::array<float, 4> kDuracoes = {10.0f, 10.0f, 10.0f, 10.0f};
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
    /// borda, e solta tiros. Cada bug cai com um tiro do aluno; zerar a frota vale um bonus.
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
    constexpr float kPremioPorBug = 0.5f;
    constexpr float kPremioDaFrotaLimpa = 5.0f;

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
        float entreQuedas = 0.55f;
        float velocidadeDaQueda = 230.0f;
        float entreLinhas = 1.15f;
        float subida = 78.0f;
        // A hitbox.
        float entreAneisDaHitbox = 1.15f;
        int tirosDoAnelDaHitbox = 12;
        float velocidadeDaHitbox = 175.0f;
        float entreLequesDaHitbox = 1.6f;
        int tirosDoLequeDaHitbox = 3;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
