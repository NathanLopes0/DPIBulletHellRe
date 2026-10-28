//
// O poder do orientador no TCC: o "especial" da materia que ele da.
//

#pragma once

#include <string>

#include "Golpe.h"
#include "Math.h"

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
 *   Ricardo     HLT               o clock para: todo tiro do Hugo congela
 *   Thiago      ROLLBACK          todo tiro do Hugo volta por onde veio
 *   Luizinho    free()            o coletor de lixo apaga os tiros em volta
 *   Vladimir    o automato        os tiros em volta viram seus, no Hugo
 *   Julio       agentes           tres robos voam com voce e atiram no Hugo
 *   Maria Lucia a bala de prata   quebra o escudo do Hugo na hora
 *   Lucas       Promise.all()     doze promessas resolvem juntas no Hugo
 *   Vitor       fork()            um processo filho atira do outro lado
 *
 * O que nao e professor de materia nao tem poder (Tipo::Nenhum).
 */
namespace Poder {

    enum class Tipo {
        Nenhum, Rajada, Capivara, Congelar, Rollback, Liberar, Converter, Agentes, BalaDePrata, PromiseAll, Fork
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
    bool Ataca(Tipo t);

    /// @brief A posicao do agente 'k' de 'n', voando em volta de 'centro', 't'
    /// segundos depois de solto: um anel de 'raio' que gira.
    Vector2 Agente(const Vector2& centro, int k, int n, float t, float raio);

    /// @brief O processo filho do fork(): o espelho de 'x' no campo de
    /// 'esquerda' a 'direita', na mesma altura.
    float Espelho(float x, float esquerda, float direita);

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
