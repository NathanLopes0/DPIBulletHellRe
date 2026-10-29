//
// O escopo da fase 1 do Luizinho.
//

#pragma once

#include <array>
#include <optional>

#include <string>
#include <vector>

#include "../Engine/Fase.h"
#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"

/**
 * CAMADA PURA.
 *
 * A FASE 1 DO LUIZINHO SAO AS AMARRACOES: a que declaracao um nome se refere.
 * O campo E O TEXTO DO PROGRAMA - cada bloco e um painel, um dentro do outro
 * como no codigo, com as declaracoes dele escritas no canto:
 *
 *   int x, y;                         o global: o campo inteiro
 *   void f() { x++; }                 um cartao no alto
 *   void g() { int y;                 o painel da esquerda...
 *       { int x;                      ...e o bloco interno, dentro dele
 *   void h() { int x;                 o painel da direita
 *
 * O professor e o comando que roda: ele vai ate o bloco em que o comando
 * esta, e o placar diz qual e. O nome e PROCURADO de dentro para fora - uma
 * linha sai dele e passa pelo canto de cada bloco ate achar a declaracao -, e
 * a variavel achada CHOVE na regiao em que ela e visivel: o bloco dela, MENOS
 * os blocos de dentro que declaram outra com o mesmo nome. Quem esta num
 * desses buracos esta em outra variavel, e nao e atingido:
 *
 *   g: x++;               g nao tem x: e o global - o campo, menos o bloco
 *                         interno e h, que tem os seus
 *   { int x; x++; }       o x do bloco interno: so ele
 *   { int x; y++; }       o bloco nao tem y, g tem: g inteiro, COM o bloco
 *                         interno - o x dele nao esconde o y
 *   h: y++;               h nao tem y: o global, menos g, que tem o seu
 *   h chama f, escopo ESTATICO    o x de f e o do texto: o global - e o
 *                                 painel de h, onde a chamada esta, e seguro
 *   h chama f, escopo DINAMICO    o x de f e o de quem chamou: o de h, e so o
 *                                 painel de h chove
 *
 * Quem desenha, move e solta a chuva e o Luizinho; aqui ficam os blocos, a
 * busca, a regiao, as gotas e os textos. Coberto por test_escopo.cpp.
 */
namespace Escopo {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoEscopo = "StateOne";
    constexpr const char* kGatilhoDaGota = "gota";

    /// O campo de referencia: Area() escala para o de verdade.
    constexpr float kLargura = 1200.0f;
    constexpr float kAltura = 746.0f;

    enum class Bloco { Global, F, G, Interno, H };
    constexpr int kBlocos = 5;

    struct Retangulo {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        [[nodiscard]] float Direita() const { return x + w; }
        [[nodiscard]] float Baixo() const { return y + h; }
    };

    /// @brief O bloco que envolve 'b' no texto - o pai lexico. O global e pai
    /// de si mesmo.
    Bloco Pai(Bloco b);

    /// @brief O painel do bloco, num campo de 'largura' x 'altura'.
    Retangulo Area(Bloco b, float largura = kLargura, float altura = kAltura);

    // ---- OS COMANDOS --------------------------------------------------------

    enum class Regra { Estatico, Dinamico };
    enum class Forma { XEmG, XNoInterno, YNoInterno, YEmH, FEstatico, FDinamico };
    constexpr int kFormas = 6;

    /// @brief Onde o comando roda (nas chamadas, dentro de f), quem chamou
    /// (so nas chamadas: h), o nome que ele usa e a regra de escopo.
    Bloco Onde(Forma f);
    std::optional<Bloco> QuemChamou(Forma f);
    char Nome(Forma f);
    Regra RegraDe(Forma f);

    /// @brief A ordem da fase: a busca simples primeiro, a pegadinha no fim.
    Forma Proxima(int vez);

    struct Ponto { float x = 0.0f; float y = 0.0f; };
    constexpr float kMeiaAlturaDoProfessor = 64.0f;

    /// @brief A largura da caixa do canto: um tanto por letra, mais a margem.
    /// O Luizinho desenha a caixa com esta conta, e o texto encolhe se nao
    /// couber.
    float LarguraDoCanto(const std::string& texto);
    constexpr float kAlturaDoCanto = 26.0f;

    // ---- A CHUVA ------------------------------------------------------------

    /// @brief As gotas da variavel achada: uma grade em colmeia que cobre a
    /// regiao - fileiras a cada kEntreFileiras, uma gota a cada kEntreGotas,
    /// a fileira seguinte meio passo ao lado -, mais uma coluna rente a cada
    /// borda e a cada buraco. 'fim' e a altura em que a gota morre: a borda de
    /// baixo do painel, ou o buraco embaixo dela.
    struct Gota { float x = 0.0f; float y = 0.0f; float fim = 0.0f; };

    /// @brief A animacao da gota do nome, apagada (o aviso) e acesa.
    std::string CaraDoAviso(char nome);
    std::string CaraDaGota(char nome);

    /// O raio da gota (o colisor e metade da largura do quadro: um teste
    /// confere a arte) e o do aluno, para a conta de quem e atingido.
    constexpr float kRaioDaGota = 14.0f;
    constexpr float kRaioDoAluno = 3.0f;

    /// A colmeia. Com a fileira de baixo meio passo ao lado, as colunas ficam
    /// a kEntreGotas / 2 umas das outras, e todo ponto da regiao fica a no
    /// maximo um quarto de kEntreGotas de uma delas.
    constexpr float kEntreGotas = 56.0f;
    constexpr float kEntreFileiras = 48.0f;

    /// A gota fica kFolga longe do buraco alem do raio: quem esta no buraco,
    /// rente a borda, nao e atingido.
    constexpr float kFolga = 2.0f;

    // ---- O RELOGIO ----------------------------------------------------------

    /// Um comando a cada kPorComando, o primeiro kPrimeiro depois de a fase
    /// abrir (antes dele o placar diz o latim).
    constexpr int kComandos = kFormas;
    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorComando = (Fase::kDuracao - kPrimeiro) / static_cast<float>(kComandos);

    /// A busca anda um bloco a cada kPorSalto. As gotas nascem apagadas
    /// kBusca depois de o comando abrir, ficam kAviso assim (sem acertar
    /// ninguem), e kVivas acesas, caindo a kQueda.
    ///
    /// A QUEDA FECHA A COLMEIA: cada coluna tem uma gota a cada duas fileiras,
    /// e quem esta entre duas colunas so e alcancado por um pedaco da gota.
    /// Caindo kQueda * kVivas, cada gota varre o bastante para emendar com a
    /// de baixo, e nao sobra ponto parado na regiao que a chuva nao pegue (um
    /// teste confere ponto a ponto).
    constexpr float kPorSalto = 0.15f;
    constexpr float kBusca = 0.35f;
    constexpr float kAviso = 0.9f;
    constexpr float kVivas = 0.7f;
    constexpr float kQueda = 125.0f;

    // ---- O DEGRAU -------------------------------------------------------------

    /// O que o Dificil muda: as gotas acendem 0,72 s depois de nascer (e nao
    /// 0,9) e caem a 150 px/s. A queda vezes o tempo aceso so cresce, entao a
    /// colmeia continua fechada.
    struct Degrau {
        float aviso = kAviso;
        float vivas = kVivas;
        float queda = kQueda;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
    // ---- O PROGRAMA E OS COMANDOS DE CADA LUTA ------------------------------
    //
    // Ate o 191 o programa era um so - o global com x e y, g com y, o bloco
    // interno com x, h com x - e os comandos eram as seis Formas de cima, so
    // em ordem sorteada: em poucas lutas o aluno decorava onde a chuva caia.
    // Agora cada luta SORTEIA O PROGRAMA (o que g, o bloco interno e h
    // declaram) e GERA OS COMANDOS, e as funcoes recebem o programa e o
    // comando. A Forma so da nome aos seis comandos de sempre (DaForma).

    /// O que cada bloco declara: x, y, os dois ou nenhum.
    struct Programa {
        std::array<bool, kBlocos> x{};
        std::array<bool, kBlocos> y{};
        [[nodiscard]] bool Declara(Bloco b, char nome) const;
        /// O de sempre: o global com x e y, g com y, o interno com x, h com x.
        static Programa DeSempre();
    };

    /// Um comando: onde ele roda, o nome que usa e a regra; nas chamadas de
    /// f, quem a chamou.
    struct Comando {
        Bloco onde = Bloco::G;
        char nome = 'x';
        Regra regra = Regra::Estatico;
        std::optional<Bloco> quemChamou;
    };
    Comando DaForma(Forma f);

    /// @brief O que o bloco declara, como o codigo escreve: "int x, y;",
    /// "int y;", "" - e o que o canto do painel diz ("void g() { int y;").
    std::string DeclaracaoDe(const Programa& p, Bloco b);
    std::string Declaracoes(const Programa& p, Bloco b);

    /// @brief A busca do nome: os blocos olhados, em ordem, do de dentro para
    /// fora - pelo texto no escopo estatico, por quem chamou no dinamico. O
    /// ultimo e o que declara (o Dono).
    std::vector<Bloco> Busca(const Programa& p, const Comando& c);
    Bloco Dono(const Programa& p, const Comando& c);

    /// @brief Os blocos de dentro de 'dono' que declaram outro 'nome' - os
    /// buracos da regiao. So os de fora: um bloco dentro de um buraco ja esta
    /// no buraco.
    std::vector<Bloco> Ocultam(const Programa& p, Bloco dono, char nome);

    /// @brief O placar: "{ int x;  x++; }", e nas chamadas a regra numa linha
    /// e o codigo de h na outra.
    std::string Placar(const Programa& p, const Comando& c);

    /// @brief Onde o professor fica para rodar o comando: dentro do bloco
    /// dele, abaixo do canto (nas chamadas, embaixo do cartao de f) - o
    /// cartao do professor, de kMeiaAlturaDoProfessor para cada lado, nao
    /// cobre a declaracao.
    Ponto OndeFicaOProfessor(const Comando& c, float largura = kLargura, float altura = kAltura);

    /// @brief O canto do painel, onde a declaracao esta escrita - o meio da
    /// caixa dela: e para la que vai a linha da busca. O do global fica no
    /// alto a direita (o da esquerda e do placar), e o de f e o cartao inteiro.
    Ponto Canto(const Programa& p, Bloco b, float largura = kLargura, float altura = kAltura);

    /// @brief As gotas da variavel achada (ver A CHUVA, acima).
    std::vector<Gota> Gotas(const Programa& p, const Comando& c,
                            float largura = kLargura, float altura = kAltura);

    /// O MAXIMO DE GOTAS de uma chuva: o pool pre-aquece 300 por tipo
    /// (Battle::kProjectilePrewarmCountPerType), e uma chuva maior criaria
    /// projetil no meio da luta. O global sem buraco nenhum daria 358.
    constexpr size_t kMaisGotas = 300;

    /// A FASE DESTA LUTA: o programa e os kComandos comandos, em ordem.
    struct Plano {
        Programa programa = Programa::DeSempre();
        std::vector<Comando> comandos;
    };

    /**
     * @brief O programa e os comandos de uma luta.
     *
     * O global declara x e y, sempre - todo nome acha um dono -, e f nao
     * declara nada. h declara x, sempre, e y por sorteio: sem o x de h, as
     * duas chamadas de f achariam o mesmo x, e a pegadinha do dinamico nao
     * existiria. g declara x, y, os dois ou nenhum; o bloco interno, x, y ou
     * os dois. Se nenhum dos tres declarar y, h declara: o global sem buraco
     * para o y choveria no campo inteiro, mais gotas que o pool tem.
     *
     * Os comandos: quatro buscas no escopo estatico, sorteadas entre g, o
     * bloco interno e h com x ou y, sem repetir e com a chuva cabendo no pool
     * (kMaisGotas) - e com pelo menos dois donos diferentes, sem precisar de
     * regra: no maximo tres das seis acabam no global; e a pegadinha

     * fecha a fase, como sempre - f chamada de h, no estatico e logo depois
     * no dinamico.

     */
    Plano Sortear(Variacao::Sorteio& s);
}
