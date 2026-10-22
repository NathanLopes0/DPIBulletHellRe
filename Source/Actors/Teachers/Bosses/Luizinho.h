//
// O chefe da INF 340 - Linguagens de Programacao.
//

#pragma once

#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Coletor.h"
#include "../../../Attacks/Rastro.h"

class DrawLinhasComponent;
class RotuloDoGoto;

/**
 * @class Luizinho
 * @brief O professor de Linguagens de Programacao (INF 340).
 *
 * A IDEIA DO CHEFE: ele e conhecido por saber de onde vem cada expressao, quase
 * sempre do latim, e por passar isso para os alunos. A batalha faz o mesmo: em
 * cada fase o projetil obedece ao sentido LITERAL da palavra latina por tras de
 * um assunto da ementa, e o placar mostra a palavra e o que ela queria dizer.
 *
 *   fase 1  Amarracoes          ligare        atar
 *   fase 2  Encapsulamento      in capsula    na caixinha
 *   fase 3  Gerencia de memoria memento mori  lembra-te de que morreras
 *   final   Sequenciadores      sequi         seguir
 *
 * O que nao cabe em dados mora aqui: a mira do leque estatico da fase 1 e a da
 * capsula da fase 2, que dependem de onde o aluno esta no instante do disparo,
 * o COLETOR DE LIXO da fase 3, que olha para a tela inteira, e o GOTO da
 * final, que manda a cabeca de cada serpente saltar.
 *
 * O BALANCEAMENTO VIVE EM DADOS, como o dos outros: Assets/Attacks/luizinho/.
 */
class Luizinho : public Boss {
public:
    explicit Luizinho(Scene* scene);
    ~Luizinho() override = default;

    void OnUpdate(float deltaTime) override;

    /// A fase da memoria, a que decide a batalha.
    static constexpr const char* kFaseDaMemoria = "StateThree";

    /// A fase dos sequenciadores, a final.
    static constexpr const char* kFaseDaSequencia = "StateFinal";

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

private:
    // ---- A FASE 3: O COLETOR DE LIXO ---------------------------------------
    //
    // Os blocos que o professor aloca param no campo e ficam. A cada quadro o
    // grafo de referencias e desenhado: ouro entre o que a raiz (ele) alcanca,
    // cinza entre o lixo. A cada kColetaACada segundos o coletor PARA O MUNDO
    // por kPausaDoMundo: o professor e todos os projeteis congelam - so o aluno
    // anda -, o lixo ganha a marca, e no fim da pausa e jogado fora, caindo
    // reto para fora da tela. O que estava ligado a ele sobrevive, e e esse o
    // vazamento. Quanto sobra depende de onde o mergulho esta quando o coletor
    // passa: medido na fase sozinha, o piloto que vai e vem deixou 11, 15 e 1
    // blocos vivos nas tres varreduras, e o que persegue o professor nenhum.

    /// Tudo o que a memoria faz num quadro: relogio, marca, varredura, grafo.
    void AtualizarMemoria(float deltaTime);

    /// Os blocos de memoria em campo, rodando ou congelados.
    [[nodiscard]] std::vector<BossProjectile*> Blocos() const;

    /// Congela (ou descongela) TODOS os projeteis do professor - stop the world.
    void PararOMundo(bool parar) const;

    /// O fim da fase e o fim do programa: toda a memoria volta, viva ou nao.
    void DevolverAMemoria();

    /// O placar da fase 3: o latim, a traducao, e o que se diz na varredura.
    void AnunciarAMemoria();

    Coletor::Relogio mColetor{kColetaACada, kPausaDoMundo, 0.0f};

    /// As linhas do grafo. O ator que as guarda e da cena, criado na primeira
    /// vez que a fase 3 roda; o professor so as escreve.
    DrawLinhasComponent* mReferencias = nullptr;

    bool mNaMemoria = false;

    /// O anuncio da fase 3 em fases.json, guardado na primeira vez: e o latim.
    std::string mLatimDaMemoria;
    float mTempoNoPlacar = 0.0f;

    /// O RELOGIO E A QUEDA FORAM MEDIDOS JUNTOS, na batalha inteira com o
    /// piloto que persegue: com 1 s de pausa e o lixo a 380 px/s ele passava
    /// com 83,1 e 9 acertos, acima da faixa dos outros chefes; com 0,7 s e
    /// 520 px/s, com 76,8 e 11. So o lixo mais rapido nao mudou nada (83,1):
    /// quem pesou foi a pausa, que e tempo de tiro livre - o professor esta
    /// parado e nada acerta o aluno.
    static constexpr float kColetaACada = 4.25f;
    static constexpr float kPausaDoMundo = 0.7f;

    /// AUMENTAR O ALCANCE AUMENTA O VAZAMENTO E DIMINUI A CHUVA. Com o professor
    /// alcancando 160 px, o piloto que persegue terminava a fase com 51 blocos
    /// vivos e passava com 82,4; com 140 entre os blocos, com 80,9. Com 110,
    /// ele nao deixa bloco vivo nenhum, e tudo o que se aloca vira chuva.
    static constexpr float kAlcanceDaReferencia = 110.0f;

    /// A velocidade com que o lixo cai para fora da tela na varredura.
    static constexpr float kQuedaDoLixo = 520.0f;

    /// A cara do bloco marcado - a da pausa, e a do lixo caindo.
    static constexpr const char* kAnimacaoDoLixo = "Marcado";

    /// O "atraso" do SlowDown dos blocos impares em regras.json: o bloco mais
    /// longe da rajada para aos 1,4 s. A velocidade da rajada sai daqui, para
    /// que ele pare em cima do aluno. MUDAR UM PEDE MUDAR O OUTRO - o canario
    /// em tests/test_arquivos_de_dados.cpp espelha este numero.
    static constexpr float kTempoAteOBlocoMaisLonge = 1.4f;

    /// Quanto tempo o placar mostra o latim antes da traducao, e vice-versa.
    static constexpr float kTempoDeCadaLingua = 1.6f;

    /// Atras dos projeteis (89 a 91) e na frente do placar (60).
    static constexpr int kOrdemDasReferencias = 85;

    // ---- A FINAL: O GOTO --------------------------------------------------
    //
    // A cada kGotoACada segundos o professor da um goto: um rotulo aparece
    // (o numero dele em algarismos romanos, "II:"), o placar diz "vade ad II"
    // - "vai para o II" - e depois de kGotoAviso segundos a cabeca de cada
    // serpente que ainda persegue salta para la. O corpo salta quando chega no
    // ponto do salto (Rastro::Caminho).

    void AtualizarGoto(float deltaTime);

    /// Onde o proximo rotulo fica: no lado oposto ao do aluno, no alto. A
    /// cabeca reaparece longe dele e tem de voltar.
    [[nodiscard]] Vector2 OndeFicaORotulo() const;

    std::shared_ptr<Rastro::Desvio> mDesvio = std::make_shared<Rastro::Desvio>();
    bool mNaSequencia = false;
    bool mAvisandoOGoto = false;
    float mTempoDoGoto = 0.0f;
    Vector2 mProximoRotulo;
    std::string mLatimDaSequencia;

    /// O marcador do rotulo. O ator e da cena; o professor so o mostra.
    RotuloDoGoto* mRotulo = nullptr;

    static constexpr float kGotoACada = 3.0f;
    static constexpr float kGotoAviso = 0.8f;
    static constexpr float kAlturaDoRotulo = 220.0f;
    static constexpr float kMargemDoRotulo = 90.0f;
};
