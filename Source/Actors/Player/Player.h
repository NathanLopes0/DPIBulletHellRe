//
// Created by nslop on 03/10/2024.
//

#ifndef DPIBULLETHELLRE_PLAYER_H
#define DPIBULLETHELLRE_PLAYER_H

#include <cstddef>

#include "../Actor.h"
#include "../../Input/Painel.h"
#include "../../Gameplay/Acerto.h"
#include "../../Gameplay/PontoExtra.h"

class PlayerProjectile;
class PlayerOrb;

class Player : public Actor {

public:
    /**
     * @param quem QUAL POSICAO do painel move este aluno, e de qual sessao sai
     *        a aparencia dele. As duas coisas sao a mesma posicao, e por isso
     *        entram num parametro so - com duas, haveria como criar um aluno
     *        que anda com o manche de um e tem a cara do outro.
     *
     *        O padrao e a posicao 1, que e o jogo de um jogador.
     */
    explicit Player(Scene* scene, size_t quem = 0);

    //Process Input -> Update, sobrescritas de Actor para Player
    void OnProcessInput(const Uint8* keyState) override;

    void UpdateOverload(float deltaTime);

    void OnUpdate(float deltaTime) override;

    void SetIsInvincible(const bool invincible) { isInvincible = invincible; }

    /// INVENCIVEL POR QUALQUER MOTIVO: os quadros de graca depois de apanhar OU
    /// a imunidade dada pelo parceiro. As duas respondem a mesma pergunta - "um
    /// tiro encosta neste aluno agora?" - e juntando-as aqui a Battle nao
    /// precisou de linha nenhuma: a regra 1 e a regra 3 ja perguntavam isto.
    [[nodiscard]] bool GetIsInvincible() const { return isInvincible || mImunidadeTimer > 0.0f; }

    /// O MESMO CAUSADOR nao acerta duas vezes seguidas (Acerto.h): 'quem' e o
    /// projetil ou o corpo do chefe, e 'idade' a vida dele (0 para o corpo).
    [[nodiscard]] bool JaFoiAcertadoPor(const void* quem, const float idade) const { return mAcertos.Lembra(quem, idade); }
    void FoiAcertadoPor(const void* quem, const float idade) { mAcertos.Lembrar(quem, idade); }

    /**
     * @brief Protege este aluno por um tempo, as custas do Ponto Extra do parceiro.
     *
     * RENOVA, NAO ACUMULA. Dois Pontos Extras seguidos dao tres segundos a
     * partir de agora, e nao seis: acumular premiaria guardar os dois estoques
     * para o mesmo instante, que e o contrario de uma ajuda dada quando o outro
     * precisa.
     */
    void Proteger(float segundos);

    /**
     * @brief Prende o aluno: ele anda a uma fracao da velocidade por um tempo.
     *
     * E o efeito das CORRENTES do Vitor - a piada de "concorrentes". Um ataque
     * que quase nao cobra nota e em troca ATRAPALHA, que e um tipo que o jogo
     * nao tinha.
     *
     * RENOVA, NAO ACUMULA, pelo mesmo motivo de Proteger: duas correntes
     * seguidas nao devem somar seis segundos de prisao, e uma corrente nova
     * tambem nao deve ENCURTAR uma mais longa que ja estivesse valendo. O fator
     * acompanha o timer vencedor - se o que vale e o novo, vale o fator dele.
     */
    void Acorrentar(float segundos, float fatorDeVelocidade);

    /// @brief O aluno esta preso agora? Battle e as regras de projetil leem isto.
    [[nodiscard]] bool EstaAcorrentado() const { return mCorrenteTimer > 0.0f; }

    /**
     * @brief Quao rapido o aluno anda NESTE instante, ja com a corrente.
     *
     * FONTE UNICA DE PROPOSITO. MoveInput usa isto, e os pilotos automaticos
     * dos arneses de medicao tambem precisam usar: se eles escreverem
     * PLAYER_SPEED_START direto, atravessam a corrente sem sentir e toda
     * medicao de dificuldade passa a mentir sobre o efeito que mais importa
     * desta fase.
     */
    [[nodiscard]] float VelocidadeDeAndar() const {
        return playerSpeed * (mCorrenteTimer > 0.0f ? mFatorDaCorrente : 1.0f);
    }

    /// Quanto ainda falta da imunidade do parceiro. Zero quer dizer desprotegido.
    [[nodiscard]] float TempoDeImunidade() const { return mImunidadeTimer; }

    /**
     * O tempo que quem gasta um Ponto Extra SOZINHO passa sem atirar - e, no
     * co-op, o tempo EXATO que o parceiro passa imune e com o escudo, e o que
     * quem gastou espera ate poder gastar outro.
     *
     * UM NUMERO SO, e publico por isso. A Battle precisa dele para pagar a
     * imunidade; se fossem dois, mexer num e esquecer o outro faria o presente
     * deixar de custar o que custa.
     *
     * NO CO-OP QUEM GASTA CONTINUA ATIRANDO. A sobrecarga era o preco do
     * presente, e deixava a dupla com um atirador so por tres segundos -
     * justamente quando o outro estava protegido e podia avancar. O preco do
     * co-op passou a ser o estoque: o Ponto Extra vai inteiro para o outro.
     */

    /// DOIS NUMEROS AGORA, e nao um (PontoExtra.h): a sobrecarga do solo caiu
    /// para 1,5 s, e o presente do co-op continuou com os 3 s de imunidade -
    /// encurtar um nao tinha motivo para encurtar o outro.
    static constexpr float OVERLOAD_DURATION = PontoExtra::kSobrecarga;
    static constexpr float IMUNIDADE_DO_PRESENTE = PontoExtra::kImunidadeDoPresente;

    [[nodiscard]] int GetExtraPoints() const { return mExtraPoints;}

    /// Quantos tiros ele ja deu na batalha. O Observer da Maria Lucia, como
    /// orientadora do TCC, atira a cada tantos (Golpe::Notificacoes).
    [[nodiscard]] int Disparos() const { return mDisparos; }
    void AddExtraPoint();


private:

    //variaveis de jogador
    float playerSpeed;
    float atkTimer;
    float invincibleTimer;
    bool isInvincible;
    /// Tempo desde a ultima troca de visibilidade durante a invencibilidade.
    /// Era um 'static float' dentro de InvincibleUpdate: estado de INSTANCIA
    /// guardado em variavel de funcao, compartilhado por todas as instancias e
    /// persistente entre cenas. Funcionava porque os dois caminhos de saida o
    /// reinicializam, mas so por sorte.
    float mBlinkTimer;
    bool mMoving;                       //ve se o jogador está se movendo. Usada para mudar a animação
    /// A posicao deste aluno no painel. Ver o construtor.
    size_t mQuem = 0;

    /// O painel que move este aluno. Uma funcao, e nao a conversao repetida em
    /// cada leitura: foi a conversao escrita duas vezes que fez a tela de
    /// matricula ler o botao do jogador 1 e anunciar o do jogador 2.
    [[nodiscard]] Painel::Jogador MeuPainel() const {
        return mQuem == 1 ? Painel::Jogador::Dois : Painel::Jogador::Um;
    }

    int mExtraPoints = 1;
    int mDisparos = 0;
    float mOverloadTimer = 0.0f;
    float mPulseTimer = 0.0f;

    /// Quanto falta para este aluno poder gastar outro Ponto Extra. No solo e a
    /// sobrecarga; no co-op, quem gasta segue atirando, e isto e o que impede
    /// que segurar o botao queime os dois estoques em dois quadros.
    float mEsperaDoPontoExtra = 0.0f;


    /// Quanto resta da imunidade dada pelo parceiro. SEPARADO de invincibleTimer
    /// de proposito: os quadros de graca de quem apanhou duram 1,2s e piscam, e
    /// misturar os dois faria "fui atingido" e "estou protegido" terem a mesma
    /// cara - justamente no momento em que o aluno precisa saber qual dos dois e.
    float mImunidadeTimer = 0.0f;

    /// Quanto falta da corrente, e quanto ela corta da velocidade. Ver
    /// Acorrentar.
    ///
    /// O FATOR NAO MEXE EM playerSpeed, de proposito: ele e aplicado no uso, em
    /// MoveInput. Guardar a velocidade antiga para devolver depois seria um
    /// estado a mais para errar - bastaria um caminho de saida esquecido para o
    /// aluno ficar lento o resto da batalha.
    /// So para o RITMO do pulso: a cor acelera conforme a corrente acaba, e
    /// precisa de uma referencia para saber "quanto falta". Nao limita nada -
    /// uma corrente mais longa que isto simplesmente comeca pulsando devagar.
    static constexpr float kDuracaoTipicaDaCorrente = 3.5f;

    float mCorrenteTimer = 0.0f;
    float mFatorDaCorrente = 1.0f;
    float mPulsoDaCorrente = 0.0f;

    /// O pulso da imunidade, SEPARADO de mPulseTimer. Quase reaproveitei aquele
    /// e o efeito sumia: UpdateOverload zera mPulseTimer a cada quadro em que
    /// nao ha sobrecarga, e quem RECEBE a imunidade normalmente nao esta
    /// sobrecarregado - o seno nunca saia de perto de zero e a cor ficava num
    /// cinza-azulado parado em vez de pulsar.
    float mPulsoDaImunidade = 0.0f;



    //OnProcessInput Sub-functions, and its sub-functions
    void MoveInput(const Uint8 *state);
    void ShootInput(const Uint8 *state);
        void Shoot();
    void SpecialInput(const Uint8 *state);


    //OnUpdate Sub-functions
    void InvincibleUpdate(float deltaTime);
    Acerto::Memoria mAcertos;
    void UpdateImunidade(float deltaTime);
    void UpdateCorrente(float deltaTime);

    /**
     * @brief O UNICO lugar que pinta o aluno.
     *
     * Os tres estados que mudam a cor podem valer ao mesmo tempo, e a ordem de
     * prioridade e esta:
     *
     *   imunidade   decide se da para atravessar um tiro - ganha de todas
     *   overload    o gatilho mudo, que dura pouco
     *   corrente    dura mais e e a menos urgente
     *
     * Cada Update* so anda o relogio e guarda o pulso dele; nenhum chama
     * SetColor. Era assim que um bug se escondia: quem rodasse por ultimo
     * ganhava, e o ramo "else" do overload pintava de branco todo quadro em que
     * o aluno nao estava sobrecarregado.
     */
    void AtualizarCorDoEstado();
    void HandleAnimation();
    void DecreaseAtkTimer(float deltaTime);

    void BorderLimitCheck();

};


#endif //DPIBULLETHELLRE_PLAYER_H
