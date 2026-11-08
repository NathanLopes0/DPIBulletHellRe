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
#include "../../Regras/Golpe.h"
#include "../../Regras/Poder.h"
#include "../../Regras/Variacao.h"

class Battle;
class Boss;
class BossProjectile;
class DrawLinhasComponent;
class PlayerProjectile;
class PoderEmCampo;

/// O professor escolhido antes do TCC (EscolhaDoOrientador), voando junto do aluno e atirando no
/// chefe.
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

    /// A reuniao de orientacao do TCC (Objetivo::Tipo::Reuniao): o Hugo marca o lugar, e ele larga
    /// o aluno e voa ate la, esperando; vazio, volta a escoltar.
    void MarcarReuniao(const std::optional<Vector2>& onde) { mReuniao = onde; }

    // ---- O poder ----

    /// Um tiro acertou o chefe: o poder carrega um acerto (Battle, regra 2).
    void Carregar();

    [[nodiscard]] bool PoderPronto() const;

    /// Solta o poder, se estiver pronto. O botao 3 de qualquer aluno chama.
    bool SoltarPoder();

    /// Para quem mede a batalha.
    [[nodiscard]] int Poderes() const { return mPoderes; }

    // ---- O que os poderes usam (PoderEmCampo) ----

    [[nodiscard]] const Poder::Descricao& Descricao() const { return mPoder; }
    /// Segundos desde que ele entrou em campo: o relogio dos poderes.
    [[nodiscard]] float Tempo() const { return mTempo; }
    [[nodiscard]] Vector2 PosicaoDoAluno();

    /// Solta um tiro com a cara da materia; com 'quadro' >= 0, a de outra folha (o poder) - 'folha'
    /// e 'escala' valem entao.
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

    // ---- O poder em curso ----

    void AtualizarPoder(Battle& batalha, float deltaTime);
    void DesenharAnel();
    void Anunciar(const std::string& texto);

    Poder::Descricao mPoder;
    std::unique_ptr<PoderEmCampo> mEmCampo;   ///< o efeito do poder (nulo: sem poder)
    int mCarga = 0;
    int mPoderes = 0;
    bool mAvisouPronto = false;
    bool mBotaoAntes = false;
    float mPoderAte = -1.0f;
    float mAnelDoPoder = -1.0f;     ///< o raio do anel que se abre no free() e no automato

    DrawLinhasComponent* mLinhas = nullptr;
};
