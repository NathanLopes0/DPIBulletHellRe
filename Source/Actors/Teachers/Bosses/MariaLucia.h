//
// A chefe da INF 221 - Engenharia de Software.
//

#pragma once

#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Observer.h"

class DrawLinhasComponent;

/**
 * @class MariaLucia
 * @brief A professora de Engenharia de Software (INF 221), que tambem da IHC.
 *
 * O BALANCEAMENTO DELA VIVE EM DADOS: as fases em
 * Assets/Attacks/marialucia/fases.json e as regras de projetil no regras.json
 * ao lado.
 *
 * A IDEIA DA CHEFE: as outras disciplinas do jogo sao OBJETOS - uma estrutura
 * de dados, uma linguagem, uma primitiva de concorrencia. Engenharia de
 * Software e um PROCESSO, e cada fase dela e um processo ou um padrao de
 * verdade:
 *
 *   fase 1  o MODELO CASCATA: cinco etapas em ordem, e ela descendo a escada
 *           do diagrama (EscadaStrategy);
 *   fase 2  os METODOS AGEIS: cinco sprints, e o cursor que arrasta as
 *           historias (SprintAttack);
 *   fase 3  o padrao OBSERVER: o aluno e o sujeito, e os olhos dela o assinam.
 *
 * As duas primeiras sao do motor e dos dados. O que mora aqui e o SUJEITO da
 * fase 3 (Observer.h): a cada quadro ela le para onde cada aluno esta indo e,
 * quando o estado muda - ou quando ninguem avisa por um tempo, o polling -,
 * puxa o gatilho dos avisos (Boss::Disparar). Desenha tambem as assinaturas,
 * uma linha de cada olho ate o aluno, e faz cada olho olhar para ele.
 */
class MariaLucia : public Boss {
public:
    explicit MariaLucia(Scene* scene);
    ~MariaLucia() override = default;

    void OnUpdate(float deltaTime) override;

    /// A fase do padrao Observer, a que decide a batalha.
    static constexpr const char* kFaseDoObserver = "StateThree";

private:
    /// Tudo o que o sujeito faz num quadro: le os alunos, avisa, desenha.
    void AtualizarObserver(float deltaTime);

    /// Os olhos em campo: os projeteis no posto dos observadores.
    [[nodiscard]] std::vector<BossProjectile*> Observadores() const;

    /// O placar da fase 3: o nome do padrao, e o que se diz num aviso.
    void AnunciarOObserver();

    /// Um sujeito por aluno: em dupla, cada um avisa por si, e os avisos de um
    /// vao para ele (Boss::Disparar com alvo).
    std::vector<Observer::Sujeito> mSujeitos;
    bool mNoObserver = false;

    /// Quanto falta do piscar dos olhos (e do brilho das assinaturas) e do
    /// aviso no placar, depois do ultimo aviso.
    float mPiscando = 0.0f;
    float mNoPlacar = 0.0f;
    bool mUltimoFoiConsulta = false;
    std::string mNomeDoPadrao;

    /// As assinaturas. O ator que as guarda e da cena, criado na primeira vez
    /// que a fase 3 roda; a professora so as escreve.
    DrawLinhasComponent* mAssinaturas = nullptr;

    static constexpr float kPiscar = 0.2f;
    static constexpr float kAvisoNoPlacar = 0.9f;

    /// Atras dos projeteis (88 a 91) e na frente do placar (60).
    static constexpr int kOrdemDasAssinaturas = 85;

    static constexpr const char* kNoPlacarNotifica = "aluno.notify()";
    static constexpr const char* kNoPlacarConsulta = "polling: aluno.getEstado()";
};
