//
// O escopo da fase 1 do Luizinho.
//

#pragma once

#include <array>
#include <optional>

#include <string>
#include <vector>

#include "Fase.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A fase 1 do Luizinho sao as amarracoes: a que declaracao um nome se refere.
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

    /// O bloco que envolve 'b' no texto - o pai lexico. O global e pai de si mesmo.
    Bloco Pai(Bloco b);

    /// O painel do bloco, num campo de 'largura' x 'altura'.
    Retangulo Area(Bloco b, float largura = kLargura, float altura = kAltura);

    // ---- Os comandos ----

    enum class Regra { Estatico, Dinamico };
    enum class Forma { XEmG, XNoInterno, YNoInterno, YEmH, FEstatico, FDinamico };
    constexpr int kFormas = 6;

    /// Onde o comando roda (nas chamadas, dentro de f), quem chamou (so nas chamadas: h), o nome
    /// que ele usa e a regra de escopo.
    Bloco Onde(Forma f);
    std::optional<Bloco> QuemChamou(Forma f);
    char Nome(Forma f);
    Regra RegraDe(Forma f);

    /// A ordem da fase: a busca simples primeiro, a pegadinha no fim.
    Forma Proxima(int vez);

    struct Ponto { float x = 0.0f; float y = 0.0f; };
    constexpr float kMeiaAlturaDoProfessor = 64.0f;

    /// A largura da caixa do canto: um tanto por letra, mais a margem. O Luizinho desenha a caixa
    /// com esta conta, e o texto encolhe se nao couber.
    float LarguraDoCanto(const std::string& texto);
    constexpr float kAlturaDoCanto = 26.0f;

    // ---- A chuva ----

    /// As gotas da variavel achada: uma grade em colmeia que cobre a regiao.
    struct Gota { float x = 0.0f; float y = 0.0f; float fim = 0.0f; };

    /// A animacao da gota do nome, apagada (o aviso) e acesa.
    std::string CaraDoAviso(char nome);
    std::string CaraDaGota(char nome);

    /// O raio da gota (o colisor e metade da largura do quadro: um teste
    /// confere a arte) e o do aluno, para a conta de quem e atingido.
    constexpr float kRaioDaGota = 14.0f;
    constexpr float kRaioDoAluno = 3.0f;

    /// A colmeia.
    constexpr float kEntreGotas = 56.0f;
    constexpr float kEntreFileiras = 48.0f;

    /// A gota fica kFolga longe do buraco alem do raio: quem esta no buraco,
    /// rente a borda, nao e atingido.
    constexpr float kFolga = 2.0f;

    // ---- O relogio ----

    /// Um comando a cada kPorComando, o primeiro kPrimeiro depois de a fase
    /// abrir (antes dele o placar diz o latim).
    constexpr int kComandos = kFormas;
    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorComando = (Fase::kDuracao - kPrimeiro) / static_cast<float>(kComandos);

    /// A busca anda um bloco a cada kPorSalto.
    constexpr float kPorSalto = 0.15f;
    constexpr float kBusca = 0.35f;
    constexpr float kAviso = 0.9f;
    constexpr float kVivas = 0.7f;
    constexpr float kQueda = 125.0f;

    // ---- O degrau ----

    /// O que o Dificil muda: as gotas acendem 0,72 s depois de nascer (e nao 0,9) e caem a 150
    /// px/s.
    struct Degrau {
        float aviso = kAviso;
        float vivas = kVivas;
        float queda = kQueda;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
    // ---- O programa e os comandos de cada luta ----

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

    /// O que o bloco declara, como o codigo escreve: "int x, y;", "int y;", "" - e o que o canto do
    /// painel diz ("void g() { int y;").
    std::string DeclaracaoDe(const Programa& p, Bloco b);
    std::string Declaracoes(const Programa& p, Bloco b);

    /// A busca do nome: os blocos olhados, em ordem, do de dentro para fora - pelo texto no escopo
    /// estatico, por quem chamou no dinamico.
    std::vector<Bloco> Busca(const Programa& p, const Comando& c);
    Bloco Dono(const Programa& p, const Comando& c);

    /// Os blocos de dentro de 'dono' que declaram outro 'nome' - os buracos da regiao. So os de
    /// fora: um bloco dentro de um buraco ja esta no buraco.
    std::vector<Bloco> Ocultam(const Programa& p, Bloco dono, char nome);

    /// O placar: "{ int x; x++; }", e nas chamadas a regra numa linha e o codigo de h na outra.
    std::string Placar(const Programa& p, const Comando& c);

    /// Onde o professor fica para rodar o comando: dentro do bloco dele, abaixo do canto (nas
    /// chamadas, embaixo do cartao de f).
    Ponto OndeFicaOProfessor(const Comando& c, float largura = kLargura, float altura = kAltura);

    /// O canto do painel, onde a declaracao esta escrita - o meio da caixa dela: e para la que vai
    /// a linha da busca.
    Ponto Canto(const Programa& p, Bloco b, float largura = kLargura, float altura = kAltura);

    /// As gotas da variavel achada.
    std::vector<Gota> Gotas(const Programa& p, const Comando& c,
                            float largura = kLargura, float altura = kAltura);

    /// O global sem buraco nenhum daria 358.
    constexpr size_t kMaisGotas = 300;

    /// A fase desta luta: o programa e os kComandos comandos, em ordem.
    struct Plano {
        Programa programa = Programa::DeSempre();
        std::vector<Comando> comandos;
    };

    /// O programa e os comandos de uma luta.
    Plano Sortear(Variacao::Sorteio& s);
}
