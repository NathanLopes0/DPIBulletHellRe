//
// O chefe da INF 250 - Organizacao de Computadores.
//

#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Logica.h"
#include "../../../Attacks/Relogio.h"
#include "../../../Attacks/Pipeline.h"
#include "../../../Attacks/Blink.h"

/**
 * @class Ricardo
 * @brief O professor de Organizacao de Computadores (INF 250). Ele tambem da
 * Sistemas Logicos e Arquitetura, e pesquisa hardware.
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/ricardo/fases.json e as regras no regras.json ao lado. O que
 * mora aqui e o que dados nao sabem fazer.
 *
 * A FASE 1 E SISTEMAS LOGICOS (Logica.h): palavras de 16 bits descem como
 * paredes - o UM machuca, o ZERO se atravessa - e cruzam uma fileira de portas
 * logicas, uma por coluna, que transformam cada bit. Aqui moram as portas (a
 * cara de cada uma, conforme a etapa), a travessia, e o sorteio da palavra.
 *
 * A FASE 2 E ORGANIZACAO DE COMPUTADORES: o clock (Relogio.h). Os tiros sao
 * registradores, que so andam na borda de subida, e a cada segundo sai um
 * leque com o valor de um contador Johnson. Aqui moram o relogio dos tiros, a
 * mira do leque e os bits dele; o professor anda na onda quadrada sozinho
 * (OndaQuadradaStrategy), lendo o mesmo placar.
 *
 * A FASE 3 E ARQUITETURA: o pipeline (Pipeline.h). Cinco estagios atravessados
 * no alto da tela, e as instrucoes de um laco em assembly andando neles, uma
 * por ciclo, cada uma atacando no seu estagio. Aqui moram os chips em campo,
 * os ataques que saem de cada estagio e o placar, que diz o que esta no EX.
 *
 * A FINAL E HARDWARE: quatro Arduinos rodando o Blink (Blink.h), duas placas
 * de cada lado dele, e o lado que acende atira. Aqui mora o LED de cada placa.
 */
class Ricardo : public Boss {
public:
    explicit Ricardo(Scene* scene);
    ~Ricardo() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- AS PORTAS DA FASE 1 ------------------------------------------------
    //
    // Logo que a fase comeca o Ricardo forma a fileira de portas (o gatilho
    // "portas"), e dali em diante, a cada quadro: poe em cada porta a cara da
    // operacao da etapa, e transforma todo bit que chega na altura delas. SO O
    // CORPO ORIGINAL - ele nao se divide, mas um clone nao teria portas suas.

    void AtualizarPortas();

    /// Os projeteis do chefe que estao no posto nomeado.
    [[nodiscard]] std::vector<BossProjectile*> NoPosto(const char* posto) const;

    /// A PORTA ESCRITA NO PLACAR no instante 't' da fase 1 - o placar e a
    /// porta (Logica::LerPorta). Se o placar daquele instante nao for porta, a
    /// ultima que valeu.
    [[nodiscard]] Logica::Porta PortaEm(float t) const;

    /// O que as portas fazem agora.
    Logica::Porta mPorta;
    bool mPortasFormadas = false;

    /// Formando as portas: o CustomizeAttackParams nao sorteia palavra para a
    /// fileira delas.
    bool mDisparandoPortas = false;

    /// Quanto depois do comeco da fase as portas se formam: antes da primeira
    /// palavra, que sai a 1 s.
    static constexpr float kFormaAsPortas = 0.2f;

    // ---- O CLOCK DA FASE 2 --------------------------------------------------
    //
    // A cada quadro: se houve borda de subida do clock do placar, todo
    // registrador em campo da um passo; se o segundo virou, o contador anda e
    // sai um leque (o gatilho "contador"). Quando a fase acaba, o clock para e
    // os registradores que sobraram escorrem para fora da tela.

    void AtualizarRelogio();

    /// Todo registrador em campo passa a andar a 'velocidade', na direcao
    /// dele (zero: parado).
    void MoverRegistradores(float velocidade);

    /// A DIRECAO DE CADA REGISTRADOR, anotada aqui e nao no vetor velocidade:
    /// parado, ele tem velocidade ZERO, e zero nao guarda direcao (e o
    /// RigidBody zera o x abaixo de 1 px/s, entao nem "quase zero" guarda).
    /// O registrador nasce andando na direcao do leque, e no primeiro quadro
    /// em que se move ela e anotada e ele para.
    std::unordered_map<const BossProjectile*, Vector2> mDirecoes;

    /// O tempo da fase no quadro anterior (negativo: fora da fase do clock).
    float mAntesNoRelogio = -1.0f;
    /// Quando o passo em curso acaba (negativo: nenhum passo em curso).
    float mFimDoPasso = -1.0f;
    /// A frequencia do ultimo quadro na fase - a dos registradores que sobram.
    float mFrequencia = 1.0f;
    /// O estado do contador Johnson: o do proximo leque.
    int mContador = 1;

    // ---- O PIPELINE DA FASE 3 -----------------------------------------------
    //
    // Logo que a fase comeca o Ricardo forma os cinco estagios (o gatilho
    // "estagios"), e a cada ciclo: o chip derrubado vira bolha, todo mundo anda
    // um estagio, a instrucao buscada entra no IF (o gatilho "buscar"), e quem
    // chegou ao estagio do seu ataque o faz, de la. O flush do bne mal previsto
    // transforma em bolha o que estava no IF e no ID.

    void AtualizarPipeline(float deltaTime);

    /// Dispara o gatilho de dentro do estagio: o ataque sai dali, e nao do
    /// professor. 'mirar': o leque sai mirado no aluno.
    void DispararDoEstagio(const std::string& gatilho, int estagio, bool mirar);

    /// Onde fica cada estagio: o meio das molduras, da esquerda para a direita.
    /// Vazio enquanto elas nao se formaram.
    [[nodiscard]] std::vector<Vector2> CentrosDosEstagios() const;

    Pipeline::Estado mPipeline;
    /// O chip de cada estagio, ou nenhum - a bolha que o aluno fez a tiro nao
    /// tem chip; a do flush tem, e anda vazia ate o fim.
    std::array<BossProjectile*, Pipeline::kEstagios> mChips{};
    bool mNoPipeline = false;
    bool mEstagiosFormados = false;
    float mTempoNoPipeline = 0.0f;
    float mProximoCiclo = 0.0f;
    std::string mNomeDaFaseDoPipeline;

    /// Disparando de um estagio: de onde, se mira, e o chip que a busca criou.
    bool mDisparandoDoEstagio = false;
    bool mMirandoDoEstagio = false;
    bool mFormandoEstagios = false;
    Vector2 mDisparoDoEstagio;
    BossProjectile* mBuscada = nullptr;

    /// Quao depressa o chip anda de um estagio ao seguinte (Ancora::Aproximar).
    static constexpr float kRapidezDoChip = 12.0f;

    // ---- O BLINK DA FINAL ---------------------------------------------------
    //
    // A cada quadro, cada placa em campo acende ou apaga conforme o lado dela
    // e o delay do placar - a acesa vai para o posto "acesas", de onde o leque
    // sai -, e quando o LED troca de lado as placas que acenderam atiram (o
    // gatilho "acender").

    void AtualizarBlink();

    /// O tempo da fase no quadro anterior (negativo: fora do Blink).
    float mAntesNoBlink = -1.0f;

    /// Quanto abaixo do professor a placa ja esta solta: a mais funda das
    /// presas fica a 100 px (regras.json), e a Ancorar nao passa do lugar.
    static constexpr float kSoltaAbaixo = 160.0f;
};
