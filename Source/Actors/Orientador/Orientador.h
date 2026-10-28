//
// O orientador do TCC, em campo.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../Actor.h"
#include "../../Golpe.h"
#include "../../Variacao.h"

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
 * ONDE ELE FICA e conta pura, em Escolta.
 *
 * O GOLPE E O DA MATERIA DELE (Golpe.h): o laco do Andre, a pilha do Salles, o
 * barramento do Ricardo, a transacao do Thiago, o ponteiro do Luizinho, a
 * gramatica do Vladimir, o gradiente do Julio, o Observer da Maria Lucia, o
 * fetch do Lucas e a janela do TCP do Vitor. Todos com o mesmo peso. Aqui mora
 * o que a conta pura nao faz: soltar cada tiro na hora e acompanhar os que
 * esperam, curvam, voltam, oscilam ou se dividem.
 */
class Orientador : public Actor {
public:
    Orientador(Scene* scene, std::string professor, const std::string& retrato);

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] const std::string& Professor() const { return mProfessor; }
    [[nodiscard]] Golpe::Materia Materia() const { return mMateria; }

    /// Quantos tiros ele ja deu. Para quem mede a batalha.
    [[nodiscard]] int Disparos() const { return mDisparos; }

    /// Poe o orientador direto no posto, sem o voo de chegada: e onde ele nasce.
    void Pousar();

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

    /// A rodada do golpe: solta os tiros cuja hora chegou.
    void Golpear(Battle& batalha, float deltaTime);
    void Disparar(Battle& batalha, const Golpe::Tiro& tiro, const Vector2& de, const Vector2& direcao);
    /// Os tiros que ainda dependem dele: a espera, a curva, a volta, a onda, a
    /// divisao.
    void Guiar(Battle& batalha, float deltaTime);

    struct Guiado {
        std::uint64_t id = 0;
        Golpe::Voo voo;
    };

    std::string mProfessor;
    Golpe::Materia mMateria = Golpe::Materia::Generico;
    int mLado = -1;
    Vector2 mPouso;            ///< onde ele esta, sem o balanco
    float mTempo = 0.0f;
    int mDisparos = 0;

    bool mSorteado = false;
    Variacao::Sorteio mSorteio{0u};
    float mEspera = kPrimeiroGolpe;
    int mRodada = -1;
    Golpe::Rodada mDaVez;
    float mNaRodada = 0.0f;
    size_t mProximoTiro = 0;
    int mNotificacoes = 0;
    std::vector<Guiado> mGuiados;
};
