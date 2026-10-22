//
// O orientador do TCC, em campo.
//

#pragma once

#include <string>

#include "../Actor.h"

class Battle;

/**
 * @class Orientador
 * @brief O professor escolhido antes do TCC (EscolhaDoOrientador), voando junto
 * do aluno e atirando no chefe.
 *
 * INVULNERAVEL, E NAO POR UMA REGRA: ele nao tem colisor, entao nenhum tiro do
 * chefe tem em que bater. Quem apanha continua sendo so o aluno.
 *
 * O TIRO DELE E DO ALUNO. Sai do mesmo pool dos tiros do aluno (PlayerProjectile),
 * entao acertar o chefe paga a mesma nota e derruba os mesmos projeteis - a
 * batalha nao precisa de uma regra nova para ele, e o que ele conquista e da
 * dupla como tudo o mais (requisitos-coop, D2).
 *
 * ONDE ELE FICA E PARA ONDE ATIRA sao contas puras, em Escolta.
 *
 * O GOLPE, POR ENQUANTO, E O MESMO PARA TODOS: um leque curto de kTiros mirado
 * no chefe a cada kIntervalo segundos. O de cada professor - o da materia que
 * ele da - vem nos proximos passos.
 */
class Orientador : public Actor {
public:
    Orientador(Scene* scene, std::string professor, const std::string& retrato);

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] const std::string& Professor() const { return mProfessor; }

    /// Quantos tiros ele ja deu. Para quem mede a batalha.
    [[nodiscard]] int Disparos() const { return mDisparos; }

    /// Poe o orientador direto no posto, sem o voo de chegada: e onde ele nasce.
    void Pousar();

    // ---- O golpe de todos, por enquanto -----------------------------------
    static constexpr float kIntervalo = 1.5f;
    static constexpr int kTiros = 3;
    static constexpr float kAbertura = 14.0f;
    static constexpr float kVelocidadeDoTiro = 800.0f;

private:
    /// O retrato e o cartao de chefe dele (128 px), em tamanho de ajudante.
    static constexpr float kEscala = 0.45f;

    /// Atras do aluno e dos tiros: e quem apanha que precisa ser visto.
    static constexpr int kOrdemDeDesenho = 90;

    /// O balanco de quem esta voando: amplitude em pixels, e ciclos por segundo.
    static constexpr float kBalanco = 5.0f;
    static constexpr float kBalancoPorSegundo = 0.6f;

    /// O primeiro golpe espera um pouco: a batalha acabou de comecar.
    static constexpr float kPrimeiroGolpe = 1.0f;

    [[nodiscard]] Battle* Batalha();
    [[nodiscard]] Vector2 PostoAgora();
    void Atirar(Battle& batalha);

    std::string mProfessor;
    int mLado = -1;
    Vector2 mPouso;            ///< onde ele esta, sem o balanco
    float mRecarga = kPrimeiroGolpe;
    float mTempo = 0.0f;
    int mDisparos = 0;
};
