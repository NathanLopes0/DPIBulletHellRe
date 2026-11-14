//
// O poder do orientador no TCC: o "especial" da materia que ele da.
//

#pragma once

#include <string>
#include <vector>

#include "Golpe.h"
#include "../Motor/Math.h"

/// Escolher o orientador e escolher um poder - e cada poder e um conceito da materia dele, jogavel:
/// os lacos aninhados do Andre, a pilha do Salles, o HLT do Ricardo, a transacao do Thiago, o
/// try/catch do Luizinho, o automato do Vladimir, os agentes do Julio, a bala de prata da Maria
/// Lucia, o Promise.all() do Lucas e o semaforo do Vitor.
namespace Poder {

    enum class Tipo {
        Nenhum, Matriz, Pilha, Congelar, Transacao, TryCatch, Converter, Agentes, BalaDePrata, PromiseAll, Mutex,
        BuscaBinaria, Rodizio, KMedias, CameraEscura
    };

    struct Descricao {
        Tipo tipo = Tipo::Nenhum;
        std::string nome;          ///< a primeira linha do placar: "ROLLBACK"
        std::string efeito;        ///< a segunda: "os tiros do Hugo voltam"
        float duracao = 0.0f;      ///< quanto o efeito dura (zero: instantaneo)
        float raio = 0.0f;         ///< o alcance, nos poderes de area
        int tiros = 0;             ///< nos poderes que atiram
        float intervalo = 0.0f;    ///< entre dois tiros
    };

    /// O poder do orientador da materia 'm'.
    Descricao Da(Golpe::Materia m);

    /// Acertos no Hugo para encher.
    constexpr int kCarga = 70;

    /// Quanto do poder esta carregado, de 0 a 1.
    float Fracao(int carga);

    /// Se ele ataca (e entao so paga nota com o escudo do Hugo aberto).
    bool Ataca(Tipo t);

    /// A posicao do agente 'k' de 'n', voando em volta de 'centro', 't' segundos depois de solto:
    /// um anel de 'raio' que gira.
    Vector2 Agente(const Vector2& centro, int k, int n, float t, float raio);

    // ---- Andre: os lacos aninhados ----

    /// for (i) for (j): uma matriz de tiros, linha por linha, cada linha da esquerda para a direita.
    constexpr int kLinhasDaMatriz = 5;
    constexpr int kColunasDaMatriz = 8;
    constexpr float kEntreLinhas = 0.09f;       ///< segundos entre duas voltas do laco de fora
    constexpr float kEntreCelulas = 0.006f;     ///< entre duas voltas do de dentro
    constexpr float kEntreColunas = 24.0f;      ///< px entre dois tiros da mesma linha
    /// Mais lenta que o tiro de sempre: a matriz sobe em bloco, e da para ve-la.
    constexpr float kVelocidadeDaMatriz = 430.0f;

    /// O tiro 'k' da matriz (0 e o primeiro): a linha i, a coluna j, quando sai (segundos desde o
    /// poder) e onde, em px a partir do aluno - a matriz e centrada nele.
    struct Celula {
        int i = 0;
        int j = 0;
        float quando = 0.0f;
        float dx = 0.0f;
    };
    Celula CelulaDaMatriz(int k);

    // ---- Salles: a pilha ----

    /// A capivara empilha o que come (push) e, no fim, devolve tudo no Hugo (pop) - o ultimo que
    /// entrou sai primeiro. Cheia, e stack overflow: ela comeca a devolver na hora, e continua
    /// comendo enquanto o poder dura.
    constexpr int kMaiorPilha = 14;
    constexpr float kEntrePops = 0.07f;

    // ---- Luizinho: try / catch ----

    /// O acerto capturado nao tira nota, e o finally responde com um leque no Hugo.
    constexpr float kAberturaDoFinally = 28.0f;   ///< graus

    /// O desvio, em graus, do tiro 'k' de um leque de 'n' com 'abertura': o do meio vai no alvo.
    float DesvioNoLeque(int k, int n, float abertura);

    // ---- Vitor: o semaforo ----

    /// P(s): uma porta acima do aluno, de parede a parede. O tiro do Hugo que chega nela descendo
    /// para e espera; passa um de cada vez. No V(s) quem ainda esperava morre de inanicao.
    constexpr float kPortaAcimaDoAluno = 170.0f;
    constexpr float kEntrePassagens = 0.55f;
    constexpr float kFolgaDaPorta = 16.0f;

    /// O tiro em 'y', andando 'vy' (para baixo e positivo), esta chegando na porta em 'porta'?
    bool NaPorta(float y, float vy, float porta);

    /// Os agentes do Julio: a velocidade 'velocidade' de um tiro em 'de' virada no maximo 'giro'
    /// radianos por segundo, durante 'dt', na direcao de 'alvo'.
    Vector2 Guiar(const Vector2& velocidade, const Vector2& de, const Vector2& alvo, float giro, float dt);
    constexpr float kGiroDosAgentes = 9.0f;   ///< rad/s

    /// O tiro do Hugo em 'tiro' esta no alcance de quem esta em 'centro', com 'raio'?
    bool NoAlcance(const Vector2& centro, const Vector2& tiro, float raio);

    /// A capivara anda no chao, embaixo do aluno, a esta rapidez (px/s).
    constexpr float kRapidezDaCapivara = 420.0f;

    // ---- Arroyo: a busca binaria ----

    /// A cada passo some a metade dos tiros do Hugo - a mais perto do aluno -, ate sobrar quase nada:
    /// n, n/2, n/4... em log2(n) passos.
    constexpr int kPassosDaBusca = 5;
    constexpr float kEntrePassosDaBusca = 0.32f;

    /// Quantos dos 'n' tiros um passo da busca leva (a metade de cima).
    int LevaNoPasso(int n);

    // ---- Goulart: o Round Robin ----

    /// Os tiros do Hugo em kGruposDoRodizio grupos, e so um anda por vez, por kQuantumDoRodizio
    /// segundos - os outros esperam a vez, parados: o campo inteiro anda a um quarto da rapidez, aos
    /// trancos.
    constexpr int kGruposDoRodizio = 4;
    constexpr float kQuantumDoRodizio = 0.35f;

    /// O grupo que tem a CPU 't' segundos depois de o poder sair.
    int GrupoDaVez(float t);

    // ---- Daniel: as k-medias ----

    /// Os tiros do Hugo em campo viram kGruposDasKMedias grupos (as k-medias, Agrupamento.h), e cada
    /// grupo vira um tiro so, no centroide: dezenas de tiros, de uma vez, viram quatro.
    constexpr int kGruposDasKMedias = 4;
    constexpr int kIteracoesDasKMedias = 6;
    /// Os centroides das k-medias dos 'pontos': as sementes espalhadas (de ponta a ponta, pelo x), e
    /// kIteracoesDasKMedias rodadas. Devolve tambem o grupo de cada ponto.
    std::vector<Vector2> KMedias(const std::vector<Vector2>& pontos, int k, std::vector<int>& grupos);

    // ---- Michel: a camera escura ----

    /// Uma parede com um furo vai com o aluno, kDistanciaDaParede acima dele: o tiro do Hugo que a
    /// cruza descendo sai do outro lado do furo, de cabeca para baixo - o lugar espelhado no furo, e
    /// a velocidade ao contrario. Ele volta para cima.
    constexpr float kDistanciaDaParede = 110.0f;
    /// A faixa logo abaixo da parede em que o tiro e pego (um quadro de voo do tiro mais rapido).
    constexpr float kFaixaDaParede = 26.0f;
    /// Se o tiro em 'y', descendo a 'vy', acabou de cruzar a parede em 'parede'.
    bool CruzouAParede(float y, float vy, float parede);
    /// O lugar de 'p' do outro lado do furo.
    Vector2 PeloFuro(const Vector2& p, const Vector2& furo);

    // ---- A cara ----

    /// A folha com o projetil de cada professor (Ferramentas/gerar_hugo.py): o
    /// tiro do orientador tem a cara da materia dele.
    constexpr const char* kFolhaDasMaterias = "Teachers/Projectiles/DPIBHHugoTcc";
    /// O ladrilho do Arroyo na folha: o quadro 22, depois da Caixa; o do Goulart, o 23; o do Daniel, o 24;
    /// o do Michel, o 25.
    constexpr int kQuadroDoArroyo = 22;
    constexpr int kQuadroDoGoulart = 23;
    constexpr int kQuadroDoDaniel = 24;
    constexpr int kQuadroDoMichel = 25;

    /// O quadro da materia 'm' em kFolhaDasMaterias; -1 sem materia (o tiro de sempre).
    int QuadroDaMateria(Golpe::Materia m);

    // ---- A previa ----

    /// O poder em miniatura, para a tela em que se escolhe o orientador: um campo de 1 x 1 (o Hugo
    /// em cima, o aluno embaixo) desenhado em pontos e tracos, mais a legenda do passo - um
    /// desenho animado do que o botao 3 vai fazer. Volta ao comeco a cada kCicloDaPrevia.
    enum class Marca { TiroDoHugo, TiroSeu, Aluno, Hugo, Figura, Estrutura, Apagado };
    struct Ponto {
        Marca marca = Marca::TiroDoHugo;
        float x = 0.0f;
        float y = 0.0f;
    };
    struct Traco {
        Marca cor = Marca::Estrutura;
        float x0 = 0.0f;
        float y0 = 0.0f;
        float x1 = 0.0f;
        float y1 = 0.0f;
    };
    struct Quadro {
        std::vector<Ponto> pontos;
        std::vector<Traco> tracos;
        std::string legenda;
    };
    constexpr float kCicloDaPrevia = 4.5f;
    Quadro Previa(Tipo t, float tempo);
}
