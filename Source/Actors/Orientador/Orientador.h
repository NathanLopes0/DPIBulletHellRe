//
// O orientador do TCC, em campo.
//

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../Actor.h"
#include "../../Gameplay/Orientador/Golpe.h"
#include "../../Gameplay/Orientador/Poder.h"
#include "../../Gameplay/Variacao.h"

class Battle;
class Boss;
class BossProjectile;
class DrawLinhasComponent;
class PlayerProjectile;
class PoderEmCampo;

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
 * dupla como tudo o mais (requisitos-coop, D2). Mas tem a CARA DA MATERIA dele
 * - o projetil do professor, com um contorno azul de aliado.
 *
 * ONDE ELE FICA e conta pura, em Escolta.
 *
 * O GOLPE E O DA MATERIA DELE (Golpe.h): o laco do Andre, a pilha do Salles, o
 * barramento do Ricardo, a transacao do Thiago, o ponteiro do Luizinho, a
 * gramatica do Vladimir, o gradiente do Julio, o Observer da Maria Lucia, o
 * fetch do Lucas e a janela do TCP do Vitor. Todos com o mesmo peso.
 *
 * E O PODER TAMBEM (Poder.h): cada acerto no chefe carrega o especial dele
 * (Carregar, chamado pela batalha), o anel em volta dele mostra a carga, e o
 * aluno solta no BOTAO 3 - a capivara do Salles, o ROLLBACK do Thiago, a bala
 * de prata da Maria Lucia... Aqui mora a carga, o anel e o anuncio, que sao
 * iguais para todos; o efeito de cada um e um PoderEmCampo (padrao Strategy).
 */
class Orientador : public Actor {
public:
    Orientador(Scene* scene, std::string professor, const std::string& retrato);
    ~Orientador() override;

    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8* keyState) override;

    [[nodiscard]] const std::string& Professor() const { return mProfessor; }
    [[nodiscard]] Golpe::Materia Materia() const { return mMateria; }

    /// Quantos tiros ele ja deu. Para quem mede a batalha.
    [[nodiscard]] int Disparos() const { return mDisparos; }

    /// Poe o orientador direto no posto, sem o voo de chegada: e onde ele nasce.
    void Pousar();

    /// A REUNIAO DE ORIENTACAO do TCC (Objetivo::Tipo::Reuniao): o Hugo marca
    /// o lugar, e ele larga o aluno e voa ate la, esperando; vazio, volta a
    /// escoltar.
    void MarcarReuniao(const std::optional<Vector2>& onde) { mReuniao = onde; }

    // ---- O PODER -------------------------------------------------------------

    /// Um tiro acertou o chefe: o poder carrega um acerto (Battle, regra 2).
    void Carregar();

    [[nodiscard]] bool PoderPronto() const;

    /// Solta o poder, se estiver pronto. O BOTAO 3 de qualquer aluno chama.
    bool SoltarPoder();

    /// Quantas vezes o poder ja foi solto. Para quem mede a batalha.
    [[nodiscard]] int Poderes() const { return mPoderes; }

    // ---- O QUE OS PODERES USAM (PoderEmCampo) ----------------------------------

    [[nodiscard]] const Poder::Descricao& Descricao() const { return mPoder; }
    /// Segundos desde que ele entrou em campo: o relogio dos poderes.
    [[nodiscard]] float Tempo() const { return mTempo; }
    [[nodiscard]] Vector2 PosicaoDoAluno();

    /// Solta um tiro com a cara da materia; com 'quadro' >= 0, a de outra
    /// folha (o poder) - 'folha' e 'escala' valem entao. Devolve o tiro (ja
    /// entregue ao manager), para quem precisa marca-lo.
    PlayerProjectile* Disparar(Battle& batalha, const Golpe::Tiro& tiro, const Vector2& de, const Vector2& direcao,
                               const std::string& folha = "", int quadro = -1, float escala = 0.0f);

    /// Uma figura do poder em campo (a capivara, um agente), onde ele esta. Quem
    /// a pediu e quem a tira (SetState(Destroy)) quando o poder acaba.
    Actor* Figura(const std::string& folha, const std::vector<int>& quadros, float escala);

    /// Os tiros do chefe que um poder alcanca: os soltos - nada que o chefe
    /// guarda num posto (um alvo, uma estrutura), nem enfeite, nem abrigo.
    [[nodiscard]] std::vector<BossProjectile*> TirosSoltos(Battle& batalha) const;

    /// A onda do free() e do automato: um anel que se abre do aluno.
    void AbrirAnel() { mAnelDoPoder = 0.0f; }

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
    /// Os tiros que ainda dependem dele: a espera, a curva, a volta, a onda, a
    /// divisao.
    void Guiar(Battle& batalha, float deltaTime);

    struct Guiado {
        std::uint64_t id = 0;
        Golpe::Voo voo;
        std::string folha;
        int quadro = -1;
        float escala = 0.0f;
    };

    std::string mProfessor;
    std::string mRetrato;
    Golpe::Materia mMateria = Golpe::Materia::Generico;
    int mLado = -1;
    std::optional<Vector2> mReuniao;
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

    // ---- O PODER EM CURSO -------------------------------------------------------

    void AtualizarPoder(Battle& batalha, float deltaTime);
    void DesenharAnel();
    void Anunciar(const std::string& texto);

    Poder::Descricao mPoder;
    std::unique_ptr<PoderEmCampo> mEmCampo;   ///< o efeito do poder (nulo: sem poder)
    int mCarga = 0;
    int mPoderes = 0;
    bool mAvisouPronto = false;
    bool mBotaoAntes = false;
    float mPoderAte = -1.0f;        ///< o efeito dura ate aqui (mTempo)
    float mAnelDoPoder = -1.0f;     ///< o raio do anel que se abre no free() e no automato

    DrawLinhasComponent* mLinhas = nullptr;
};
