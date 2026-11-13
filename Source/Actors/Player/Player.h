//
// O aluno: anda, atira, gasta o Ponto Extra e mostra no corpo o estado em que esta.
//

#ifndef DPIBULLETHELLRE_PLAYER_H
#define DPIBULLETHELLRE_PLAYER_H

#include <cstddef>

#include "../Actor.h"
#include "PilotoDoAluno.h"
#include "../../Entrada/Painel.h"
#include "../../Regras/Acerto.h"
#include "../../Regras/PontoExtra.h"
#include "../../Regras/Raspao.h"
#include "../../Personagem/ComporPersonagem.h"

class DrawAnimatedComponent;
class RigidBodyComponent;

class Player : public Actor {

public:
    /// 'quem': a posicao do aluno no painel (0 e o jogador 1).
    explicit Player(Scene* scene, size_t quem = 0);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

    /// Os quadros de graca depois de apanhar. Liga-los recomeca o pisca-pisca; ligar de novo quem ja
    /// esta invencivel nao estende nada.
    void DefinirInvencivel(bool invencivel);

    /// Monta a personagem de novo, da sessao: e assim que o aluno que acabou de se formar aparece
    /// de capelo ainda na tela do resultado.
    void RecomporPersonagem();

    /// Invencivel por qualquer motivo: os quadros de graca depois de apanhar ou a imunidade dada
    /// pelo parceiro.
    [[nodiscard]] bool EstaInvencivel() const { return mInvencivel || mTempoDeImunidade > 0.0f; }

    /// A classe do aluno (-1: nenhuma): o tiro do chefe da mesma classe (BossProjectile::Classe)
    /// passa por ele. Quem a muda e o chefe - a fronteira de decisao do Daniel.
    void DefinirClasse(const int classe) { mClasse = classe; }
    [[nodiscard]] int Classe() const { return mClasse; }

    /// O mesmo causador nao acerta duas vezes seguidas (Acerto.h): 'quem' e o projetil ou o corpo
    /// do chefe, e 'idade' a vida dele (0 para o corpo).
    [[nodiscard]] bool JaFoiAcertadoPor(const void* quem, const float idade) const { return mAcertos.Lembra(quem, idade); }
    void FoiAcertadoPor(const void* quem, const float idade) { mAcertos.Lembrar(quem, idade); }

    /// Um projetil raspa o aluno uma vez so (Raspao.h): a mesma memoria dos acertos, com o prazo da
    /// vida de um projetil.
    [[nodiscard]] bool JaRaspou(const void* quem, const float idade) const { return mRaspoes.Lembra(quem, idade); }
    void Raspou(const void* quem, const float idade) { mRaspoes.Lembrar(quem, idade); }

    /// Protege este aluno por um tempo, as custas do Ponto Extra do parceiro. Renova, nao acumula.
    void Proteger(float segundos);

    /// Prende o aluno: ele anda a uma fracao da velocidade por um tempo. E o efeito das correntes
    /// do Vitor - a piada de "concorrentes".
    void Acorrentar(float segundos, float fatorDeVelocidade);

    /// Battle e as regras de projetil leem isto.
    [[nodiscard]] bool EstaAcorrentado() const { return mTempoDeCorrente > 0.0f; }

    /// Quao rapido o aluno anda neste instante, ja com a corrente. Fonte unica.
    [[nodiscard]] float VelocidadeDeAndar() const {
        return kVelocidade * (mTempoDeCorrente > 0.0f ? mFatorDaCorrente : 1.0f);
    }

    /// Quanto ainda falta da imunidade do parceiro. Zero quer dizer desprotegido.
    [[nodiscard]] float TempoDeImunidade() const { return mTempoDeImunidade; }

    [[nodiscard]] int PontosExtras() const { return mPontosExtras; }
    void GanharPontoExtra();

    /// Quantos tiros ele ja deu na batalha. O Observer da Maria Lucia, como
    /// orientadora do TCC, atira a cada tantos (Golpe::Notificacoes).
    [[nodiscard]] int Disparos() const { return mDisparos; }

private:
    // ---- As constantes do aluno ----
    static constexpr float kVelocidade = 300.0f;
    /// O tempo entre dois tiros: o mesmo que o Ponto Extra paga por tiro.
    static constexpr float kEntreTiros = PontoExtra::kEntreTiros;
    /// De quanto em quanto tempo o aluno pisca enquanto esta invencivel.
    static constexpr float kPiscar = 0.1f;
    static constexpr int kMaximoDePontosExtras = 3;
    static constexpr int kVolumeDoTiro = 10;
    /// A duracao de referencia da corrente, para o pulso dela acelerar ate o fim.
    static constexpr float kDuracaoTipicaDaCorrente = 3.5f;

    /// A posicao deste aluno no painel, e o painel que o move.
    size_t mQuem = 0;
    [[nodiscard]] Painel::Jogador MeuPainel() const {
        return mQuem == 1 ? Painel::Jogador::Dois : Painel::Jogador::Um;
    }

    /// Os componentes que o aluno usa todo quadro, guardados ao nascer.
    DrawAnimatedComponent* mDesenho = nullptr;
    RigidBodyComponent* mCorpo = nullptr;

    /// A personagem da sessao deste aluno (ComporPersonagem.h), com o capelo de quem se formou.
    Personagens::Composta MinhaPersonagem();

    // ---- A entrada ----
    void LerManche(const Uint8* state);
    void LerBotaoDeTiro(const Uint8* state);
    void Atirar();
    void LerBotaoDoPontoExtra(const Uint8* state);
    /// O Botao 3: na dupla, fora do TCC, o sinal ao parceiro (Battle::Sinalizar). Vale o aperto, e
    /// nao o botao segurado.
    void LerBotaoDoSinal(const Uint8* state);
    bool mSinalAntes = false;
    /// Quem anda no lugar do manche quando o jogo roda sozinho.
    PilotoDoAluno mPiloto;
    /// Se o aluno esta andando: decide a animacao.
    bool mAndando = false;

    // ---- O tiro e o Ponto Extra ----
    float mRecargaDoTiro = kEntreTiros;
    int mDisparos = 0;
    int mPontosExtras = 1;
    /// Quanto falta para este aluno poder gastar outro Ponto Extra.
    float mEsperaDoPontoExtra = 0.0f;
    /// A sobrecarga do solo: o tempo que falta e o pulso do vermelho.
    float mTempoDeSobrecarga = 0.0f;
    float mPulsoDaSobrecarga = 0.0f;

    // ---- Os estados que o corpo mostra ----
    bool mInvencivel = false;
    float mTempoInvencivel = 0.0f;
    /// Tempo desde a ultima troca de visibilidade durante a invencibilidade.
    float mTempoDePiscar = kPiscar;
    /// A imunidade dada pelo parceiro: o que resta, e o pulso do azul.
    float mTempoDeImunidade = 0.0f;
    int mClasse = -1;
    float mPulsoDaImunidade = 0.0f;
    /// A corrente: o que resta, quanto ela corta da velocidade, e o pulso do ferro.
    float mTempoDeCorrente = 0.0f;
    float mFatorDaCorrente = 1.0f;
    float mPulsoDaCorrente = 0.0f;
    Acerto::Memoria mAcertos;
    Acerto::Memoria mRaspoes{Raspao::kMemoria};

    // ---- O quadro ----
    void AtualizarInvencibilidade(float deltaTime);
    void AtualizarAnimacao();
    void DescontarRecarga(float deltaTime);
    void PrenderNoCampo();
    void AtualizarCorrente(float deltaTime);
    void AtualizarSobrecarga(float deltaTime);
    void AtualizarImunidade(float deltaTime);
    /// O unico lugar que pinta o aluno; a ordem dos estados e a prioridade.
    void AtualizarCorDoEstado();
};

#endif // DPIBULLETHELLRE_PLAYER_H
