//
// A INF 216 do Lucas Ferreira: as quatro provas que mexem nos sistemas do proprio jogo.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A optativa secreta nao tem materia para ensinar: o professor de Projeto e Implementacao de Jogos
/// Digitais pega o jogo que o aluno ja conhece e mexe nele. Cada prova e um sistema:
///   1. a interface (HudHostil): a barra da nota sai do rodape e varre o campo - cheia ate a nota do
///      aluno, e so se passa pelo pedaco vazio -, e as janelas de dica abrem em volta dele;
///   2. as regras (RegrasTrocadas): os acertos passam a tirar nota e os tiros a dar; a gravidade
///      liga; e o fogo amigo - o tiro do aluno que erra volta por baixo;
///   3. o motor (MotorQuebrado): o lag, a hitbox que descola do desenho, e o quick save / quick load;
///   4. o lancamento (Lancamento, a repescagem): o jogo trava, os bugs marcham, os creditos sobem.
/// Aqui ficam as contas; quem poe os tiros no campo e Ferreira.cpp.
namespace JogosDigitais {

    /// Toda fase dele dura isto: o dobro das dos outros professores (Fase::kDuracao).
    constexpr float kDuracaoDaFase = 40.0f;
    /// O que cada acerto nele paga do ganho de sempre (Boss::FatorDoGanho): com o dobro do tempo, o
    /// aluno acerta o dobro, e a nota andaria o dobro. Assim ela anda no ritmo das outras materias -
    /// e o perigo dura o dobro.
    constexpr float kFatorDoGanho = 0.55f;

    /// A etapa (0, 1 ou 2) em 't' segundos de fase, para as duracoes 'd'.
    int Etapa(const std::array<float, 3>& d, float t);
    /// Os segundos desde que a etapa de 't' comecou.
    float NaEtapa(const std::array<float, 3>& d, float t);
}

// ---- Fase 1: a interface ----

namespace HudHostil {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateOne";
    constexpr std::array<const char*, 3> kEtapas = {"BarraSobe", "BarraJoga", "Janelas"};
    constexpr std::array<float, 3> kDuracoes = {13.0f, 13.0f, 14.0f};
    constexpr const char* kGatilhoDaBarra = "barra";
    constexpr const char* kGatilhoDoLeque = "leque";
    constexpr const char* kGatilhoDaJanela = "janela";
    constexpr const char* kGatilhoDeFechar = "fechar";

    // ---- A barra da nota ----

    /// A barra da nota em campo: uma fileira de segmentos de kSegmento px, cheia ate a nota do aluno
    /// (como a do rodape). So o pedaco cheio machuca: passa-se pelo vazio - e com 100 nao ha vazio.
    constexpr float kSegmento = 28.0f;
    /// Antes de andar ela pisca no lugar, e entre uma passada e outra descansa na ponta.
    constexpr float kAvisoDaBarra = 1.4f;
    constexpr float kPausaDaBarra = 0.5f;
    /// Quantos dos 'segmentos' estao cheios com a nota 'nota' (0 a 100).
    int Cheios(float nota, int segmentos);
    /// Se o segmento 'k' esta cheio; a barra 'espelhada' enche a partir do fim.
    bool Cheio(int k, int segmentos, float nota, bool espelhada);
    /// A varredura 't' segundos depois de a etapa comecar: 'onde' vai de 0 (a ponta de onde ela saiu)
    /// a 1 (a outra), indo e voltando, cada passada em 'ida' segundos. A barra espelha a cada passada,
    /// ja no descanso antes dela: quem passou pela direita tem de passar pela esquerda na volta.
    struct Varredura {
        float onde = 0.0f;
        int passada = 0;
        bool parada = true;
        bool espelhada = false;
    };
    Varredura NaVarredura(float t, float ida);
    /// A cor da barra, como no rodape: "Vermelha" abaixo do exame, "Amarela" abaixo da aprovacao, "Verde".
    const char* CaraDaBarra(float nota);

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

    struct Degrau {
        float idaDaBarra = 3.0f;
        float entreLeques = 1.25f;
        int tirosDoLeque = 3;
        float velocidade = 230.0f;
        float entreJanelas = 2.6f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}

// ---- Fase 2: as regras ----

namespace RegrasTrocadas {

    constexpr const char* kFase = "StateTwo";
    constexpr std::array<const char*, 3> kEtapas = {"Inversao", "Gravidade", "FogoAmigo"};
    constexpr std::array<float, 3> kDuracoes = {13.0f, 13.0f, 14.0f};
    constexpr const char* kGatilhoDoAnel = "anel";
    constexpr const char* kGatilhoDoTiro = "tiro";
    constexpr const char* kGatilhoDoEspinho = "espinho";
    constexpr const char* kGatilhoDoArremesso = "arremesso";
    constexpr const char* kGatilhoDoRetorno = "retorno";

    // ---- A inversao ----

    /// As regras da nota viram do avesso por um tempo: o acerto no professor TIRA nota, e o tiro dele
    /// vira moeda - passa pelo aluno e DA nota. Depois elas voltam, com um aviso curto: as moedas que
    /// ainda estao no campo viram tiros de novo.
    enum class Regra { Aviso, Invertida, Voltando, Normal };
    constexpr float kComecaAInversao = 1.5f;
    constexpr float kVoltaEm = 9.3f;
    constexpr float kVoltou = 10.5f;
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

    // ---- A gravidade ----

    /// O jogo vira de plataforma: o aluno cai (kQueda px/s, menos que ele anda - da para subir,
    /// devagar), o chao ganha espinhos, e os tiros do professor sao arremessados em arco.
    constexpr float kAvisoDaGravidade = 1.3f;
    constexpr float kQueda = 215.0f;
    constexpr float kGravidade = 430.0f;
    constexpr float kVooDoArremesso = 1.45f;
    /// A velocidade que leva de 'de' a 'alvo' em 'voo' segundos, com a gravidade 'g'.
    Vector2 Lancar(Vector2 de, Vector2 alvo, float voo, float g);
    /// Onde esta, 't' segundos depois, o que saiu de 'de' com a velocidade 'v'.
    Vector2 NoArco(Vector2 de, Vector2 v, float t, float g);

    // ---- O fogo amigo ----

    /// O campo da a volta: o tiro do aluno que sai por cima (o que errou o professor) volta por
    /// baixo, no mesmo x, e machuca; o tiro do professor que sai por baixo reaparece em cima, uma vez.
    constexpr float kAvisoDoFogoAmigo = 1.3f;
    constexpr float kVelocidadeDoRetorno = 330.0f;
    constexpr float kFaixaDeSaida = 30.0f;
    bool SaiuPorCima(float y, float topo);
    bool SaiuPorBaixo(float y, float fundo);
    Vector2 OndeVolta(float x, float fundo);

    struct Degrau {
        float entreAneis = 1.0f;
        int tirosDoAnel = 14;
        float velocidade = 190.0f;
        float entreLeques = 1.5f;
        int tirosDoLeque = 5;
        float entreArremessos = 1.05f;
        int arremessos = 3;
        float entreTiros = 0.85f;      ///< no fogo amigo: os leques que dao a volta
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}

// ---- Fase 3: o motor ----

namespace MotorQuebrado {

    constexpr const char* kFase = "StateThree";
    constexpr std::array<const char*, 3> kEtapas = {"Lag", "Hitbox", "SaveLoad"};
    constexpr std::array<float, 3> kDuracoes = {13.0f, 13.0f, 14.0f};
    constexpr const char* kGatilhoDoAnel = "anel";
    constexpr const char* kGatilhoDoTiro = "tiro";
    constexpr const char* kGatilhoDoSprite = "sprite";
    constexpr const char* kGatilhoDoColisor = "colisor";
    constexpr const char* kGatilhoDoDisquete = "disquete";

    // ---- O lag ----

    /// Os tiros so andam de kQuadroDoLag em kQuadroDoLag segundos, aos saltos: um circulo mostra onde
    /// cada um vai estar no proximo quadro.
    constexpr float kQuadroDoLag = 0.4f;
    int QuadroEm(float t);
    /// Onde esta, 'quadros' quadros depois, o tiro que estava em 'p0' com a velocidade 'v'.
    Vector2 NoQuadro(Vector2 p0, Vector2 v, int quadros);

    // ---- A hitbox ----

    /// O desenho de cada tiro vira enfeite, e a hitbox dele descola: fica a Descolamento(t) do
    /// desenho, com o circulo do modo debug em volta. O descolamento abre em kAbreODescolamento
    /// segundos e gira kGiroDoDescolamento graus por segundo.
    constexpr float kDescolamento = 56.0f;
    constexpr float kAbreODescolamento = 2.2f;
    constexpr float kGiroDoDescolamento = 38.0f;
    Vector2 Descolamento(float t);

    // ---- O quick save e o quick load ----

    /// O professor salva o jogo e, segundos depois, carrega: o aluno volta para onde estava, os
    /// tiros voltam para onde estavam, e a nota volta a ser a do save - nada do que aconteceu no
    /// meio conta, para o bem e para o mal.
    enum class Evento { Nada, Salvar, Carregar };
    constexpr std::array<float, 2> kSaves = {1.2f, 7.6f};
    constexpr std::array<float, 2> kLoads = {5.8f, 12.2f};
    /// O evento que cai depois de 'antes' e ate 'agora'.
    Evento EventoEntre(float antes, float agora);
    /// De 0 a 1, quanto do caminho entre o save e o load dele ja passou em 't'; -1 fora dele.
    float RumoAoLoad(float t);

    struct Degrau {
        float entreAneis = 1.15f;
        int tirosDoAnel = 12;
        float velocidade = 175.0f;
        float entreLeques = 1.6f;
        int tirosDoLeque = 3;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}

// ---- A repescagem: o lancamento ----

namespace Lancamento {

    constexpr const char* kFase = "StateFinal";
    constexpr std::array<const char*, 3> kEtapas = {"Crash", "Frota", "Creditos"};
    constexpr std::array<float, 3> kDuracoes = {12.0f, 14.0f, 14.0f};
    constexpr const char* kGatilhoDoAnel = "anel";
    constexpr const char* kGatilhoDoTiro = "tiro";
    constexpr const char* kGatilhoDoBotao = "botao";
    constexpr const char* kGatilhoDoEstilhaco = "estilhaco";
    constexpr const char* kGatilhoDoBug = "bug";
    constexpr const char* kGatilhoDaQueda = "queda";
    constexpr const char* kGatilhoDoCredito = "credito";

    // ---- A travada ----

    /// O jogo "para de responder": os tiros congelam e uma caixa de dialogo abre, com dois botoes que
    /// se derrubam a tiro. ESPERAR destrava do jeito que estava (e vale nota); FECHAR estilhaca cada
    /// tiro congelado; e quem nao escolhe em kEspera segundos ve o jogo voltar acelerado.
    constexpr std::array<float, 2> kTravadas = {2.2f, 7.4f};
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
        float entreLinhas = 1.7f;
        float subida = 78.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
