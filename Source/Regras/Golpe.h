//
// O golpe do orientador no TCC: o da materia que ele da.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "Variacao.h"

/// No TCC o orientador luta do lado do aluno (Orientador), e cada professor ataca com a materia
/// dele.
namespace Golpe {

    enum class Materia {
        Generico, Andre, Salles, Ricardo, Thiago, Luizinho, Vladimir, Julio, MariaLucia, Lucas, Vitor,
        Arroyo,  ///< entrou depois: no fim, para nenhum indice mudar
        Goulart, ///< tambem
        Daniel   ///< a primeira optativa: o professor dela orienta quem passa nela
    };

    /// A materia pelo nome do professor em materias.json ("Andre", "Maria Lucia", com ou sem
    /// acento). Desconhecido: Generico.
    Materia DoProfessor(const std::string& professor);

    constexpr float kVelocidade = 800.0f;

    /// Um tiro da rodada. Tudo e relativo: a hora ao comeco da rodada, o lugar
    /// ao orientador, o rumo a direcao do chefe.
    struct Tiro {
        float quando = 0.0f;
        float dx = 0.0f;
        float dy = 0.0f;
        float desvio = 0.0f;        ///< graus a partir da direcao do chefe
        bool paralelo = false;      ///< o rumo e o do orientador ao chefe, e nao o do lugar do tiro
        float velocidade = kVelocidade;
        float espera = 0.0f;        ///< parado no lugar, junto do orientador, antes de partir
        float curva = 0.0f;         ///< graus por segundo de curva na direcao do chefe
        float reMira = -1.0f;       ///< nesse instante do voo, aponta de novo para o chefe
        float onda = 0.0f;          ///< amplitude da oscilacao de lado, que se apaga
        float divide = -1.0f;       ///< nesse instante do voo, vira dois
        int geracoes = 0;           ///< quantas vezes os filhos ainda se dividem
        float abertura = 9.0f;      ///< quanto os dois filhos abrem um do outro, em graus
        bool herdaRumo = false;     ///< os filhos abrem do rumo do pai (uma arvore), e nao miram de novo
        float curvaAte = 1.2f;
                                    /// Sempre podia ficar girando em volta do alvo.

        /// O orientador precisa acompanhar o tiro depois de solta-lo.
        [[nodiscard]] bool Guiado() const {
            return espera > 0.0f || curva > 0.0f || reMira > 0.0f || onda > 0.0f || divide > 0.0f;
        }
    };

    struct Rodada {
        float duracao = 1.5f;
        std::vector<Tiro> tiros;
    };

    /// A rodada 'k' (0 e a primeira) do golpe da materia 'm'. A da Maria Lucia nao tem tiro: o dela
    /// vem do aluno (Notificacoes).
    Rodada DaVez(Materia m, int k, Variacao::Sorteio& s);

    // ---- As contas de cada quadro ----

    /// A direcao (comprimento 1) de 'de' para 'alvo', girada 'desvio' graus. Com o alvo em cima do
    /// atirador, para cima.
    Vector2 Rumo(const Vector2& de, const Vector2& alvo, float desvio);

    /// A 'velocidade' girada na direcao de 'paraOAlvo' no maximo 'grausPorSegundo' * 'dt', sem
    /// mudar o modulo. O ponteiro.
    Vector2 Curvar(const Vector2& velocidade, const Vector2& paraOAlvo, float grausPorSegundo, float dt);

    /// O deslocamento de lado do tiro do gradiente aos 't' segundos de voo: uma senoide que se
    /// apaga (kTempoDaOnda), comecando do zero.
    float Oscilacao(float t, float amplitude);
    constexpr float kTempoDaOnda = 0.22f;
    constexpr float kVoltasDaOnda = 3.0f;    ///< por segundo

    /// O Observer: a cada quantos tiros do aluno ela atira um.
    constexpr int kNotificacoes = 4;

    int Notificacoes(int doAluno, int dela);

    /// A janela do TCP na rodada 'k': o slow start 1, 2, 4; depois de um em um ate 6; a perda corta
    /// pela metade (3) e o dente recomeca.
    int Janela(int k);

    /// Os bits do byte do barramento: oito, com exatamente 'uns' em 1.
    std::array<bool, 8> Byte(Variacao::Sorteio& s, int uns = 3);
    constexpr int kBitsUm = 3;

    /// De onde o tiro sai e para onde, com quem atira em 'fonte' mirando em 'alvo'. O tiro paralelo
    /// (o barramento) tem o deslocamento de lado.
    struct Saida {
        Vector2 de;
        Vector2 direcao;
    };
    Saida Sair(const Tiro& t, const Vector2& fonte, const Vector2& alvo);

    // ---- O voo de um tiro guiado ----

    struct Voo {
        Tiro tiro;
        float idade = 0.0f;
        bool partiu = false;
        bool remirou = false;
        float deslocado = 0.0f;   ///< a parte da onda ja aplicada
        Vector2 deLado;           ///< para onde a onda empurra
    };

    /// Comeca o voo de 'tiro', solto no rumo 'direcao'.
    Voo Comecar(const Tiro& tiro, const Vector2& direcao);

    struct Passo {
        Vector2 posicao;
        Vector2 velocidade;
        bool divide = false;
    };

    /// Um quadro de 'dt' do voo 'v'.
    Passo Avancar(Voo& v, const Vector2& posicao, const Vector2& velocidade,
                  const Vector2& ancora, const Vector2& alvo, float dt);

    struct Filho {
        Tiro tiro;
        Vector2 direcao;
    };

    /// Os dois tiros em que 'v' se divide, em 'posicao' com 'velocidade': abrindo 'abertura' graus
    /// do rumo do pai (herdaRumo) ou da direcao do alvo.
    std::array<Filho, 2> Filhos(const Voo& v, const Vector2& posicao, const Vector2& velocidade,
                                const Vector2& alvo);
}
