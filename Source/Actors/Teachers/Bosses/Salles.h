//
// O chefe da INF 213 - Estruturas de Dados.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Capivara.h"
#include "../../../Attacks/Pilha.h"

/**
 * @class Salles
 * @brief O professor de Estruturas de Dados (INF 213).
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/salles/fases.json e as regras no regras.json ao lado. O que
 * mora aqui e o que dados nao sabem fazer.
 *
 * A CAPIVARA (Capivara.h). Ele poe capivara em todo exercicio, e de vez em
 * quando ela atravessa o campo no meio da fase - em qualquer fase dele. Ela e
 * um abrigo: come os projeteis dele e os tiros do aluno. Aqui mora o sorteio
 * de cada fase e o aviso no placar.
 *
 * A FASE 1 E UMA PILHA (Pilha.h), pendurada nele: push poe um bloco numerado
 * no topo, que e a ponta de baixo, e pop o tira e o atira no aluno - e o bloco
 * estoura em tantas lascas quanto o numero dele. O tiro do aluno so alcanca o
 * topo. Aqui mora a pilha em campo e o programa dela; ele vai de coluna em
 * coluna (o movimento "Conduzido"), parando em cada uma.
 */
class Salles : public Boss {

public:
    explicit Salles(Scene* scene);
    ~Salles() override = default;
    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;

protected:
    void CustomizeAttackParams(AttackParams &params, const std::string &stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    /// Quantos graus a arvore da fase 3 erra o jogador DE PROPOSITO quando mira
    /// nele. Ver CustomizeAttackParams: e o que impede o corredor vazio do eixo
    /// de cair em cima de quem esta parado.
    ///
    /// 4 graus porque o jogador costuma estar a uns 450px do chefe, e meio
    /// espacamento de folha (30px) a essa distancia da pouco menos de 4 graus.
    static constexpr float kDesvioDaBusca = 4.0f;

    /// Para que lado a busca desceu da ultima vez. Alterna.
    float mLadoDaBusca = 1.0f;

    // ---- A CAPIVARA ---------------------------------------------------------
    //
    // A cada fase nova, um sorteio diz se ela vem, quando e de que lado. Na hora,
    // o Salles puxa o gatilho "capivara" e o placar avisa por um tempo.

    void AtualizarCapivara(float deltaTime);

    std::string mFaseDaCapivara;
    Capivara::Visita mVisita;
    bool mVeio = false;
    bool mDisparandoCapivara = false;

    /// O placar antes do aviso, e quanto falta para ele voltar.
    std::string mAnuncioAntesDaCapivara;
    float mAvisoDaCapivara = 0.0f;

    // ---- A PILHA DA FASE 1 --------------------------------------------------
    //
    // Os blocos em campo, do da base (colado no professor) ao topo, cada um com
    // o numero dele. A cada Pilha::kPasso, a operacao do programa; a cada
    // quadro, cada bloco vai para o lugar dele embaixo do professor, e o bloco
    // que saiu num pop estoura na hora dele.

    void AtualizarPilha(float deltaTime);

    /// Solta a pilha inteira para baixo, aberta em leque: o overflow, e o fim
    /// da fase.
    void DerrubarPilha();

    [[nodiscard]] std::vector<BossProjectile*> NoPosto(const char* posto) const;

    std::vector<BossProjectile*> mPilha;
    std::vector<int> mValores;

    struct Popado {
        BossProjectile* bloco = nullptr;
        float idade = 0.0f;
        int valor = 0;
    };
    std::vector<Popado> mPopados;

    bool mNaPilha = false;
    float mProximaOperacao = 0.0f;
    std::string mAnuncioDaPilha;

    /// Empurrando: o bloco que o push acabou de criar.
    bool mEmpurrando = false;
    BossProjectile* mEmpurrado = nullptr;

    /// Estourando um pop: de onde, e em quantas lascas.
    bool mEstourando = false;
    Vector2 mDisparoDoEstouro;
    int mLascas = 0;

    /// Para onde o professor vai: uma das colunas, e quando ele muda.
    std::optional<Vector2> mAlvo;
    float mProximaMudanca = 0.0f;
    int mColuna = 1;
    static constexpr std::array<float, 3> kColunasDaPilha = {260.0f, 600.0f, 940.0f};
    static constexpr float kAlturaDaPilha = 133.0f;
    static constexpr float kParadoNaColuna = 2.6f;

    /// Quao depressa um bloco chega ao lugar dele (Ancora::Aproximar).
    static constexpr float kRapidezDoBloco = 14.0f;
};
