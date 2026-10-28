//
// A gerencia de memoria da fase 3 do Luizinho.
//

#pragma once

#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Fase.h"
#include "../Dificuldade.h"
#include "../Variacao.h"

/**
 * CAMADA PURA.
 *
 * A FASE 3 DO LUIZINHO E A GERENCIA DE MEMORIA: o mesmo heap nas maos de um
 * coletor de lixo e nas de quem libera tudo sozinho. O campo e o heap, e o
 * ALOCADOR ESCOLHE O ALUNO: cada new poe o bloco - uma fileira de celulas, uma
 * por int - no lugar livre mais perto dele. As variaveis sao do professor (as
 * raizes): uma etiqueta com o nome, e uma linha ate o bloco para onde ela
 * aponta.
 *
 *   Java - coletor de lixo
 *     No a = new No(); a.prox = new No();   dois nos, um apontando o outro
 *     a = new No();                         os dois de antes ficam sem
 *                                           ninguem que os alcance: lixo, mas
 *                                           ainda no campo
 *     for (...) new int[3];                 seis blocos que nascem lixo
 *     System.gc();                          o mundo PARA, o coletor marca o
 *                                           que as raizes alcancam, e o resto
 *                                           cai
 *   C++ - gerencia manual
 *     int* p = new int[4];
 *     delete[] p;                           o bloco sai, e p continua
 *                                           apontando para onde ele estava: o
 *                                           ponteiro PENDENTE
 *     p[2] = 7;                             escrever ali e comportamento
 *                                           indefinido: estoura
 *     while (1) new int[4];                 cada bloco VAZA - ninguem o
 *                                           libera, e nao ha coletor -, ate o
 *                                           std::bad_alloc, em que todos
 *                                           estouram de uma vez
 *
 * Aqui ficam o heap (quem aponta para quem, quem e lixo, o que a varredura
 * leva), o roteiro, onde cada bloco cabe e os textos. Quem desenha, para o
 * mundo e estoura e o Luizinho. Coberto por test_memoria.cpp.
 */
namespace Memoria {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaMemoria = "StateThree";
    constexpr const char* kGatilhoDaCelula = "celula";
    constexpr const char* kGatilhoDoPendente = "pendente";
    constexpr const char* kGatilhoDoEstouro = "estouro";

    // ---- O HEAP -------------------------------------------------------------

    /// Vivo: alocado. Liberado: o delete passou (o bloco saiu, mas quem
    /// apontava continua apontando). Vazou: vivo e sem ninguem que o alcance,
    /// numa linguagem sem coletor - vai ficar ate o programa acabar.
    enum class Estado { Vivo, Liberado, Vazou };

    struct Bloco {
        int id = 0;
        int celulas = 1;
        Estado estado = Estado::Vivo;
        int prox = -1;      ///< o bloco para onde o campo prox aponta; -1 e null
    };

    struct Heap {
        std::vector<Bloco> blocos;
        std::map<std::string, int> raizes;   ///< a variavel e o bloco dela; -1 e null
        int proximoId = 0;
    };

    /// @brief Aloca um bloco de 'celulas' e devolve o id dele.
    int Novo(Heap& h, int celulas);

    /// @brief raiz = bloco. -1 e null.
    void Apontar(Heap& h, const std::string& raiz, int id);

    /// @brief bloco.prox = outro.
    void ApontarProx(Heap& h, int de, int para);

    /// @brief delete[] raiz: o bloco vira Liberado, e a raiz continua nele.
    void Liberar(Heap& h, const std::string& raiz);

    /// @brief Se a raiz aponta para um bloco que ja foi liberado.
    bool Pendente(const Heap& h, const std::string& raiz);

    /// @brief O bloco de id, ou nullptr.
    const Bloco* Achar(const Heap& h, int id);

    /// @brief A MARCACAO: os blocos vivos que as raizes alcancam, seguindo os
    /// prox. Um bloco que so e alcancado por outro bloco inalcancavel e lixo
    /// tambem.
    std::set<int> Alcancaveis(const Heap& h);

    /// @brief Os vivos que ninguem alcanca, em ordem de alocacao.
    std::vector<int> Lixo(const Heap& h);

    /// @brief A VARREDURA: tira o lixo do heap e devolve quem saiu.
    std::vector<int> Varrer(Heap& h);

    /// @brief Sem coletor, o lixo VAZA: marca os vivos inalcancaveis como
    /// Vazou e devolve os que vazaram agora.
    std::vector<int> Vazar(Heap& h);

    // ---- O ROTEIRO ----------------------------------------------------------

    enum class Linguagem { Java, Cpp };

    enum class Passo { NovosAeB, NovoC, LacoJava, Coleta, NovoP, Libera, Escreve, LacoCpp };
    constexpr int kPassos = 8;

    Linguagem LinguagemDe(Passo p);

    /// O que acontece num passo, e quando (segundos desde o comeco dele).
    enum class Acao { Alocar, Parar, Varrer, Liberar, Escrever, Estourar };
    struct Evento {
        float quando = 0.0f;
        Acao acao = Acao::Alocar;
        int celulas = 0;           ///< Alocar
        std::string raiz;          ///< Alocar (quem recebe; vazio: ninguem), Liberar, Escrever
        bool noProx = false;       ///< Alocar: vai para raiz->prox, e nao para a raiz
        int indice = 0;            ///< Escrever: raiz[indice]
    };
    std::vector<Evento> Eventos(Passo p);

    /// Os lacos: um bloco a cada kEntreAlocacoes, o primeiro kPrimeiraAlocacao
    /// depois de o passo abrir. O do Java aloca kNoLacoJava - e o placar diz
    /// "i < 6" -, e o while (1) do C++ aloca kNoLacoCpp ate o bad_alloc.
    constexpr float kPrimeiraAlocacao = 0.25f;
    constexpr float kEntreAlocacoes = 0.3f;
    constexpr int kNoLacoJava = 6;
    constexpr int kNoLacoCpp = 7;
    constexpr float kEstouro = 2.95f;   ///< o bad_alloc, no passo do while (1)

    /// @brief Quanto dura cada passo, e quando ele comeca (desde o comeco da
    /// fase). Antes de kPrimeiro o placar diz o latim.
    float Duracao(Passo p);
    float Inicio(Passo p);
    constexpr float kPrimeiro = 0.5f;

    /// @brief O passo em curso no instante t da fase; -1 antes do primeiro.
    int PassoEm(float t);

    /// @brief A ORDEM DOS PASSOS: as duas metades, cada uma com o seu programa.
    /// A de sempre e o Java primeiro; a da luta pode por o C++ primeiro
    /// (Sortear). Dentro de cada metade a ordem e a do programa - alocar antes
    /// de coletar, liberar antes de escrever no pendente.
    using Roteiro = std::array<Passo, kPassos>;
    Roteiro RoteiroDeSempre();
    Roteiro Sortear(Variacao::Sorteio& s);

    /// @brief Inicio e PassoEm para um roteiro: o passo da vez 'i' e
    /// roteiro[i], e PassoEm devolve esse i.
    float InicioDaVez(int i, const Roteiro& roteiro);
    int PassoEm(float t, const Roteiro& roteiro);

    /// @brief O passo da vez 'i' abre um programa novo (o heap comeca vazio)?
    bool TrocaDeLinguagem(int i, const Roteiro& roteiro);

    /// @brief O placar: a linguagem e como ela gerencia a memoria, e embaixo o
    /// codigo; 'depois' acrescenta o que aconteceu, quando ha o que dizer.
    std::string Placar(Passo p, bool depois);

    // ---- A GEOMETRIA --------------------------------------------------------

    struct Ponto { float x = 0.0f; float y = 0.0f; };
    struct Retangulo {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        [[nodiscard]] bool Cruza(const Retangulo& o) const;
        [[nodiscard]] Ponto Centro() const { return {x + w / 2.0f, y + h / 2.0f}; }
    };

    /// A celula: o colisor e metade do lado (um teste confere a arte).
    constexpr float kLadoDaCelula = 30.0f;
    constexpr float kEntreCelulas = 32.0f;

    /// @brief O retangulo do bloco de 'celulas' centrado em 'centro', e o meio
    /// de cada celula dele, da esquerda para a direita.
    Retangulo Area(Ponto centro, int celulas);
    std::vector<Ponto> Celulas(Ponto centro, int celulas);

    /// @brief ONDE O ALOCADOR POE O BLOCO: no aluno, se ali estiver livre; se
    /// nao, no lugar livre mais perto dele, procurado em aneis cada vez mais
    /// largos. Nunca encosta num bloco que ja esta no campo (kFolga entre
    /// eles), e fica no campo, abaixo de kAlturaMinima.
    Ponto OndeAlocar(int celulas, Ponto aluno, const std::vector<Retangulo>& ocupados,
                     float largura, float altura);
    constexpr float kFolga = 8.0f;
    constexpr float kAlturaMinima = 260.0f;
    constexpr float kMargem = 20.0f;

    // ---- AS CARAS E O RELOGIO -----------------------------------------------

    /// As caras da celula: o aviso de onde o bloco vai nascer, a viva (com o
    /// 0 com que o Java inicia todo int), a crua (o C++ nao inicia: "?"), o
    /// campo prox de um no, o lixo, o lixo marcado pelo coletor e a que vazou.
    constexpr const char* kCaraDoAviso = "Aviso";
    constexpr const char* kCaraViva = "Viva";
    constexpr const char* kCaraCrua = "Crua";
    constexpr const char* kCaraDoProx = "Prox";
    constexpr const char* kCaraDoLixo = "Lixo";
    constexpr const char* kCaraMarcada = "Marcada";
    constexpr const char* kCaraQueVazou = "Vazou";
    constexpr const char* kCaraDoEstilhaco = "Corrompido";

    /// O bloco nasce apagado - sem colisor - e acende kAvisoDaAlocacao depois:
    /// o tempo de o aluno sair de baixo dele.
    constexpr float kAvisoDaAlocacao = 0.8f;

    /// Na varredura o lixo cai a kQuedaDoLixo, para fora da tela.
    constexpr float kQuedaDoLixo = 520.0f;

    /// O ponteiro pendente estoura num anel de kEstilhacosDoPendente; no
    /// bad_alloc, cada celula vazada estoura em kEstilhacosPorCelula, em X.
    /// As velocidades sao as dos ataques em fases.json (um teste confere).
    constexpr int kEstilhacosDoPendente = 16;
    constexpr int kEstilhacosPorCelula = 4;
    constexpr float kVelocidadeDoPendente = 240.0f;
    constexpr float kVelocidadeDoEstouro = 280.0f;

    /// O professor: no alto, num x sorteado a cada passo - e, ao contrario da
    /// fase 2, PARADO nele. Aqui quem fica embaixo dele ja tem os blocos em
    /// volta: balancando 110 px, o piloto que persegue perdia 18 pontos na
    /// fase sozinha (9 acertos sofridos, 41 dados), e com 60 perdia de 10 a 28.
    /// Parado, ele entra na fase 3 com 68 a 71 e sai com 63 a 76 (quatro
    /// sementes).
    constexpr float kYDoProfessor = 150.0f;
    constexpr float kXMinimo = 300.0f;
    constexpr float kXMaximo = 900.0f;

    // ---- O DEGRAU -------------------------------------------------------------

    /// O que o Dificil muda: o bloco acende 0,55 s depois de alocado (e nao
    /// 0,8); os lacos alocam um bloco a cada 0,24 s (e nao 0,3), e o while (1)
    /// chega a nove antes do bad_alloc (e nao sete) - o do Java continua nos
    /// seis do placar -; o ponteiro pendente estoura em 20 estilhacos (e nao
    /// 16); e os estilhacos saem 20% mais rapido.
    struct Degrau {
        float avisoDaAlocacao = kAvisoDaAlocacao;
        float entreAlocacoes = kEntreAlocacoes;
        int noLacoCpp = kNoLacoCpp;
        int estilhacosDoPendente = kEstilhacosDoPendente;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// @brief Os eventos do passo com o degrau do modo (o do Normal da os de
    /// sempre).
    std::vector<Evento> Eventos(Passo p, const Degrau& d);
    // ---- OS NUMEROS DE CADA LUTA --------------------------------------------

    /**
     * @brief Os numeros do programa: o tamanho de cada new, as voltas do for
     * do Java, e onde e o que o p[i] = v escreve. Ate o 192 eram sempre os
     * mesmos - new int[3], i < 6, new int[4], p[2] = 7 -, e o programa da fase
     * 3 era o unico texto do Luizinho que nunca mudava. O placar escreve os da
     * luta, e o heap no campo segue eles.
     */
    struct Numeros {
        int noLacoJava = kNoLacoJava;     ///< for (i < n): de 4 a 7
        int celulasDoLacoJava = 3;        ///< new int[k] no for: de 2 a 4
        int celulasDeP = 4;               ///< int* p = new int[k]: de 3 a 5
        int indice = 2;                   ///< p[indice]: dentro do bloco
        int valor = 7;                    ///< = valor: de 1 a 9
        int celulasDoLacoCpp = 4;         ///< while (1) new int[k]: de 3 a 5
    };
    Numeros NumerosDeSempre();
    Numeros SortearNumeros(Variacao::Sorteio& s);

    std::vector<Evento> Eventos(Passo p, const Degrau& d, const Numeros& n);
    std::string Placar(Passo p, bool depois, const Numeros& n);
}
