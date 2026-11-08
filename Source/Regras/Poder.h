//
// O poder do orientador no TCC: o "especial" da materia que ele da.
//

#pragma once

#include <string>

#include "Golpe.h"
#include "../Motor/Math.h"

/// Escolher o orientador e escolher um poder.
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

    /// A rede do Vitor: com as paredes abertas, quem passa de 'esquerda' sai em 'direita' (e
    /// vice-versa), com a sobra do passo.
    float Atravessar(float x, float esquerda, float direita);

    /// Os agentes do Julio: a velocidade 'velocidade' de um tiro em 'de' virada no maximo 'giro'
    /// radianos por segundo, durante 'dt', na direcao de 'alvo'.
    Vector2 Guiar(const Vector2& velocidade, const Vector2& de, const Vector2& alvo, float giro, float dt);
    constexpr float kGiroDosAgentes = 9.0f;   ///< rad/s

    /// O tiro do Hugo em 'tiro' esta no alcance de quem esta em 'centro', com 'raio'?
    bool NoAlcance(const Vector2& centro, const Vector2& tiro, float raio);

    /// A capivara anda no chao, embaixo do aluno, a esta rapidez (px/s).
    constexpr float kRapidezDaCapivara = 420.0f;

    // ---- A cara ----

    /// A folha com o projetil de cada professor (Ferramentas/gerar_hugo.py): o
    /// tiro do orientador tem a cara da materia dele.
    constexpr const char* kFolhaDasMaterias = "Teachers/Projectiles/DPIBHHugoTcc";

    /// O quadro da materia 'm' em kFolhaDasMaterias; -1 sem materia (o tiro de sempre).
    int QuadroDaMateria(Golpe::Materia m);
}
