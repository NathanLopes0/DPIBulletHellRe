//
// O poder do orientador no TCC: o "especial" da materia que ele da.
//

#pragma once

#include <string>

#include "Golpe.h"
#include "../../Core/Math.h"

/**
 * CAMADA PURA. Coberto por test_poder.cpp.
 *
 * ESCOLHER O ORIENTADOR E ESCOLHER UM PODER. Alem do golpe de sempre (Golpe.h),
 * cada orientador tem um especial da materia, que o aluno solta no BOTAO 3. Ele
 * CARREGA ACERTANDO O HUGO - os tiros do aluno e os do orientador, com o Hugo
 * blindado ou nao (Battle, regra 2) -, kCarga acertos para encher, e esvazia
 * quando e solto.
 *
 * Uns poderes atacam, e so pagam nota com o escudo do Hugo aberto; outros
 * defendem, e servem a qualquer hora; a bala de prata abre o escudo. A escolha
 * do orientador muda o jeito de jogar o TCC:
 *
 *   Andre       for (;;)          uma rajada sem fim no Hugo
 *   Salles      a capivara        anda embaixo de voce e come os tiros
 *   Ricardo     HLT               o clock para: o Hugo e os tiros dele congelam
 *   Thiago      ROLLBACK          todo tiro do Hugo volta por onde veio
 *   Luizinho    free()            o coletor de lixo apaga os tiros em volta
 *   Vladimir    o automato        os tiros em volta viram seus, no Hugo
 *   Julio       agentes           os tiros viram agentes e cacam o alvo
 *   Maria Lucia a bala de prata   quebra o escudo do Hugo na hora
 *   Lucas       Promise.all()     doze promessas resolvem juntas no Hugo e VOLTAM,
 *                                 desfazendo os tiros dele na ida e na volta
 *   Vitor       socket()          as paredes viram rede: sai de um lado, entra no outro
 *
 * O que nao e professor de materia nao tem poder (Tipo::Nenhum).
 */
namespace Poder {

    enum class Tipo {
        Nenhum, Rajada, Capivara, Congelar, Rollback, Liberar, Converter, Agentes, BalaDePrata, PromiseAll, Rede
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

    /// @brief O poder do orientador da materia 'm'.
    Descricao Da(Golpe::Materia m);

    /// Acertos no Hugo para encher.
    constexpr int kCarga = 70;

    /// @brief Quanto do poder esta carregado, de 0 a 1.
    float Fracao(int carga);

    /// @brief Se ele ataca (e entao so paga nota com o escudo do Hugo aberto).
    /// Quem pergunta e o piloto do arnes de medicao (Ferramentas/
    /// medir-dificuldade.sh, DPI_PODER), que espera o escudo abrir para solta-lo.
    bool Ataca(Tipo t);

    /// @brief A posicao do agente 'k' de 'n', voando em volta de 'centro', 't'
    /// segundos depois de solto: um anel de 'raio' que gira.
    Vector2 Agente(const Vector2& centro, int k, int n, float t, float raio);

    /// @brief A REDE DO VITOR: com as paredes abertas, quem passa de 'esquerda'
    /// sai em 'direita' (e vice-versa), com a sobra do passo. Dentro do campo,
    /// 'x' fica como esta.
    float Atravessar(float x, float esquerda, float direita);

    /// @brief OS AGENTES DO JULIO: a velocidade 'velocidade' de um tiro em 'de'
    /// virada no maximo 'giro' radianos por segundo, durante 'dt', na direcao
    /// de 'alvo' - a rapidez nao muda. Curva, e nao teleporta: o tiro que
    /// passou do alvo da a volta.
    Vector2 Guiar(const Vector2& velocidade, const Vector2& de, const Vector2& alvo, float giro, float dt);
    constexpr float kGiroDosAgentes = 9.0f;   ///< rad/s

    /// @brief O tiro do Hugo em 'tiro' esta no alcance de quem esta em
    /// 'centro', com 'raio'?
    bool NoAlcance(const Vector2& centro, const Vector2& tiro, float raio);

    /// A capivara anda no chao, embaixo do aluno, a esta rapidez (px/s).
    constexpr float kRapidezDaCapivara = 420.0f;

    // ---- A CARA ------------------------------------------------------------------

    /// A folha com o projetil de cada professor (Ferramentas/gerar_hugo.py): o
    /// tiro do orientador tem a cara da materia dele.
    constexpr const char* kFolhaDasMaterias = "Teachers/Projectiles/DPIBHHugoTcc";

    /// @brief O quadro da materia 'm' em kFolhaDasMaterias; -1 sem materia (o
    /// tiro de sempre).
    int QuadroDaMateria(Golpe::Materia m);
}
