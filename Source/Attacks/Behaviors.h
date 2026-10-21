//
// Created by gensh on 17/10/2025.
//

#pragma once

#include "../Math.h"
#include "PathAim.h"
#include "Amarra.h"
#include <vector>
#include <memory>
#include <string>
#include <utility>

class Projectile;

/**
 * @interface ProjectileBehavior
 * @brief Define o contrato para qualquer "acessório" (behavior)
 * que pode ser anexado a um Projétil.
 */
class ProjectileBehavior {
public:
    virtual ~ProjectileBehavior() = default;

    virtual void update(class Projectile* projectile, float deltaTime) = 0;
    virtual bool isFinished() const { return false; }
};

/**
 * @brief Behavior que ESCREVE a velocidade do projetil.
 *
 * No maximo UM por projetil: dois escritores brigariam frame a frame e o
 * ultimo a rodar venceria, o que produz movimento aleatorio e dificil de
 * depurar. Projectile::insertMotion garante a exclusividade.
 *
 * Sao Motion: Tracking, Wobble, Path, MiraPeriodica e Amarrar. (O antigo HomingBehavior foi removido
 * na fase 3: um homing e um PathBehavior com PathShapes::Reta() e
 * Mira(MirarNoJogador).)
 */
class ProjectileMotion : public ProjectileBehavior {
};

/**
 * @brief Behavior que MODIFICA a velocidade ja existente, ou agenda quando ela
 * comeca a valer. Varios podem coexistir no mesmo projetil.
 *
 * Sao Modifier: Accelerate, SlowDown, Activate, Deactivate, PulsoDeVelocidade,
 * Reescrever, Deslocar e Promessa.
 *
 * O QUE ESTA SEPARACAO GARANTE, E O QUE NAO GARANTE
 *
 * Garante, pelo tipo: no maximo UMA Motion por projetil. insertMotion
 * substitui a anterior, entao duas Motions nunca brigam pela velocidade.
 *
 * NAO garante, pelo tipo, que um Modifier sobreviva a uma Motion. Isso depende
 * de cada Motion cumprir o contrato abaixo. Uma versao anterior deste
 * comentario afirmava que "Path + Accelerate agora nao compila" - era falso:
 * compilava, e o Accelerate era anulado no frame seguinte pelo Path, que
 * reescrevia o modulo. Corrigido fazendo toda Motion cumprir o contrato.
 *
 * CONTRATO DE ProjectileMotion: decide a DIRECAO, preserva o MODULO. Use
 * DirecionarPreservandoModulo (PathAim.h) em vez de SetVelocity(dir * x) para
 * nao quebra-lo. Coberto por test_motion_contract.cpp.
 */
class ProjectileModifier : public ProjectileBehavior {
};

struct AccelerateBehavior : public ProjectileModifier {
    float accelerateDelay, accelerateSpeedPercent, elapsedTime;
    int accelerateSpeedValue;
    bool accelerated;

    /**
     * @brief comportamento de acelerar o projétil. Se o segundo parametro for um INT, aumentará exatamente aquela velocidade.
     * Se for um FLOAT, multiplicará a velocidade do projétil pelo valor passado (1.5f aumentará a velocidade em 50%, por exemplo)
     * @param delay quanto tempo até o projétil acelerar
     * @param accelerateByXTimes valor que será multiplicado pela velocidade
     */
    explicit AccelerateBehavior(float delay = 0, float accelerateByXTimes = 0) :
    accelerateDelay(delay), accelerateSpeedPercent(accelerateByXTimes), elapsedTime(0.0f), accelerated(false),
    accelerateSpeedValue(-1) {};

    explicit AccelerateBehavior(float delay = 0, int accelerateByValue = 0) :
    accelerateDelay(delay), accelerateSpeedValue(accelerateByValue), elapsedTime(0.0f), accelerated(false),
    accelerateSpeedPercent(-1.0f) {};


    void update(Projectile* p, float deltaTime) override; // Assinatura corrigida
    bool isFinished() const override { return accelerated; }
};
struct SlowDownBehavior : public ProjectileModifier {
    float slowdownDelay, slowdownSpeedPercent, elapsedTime;
    int slowdownSpeedValue;
    bool slowedDown;


    explicit SlowDownBehavior(float delay = 0, float desaccelerateByXTimes = 0) :
            slowdownDelay(delay), slowdownSpeedPercent(desaccelerateByXTimes), elapsedTime(0.0f), slowdownSpeedValue(-1),
            slowedDown(false) {};
    explicit SlowDownBehavior(float delay = 0, int desaccelerateByValue = 0) :
            slowdownDelay(delay), slowdownSpeedPercent(-1.f), elapsedTime(0.0f), slowdownSpeedValue(desaccelerateByValue),
            slowedDown(false) {};

    void update(Projectile* p, float deltaTime) override; // Assinatura corrigida
    bool isFinished() const override { return slowedDown; }
};

struct ActivateBehavior : public ProjectileModifier {
    float activationDelay, elapsedTime;
    Vector2 activationVelocity;
    bool activated;

    explicit ActivateBehavior(const float delay = 0, const Vector2 activateVelocity = Vector2::Zero) :
    activationDelay(delay), elapsedTime(0.0f), activationVelocity(activateVelocity), activated(false) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return activated; }

};

/**
 * @brief Correcao de rota CONTINUA em direcao ao jogador, com forca que decai
 * ate zero: o projetil persegue e depois se compromete.
 *
 * Diferente de um homing, que e um evento UNICO (aponta pro jogador uma
 * vez e se encerra), aqui a direcao e reajustada todo frame por uma fracao da
 * diferenca. O resultado visual e uma curva que fecha rapido no comeco e vai
 * afrouxando ate travar.
 *
 * A forca decai linearmente ao longo de trackingTime, o que da ao jogador uma
 * janela previsivel para desviar: fugir cedo nao resolve, fugir tarde sim.
 *
 * O modulo da velocidade e SEMPRE preservado: so a direcao muda.
 */
struct TrackingBehavior : public ProjectileMotion {

    /**
     * @param delay tempo ate comecar a corrigir a rota
     * @param trackingStrength quao forte e a correcao por segundo. Valores entre
     *        1.5 e 4.0 dao curvas legiveis; acima de ~6 o projetil praticamente
     *        gruda no jogador e o ataque fica injusto.
     * @param trackingTime em quantos segundos a forca decai a zero
     */
    explicit TrackingBehavior(float delay = 0.0f,
                                     float trackingStrength = 2.5f,
                                     float trackingTime = 2.0f)
        : startDelay(delay), trackingStrength(trackingStrength),
          trackingTime(trackingTime), elapsedTime(0.0f), finished(false) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return finished; }

    float startDelay;
    float trackingStrength;
    float trackingTime;
    float elapsedTime;
    bool finished;
};

/**
 * @brief Oscilacao lateral com amplitude decrescente em torno da trajetoria
 * original: o projetil serpenteia e vai se acalmando.
 *
 * Se dois projeteis vizinhos receberem amplitudes de sinal oposto, eles se
 * cruzam - visualmente muito bom para um leque.
 *
 * A direcao base e capturada UMA vez na ativacao, e nao relida a cada frame.
 * Se fosse relida, a oscilacao realimentaria a si mesma e o projetil sairia
 * girando em espiral.
 *
 * ATENCAO - NAO COMBINE com TrackingBehavior, AccelerateBehavior nem
 * SlowDownBehavior no mesmo projetil. Este behavior REESCREVE a velocidade
 * todo frame a partir da direcao e do modulo capturados na ativacao, entao
 * qualquer alteracao feita por outro behavior e desfeita no frame seguinte.
 * Use-o sozinho.
 */
struct WobbleBehavior : public ProjectileMotion {

    /**
     * @param delay tempo ate comecar a oscilar
     * @param maxAngleDegrees amplitude maxima do desvio, em graus. Negativo
     *        inverte a fase (util para alternar projeteis vizinhos).
     * @param frequency oscilacoes completas por segundo
     * @param duration em quantos segundos a amplitude decai a zero
     */
    explicit WobbleBehavior(float delay = 0.0f,
                             float maxAngleDegrees = 45.0f,
                             float frequency = 1.5f,
                             float duration = 2.5f)
        : startDelay(delay), maxAngle(maxAngleDegrees), frequency(frequency),
          duration(duration), elapsedTime(0.0f), baseDirection(Vector2::Zero),
          baseSpeed(0.0f), started(false), finished(false) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return finished; }

    float startDelay;
    float maxAngle;
    float frequency;
    float duration;
    float elapsedTime;
    Vector2 baseDirection;
    float baseSpeed;
    bool started;
    bool finished;
};

/**
 * @brief Conduz o projetil por uma lista de waypoints - "pathing" autoral.
 *
 * COMO FUNCIONA
 * Os waypoints sao OFFSETS em "espaco de caminho", nao coordenadas de tela:
 *   +X = a direcao em que o projetil ja estava viajando quando o behavior
 *        ativou;
 *   +Y = a perpendicular, a direita desse movimento.
 * Na ativacao, o behavior captura a posicao e a direcao atuais do projetil e
 * gira o caminho inteiro para alinhar com elas. A consequencia util: o MESMO
 * caminho disparado por um CircleSpreadAttack vira um caminho diferente para
 * cada projetil do anel, ja que cada um tem sua propria direcao inicial.
 *
 * POR QUE ESCREVE VELOCIDADE, E NAO POSICAO
 * Seria mais simples chamar SetPosition a cada frame, mas o RigidBodyComponent
 * roda ANTES dos behaviors (Actor::Update atualiza componentes e so depois
 * chama OnUpdate) e faz position += velocity * dt. Escrever posicao direto
 * colocaria os dois para brigar todo frame. Definindo a velocidade que aponta
 * ao proximo waypoint, o RigidBody continua sendo o unico que escreve posicao,
 * e colisao, IsOffScreen e todo o resto seguem funcionando sem saber que existe
 * um caminho.
 *
 * AO TERMINAR O CAMINHO o projetil segue RETO na ultima direcao. Ele nunca
 * para: um projetil parado nunca satisfaz IsOffScreen(), nunca e devolvido ao
 * pool e fica ocupando a tela para sempre.
 *
 * ATENCAO - EXCLUSIVO. Vale aqui a mesma regra do WobbleBehavior: behaviors que
 * ESCREVEM velocidade nao compoem com os que a modificam. Nao combine com
 * TrackingBehavior, AccelerateBehavior, SlowDownBehavior nem WobbleBehavior.
 * Activate/DeactivateBehavior continuam seguros, desde que o Activate dispare
 * ANTES deste (senao a direcao capturada seria zero).
 */
struct PathBehavior : public ProjectileMotion {

    /**
     * @param waypoints Offsets em espaco de caminho, na ordem de percurso.
     *        Use as funcoes de PathShapes.h em vez de digitar na mao.
     * @param pathSpeed Velocidade ao longo do caminho. 0 = mantem o modulo da
     *        velocidade que o projetil ja tinha.
     * @param delay Segundos ate comecar a seguir o caminho.
     * @param mira Regra de orientacao. O padrao alinha o caminho com a direcao
     *        de voo, que e o comportamento historico - por isso todas as
     *        chamadas existentes continuam validas sem alteracao.
     * @param pararNoPonto Indice do waypoint ao chegar no qual o projetil FICA
     *        PARADO por pararPor segundos. -1 (o padrao) e "nunca para", entao
     *        todo caminho escrito antes desta opcao continua igual.
     * @param pararPor Duracao da parada, em segundos.
     */
    explicit PathBehavior(std::shared_ptr<const std::vector<Vector2>> waypoints,
                          float pathSpeed = 0.0f,
                          float delay = 0.0f,
                          Mira mira = Mira(),
                          int pararNoPonto = -1,
                          float pararPor = 0.0f)
        : waypoints(std::move(waypoints)), pathSpeed(pathSpeed), mira(mira),
          startDelay(delay), elapsedTime(0.0f), current(0),
          origin(Vector2::Zero), cosR(1.0f), sinR(0.0f),
          started(false), finished(false),
          pararNoPonto(pararNoPonto), pararPor(pararPor),
          paradaRestante(0.0f), paradaFeita(false) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return finished; }

    std::shared_ptr<const std::vector<Vector2>> waypoints;  // Flyweight: compartilhada
    float pathSpeed;
    Mira mira;   ///< regra que decide a orientacao do caminho
    float startDelay;
    float elapsedTime;
    size_t current;
    Vector2 origin;   // posicao capturada na ativacao
    float cosR, sinR; // rotacao capturada na ativacao
    bool started;
    bool finished;

    /// A PARADA NUM PONTO DO CAMINHO.
    ///
    /// Existe porque uma pausa nao da para compor com o que ja havia: Motion e
    /// EXCLUSIVA (insertMotion substitui), entao nao ha como ter um Path mais
    /// outra coisa que o segure, e fazer a parada com SlowDown + Accelerate
    /// dependeria de fatores reciprocos exatos e de adivinhar o instante em que o
    /// projetil chega ao ponto - que varia com a posicao de onde ele partiu.
    ///
    /// Quem sabe que o projetil CHEGOU num waypoint e o proprio PathBehavior, e e
    /// por isso que a parada mora aqui.
    ///
    /// Durante a parada a velocidade e zero, mas o projetil continua VISIVEL e com
    /// colisor ligado - ele vira um obstaculo parado, nao some. Retomar sai de
    /// graca: DirecionarPreservandoModulo cai no pathSpeed quando a velocidade
    /// atual e zero, entao o projetil parte de novo no modulo certo, ja na direcao
    /// do proximo waypoint.
    int   pararNoPonto;      ///< -1 = nunca para
    float pararPor;          ///< segundos
    float paradaRestante;
    bool  paradaFeita;       ///< so para UMA vez, mesmo que o caminho volte ao ponto
};

/**
 * @brief Re-aponta a velocidade para o jogador em instantes DISCRETOS.
 *
 * Diferente do TrackingBehavior, que corrige a rota um pouco a cada frame, esta
 * Motion nao faz nada entre uma mirada e outra: no instante marcado ela gira a
 * velocidade para o jogador e volta a dormir. E o que produz a leitura de
 * "parou, decidiu, e partiu" - o jogador ve a decisao acontecer.
 *
 * Nao mexe no modulo, como manda o contrato de ProjectileMotion. Quem faz a
 * velocidade subir e descer e um Modifier, tipicamente o
 * PulsoDeVelocidadeBehavior; as duas pecas nao sabem uma da outra.
 *
 * ATENCAO: a direcao mora dentro do vetor velocidade. Se o projetil estiver com
 * modulo ZERO no instante da mirada, nao ha o que girar e ela nao tem efeito.
 * Por isso uma "pausa" deve deixar um resto de velocidade (10 a 20 px/s), e nao
 * parar de verdade.
 */
struct MiraPeriodicaBehavior : public ProjectileMotion {

    /**
     * @param ritmo os tempos do ataque. Esta Motion re-aponta no INICIO de cada
     *        investida, entao ela deriva os instantes do mesmo objeto que o
     *        PulsoDeVelocidadeBehavior usa para pulsar - nao ha um intervalo
     *        calculado a mao que precise bater com nada.
     */
    explicit MiraPeriodicaBehavior(const RitmoDeCiclos& ritmo)
        : ritmo(ritmo), elapsedTime(0.0f), feitas(0) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return feitas >= ritmo.repeticoes; }

    RitmoDeCiclos ritmo;
    float elapsedTime;
    int feitas;
};

/**
 * @brief Alterna o modulo da velocidade entre uma investida rapida e uma pausa
 * quase parada, repetidas vezes.
 *
 * Nao toca na direcao, como manda o contrato de ProjectileModifier: o projetil
 * acelera e freia exatamente no rumo em que a Motion o deixou.
 *
 * Diferente de Accelerate e SlowDown, que multiplicam uma vez e se encerram,
 * este DEFINE o modulo enquanto vive. A diferenca importa: fatores
 * multiplicativos se acumulam, e voltar a uma velocidade depois de varios
 * ciclos exigiria fatores reciprocos exatos, o que erra por arredondamento.
 *
 * Toda a aritmetica de tempo esta em CalcularFaseDoPulso (PathAim.h), que e
 * pura e coberta por tests/test_investida.cpp.
 */
struct PulsoDeVelocidadeBehavior : public ProjectileModifier {

    /**
     * @param ritmo os tempos do ataque, os MESMOS entregues a Motion.
     * @param moduloInvestida velocidade durante a investida
     * @param moduloPausa velocidade durante a pausa. NAO use zero - a direcao
     *        mora dentro do vetor velocidade, e com modulo zero ela se perde.
     *
     * Este Modifier DEFINE o modulo enquanto vive, em vez de multiplicar. Como
     * consequencia, ele nao convive com Accelerate nem SlowDown no mesmo
     * intervalo: o que eles fizerem sera reescrito no frame seguinte.
     */
    explicit PulsoDeVelocidadeBehavior(const RitmoDeCiclos& ritmo,
                                       float moduloInvestida = 800.0f,
                                       float moduloPausa = 15.0f)
        : ritmo(ritmo), moduloInvestida(moduloInvestida), moduloPausa(moduloPausa),
          elapsedTime(0.0f), terminou(false) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return terminou; }

    RitmoDeCiclos ritmo;
    float moduloInvestida, moduloPausa;
    float elapsedTime;
    bool terminou;
};

struct DeactivateBehavior : public ProjectileModifier {
    float elapsedTime, deactivationDelay;
    bool deactivated;

    explicit DeactivateBehavior(const float delay = 0) : elapsedTime(0.0f), deactivationDelay(delay), deactivated(false) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return deactivated; }
};

/**
 * @brief A CASA DA FITA: o projetil fica onde esta ate a cabeca passar por cima,
 * e entao e REESCRITO - troca de letra e sai perseguindo o alvo.
 *
 * E a maquina de Turing da fase final do Vladimir. O chefe escreve a fita na
 * passagem (simbolos que descem e param), e quando a varredura o traz de volta
 * por cima de uma casa, ele a le e a reescreve: o "a" parado vira "b".
 *
 * A CABECA E QUALQUER CORPO DO CHEFE (Battle::Corpos), e nao o dono do
 * projetil. O dono e quem disparou; quem LE a fita e quem estiver em cima dela
 * agora, e com o chefe dividido em dois os dois leem. Ir pela cena tambem evita
 * guardar um ponteiro para um corpo que pode deixar de existir.
 *
 * A decisao de "quando" e pura e esta em Fita.h, coberta por test_fita.cpp. Aqui
 * fica so o efeito.
 *
 * ESTE MODIFIER TROCA A MOTION DO PROJETIL, e isso so e seguro porque
 * Projectile::OnUpdate roda a Motion ANTES do laco dos Modifiers: substituir
 * mMotion dentro do laco nao mexe em nada que esteja sendo percorrido. Inserir
 * um MODIFIER daqui, ao contrario, realocaria o vetor no meio do laco - por isso
 * a reescrita nao insere modifier nenhum.
 */
struct ReescreverBehavior : public ProjectileModifier {

    /**
     * @param idadeMinima segundos antes de a casa poder ser lida. A casa nasce
     *        EMBAIXO da cabeca: sem isto, seria reescrita no quadro em que foi
     *        escrita.
     * @param raio        a que distancia, no x, a cabeca le a casa
     * @param escreve     a animacao da letra nova
     * @param velocidade  com que modulo a casa reescrita sai em direcao ao alvo
     * @param forca, duracao o Tracking que ela leva depois de sair
     */
    ReescreverBehavior(float idadeMinima, float raio, std::string escreve,
                       float velocidade, float forca, float duracao)
        : idadeMinima(idadeMinima), raio(raio), escreve(std::move(escreve)),
          velocidade(velocidade), forca(forca), duracao(duracao) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return reescrita; }

    float idadeMinima, raio;
    std::string escreve;
    float velocidade, forca, duracao;
    float idade = 0.0f;
    bool reescrita = false;
};

/**
 * @brief Desloca o projetil 'dx' pixels na horizontal, comecando em 'atraso' e
 * levando 'duracao' segundos, pela curva suave de uma transition do CSS.
 *
 * E a transicao de justify-content da FlexAttack: cada item da fileira escorrega
 * de onde o primeiro arranjo o pos ate onde o segundo o poe.
 *
 * MEXE NA POSICAO, E NAO NA VELOCIDADE. Somar uma velocidade horizontal por
 * 'duracao' segundos erraria o destino em ate um quadro de cada lado - a
 * fileira chegaria torta, e um arranjo torto nao se le como arranjo. Aqui cada
 * quadro aplica so o que falta para a posicao da curva (Flex::Progresso), entao
 * o total e exatamente 'dx', e a queda - que e da velocidade - segue intacta.
 */
struct DeslocarBehavior : public ProjectileModifier {

    DeslocarBehavior(float atraso, float duracao, float dx)
        : atraso(atraso), duracao(duracao), dx(dx) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return terminou; }

    float atraso, duracao, dx;
    float t = 0.0f;
    float jaDeslocado = 0.0f;
    bool terminou = false;
};

/**
 * @brief A PROMISE: o projetil espera pendente e, aos 'atraso' segundos, se
 * resolve ou e rejeitado.
 *
 * Resolvida, ela sai a 'velocidade' ja apontada para o alvo e troca para a
 * animacao 'escreve' (se houver). Rejeitada, ela some - vai para o pool, como
 * um projetil que saiu da tela. 'fracao' e a parte das promessas que resolvem;
 * a decisao e pura e esta em Promessa.h.
 *
 * Ela nao espera parada sozinha: quem a para e um SlowDown antes dela, na mesma
 * regra. Assim a promessa pode tambem resolver no meio do voo, se a regra
 * quiser.
 *
 * NAO INSERE MODIFIER NENHUM, pelo mesmo motivo do Reescrever: roda dentro do
 * laco de modifiers. Matar o proprio projetil daqui e seguro - Kill() so marca
 * o estado, e quem limpa os behaviors e o pool, depois.
 */
struct PromessaBehavior : public ProjectileModifier {

    PromessaBehavior(float atraso, float fracao, float velocidade, std::string escreve)
        : atraso(atraso), fracao(fracao), velocidade(velocidade), escreve(std::move(escreve)) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return assentou; }

    float atraso, fracao, velocidade;
    std::string escreve;
    float t = 0.0f;
    bool assentou = false;
};

/**
 * @brief A LETRA AMARRADA (ligare, "atar"): presa ao professor por uma corda que
 * nao se ve, ela gira em volta dele enquanto a amarracao dura, e depois sai
 * pela tangente, como a pedra de uma funda.
 *
 * E a amarracao DINAMICA da fase 1 do Luizinho: o no da corda e procurado a
 * cada quadro, entao a letra vai junto para onde o professor for. A conta do
 * quadro e pura e esta em Amarra.h, coberta por test_amarra.cpp.
 *
 * O NO E O CORPO DO CHEFE MAIS PERTO DA LETRA (Battle::Corpos), procurado a
 * cada quadro, e nao o dono guardado num ponteiro: um clone que deixasse de
 * existir deixaria a corda presa a memoria solta. Com um corpo so, e ele.
 *
 * E UMA MOTION, e cumpre o contrato: decide a direcao e preserva o modulo. Ela
 * tambem poe a letra de volta na circunferencia a cada quadro (o RigidBody,
 * que roda antes, a leva pela tangente, um pouco para fora), como o Deslocar
 * mexe na posicao.
 *
 * 'escreve', se houver, e a animacao que a letra passa a ter quando a corda
 * estica - a amarracao acontecendo na frente do aluno.
 */
struct AmarrarBehavior : public ProjectileMotion {
    AmarrarBehavior(const Amarra::Corda& corda, std::string escreve)
        : corda(corda), escreve(std::move(escreve)) {}
    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return corda.cortada; }
    Amarra::Corda corda;
    std::string escreve;
};
